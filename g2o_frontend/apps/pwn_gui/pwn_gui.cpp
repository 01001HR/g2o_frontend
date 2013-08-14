#include "g2o_frontend/pwn2/informationmatrixcalculator.h"
#include "g2o_frontend/pwn2/statscalculator.h"
#include "g2o_frontend/pwn2/pinholepointprojector.h"
#include "g2o_frontend/pwn2/aligner.h"
#include "g2o_frontend/pwn2/merger.h"

#include "g2o_frontend/basemath/bm_se3.h"

#include "g2o_frontend/pwn2/depthimageconverter.h"
#include "g2o_frontend/pwn_viewer/pwn_qglviewer.h"
#include "g2o_frontend/pwn_viewer/pwn_imageview.h"
#include "g2o_frontend/pwn_viewer/drawable_points.h"
#include "g2o_frontend/pwn_viewer/drawable_normals.h"
#include "g2o_frontend/pwn_viewer/drawable_covariances.h"
#include "g2o_frontend/pwn_viewer/drawable_correspondences.h"
#include "g2o_frontend/pwn_viewer/gl_parameter.h"
#include "g2o_frontend/pwn_viewer/gl_parameter_points.h"
#include "g2o_frontend/pwn_viewer/gl_parameter_normals.h"
#include "g2o_frontend/pwn_viewer/gl_parameter_covariances.h"
#include "g2o_frontend/pwn_viewer/gl_parameter_correspondences.h"

#include "pwn_gui_main_window.h"

#include "g2o/stuff/command_args.h"
#include "g2o/stuff/timeutil.h"

#include <unistd.h>

#undef _PWN_USE_CUDA_

#ifdef _PWN_USE_CUDA_
#include "g2o_frontend/pwn_cuda/cualigner.h"
#endif// PWN_CUDA

#include <qapplication.h>
#include <iostream>
#include <fstream>
#include <set>
#include <dirent.h>
#include <sys/stat.h>

using namespace Eigen;
using namespace g2o;
using namespace std;
using namespace pwn;

#include "g2o_frontend/pwn2/gaussian3.h"

// Variables for the input parameters.
int ng_minImageRadius;
int ng_maxImageRadius;
int ng_minPoints;
float ng_worldRadius;
float ng_scale;
float ng_curvatureThreshold;

float cf_inlierNormalAngularThreshold;
float cf_flatCurvatureThreshold;
float cf_inlierCurvatureRatioThreshold;
float cf_inlierDistanceThreshold;

int al_innerIterations;
int al_outerIterations;
int al_minNumInliers; 
float al_minError;
float al_inlierMaxChi2;
string sensorType;

int vz_step;

void convertCovariances(StatsVector &statsVector, Gaussian3fVector &gaussians) {
  statsVector.resize(gaussians.size());
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigenSolver;
  for(size_t i = 0; i < statsVector.size(); i++) {
    const Eigen::Matrix3f &covariance = gaussians[i].covarianceMatrix();
    const Point mean(gaussians[i].mean());

    eigenSolver.computeDirect(covariance, Eigen::ComputeEigenvectors);

    Stats &stats = statsVector[i];
    stats.setZero();
    stats.setEigenVectors(eigenSolver.eigenvectors());
    stats.setMean(mean);
    Eigen::Vector3f eigenValues = eigenSolver.eigenvalues();
    if(eigenValues(0) < 0.0f)
      eigenValues(0) = 0.0f;
    stats.setEigenValues(eigenValues);
    stats.setN(1);
  }
}

set<string> readDir(std::string dir) {
  DIR *dp;
  struct dirent *dirp;
  struct stat filestat;
  std::set<std::string> filenames;
  dp = opendir(dir.c_str());
  if (dp == NULL){
    return filenames;
  }
  
  while ((dirp = readdir( dp ))) {
    string filepath = dir + "/" + dirp->d_name;

    // If the file is a directory (or is in some way invalid) we'll skip it 
    if (stat(filepath.c_str(), &filestat)) 
      continue;
    if (S_ISDIR(filestat.st_mode))         
      continue;

    filenames.insert(filepath);
  }

  closedir(dp);
  return filenames;
}

