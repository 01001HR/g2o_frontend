#include <QApplication>
#include <QMainWindow>
#include <QHBoxLayout>

#include "g2o/stuff/command_args.h"

#include "g2o_frontend/pwn_viewer/pwn_qglviewer.h"
#include "g2o_frontend/pwn_viewer/drawable_frame.h"
#include "g2o_frontend/pwn_viewer/gl_parameter_cloud.h"
#include "g2o_frontend/pwn_viewer/drawable_trajectory.h"
#include "g2o_frontend/pwn_viewer/gl_parameter_trajectory.h"

#include "pwn_odometry_controller.h"

using namespace std;
using namespace Eigen;
using namespace g2o;
using namespace pwn;

int main (int argc, char** argv) {
  // Handle input
  int vz_applyTransform, vz_pointStep;
  float vz_pointSize, vz_alpha;
  float pwn_scaleFactor, pwn_scale, pwn_inliersFraction;

  string pwn_configFilename, pwn_logFilename, pwn_sensorType;
  string oc_benchmarkFilename, oc_trajectoryFilename, oc_groundTruthFilename, oc_associationsFilename;

  string directory;

  g2o::CommandArgs arg;
  arg.param("vz_pointSize", vz_pointSize, 1.0f, "Size value of the points to visualize");
  arg.param("vz_transform", vz_applyTransform, 1, "Apply absolute transform to the point clouds");
  arg.param("vz_alpha", vz_alpha, 1.0f, "Alpha channel value of the points to visualize");
  arg.param("vz_pointStep", vz_pointStep, 1, "Step value at which points are drawn");
  
  arg.param("pwn_inliersFraction", pwn_inliersFraction, 0.6f, "Inliers fraction to accept before to take a new reference frame");
  arg.param("pwn_scaleFactor", pwn_scaleFactor, 0.001f, "Scale factor at which the depth images were saved");
  arg.param("pwn_scale", pwn_scale, 4.0f, "Scale factor the depth images are loaded");
  arg.param("pwn_configFilename", pwn_configFilename, "pwn.conf", "Specify the name of the file that contains the parameter configurations of the pwn structures");
  arg.param("pwn_logFilename", pwn_logFilename, "pwn.log", "Specify the name of the file that will contain the trajectory computed in boss format");
  arg.param("pwn_sensorType", pwn_sensorType, "kinect", "Sensor type: xtion640/xtion320/kinect/kinectFreiburg1/kinectFreiburg2/kinectFreiburg3");  
  arg.param("oc_benchmarkFilename", oc_benchmarkFilename, "pwn_benchmark.txt", "Specify the name of the file that will contain the results of the benchmark");
  arg.param("oc_trajectoryFilename", oc_trajectoryFilename, "pwn_trajectory.txt", "Specify the name of the file that will contain the trajectory computed");
  arg.param("oc_associationsFilename", oc_associationsFilename, "pwn_associations.txt", "Specify the name of the file that contains the images associations");
  arg.param("oc_groundTruthFilename", oc_groundTruthFilename, "groundtruth.txt", "Specify the name of the file that contains the ground truth trajectory");

  arg.paramLeftOver("directory", directory, ".", "Directory where the program will find the input needed files and the subfolders depth and rgb containing the images", true);

  arg.parseArgs(argc, argv);
  
  // Create GUI
  QApplication application(argc,argv);
  QWidget* mainWindow = new QWidget();
  mainWindow->setWindowTitle("PWN Odometry");
  QHBoxLayout* hlayout = new QHBoxLayout();
  mainWindow->setLayout(hlayout);
  PWNQGLViewer* viewer = new PWNQGLViewer(mainWindow);
  hlayout->addWidget(viewer);
  viewer->init();
  viewer->setAxisIsDrawn(true);
  viewer->show();
  mainWindow->showMaximized();
  mainWindow->show();
  
  // Init odometry controller
  PWNOdometryController *pwnOdometryController = new PWNOdometryController(pwn_configFilename.c_str(), pwn_logFilename.c_str());
  pwnOdometryController->fileInitialization(oc_groundTruthFilename.c_str(), oc_associationsFilename.c_str(),
					    oc_trajectoryFilename.c_str(), oc_benchmarkFilename.c_str());
  pwnOdometryController->setScaleFactor(pwn_scaleFactor);
  pwnOdometryController->setScale(pwn_scale);
  pwnOdometryController->setSensorType(pwn_sensorType);
  pwnOdometryController->setInliersFraction(pwn_inliersFraction);

  // Compute odometry
  bool sceneHasChanged;
  Cloud *cloud = 0, *groundTruthReferenceCloud = 0;
  Isometry3f groundTruthPose;	
  GLParameterTrajectory *groundTruthTrajectoryParam = new GLParameterTrajectory(0.02f, Vector4f(0.0f, 1.0f, 0.0f, 0.6f));
  GLParameterTrajectory *trajectoryParam = new GLParameterTrajectory(0.02f, Vector4f(1.0f, 0.0f, 0.0f, 0.6f));
  std::vector<Isometry3f> trajectory, groundTruthTrajectory;
  std::vector<Vector4f, Eigen::aligned_allocator<Eigen::Vector4f> > trajectoryColors, groundTruthTrajectoryColors;
  DrawableTrajectory *drawableGroundTruthTrajectory = new DrawableTrajectory(Isometry3f::Identity(), 
									    groundTruthTrajectoryParam, 
									    &groundTruthTrajectory, 
									    &groundTruthTrajectoryColors);
  DrawableTrajectory *drawableTrajectory = new DrawableTrajectory(Isometry3f::Identity(), 
								 trajectoryParam, 
								 &trajectory, 
								 &trajectoryColors);
  viewer->addDrawable(drawableGroundTruthTrajectory);
  viewer->addDrawable(drawableTrajectory);
  while(mainWindow->isVisible()) {
    if(pwnOdometryController->loadCloud(cloud)) {
      sceneHasChanged = false;
    
      if(pwnOdometryController->counter() == 1) {
	pwnOdometryController->getGroundTruthPose(groundTruthPose, atof(pwnOdometryController->timestamp().c_str()));
	// Add first frame to draw
	groundTruthReferenceCloud = new Cloud();
	*groundTruthReferenceCloud = *cloud;
	GLParameterCloud *groundTruthReferenceCloudParams = new GLParameterCloud();
	groundTruthReferenceCloudParams->setStep(vz_pointStep);
	groundTruthReferenceCloudParams->setShow(true);	
	groundTruthReferenceCloudParams->parameterPoints()->setColor(Vector4f(1.0f, 0.0f, 1.0f, vz_alpha));
	DrawableCloud *drawableGroundTruthReferenceCloud = new DrawableCloud(groundTruthPose, groundTruthReferenceCloudParams, groundTruthReferenceCloud);

	GLParameterCloud *cloudParams = new GLParameterCloud();
	cloudParams->setStep(vz_pointStep);
	cloudParams->setShow(true);	
	DrawableCloud *drawableCloud = new DrawableCloud(pwnOdometryController->globalPose(), cloudParams, cloud);

	viewer->addDrawable(drawableGroundTruthReferenceCloud);
	viewer->addDrawable(drawableCloud);

	sceneHasChanged = true;
      }
      // Compute current transformation
      if(pwnOdometryController->processCloud()) {
	pwnOdometryController->getGroundTruthPose(groundTruthPose, atof(pwnOdometryController->timestamp().c_str()));	
	
	// Add frame to draw
	GLParameterCloud *frameParams = new GLParameterCloud();
	frameParams->setStep(vz_pointStep);
	frameParams->setShow(true);	
	DrawableCloud *drawableCloud = new DrawableCloud(pwnOdometryController->globalPose(), frameParams, cloud);
	viewer->addDrawable(drawableCloud);
	
	// Add trajectory pose
	groundTruthTrajectory.push_back(groundTruthPose);
	groundTruthTrajectoryColors.push_back(Vector4f(0.0f, 1.0f, 0.0f, 0.6f));
	trajectory.push_back(pwnOdometryController->globalPose());
	trajectoryColors.push_back(Vector4f(1.0f, 0.0f, 0.0f, 0.6f));
	drawableGroundTruthTrajectory->updateTrajectoryDrawList();
	drawableTrajectory->updateTrajectoryDrawList();
      
	// Write results
	pwnOdometryController->writeResults();
      
	sceneHasChanged = true;
      }
 
      // Remove old frames
      if(viewer->drawableList().size() > 23) {
	DrawableCloud *d = dynamic_cast<DrawableCloud*>(viewer->drawableList()[3]);
	if(d) {
	  Cloud *f = d->cloud();
	  if(pwnOdometryController->referenceCloud() != f &&
	     pwnOdometryController->currentCloud() != f) {
	    viewer->erase(3);
	    delete d;
	    delete f; 
	  }
	}
      }
      cloud = 0;
    }

    if(sceneHasChanged)
      viewer->updateGL();
    application.processEvents();
  }

  return 0;
}
