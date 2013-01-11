/*
 * viewerGUI.h
 *
 *  Created on: Dic 05, 2012
 *      Author: Martina
 */

#include "leQGLViewer.h"

leQGLViewer::leQGLViewer(QWidget *parent): QGLViewer(parent), data(NULL), lineFound(false), lineContainer(NULL)
{
	setAxisIsDrawn(false);
} 

void leQGLViewer::init()
{
	QGLViewer::init();
//	Light disabled
// 	glDisable(GL_LIGHTING);
// 	glDisable(GL_LIGHT0);
// 	glDisable(GL_LIGHT1);

	setBackgroundColor(QColor::fromRgb(51, 51, 51));

  // some default settings i like
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND); 
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  //glEnable(GL_CULL_FACE);
  glShadeModel(GL_FLAT);
  //glShadeModel(GL_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	setAxisIsDrawn(false);

  // don't save state
  setStateFileName(QString::null);

  // mouse bindings
  setMouseBinding(Qt::RightButton, CAMERA, ZOOM);
  setMouseBinding(Qt::MiddleButton, CAMERA, TRANSLATE);

  // keyboard shortcuts
  setShortcut(CAMERA_MODE, 0);
  setShortcut(EXIT_VIEWER, 0);
  //setShortcut(SAVE_SCREENSHOT, 0);

  // replace camera
  qglviewer::Camera* oldcam = camera();
  qglviewer::Camera* cam = new StandardCamera();
  setCamera(cam);
  cam->setPosition(qglviewer::Vec(0., 0., 75.));
  cam->setUpVector(qglviewer::Vec(0., 1., 0.));
  cam->lookAt(qglviewer::Vec(0., 0., 0.));
  delete oldcam;
}

void leQGLViewer::draw()
{	
	int beamsDownsampling = 1;

	drawAxis();
	glPointSize(4.f);
	// draw all the points
	if(!lineFound) 
	{
		for (int i=0;i<(int)data->size();i+=beamsDownsampling)
		{
			//for depth image points
			//if((*dataptr)[i][2]!=0)
				//glVertex3f((*dataptr)[i][0]/1000, (*dataptr)[i][1]/1000, (*dataptr)[i][2]/1000);
			glBegin(GL_POINTS);
			glColor4f(1.f,0.f,0.f,0.5f);
			glVertex3f((*data)[i].x(), (*data)[i].y(), 0.f);
			glEnd();
		}
	}
	//draw the point belonging to the lines found
	else 
	{
		glBegin(GL_LINES);
		
		//cout << "line found!" << endl;
		for(int i  = 0; i < lineContainer->size(); ++i)
		{
			glPointSize(4.f);
			glColor3f((*lineContainer)[i].rgbColors[0], 
								(*lineContainer)[i].rgbColors[1], 
								(*lineContainer)[i].rgbColors[2]);
			//cout << "lineVertices size is: " << (*lineContainer)[i].lineVertices.size() << endl;
			//for cicle useless, I know that lineVertices.size() is 2..TODO
			for(int j = 0; j<(*lineContainer)[i].lineVertices.size(); ++j)
			{
				//there are only 2 vertices, the start and the end of the line, j is useless TODO
				glVertex3f((*lineContainer)[i].lineVertices[0].x(), 
									 (*lineContainer)[i].lineVertices[0].y(), 
									 0.f);
				glVertex3f((*lineContainer)[i].lineVertices[1].x(), 
									 (*lineContainer)[i].lineVertices[1].y(), 
									 0.f);
			}
		}
		glEnd();
	}
	glColor3f(1,1,1);
}

void leQGLViewer::setDataPointer(LaserRobotData::Point2DVector* p)
{
	data = p;
}