struct DrawableFrame {
  DrawableFrame(string f, int s, const Eigen::Matrix3f cameraMatrix_) {
    filename = f;
    step = s;
    cameraMatrix = cameraMatrix_;

    // float r = 0.0f + 0.75f*rand()/double(RAND_MAX);
    // float g = 0.0f + 0.75f*rand()/double(RAND_MAX);
    // float b = 0.0f + 0.75f*rand()/double(RAND_MAX);

    float r = 1.0;
    float g = 0.3f;
    float b = 0.0f;

    pPoints = new GLParameterPoints(1.0f, Vector4f(r, g, b, 1.0f));
    pPoints->setStep(step);
    pNormals = new GLParameterNormals(1.0f, Vector4f(0.0f, 0.0f, 1.0f, 1.0f), 0.0f);
    pNormals->setStep(step);
    pCovariances = new GLParameterCovariances(1.0f, 
					      Vector4f(0.0f, 1.0f, 0.0f, 1.0f), Vector4f(1.0f, 0.0f, 0.0f, 1.0f),
					      0.02f, 0.0f);
    pCovariances->setStep(step);
    pCorrespondences = new GLParameterCorrespondences(1.0f, Vector4f(1.0f, 0.0f, 1.0f, 1.0f), 0.0f);
    pCorrespondences->setStep(step);

    dPoints = new DrawablePoints(Isometry3f::Identity(), (GLParameter*)pPoints, &frame.points(), &frame.normals());
    dNormals = new DrawableNormals(Isometry3f::Identity(), (GLParameter*)pNormals, &frame.points(), &frame.normals());
    dCovariances = new DrawableCovariances(Isometry3f::Identity(), (GLParameter*)pCovariances, &frame.stats());
    dCorrespondences = new DrawableCorrespondences(Isometry3f::Identity(), (GLParameter*)pCorrespondences, 0,
						   &frame.points(), &frame.points(), &correspondences);
  }
  
  DrawableFrame(Frame *frame_, int s, const Eigen::Matrix3f cameraMatrix_) {
    filename = "";
    step = s;
    frame = *frame_;
    cameraMatrix = cameraMatrix_;

    // float r = 0.0f + 0.75f*rand()/double(RAND_MAX);
    // float g = 0.0f + 0.75f*rand()/double(RAND_MAX);
    // float b = 0.0f + 0.75f*rand()/double(RAND_MAX);

    float r = 1.0;
    float g = 0.3f;
    float b = 0.0f;

    pPoints = new GLParameterPoints(1.0f, Vector4f(r, g, b, 1.0f));
    pPoints->setStep(step);
    pNormals = new GLParameterNormals(1.0f, Vector4f(0.0f, 0.0f, 1.0f, 1.0f), 0.0f);
    pNormals->setStep(step);
    pCovariances = new GLParameterCovariances(1.0f, 
					      Vector4f(0.0f, 1.0f, 0.0f, 1.0f), Vector4f(1.0f, 0.0f, 0.0f, 1.0f),
					      0.02f, 0.0f);
    pCovariances->setStep(step);
    pCorrespondences = new GLParameterCorrespondences(1.0f, Vector4f(1.0f, 0.0f, 1.0f, 1.0f), 0.0f);
    pCorrespondences->setStep(step);

    dPoints = new DrawablePoints(Isometry3f::Identity(), (GLParameter*)pPoints, &frame.points(), &frame.normals());
    dNormals = new DrawableNormals(Isometry3f::Identity(), (GLParameter*)pNormals, &frame.points(), &frame.normals());
    dCovariances = new DrawableCovariances(Isometry3f::Identity(), (GLParameter*)pCovariances, &frame.stats());
    dCorrespondences = new DrawableCorrespondences(Isometry3f::Identity(), (GLParameter*)pCorrespondences, 0,
						   &frame.points(), &frame.points(), &correspondences);
  }
    
