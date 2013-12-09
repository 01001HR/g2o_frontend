#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QListWidget>

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>

#include "g2o_frontend/pwn_viewer/pwn_qglviewer.h"
#include "g2o_frontend/pwn_viewer/drawable_frame.h"
#include "g2o_frontend/pwn_viewer/gl_parameter_frame.h"
#include "g2o/stuff/command_args.h"
#include "g2o_frontend/basemath/bm_se3.h"
#include "g2o_frontend/pwn_utils/pwn_utils.h"

#include "g2o_frontend/pwn_core/frame.h"

#include "pwn_tracker.h"
#include "g2o_frontend/boss_map/map_manager.h"

using namespace Eigen;
using namespace g2o;
using namespace std;
using namespace pwn;
using namespace pwn_tracker;

MapManager* load(std::vector<PwnTrackerFrame*> &trackerFrames,
		 std::vector<PwnTrackerRelation*> &trackerRelations,
		 std::vector<Serializable*> &objects,
		 Deserializer &des);
  
int main (int argc, char** argv) {
  // Handle input
  float pointSize;
  float pointStep;
  float alpha;
  int applyTransform;
  string logFilename;
  string configFilename;
  float di_scaleFactor;
  float scale;

  g2o::CommandArgs arg;
  arg.param("vz_pointSize", pointSize, 1.0f, "Size of the points where are visualized");
  arg.param("vz_transform", applyTransform, 1, "Choose if you want to apply the absolute transform of the point clouds");
  arg.param("vz_alpha", alpha, 1.0f, "Alpha channel value used for the color points");
  arg.param("vz_pointStep", pointStep, 1, "Step at which point are drawn");  
  arg.param("vz_scale", scale, 2, "Depth image size reduction factor");
  arg.param("di_scaleFactor", di_scaleFactor, 0.001f, "Scale factor to apply to convert depth images in meters");
  arg.paramLeftOver("configFilename", configFilename, "", "Configuration filename", true);
  arg.paramLeftOver("logFilename", logFilename, "", "Log filename", true);
  arg.parseArgs(argc, argv);
  
  // Create GUI
  QApplication application(argc,argv);
  QWidget *mainWindow = new QWidget();
  mainWindow->setWindowTitle("pwn_tracker_log_viewer");
  QHBoxLayout *hlayout = new QHBoxLayout();
  mainWindow->setLayout(hlayout);
  QVBoxLayout *vlayout = new QVBoxLayout();
  hlayout->addItem(vlayout);
  QVBoxLayout *vlayout2 = new QVBoxLayout();
  hlayout->addItem(vlayout2);
  hlayout->setStretch(1, 1);

  QListWidget* listWidget = new QListWidget(mainWindow);
  listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
  vlayout->addWidget(listWidget);
  PWNQGLViewer* viewer = new PWNQGLViewer(mainWindow);
  vlayout2->addWidget(viewer);
  Eigen::Isometry3f T;
  T.setIdentity();
  T.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;

  // Read config file
  cout << "Loading config file..." << endl;
  Aligner *aligner;
  DepthImageConverter *converter;
  std::vector<Serializable*> instances = readConfig(aligner, converter, configFilename.c_str());
  converter->projector()->scale(1.0f/scale);
  cout << "... done" << endl;

  // Read and parse log file
  std::vector<boss::Serializable*> objects;
  std::vector<PwnTrackerFrame*> trackerFrames;
  std::vector<PwnTrackerRelation*> trackerRelations;
  Deserializer des;
  des.setFilePath(logFilename);
  cout << "Loading log file..." << endl;
  load(trackerFrames, trackerRelations, objects, des);
  cout << "... done" << endl;

  // Load the drawable list with the PwnTrackerFrame objects
  std::vector<Frame*> pointClouds;
  pointClouds.resize(trackerFrames.size());
  Frame *dummyFrame = 0; 
  std::fill(pointClouds.begin(), pointClouds.end(), dummyFrame);
  for(size_t i = 0; i < trackerFrames.size(); i++) {
    char nummero[1024];
    sprintf(nummero, "%05d", (int)i);
    listWidget->addItem(QString(nummero));
  }
  // Manage GUI
  viewer->init();
  mainWindow->show();
  mainWindow->showMaximized();
  viewer->show();
  listWidget->show();
  //viewer->setAxisIsDrawn(true);
  DepthImage depthImage;
  DepthImage scaledDepthImage;
  while (mainWindow->isVisible()) {
    for (int i = 0; i < listWidget->count() && mainWindow->isVisible(); i++) {
      PwnTrackerFrame *pwnTrackerFrame = trackerFrames[i];
      Frame *frame = new Frame();
      boss_map::ImageBLOB *fromDepthBlob = pwnTrackerFrame->depthImage.get();
      DepthImage depthImage;
      depthImage.fromCvMat(fromDepthBlob->cvImage());
      DepthImage::scale(scaledDepthImage, depthImage, scale);
      converter->compute(*frame, scaledDepthImage);
      delete fromDepthBlob;
      Isometry3f transform = Isometry3f::Identity();
      if(applyTransform) {
	isometry3d2f(transform, pwnTrackerFrame->transform());
	transform = transform * pwnTrackerFrame->sensorOffset;
      }
      transform.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;
      GLParameterFrame *frameParams = new GLParameterFrame();
      frameParams->setStep(pointStep);
      frameParams->setShow(true);
      DrawableFrame *drawableFrame = new DrawableFrame(transform, frameParams, frame);
      viewer->addDrawable(drawableFrame);
      
      if(viewer->drawableList().size() > 30) {
	DrawableFrame *d = dynamic_cast<DrawableFrame*>(viewer->drawableList()[0]);
	Frame *f = d->frame();
	viewer->popFront();
	delete d;
	delete f; 
      }

      Isometry3f temp;
      isometry3d2f(temp, pwnTrackerFrame->transform());
      temp.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;
      viewer->updateCameraPosition(temp);
      viewer->updateGL();

      application.processEvents();
    }   
  }
  return 0;
}

MapManager* load(std::vector<PwnTrackerFrame*> &trackerFrames,
		 std::vector<PwnTrackerRelation*> &trackerRelations,
		 std::vector<Serializable*> &objects,
		 Deserializer &des) {
  ofstream os("path.dat");
  Serializable *o = 0;
  boss_map::MapManager *manager = 0;
  while ((o = des.readObject())) {
    objects.push_back(o);
    boss_map::MapManager *m = dynamic_cast<boss_map::MapManager*>(o);
    if (m)
      manager = m;
    PwnTrackerFrame *f = dynamic_cast<PwnTrackerFrame*>(o);
    if (f) {
      trackerFrames.push_back(f);
      os << f->transform().translation().transpose() << endl;
    }
    PwnTrackerRelation *r = dynamic_cast<PwnTrackerRelation*>(o);
    if (r) {
      trackerRelations.push_back(r);
    }
  }
  
  cerr << "Found " << objects.size() << " elements" << endl;
  cerr << "Manager: " << manager << endl;
  cerr << "Num. frames: " << trackerFrames.size() << endl;
  cerr << "Num. relations: " << trackerRelations.size() << endl;
  return manager;
}
