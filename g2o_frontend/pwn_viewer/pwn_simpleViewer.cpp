#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QListWidget>


#include "pwn_qglviewer.h"
#include "drawable_points.h"
#include "gl_parameter_points.h"
#include "drawable_normals.h"
#include "gl_parameter_normals.h"

#include "g2o/stuff/command_args.h"

using namespace Eigen;
using namespace g2o;
using namespace std;

int main (int argc, char** argv) {
  float pointSize;
  float pointStep;
  float normalLenght;
  float normalStep;
  float alpha;
  const int maxFiles = 1000;
  std::vector<string> filenames(maxFiles);

  
  g2o::CommandArgs arg;
  arg.param("pointSize",pointSize,1,"size of the points") ;
  arg.param("normalLenght",normalLenght,0,"lenght of the normals") ;
  arg.param("alpha",alpha,0.5,"alpha channel for points") ;
  arg.param("pointStep",pointStep,1,"step of the points") ;
  arg.param("normalStep",normalStep,10,"step of the normals") ;
  arg.paramLeftOver("filenames", filenames[0], "", "filenames", true);
  for (int i=1; i<maxFiles; i++){
    arg.paramLeftOver("", filenames[i], "", "", true);
  }
  arg.parseArgs(argc, argv);


  
  QApplication application(argc,argv);
  QWidget* mainWindow = new QWidget();
  mainWindow->setWindowTitle("pwn_simpleViewer");
  QHBoxLayout* hlayout = new QHBoxLayout();
  mainWindow->setLayout(hlayout);
  QVBoxLayout* vlayout = new QVBoxLayout();
  hlayout->addItem(vlayout);
  QVBoxLayout* vlayout2 = new QVBoxLayout();
  hlayout->addItem(vlayout2);
  hlayout->setStretch(1,1);

  QListWidget* listWidget = new QListWidget(mainWindow);
  vlayout->addWidget(listWidget);
  PWNQGLViewer* viewer = new PWNQGLViewer(mainWindow);
  vlayout2->addWidget(viewer);
  Eigen::Isometry3f T;
  T.setIdentity();

  for (int i=0; i<maxFiles; i++){
    if (filenames[i]=="")
      break;
    float r=0.5+0.6*drand48();
    float g=0.5+0.6*drand48();
    float b=0.5+0.6*drand48();

    PointWithNormalVector* points = new PointWithNormalVector;
    
    if (! points->load(filenames[i].c_str())){
      cerr << "unable to load points from file [" << filenames[i] << "]" << endl;
      return 0;
    } else {
      listWidget->addItem(QString(filenames[i].c_str()));
    }
    GLParameterPoints* pointsParams = new GLParameterPoints(pointSize,Eigen::Vector4f(r,g,b,alpha));
    DrawablePoints* drawablePoints = new DrawablePoints(T, pointsParams, points);
    pointsParams->setStep(normalStep);
    
    viewer->addDrawable(drawablePoints);
    
    GLParameterNormals* normalParams = new GLParameterNormals(0.1, Eigen::Vector4f(0.1,0.1,0.5,0.2), normalLenght);
    DrawableNormals* drawableNormals = new DrawableNormals(T, normalParams, points);
    normalParams->setStep(normalStep);
    normalParams->setNormalLength(normalLenght);
  
    viewer->addDrawable(drawableNormals);
  }

  viewer->init();
  mainWindow->show();
  viewer->show();
  listWidget->show();
  while (1) {
    /*    
      for (int i=0; i<listWidget....; i++){
      if (the widget is selected){
	unselect it;
	set the corresponding points to 0;
      } else {
	select itl
	  set the co
      }
    }
    */
    application.processEvents();
  }
}