  void computeStats() {
    // Set the camera matrix of the projector object.
    projector.setCameraMatrix(cameraMatrix);
    
    statsCalculator.setWorldRadius(ng_minImageRadius);
    statsCalculator.setMinImageRadius(ng_maxImageRadius);
    statsCalculator.setMinImageRadius(ng_minPoints);
    statsCalculator.setWorldRadius(ng_worldRadius);

    pointInformationMatrixCalculator.setCurvatureThreshold(ng_curvatureThreshold);
    normalInformationMatrixCalculator.setCurvatureThreshold(ng_curvatureThreshold);
    DepthImageConverter depthImageConverter = DepthImageConverter(&projector, &statsCalculator,
								  &pointInformationMatrixCalculator,
								  &normalInformationMatrixCalculator);
    depthImageConverter._curvatureThreshold = ng_curvatureThreshold;
    sensorOffset = Isometry3f::Identity();
    sensorOffset.translation() = Vector3f(0.0f, 0.0f, 0.0f);
    Quaternionf quat = Quaternionf(0.5, -0.5, 0.5, -0.5);
    sensorOffset.linear() = quat.toRotationMatrix();
    sensorOffset.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;

    // Check if it is a pwn or a pgm file
    T = Isometry3f::Identity();
    string extension = filename.substr(filename.rfind(".") + 1);
    if(extension == "pwn") {
      frame.load(T, filename.c_str());
      // HAKKE
      indexImage.resize(640, 480);
      projector.setTransform(sensorOffset);
      projector.project(indexImage, 
			depthImage, 
			frame.points());
      frame.clear();
    }
    else {
      if(!depthImage.load(filename.c_str(), true)) {
	cout << "Failure while loading the depth image: " << filename<< " skipping image!" << endl;
	return;
      }
      cout << endl << "Loaded depth image " << filename << " of size: " << depthImage.rows() << "x" << depthImage.cols() << endl;
    
    }
    T.matrix().row(3) << 0.0f, 0.0f, 0.0f, 0.0f, 1.0f;
    
    depthImageConverter.compute(frame, depthImage, sensorOffset);
    
    dPoints->setPoints(&frame.points());
    dPoints->setNormals(&frame.normals());
    dNormals->setPoints(&frame.points());
    dNormals->setNormals(&frame.normals());
    dCovariances->setCovariances(&frame.stats());
  }
  
  // Creating the stas generator object. 
  StatsCalculator statsCalculator;

  // Creating the omegas generators objects.
  PointInformationMatrixCalculator pointInformationMatrixCalculator;
  NormalInformationMatrixCalculator normalInformationMatrixCalculator;
  Frame frame;
  CorrespondenceVector correspondences;
  PinholePointProjector projector;
  string filename;
  int step;
  DepthImage depthImage;
  MatrixXi indexImage;

  Isometry3f sensorOffset;
  Isometry3f T;

  GLParameterPoints *pPoints;
  GLParameterNormals *pNormals;
  GLParameterCovariances *pCovariances;
  GLParameterCorrespondences *pCorrespondences;

  DrawablePoints *dPoints;
  DrawableNormals *dNormals;
  DrawableCovariances *dCovariances;
  DrawableCorrespondences *dCorrespondences;
  Eigen::Matrix3f cameraMatrix;
  
};

