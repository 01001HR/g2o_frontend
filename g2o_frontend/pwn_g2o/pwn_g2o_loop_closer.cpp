#include <fstream>
#include <unistd.h>
#include <set>
#include <sys/stat.h>

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>

#include "g2o/stuff/command_args.h"
#include "g2o/stuff/timeutil.h"

#include "g2o_frontend/pwn_mapper/pwn_loop_closer_controller.h"
#include "g2o_frontend/pwn_viewer/pwn_qglviewer.h"
#include "g2o_frontend/pwn_viewer/drawable_frame.h"
#include "g2o_frontend/pwn_viewer/gl_parameter_frame.h"
#include "g2o_frontend/pwn_viewer/drawable_trajectory.h"
#include "g2o_frontend/pwn_viewer/gl_parameter_trajectory.h"

#include "g2o_frontend/pwn_utils/pwn_utils.h"

using namespace std;
using namespace Eigen;
using namespace pwn;
using namespace g2o;

int main(int argc, char** argv) {
  /************************************************************************
   * Input Handling *
   ************************************************************************/
  string g2o_filename;

  // Variables for the input parameters.
  float al_scale;
  int al_innerIterations;
  int al_outerIterations;
  int al_minNumInliers;

  int pj_imageRows;
  int pj_imageCols;
  float pj_fov;
  float pj_fy;
  float al_minError;
  float al_curvatureThreshold;
  int vz_startingVertex;
  int vz_endingVertex;
  int vz_step;
  float cf_inlierNormalAngularThreshold;
  float cf_flatCurvatureThreshold;
  float cf_inlierCurvatureRatioThreshold;
  float cf_inlierDistanceThreshold;

  // Input parameters handling.
  g2o::CommandArgs arg;
  
  // Optional input parameters.
  arg.param("al_scale", al_scale, 1.0f, "Specify the scaling factor to apply on the depth image");
  arg.param("pj_imageRows", pj_imageRows, 120, "Specify the number of rows of the depth image associated to the pinhole point projector");
  arg.param("pj_imageCols", pj_imageCols, 320, "Specify the number of columns of the depth image associated to the pinhole point projector");
  arg.param("pj_fov", pj_fov, M_PI, "Specify field of view of the cylindrical projector");
  arg.param("pj_fy", pj_fy, 50, "Specify the focal lenght of the y for the projector");
  
  arg.param("al_innerIterations", al_innerIterations, 1, "Specify the inner iterations");
  arg.param("al_outerIterations", al_outerIterations, 10, "Specify the outer iterations");
  arg.param("al_minNumInliers", al_minNumInliers, 10000, "Specify the minimum number of inliers to consider an alignment good");
  arg.param("al_minError", al_minError, 10.0f, "Specify the minimum error to consider an alignment good");
  arg.param("al_curvatureThreshold", al_curvatureThreshold, 0.1f, "Specify the curvature treshshold for information matrix computation");
  arg.param("cf_inlierNormalAngularThreshold", cf_inlierNormalAngularThreshold, M_PI / 6.0f, "Maximum angle between the normals of two points to regard them as iniliers");
  arg.param("cf_flatCurvatureThreshold", cf_flatCurvatureThreshold, 0.02f, "Maximum curvature value for a point to be used for data association");
  arg.param("cf_inlierCurvatureRatioThreshold", cf_inlierCurvatureRatioThreshold, 1.3f, "Maximum curvature ratio value between two points to regard them as iniliers");
  arg.param("cf_inlierDistanceThreshold", cf_inlierDistanceThreshold, 0.5f, "Maximum metric distance between two points to regard them as iniliers");

  arg.param("vz_startingVertex", vz_startingVertex, 0, "Specify the vertex id from which to start the process");
  arg.param("vz_endingVertex", vz_endingVertex, -1, "Specify the vertex id where to end the process"); 
  arg.param("vz_step", vz_step, 5, "A graphic element is drawn each vz_step elements");

  // Last parameter has to be the working directory.
  arg.paramLeftOver("g2o_input_filename", g2o_filename, "", "g2o inputfilename", true);

  // Set parser input.
  arg.parseArgs(argc, argv);
  
  /************************************************************************
   * Window Creation *
   ************************************************************************/
  QApplication application(argc,argv);
  QWidget* mainWindow = new QWidget();
  mainWindow->setWindowTitle("pwn_g2o_loop_closer");
  QHBoxLayout* baseLayout = new QHBoxLayout();
  mainWindow->setLayout(baseLayout);
  QVBoxLayout* listWidgetLayout = new QVBoxLayout();
  baseLayout->addItem(listWidgetLayout);
  QVBoxLayout* qglviewerLayout = new QVBoxLayout();
  baseLayout->addItem(qglviewerLayout);
  baseLayout->setStretch(1.0f, 1.0f);

  QListWidget* listWidget = new QListWidget(mainWindow);
  listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
  listWidgetLayout->addWidget(listWidget);
  QPushButton *alignButton = new QPushButton("&Align", mainWindow);
  listWidgetLayout->addWidget(alignButton);
  PWNQGLViewer* viewer = new PWNQGLViewer(mainWindow);
  qglviewerLayout->addWidget(viewer);

  /************************************************************************
   * Loading Graph *
   ************************************************************************/
  typedef BlockSolver< BlockSolverTraits<-1, -1> > SlamBlockSolver;
  typedef LinearSolverCSparse<SlamBlockSolver::PoseMatrixType> SlamLinearSolver;
  SlamLinearSolver *linearSolver = new SlamLinearSolver();
  linearSolver->setBlockOrdering(false);
  SlamBlockSolver *blockSolver = new SlamBlockSolver(linearSolver);
  OptimizationAlgorithmLevenberg *solverGauss = new OptimizationAlgorithmLevenberg(blockSolver);
  //OptimizationAlgorithmGaussNewton *solverGauss = new OptimizationAlgorithmGaussNewton(blockSolver);
  SparseOptimizer *graph = new SparseOptimizer();
  graph->setAlgorithm(solverGauss);
  graph->load(g2o_filename.c_str());
  vector<int> vertexIds(graph->vertices().size());
  int k = 0;
  for(OptimizableGraph::VertexIDMap::iterator it = graph->vertices().begin(); it != graph->vertices().end(); ++it) {
    vertexIds[k++] = (it->first);
  }
  sort(vertexIds.begin(), vertexIds.end());
  
  for(size_t i = 0; i < vertexIds.size(); ++i) {
    OptimizableGraph::Vertex *_v = graph->vertex(vertexIds[i]);
    g2o::VertexSE3 *v = dynamic_cast<g2o::VertexSE3*>(_v);
    if(!v)
      continue;
    OptimizableGraph::Data *d = v->userData();
    while(d) {
      PWNData *pwnData = dynamic_cast<PWNData*>(d);
      if(!pwnData) {
	d = d->next();
	continue;
      }
      
      char buff[1024];
      sprintf(buff, "%d", v->id());
      QString listItem(buff);
      listWidget->addItem(listItem);
      QListWidgetItem *lastItem = listWidget->item(listWidget->count() - 1);
      lastItem->setHidden(false);
      d = d->next();
    }
  }

  /************************************************************************
   * Setting Variables *
   ************************************************************************/
  if(vz_startingVertex < 0)
    vz_startingVertex = 0;
  if(vz_endingVertex < 0 || vz_endingVertex > listWidget->count())
    vz_endingVertex = listWidget->count();
  
  std::vector<Isometry3f> trajectory;
  std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f> > trajectoryColors;
  std::vector<G2OFrame*> frames;
  frames.resize(listWidget->count());
  std::fill(frames.begin(), frames.end(), (G2OFrame*)0);

  Isometry3f sensorOffset = Isometry3f::Identity();
  sensorOffset.translation() = Vector3f(0.15f, 0.0f, 0.05f);
  Quaternionf quaternion;
  //xyzToQuat(quaternion, -0.579275, 0.56288, -0.41087); // segway_02
  quaternion = Quaternionf(0.5f, -0.5f, 0.5f, -0.5f);
  sensorOffset.linear() = quaternion.toRotationMatrix();
  sensorOffset.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;
  PWNLoopCloserController *controller = new PWNLoopCloserController(graph);
  controller->setMinNumInliers(al_minNumInliers);
  controller->setMinError(al_minError);
  //controller->setCameraMatrix(cameraMatrix);
  controller->setSensorOffset(sensorOffset);
  //controller->setReduction(al_scale);
  //controller->setImageRows(al_imageCols);
  //controller->setImageCols(al_imageRows);
  controller->setImageRows(pj_imageCols);
  controller->setImageCols(pj_imageRows);
  CylindricalPointProjector* projector = controller->cylindricalPointProjector();
  projector->setAngularFov(pj_fov);
  projector->setAngularResolution(pj_imageCols *.5/pj_fov);
  projector->setVerticalFocalLenght(pj_fy);
  projector->setVerticalCenter(pj_imageRows/2);

  cerr << "imageRows: " << controller->imageRows() << endl;
  cerr << "imageCols: " << controller->imageCols() << endl;
  cerr << "fov:       "  << projector->angularFov() << endl;
  cerr << "resolution:       "  << projector->angularResolution() << endl;
  cerr << "cy:       "  << projector->verticalCenter() << endl;
  cerr << "fy:       "  << projector->verticalFocalLenght() << endl;

  controller->setCurvatureThreshold(al_curvatureThreshold);
  controller->setInnerIterations(al_innerIterations);
  controller->setOuterIterations(al_outerIterations);

  CorrespondenceFinder* correspondenceFinder =   controller->correspondenceFinder();
  correspondenceFinder->setInlierDistanceThreshold(cf_inlierDistanceThreshold);
  correspondenceFinder->setFlatCurvatureThreshold(cf_flatCurvatureThreshold);  
  correspondenceFinder->setInlierCurvatureRatioThreshold(cf_inlierCurvatureRatioThreshold);
  correspondenceFinder->setInlierNormalAngularThreshold(cosf(cos(cf_inlierNormalAngularThreshold)));


  viewer->init();
  viewer->setAxisIsDrawn(true);
  viewer->show();

  mainWindow->show();
  
  listWidget->show();

  GLParameterTrajectory *parameterTrajectory = new GLParameterTrajectory(0.03f, Vector4f(1.0f, 0.0f, 1.0f, 1.0f));
  DrawableTrajectory *drawableTrajectory = new DrawableTrajectory(Isometry3f::Identity(), parameterTrajectory, &trajectory, &trajectoryColors);
  viewer->addDrawable(drawableTrajectory);
  GLParameterFrame *parameterFrame = new GLParameterFrame(vz_step);


  G2OFrame *referenceFrame = 0, *currentFrame = 0;
  /************************************************************************
   * MAIN DRAWING LOOP *
   ************************************************************************/
  while(viewer->isVisible()) {
    bool changed = false;
    
    /************************************************************************
     * Drawing Trajectory *
     ************************************************************************/
    trajectory.clear();
    trajectoryColors.clear();

    for(size_t i = 0; i < vertexIds.size(); ++i) {
      OptimizableGraph::Vertex *_v = graph->vertex(vertexIds[i]);
      g2o::VertexSE3 *v = dynamic_cast<g2o::VertexSE3*>(_v);
      if(!v)
    	continue;
      OptimizableGraph::Data *d = v->userData();
      bool foundPwnData = false;
      while(d) {
    	PWNData *pwnData = dynamic_cast<PWNData*>(d);
    	if(!pwnData) {
    	  d = d->next();
    	  continue;
    	}
      
    	foundPwnData = true;
    	d = d->next();
      }

      Isometry3f estimate;
      isometry3d2f(estimate, v->estimate());
      if(foundPwnData) {
    	trajectory.push_back(estimate);
    	trajectoryColors.push_back(Eigen::Vector4f(1.0f, 0.0f, 0.0f, 1.0f));
      }
      else {
    	trajectory.push_back(estimate);
    	trajectoryColors.push_back(Eigen::Vector4f(0.5f, 0.5f, 0.5f, 0.3f));
      }
    }

    // for(int i = vz_startingVertex; i < vz_endingVertex; i++) {
    //   QListWidgetItem *listItem = listWidget->item(i);
    //   listItem->setHidden(false);
    //   string idString = listItem->text().toUtf8().constData();
    //   int index = atoi(idString.c_str());
    //   VertexSE3 *v = dynamic_cast<VertexSE3*>(graph->vertex(index));
    //   if(v) {
    // 	Isometry3f estimate;
    // 	for(int r = 0; r < 3; r++) {
    // 	  for(int c = 0; c < 4; c++) {
    // 	    estimate(r, c) = v->estimate()(r, c);
    // 	  }
    // 	}
    // 	estimate.matrix().row(3) << 0.0l, 0.0l, 0.0l, 1.0l;
    // 	trajectory.push_back(estimate);
    // 	trajectoryColors.push_back(Eigen::Vector3f(0.3f, 0.3f, 0.3f));
    //   }
    // }

    // Align button was pressed
    if(alignButton->isDown()) {
      // Clean old aligned frames
      if(referenceFrame)
	delete referenceFrame;
      referenceFrame = 0;
      if(currentFrame)
	delete currentFrame;
      currentFrame = 0;

      // Search for the two frames selected
      for(int k = vz_startingVertex; k < vz_endingVertex; k++) {
	QListWidgetItem* item = listWidget->item(k);
	if(item) {
	  if(item->isSelected()) {
	    string idString = item->text().toUtf8().constData();
	    int index = atoi(idString.c_str());
	    VertexSE3 *v = dynamic_cast<VertexSE3*>(graph->vertex(index));
	    if(v) {
	      if(!referenceFrame) {
		referenceFrame = new G2OFrame(v);	
		controller->extractPWNData(referenceFrame);
	      }
	      else if(!currentFrame) {
		currentFrame = new G2OFrame(v);
		controller->extractPWNData(currentFrame);
	      }
	      else {
		break;
	      }
	    }	
	  }
	}
      }
      
      // Align
      Isometry3f transform = Isometry3f::Identity();
      transform.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;
      if(controller->alignVertexWithPWNData(transform, referenceFrame, currentFrame)) {
	cerr << "Edge added" << endl;
	OptimizableGraph::Vertex* gauge=graph->findGauge();
	if(gauge){
	  cerr << "gauge found, vertex" << gauge->id() << endl;
	  gauge->setFixed(true);
	}
	graph->initializeOptimization();
	graph->setVerbose(true);
	graph->optimize(10);
	cerr << "optimization done" << endl;
	changed = true;
      } else {
	cerr << "Edge was not added because of a bad alignment" << endl;
      }
    }

    // Manage user ListWidget elements highlighting
    for(int k = vz_startingVertex; k < vz_endingVertex; k++) {
      QListWidgetItem* item = listWidget->item(k);
      if(item) {
	if(item->isSelected()) {
	  //trajectoryColors[k] = Eigen::Vector3f(1.0f, 0.3f, 0.3f);
	  string idString = item->text().toUtf8().constData();
	  int index = atoi(idString.c_str());
	  VertexSE3 *v = dynamic_cast<VertexSE3*>(graph->vertex(index));
	  trajectoryColors[index] = Eigen::Vector4f(0.0f, 1.0f, 0.0f, 1.0f);
	  drawableTrajectory->updateTrajectoryDrawList();
	  if(v && !frames[k]) {
	    G2OFrame *currentFrame = new G2OFrame(v);
	    controller->extractPWNData(currentFrame);
	    frames[k] = currentFrame;
	    Eigen::Isometry3f isotta;
	    for (int c = 0; c<4; c++)
	      for (int r = 0; r<3; r++)
		isotta.matrix()(r,c) = v->estimate().matrix()(r,c);
	    DrawableFrame *drawableFrame = new DrawableFrame(isotta, parameterFrame, frames[k]);
	    viewer->addDrawable(drawableFrame);	    
	    changed = true;
	  }	
	}
	else {
	  if(frames[k]) {
	    for(size_t j = 0; j < viewer->drawableList().size(); j++) {
	      DrawableFrame *drawableFrame = dynamic_cast<DrawableFrame*>(viewer->drawableList()[j]);
	      if(drawableFrame) {
		G2OFrame *currentFrame = dynamic_cast<G2OFrame*>(drawableFrame->frame());
		if(currentFrame) {
		  if(currentFrame->vertex()->id() == frames[k]->vertex()->id()) {
		    viewer->erase(j);
		    delete drawableFrame;
		    delete frames[k];
		    frames[k] = 0;
		    changed = true;
		    break;
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    if(changed)
      viewer->updateGL();
    application.processEvents();
  }

  graph->save("graphwithpwnandedges.g2o");

  return 0;
};