int main(int argc, char** argv) {
  /************************************************************************
   *                           Input Handling                             *
   ************************************************************************/
  string workingDirectory = ".";

  // Input parameters handling.
  g2o::CommandArgs arg;
  
  // Optional input parameters.
  // Optional input parameters.
  arg.param("ng_minImageRadius", ng_minImageRadius, 10, "Specify the minimum number of pixels composing the square where to take points for a normal computation");
  arg.param("ng_maxImageRadius", ng_maxImageRadius, 30, "Specify the maximum number of pixels composing the square where to take points for a normal computation");
  arg.param("ng_minPoints", ng_minPoints, 50, "Specify the minimum number of points to be used to compute a normal");
  arg.param("ng_worldRadius", ng_worldRadius, 0.1f, "Specify the max distance for a point to be used to compute a normal");
  arg.param("ng_scale", ng_scale, 1.0f, "Specify the scaling factor to apply on the depth image");
  arg.param("ng_curvatureThreshold", ng_curvatureThreshold, 1.0f, "Specify the max surface curvature threshold for which normals are discarded");
  
  arg.param("cf_inlierNormalAngularThreshold", cf_inlierNormalAngularThreshold, M_PI / 6.0f, "Maximum angle between the normals of two points to regard them as iniliers");
  arg.param("cf_flatCurvatureThreshold", cf_flatCurvatureThreshold, 0.02f, "Maximum curvature value for a point to be used for data association");
  arg.param("cf_inlierCurvatureRatioThreshold", cf_inlierCurvatureRatioThreshold, 1.3f, "Maximum curvature ratio value between two points to regard them as iniliers");
  arg.param("cf_inlierDistanceThreshold", cf_inlierDistanceThreshold, 0.5f, "Maximum metric distance between two points to regard them as iniliers");

  arg.param("al_innerIterations", al_innerIterations, 1, "Specify the inner iterations");
  arg.param("al_outerIterations", al_outerIterations, 10, "Specify the outer iterations");
  arg.param("al_minNumInliers", al_minNumInliers, 10000, "Specify the minimum number of inliers to consider an alignment good");
  arg.param("al_minError", al_minError, 10.0f, "Specify the minimum error to consider an alignment good");
  arg.param("al_inlierMaxChi2", al_inlierMaxChi2, 9e3, "Max chi2 error value for the alignment step");
  arg.param("vz_step", vz_step, 1, "A graphic element is drawn each vz_step elements. [int]");
  arg.param("sensorType", sensorType, "kinect", "sensor type: xtion640/xtion480/kinect");

  // Last parameter has to be the working directory.
  arg.paramLeftOver("workingDirectory", workingDirectory, ".", "Path of the working directory. [string]", true);

  // Set parser input.
  arg.parseArgs(argc, argv);


  Eigen::Matrix3f cameraMatrix;
  cameraMatrix.setIdentity();

  if (sensorType=="xtion640") {
    cameraMatrix << 
    570.342, 0,       320,
    0,       570.342, 240,
    0.0f, 0.0f, 1.0f;  
  }  else if (sensorType=="xtion320") {
    cameraMatrix << 
      570.342, 0,       320,
      0,       570.342, 240,
      0.0f, 0.0f, 1.0f;  
    cameraMatrix.block<2,3>(0,0)*=0.5;
  } else if (sensorType=="kinect") {
    cameraMatrix << 
      525.0f, 0.0f, 319.5f,
      0.0f, 525.0f, 239.5f,
      0.0f, 0.0f, 1.0f;  
  } else {
    cerr << "unknown sensor type: [" << sensorType << "], aborting (you need to specify either xtion or kinect)" << endl;
    return 0;
  }

  QApplication qApplication(argc, argv);
  PWNGuiMainWindow pwnGMW;
  QGraphicsScene *refScn, *currScn;
  
  pwnGMW.viewer_3d->setAxisIsDrawn(true);

  std::vector<string> filenames;
  std::set<string> filenamesset = readDir(workingDirectory);
  for (set<string>::const_iterator it = filenamesset.begin(); it != filenamesset.end(); it++) {
    filenames.push_back(*it);      
    QString listItem(&(*it)[0]);
    if (listItem.endsWith(".pgm", Qt::CaseInsensitive))
      pwnGMW.listWidget->addItem(listItem);
    if (listItem.endsWith(".pwn", Qt::CaseInsensitive))
      pwnGMW.listWidget->addItem(listItem);
  }

  // Creating and setting aligner object.
  //Aligner aligner;
  CorrespondenceFinder correspondenceFinder;
  correspondenceFinder.setInlierDistanceThreshold(cf_inlierDistanceThreshold);
  correspondenceFinder.setFlatCurvatureThreshold(cf_flatCurvatureThreshold);  
  correspondenceFinder.setInlierCurvatureRatioThreshold(cf_inlierCurvatureRatioThreshold);
  correspondenceFinder.setInlierNormalAngularThreshold(cosf(cf_inlierNormalAngularThreshold));
  Linearizer linearizer;
  linearizer.setInlierMaxChi2(al_inlierMaxChi2);

#ifdef _PWN_USE_CUDA_
  CuAligner aligner;
#else
  Aligner aligner;
#endif

  aligner.setLinearizer(&linearizer);
  aligner.setCorrespondenceFinder(&correspondenceFinder);
  aligner.setInnerIterations(al_innerIterations);
  aligner.setOuterIterations(al_outerIterations);
  
  pwnGMW.show();
  refScn = pwnGMW.scene0();
  currScn = pwnGMW.scene1();

  bool newCloudAdded = false, wasInitialGuess = false;
  bool *initialGuessViewer = 0, *optimizeViewer = 0, *addCloud = 0, *clearLast = 0, *clearAll = 0, *merge = 0;
  
  int *stepViewer = 0, *stepByStepViewer = 0;
  float *pointsViewer = 0, *normalsViewer = 0, *covariancesViewer = 0, *correspondencesViewer = 0;
  QListWidgetItem* itemList = 0;

  std::vector<DrawableFrame*> drawableFrameVector;
  DrawableFrame *drawableFrame = 0;
  Isometry3f initialGuess = Isometry3f::Identity();
  Isometry3f globalT = Isometry3f::Identity();
  Isometry3f stepByStepInit = Isometry3f::Identity();
  std::vector<Isometry3f> localT; 

  Merger merger;
  DepthImageConverter depthImageConverter(0, 0, 0, 0);
  depthImageConverter._curvatureThreshold = ng_curvatureThreshold;
  Frame mergingFrame;

  while(!(*pwnGMW.closing())) {
    qApplication.processEvents();

    // Check window status changes.
    stepViewer = pwnGMW.step();
    pointsViewer = pwnGMW.points();
    normalsViewer = pwnGMW.normals();
    covariancesViewer = pwnGMW.covariances();
    correspondencesViewer = pwnGMW.correspondences();
    initialGuessViewer = pwnGMW.initialGuess();
    optimizeViewer = pwnGMW.optimize();
    stepByStepViewer = pwnGMW.stepByStep();
    merge = pwnGMW.merge();
    addCloud = pwnGMW.addCloud();
    clearLast = pwnGMW.clearLast();    
    clearAll = pwnGMW.clearAll();
    itemList = pwnGMW.itemList();
    
    // Check feature visualization options.   
    for(size_t i = 0; i < drawableFrameVector.size(); i++) {
      if(stepViewer[0]) {
	drawableFrameVector[i]->dPoints->setStep(stepViewer[1]);
	drawableFrameVector[i]->dNormals->setStep(stepViewer[1]);
	drawableFrameVector[i]->dCovariances->setStep(stepViewer[1]);
	drawableFrameVector[i]->dCorrespondences->setStep(stepViewer[1]);
      }
      else {
	drawableFrameVector[i]->dPoints->setStep(1.0f);
	drawableFrameVector[i]->dNormals->setStep(1.0f);
	drawableFrameVector[i]->dCovariances->setStep(1.0f);
	drawableFrameVector[i]->dCorrespondences->setStep(1.0f);
      }
      
      if(pointsViewer[0])
	drawableFrameVector[i]->dPoints->setPointSize(pointsViewer[1]);
      else
	drawableFrameVector[i]->dPoints->setPointSize(0.0f);
      
      if(normalsViewer[0])
	drawableFrameVector[i]->dNormals->setNormalLength(normalsViewer[1]);
      else
	drawableFrameVector[i]->dNormals->setNormalLength(0.0f);
      
      if(covariancesViewer[0])
	drawableFrameVector[i]->dCovariances->setEllipsoidScale(covariancesViewer[1]);
      else
	drawableFrameVector[i]->dCovariances->setEllipsoidScale(0.0f);
      
      if(correspondencesViewer[0])
	drawableFrameVector[i]->dCorrespondences->setLineWidth(correspondencesViewer[1]);
      else
	drawableFrameVector[i]->dCorrespondences->setLineWidth(0.0f);
    }
    
    if(!wasInitialGuess && !newCloudAdded && drawableFrameVector.size() > 1 && *initialGuessViewer) {
      drawableFrameVector[drawableFrameVector.size()-1]->dPoints->setTransformation(globalT * localT[localT.size()-1].inverse());
      drawableFrameVector[drawableFrameVector.size()-1]->dNormals->setTransformation(globalT * localT[localT.size()-1].inverse());
      drawableFrameVector[drawableFrameVector.size()-1]->dCovariances->setTransformation(globalT * localT[localT.size()-1].inverse());
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setReferencePointsTransformation(globalT * localT[localT.size()-1].inverse());
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setTransformation(globalT * localT[localT.size()-1].inverse());
      newCloudAdded = true;
      wasInitialGuess = true;
      *initialGuessViewer = 0;
    }
    // Optimize pressed with no step by step mode.
    else if(newCloudAdded && drawableFrameVector.size() > 1 && *optimizeViewer && !(*stepByStepViewer)) {
      if(!wasInitialGuess) {
	aligner.setOuterIterations(al_outerIterations);
	
	aligner.correspondenceFinder()->setSize(drawableFrameVector[drawableFrameVector.size()-2]->depthImage.rows(), drawableFrameVector[drawableFrameVector.size()-2]->depthImage.cols());
	
	aligner.setProjector(&drawableFrameVector[drawableFrameVector.size()-2]->projector);
	aligner.setReferenceFrame(&drawableFrameVector[drawableFrameVector.size()-2]->frame);
	aligner.setCurrentFrame(&drawableFrameVector[drawableFrameVector.size()-1]->frame);
	
	// HAKKE
	initialGuess = (drawableFrameVector[drawableFrameVector.size()-2]->T).inverse() * drawableFrameVector[drawableFrameVector.size()-1]->T;
	initialGuess.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;
	initialGuess.matrix().col(3) << 0.0f, 0.0f, 0.0f, 1.0f;

	aligner.setInitialGuess(initialGuess);
	aligner.setSensorOffset(drawableFrameVector[drawableFrameVector.size()-1]->sensorOffset);
	aligner.align();
	localT.push_back(aligner.T());
	globalT = globalT * localT[localT.size()-1];
      }
      cout << "Local transformation: " << endl << localT[localT.size()-1].matrix() << endl;

      mergingFrame.add(drawableFrameVector[drawableFrameVector.size()-1]->frame, globalT);

      // Update cloud drawing position.
      drawableFrameVector[drawableFrameVector.size()-1]->dPoints->setTransformation(globalT);
      drawableFrameVector[drawableFrameVector.size()-1]->dNormals->setTransformation(globalT);
      drawableFrameVector[drawableFrameVector.size()-1]->dCovariances->setTransformation(globalT);
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setReferencePointsTransformation(globalT * localT[localT.size()-1].inverse());
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setTransformation(globalT);
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setReferencePoints(&drawableFrameVector[drawableFrameVector.size()-2]->frame.points());
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setCurrentPoints(&drawableFrameVector[drawableFrameVector.size()-1]->frame.points());
      drawableFrameVector[drawableFrameVector.size()-1]->correspondences = CorrespondenceVector(aligner.correspondenceFinder()->correspondences());
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setCorrespondences(&drawableFrameVector[drawableFrameVector.size()-1]->correspondences);
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setNumCorrespondences(aligner.correspondenceFinder()->numCorrespondences());

      // Show zBuffers.
      refScn->clear();
      currScn->clear();
      QImage refQImage;
      QImage currQImage;
      DepthImageView div;
      div.computeColorMap(300, 2000, 128);
      div.convertToQImage(refQImage, aligner.correspondenceFinder()->referenceDepthImage());
      div.convertToQImage(currQImage, aligner.correspondenceFinder()->currentDepthImage());
      refScn->addPixmap((QPixmap::fromImage(refQImage)).scaled(QSize((int)refQImage.width()/(ng_scale*3), (int)(refQImage.height()/(ng_scale*3)))));
      currScn->addPixmap((QPixmap::fromImage(currQImage)).scaled(QSize((int)currQImage.width()/(ng_scale*3), (int)(currQImage.height()/(ng_scale*3)))));
      pwnGMW.graphicsView1_2d->show();
      pwnGMW.graphicsView2_2d->show();
      
      wasInitialGuess = false;
      newCloudAdded = false;
      *initialGuessViewer = 0;
      *optimizeViewer = 0;
    }
    // Step-by-step optimization.
    else if(drawableFrameVector.size() > 1 && *optimizeViewer && *stepByStepViewer) {
      if(newCloudAdded)
	stepByStepInit = globalT;
      if(!wasInitialGuess) {
	aligner.setOuterIterations(1);

	aligner.correspondenceFinder()->setSize(drawableFrameVector[drawableFrameVector.size()-2]->depthImage.rows(), drawableFrameVector[drawableFrameVector.size()-2]->depthImage.cols());
	
	aligner.setProjector(&drawableFrameVector[drawableFrameVector.size()-2]->projector);
	aligner.setReferenceFrame(&drawableFrameVector[drawableFrameVector.size()-2]->frame);
	aligner.setCurrentFrame(&drawableFrameVector[drawableFrameVector.size()-1]->frame);
	
	if(newCloudAdded)
	  aligner.setInitialGuess(initialGuess);
	else
	  aligner.setInitialGuess(localT[localT.size()-1]);
	aligner.setSensorOffset(drawableFrameVector[drawableFrameVector.size()-1]->sensorOffset);
	
	aligner.align();
	
	if(newCloudAdded)
	  localT.push_back(aligner.T());
	else
	  localT[localT.size()-1] = aligner.T();
	  
	globalT = stepByStepInit * localT[localT.size()-1];
      }
      cout << "Local transformation: " << endl << aligner.T().matrix() << endl;

      // Update cloud drawing position.
      drawableFrameVector[drawableFrameVector.size()-1]->dPoints->setTransformation(globalT);
      drawableFrameVector[drawableFrameVector.size()-1]->dNormals->setTransformation(globalT);
      drawableFrameVector[drawableFrameVector.size()-1]->dCovariances->setTransformation(globalT);
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setReferencePointsTransformation(globalT * localT[localT.size()-1].inverse());
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setTransformation(globalT);
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setReferencePoints(&drawableFrameVector[drawableFrameVector.size()-2]->frame.points());
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setCurrentPoints(&drawableFrameVector[drawableFrameVector.size()-1]->frame.points());
      drawableFrameVector[drawableFrameVector.size()-1]->correspondences = CorrespondenceVector(aligner.correspondenceFinder()->correspondences());
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setCorrespondences(&drawableFrameVector[drawableFrameVector.size()-1]->correspondences);
      drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences->setNumCorrespondences(aligner.correspondenceFinder()->numCorrespondences());

      // Show zBuffers.
      refScn->clear();
      currScn->clear();
      QImage refQImage;
      QImage currQImage;
      DepthImageView div;
      div.computeColorMap(300, 2000, 128);
      div.convertToQImage(refQImage, aligner.correspondenceFinder()->referenceDepthImage());
      div.convertToQImage(currQImage, aligner.correspondenceFinder()->currentDepthImage());
      refScn->addPixmap((QPixmap::fromImage(refQImage)).scaled(QSize((int)refQImage.width()/(ng_scale*3), (int)(refQImage.height()/(ng_scale*3)))));
      currScn->addPixmap((QPixmap::fromImage(currQImage)).scaled(QSize((int)currQImage.width()/(ng_scale*3), (int)(currQImage.height()/(ng_scale*3)))));
      pwnGMW.graphicsView1_2d->show();
      pwnGMW.graphicsView2_2d->show();

      wasInitialGuess = false;
      newCloudAdded = false;
      *initialGuessViewer = 0;
      *optimizeViewer = 0;
    }
    // Merge button pressed.
    else if(*merge) {
      if(mergingFrame.points().size() > 0) {
	merger.merge(&mergingFrame, globalT * drawableFrameVector[drawableFrameVector.size()-1]->sensorOffset);
	// Clear drawable frames.
	pwnGMW.viewer_3d->clearDrawableList();
	for(size_t i = 0; i < drawableFrameVector.size(); i++)
	  delete(drawableFrameVector[i]);
	drawableFrameVector.clear();
	globalT = Isometry3f::Identity();
	localT.clear();
	refScn->clear();
	currScn->clear();
	wasInitialGuess = false;
	// if(drawableFrameVector.size() > 0) {
	//   pwnGMW.viewer_3d->popBack();
	//   pwnGMW.viewer_3d->popBack();
	//   pwnGMW.viewer_3d->popBack();
	//   pwnGMW.viewer_3d->popBack();
	//   delete(drawableFrameVector[drawableFrameVector.size()-1]);
	//   drawableFrameVector.pop_back();
	// }
	// refScn->clear();
	// currScn->clear();
	// wasInitialGuess = false;
	// newCloudAdded = false;
	// Add drawable items.	
	drawableFrame = new DrawableFrame(&mergingFrame, vz_step, cameraMatrix);
	drawableFrameVector.push_back(drawableFrame);
	drawableFrame = 0;
	drawableFrameVector[drawableFrameVector.size()-1]->dPoints->setTransformation(Isometry3f::Identity());
	drawableFrameVector[drawableFrameVector.size()-1]->dNormals->setTransformation(Isometry3f::Identity());
	drawableFrameVector[drawableFrameVector.size()-1]->dCovariances->setTransformation(Isometry3f::Identity());
	pwnGMW.viewer_3d->addDrawable((Drawable*)drawableFrameVector[drawableFrameVector.size()-1]->dPoints);
	pwnGMW.viewer_3d->addDrawable((Drawable*)drawableFrameVector[drawableFrameVector.size()-1]->dNormals);
	pwnGMW.viewer_3d->addDrawable((Drawable*)drawableFrameVector[drawableFrameVector.size()-1]->dCovariances);
	pwnGMW.viewer_3d->addDrawable((Drawable*)drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences);
      }	
      *merge = 0;
    }
    // Add cloud was pressed.
    else if(*addCloud) {
      if(itemList) {
	drawableFrame = new DrawableFrame(itemList->text().toStdString(), vz_step, cameraMatrix);
	drawableFrame->computeStats();
	drawableFrameVector.push_back(drawableFrame);
	if(drawableFrameVector.size() == 1) {
	  depthImageConverter = DepthImageConverter(&drawableFrame->projector, 
						    &drawableFrame->statsCalculator,
						    &drawableFrame->pointInformationMatrixCalculator,
						    &drawableFrame->normalInformationMatrixCalculator);
	  merger.setDepthImageConverter(&depthImageConverter);
	  merger.setImageSize(drawableFrame->depthImage.rows(), drawableFrame->depthImage.cols());
	  mergingFrame.add(drawableFrameVector[0]->frame);
	}
	drawableFrame = 0;
	// Add drawable items.
	drawableFrameVector[drawableFrameVector.size()-1]->dPoints->setTransformation(globalT);
	drawableFrameVector[drawableFrameVector.size()-1]->dNormals->setTransformation(globalT);
	drawableFrameVector[drawableFrameVector.size()-1]->dCovariances->setTransformation(globalT);
	pwnGMW.viewer_3d->addDrawable((Drawable*)drawableFrameVector[drawableFrameVector.size()-1]->dPoints);
	pwnGMW.viewer_3d->addDrawable((Drawable*)drawableFrameVector[drawableFrameVector.size()-1]->dNormals);
	pwnGMW.viewer_3d->addDrawable((Drawable*)drawableFrameVector[drawableFrameVector.size()-1]->dCovariances);
	pwnGMW.viewer_3d->addDrawable((Drawable*)drawableFrameVector[drawableFrameVector.size()-1]->dCorrespondences);
      }
      newCloudAdded = true;
      *addCloud = 0;
    }
    // clear buttons pressed.
    else if(*clearAll) {
      pwnGMW.viewer_3d->clearDrawableList();
      for(size_t i = 0; i < drawableFrameVector.size(); i++)
	delete(drawableFrameVector[i]);
      drawableFrameVector.clear();
      globalT = Isometry3f::Identity();
      localT.clear();
      refScn->clear();
      currScn->clear();
      wasInitialGuess = false;
      newCloudAdded = false;
      *clearAll = 0;
    }
    else if(*clearLast) {
      if(drawableFrameVector.size() > 0) {
        pwnGMW.viewer_3d->popBack();
        pwnGMW.viewer_3d->popBack();
        pwnGMW.viewer_3d->popBack();
        pwnGMW.viewer_3d->popBack();
	delete(drawableFrameVector[drawableFrameVector.size()-1]);
	drawableFrameVector.pop_back();
      }
      if(localT.size() > 0) { 
	globalT = globalT * localT[localT.size()-1].inverse();
	localT.pop_back();
      }
      refScn->clear();
      currScn->clear();
      wasInitialGuess = false;
      newCloudAdded = false;
      *clearLast = 0;
    }

    // To avoid memorized commands to be managed.
    *initialGuessViewer = 0;
    *optimizeViewer = 0;
    *merge = 0;
    *addCloud = 0; 
    *clearAll = 0;
    *clearLast = 0;

    pwnGMW.viewer_3d->updateGL();

    usleep(10000);
  }
  return 0;  
}
