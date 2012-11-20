/*
 * RGBDData.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: jacopo
 */

#include "RGBDData.h"
#include <fstream>
#include "g2o/stuff/macros.h"
#include "g2o/core/factory.h"

#ifdef WINDOWS
#include <windows.h>
#endif

#ifdef G2O_HAVE_OPENGL
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

using namespace g2o;
using namespace std;

RGBDData::RGBDData()
{
  _paramIndex = -1;
  _baseFilename = "none";
  _rgbdCameraSensor = 0;
  _ts_sec = 0;
  _ts_usec = 0;
  _intensityImage = 0;
  _depthImage = 0;
  _dataContainer = 0;
}

RGBDData::RGBDData(cv::Mat* intensityImage_, cv::Mat* depthImage_) {
	_paramIndex = -1;
  _baseFilename = "none";
  _rgbdCameraSensor = 0;
  _ts_sec = 0;
  _ts_usec = 0;
  _intensityImage = intensityImage_;
  _depthImage = depthImage_;
  _dataContainer = 0;
}

RGBDData::RGBDData(Sensor* sensor_, cv::Mat* intensityImage_, cv::Mat* depthImage_)
{
	_paramIndex = -1;
  _baseFilename = "none";
  _rgbdCameraSensor = (SensorRGBDCamera*)sensor_;
  _ts_sec = 0;
  _ts_usec = 0;
  _intensityImage = intensityImage_;
  _depthImage = depthImage_;
  _dataContainer = 0;
}

RGBDData::~RGBDData(){
	if(! _intensityImage)
		delete _intensityImage;
	if(! _depthImage)		
		delete _depthImage;
}

//! read the data from a stream
bool RGBDData::read(std::istream& is) 
{
  is >> _paramIndex >> _baseFilename;
  is >> _ts_sec >> _ts_usec;
  _intensityImage = 0;
  _depthImage = 0;
  update();
  return true;
}

//! write the data to a stream
bool RGBDData::write(std::ostream& os) const 
{
  if (_rgbdCameraSensor)
    os << _rgbdCameraSensor->getParameter()->id();
  else
    os << -1;
  os << " " <<  _baseFilename << " ";
  os << _ts_sec << " " << _ts_usec;
  return true;
}

void RGBDData::writeOut()
{
	int num = _rgbdCameraSensor->getNum();
	_rgbdCameraSensor->setNum(num+1);
	
	char name[8];	
	sprintf(name, "%05d", num);
	_baseFilename = string(name);
	//cout << _baseFilename << endl;
	
	char buf[25];
	sprintf(buf, "%05d_intensity.pgm", num);
	cv::imwrite(buf, *_intensityImage);
	sprintf(buf, "%05d_depth.pgm", num);
	cv::imwrite(buf, *_depthImage);
	cout << "Saved frame #" << num << endl;
}

void RGBDData::update()
{
  if (!_intensityImage) 
  {
    _intensityImage = new cv::Mat();
    _depthImage = new cv::Mat();
    *_intensityImage = cv::imread((_baseFilename + "_intensity.pgm") .c_str(), -1);
    *_depthImage = cv::imread((_baseFilename + "_depth.pgm") .c_str(), -1);
  }
}

void RGBDData::setSensor(Sensor* rgbdCameraSensor_)
{
  _rgbdCameraSensor = dynamic_cast<SensorRGBDCamera*>(rgbdCameraSensor_) ;
}

void RGBDData::release()
{
  if (_intensityImage) 
  {
    delete _intensityImage;
    _intensityImage = 0;
  }
  if (_depthImage) 
  {
    delete _depthImage;
    _depthImage = 0;
  }
}

bool RGBDDataDrawAction::refreshPropertyPtrs(HyperGraphElementAction::Parameters* params_)
{
  if (!DrawAction::refreshPropertyPtrs(params_))
    return false;
  if (_previousParams)
  {
    _beamsDownsampling = _previousParams->makeProperty<IntProperty>(_typeName + "::BEAMS_DOWNSAMPLING", 20);
    _pointSize = _previousParams->makeProperty<FloatProperty>(_typeName + "::POINT_SIZE", .05f);
  } 
  else 
  {
    _beamsDownsampling = 0;
    _pointSize = 0;
  }
  return true;
}

HyperGraphElementAction* RGBDDataDrawAction::operator()(HyperGraph::HyperGraphElement* element, 
																HyperGraphElementAction::Parameters* params_)
{
  if(typeid(*element).name()!=_typeName)
    return 0;
  
  refreshPropertyPtrs(params_);
  if (!_previousParams)
  {
    return this;
  }

  if (_show && !_show->value())
    return this;

  glPushMatrix();
  int step = 1;
  if(_beamsDownsampling )
    step = _beamsDownsampling->value();
  if(_pointSize)
    glPointSize(_pointSize->value());
  else 
    glPointSize(1);
  
  RGBDData* that = static_cast<RGBDData*>(element);
  unsigned short* dptr = reinterpret_cast<unsigned short*>(that->_depthImage->data);
	
  glBegin(GL_POINTS);
  glColor4f(1.f, 0.f, 0.f, 0.5f);
  
  
  g2o::HyperGraph::DataContainer* container = that->dataContainer();
  cout << "datacontainer" << container << endl;
  g2o::OptimizableGraph::Vertex* v= dynamic_cast<g2o::OptimizableGraph::Vertex*>(container);
  if (0 && !v) {
    cout << "die" << v << endl;
  }
  OptimizableGraph* g=v->graph();
  if (0 && !g) {
    cout << " now" << g << endl;
  }
 
 g2o::Parameter* p =g->parameters().getParameter(that->paramIndex());
  if (0 && !p) {
    cout << "with" << p << " " << that->paramIndex() << endl;
  }
  g2o::ParameterCamera* param = dynamic_cast<g2o::ParameterCamera*> (p);
  if (0 &&!param) {
    cout << " extreme" << endl;
  }
  cout << "param is";
  param->write(cout);
  cout << endl;
  Eigen::Matrix3d K = param->Kcam();
  
  static const double fx = K(0, 0);
	static const double fy = K(1, 1);
	static const double center_x = K(0, 2);
	static const double center_y = K(1, 2);
	double unit_scaling = 0.001f;
  float constant_x = unit_scaling / fx;
  float constant_y = unit_scaling / fy;
  
  int k = 0;
  cout << "the image is: " <<that->_depthImage->rows << " " << that->_depthImage->cols << endl;
  for(int i = 0; i < that->_depthImage->rows; i++)  {
    for(int j = 0; j < that->_depthImage->cols; j ++) {
    	unsigned short d = *dptr;
    	if(d != 0)
      {
      	// Computing the Cartesian coordinates of the current pixel
	float x = (j - center_x) * d * constant_x;
      	float y = (i - center_y) * d * constant_y;
      	float z = ((float)d) * unit_scaling;
      	if (k<5) {
	  cout << x << " " << y << " " << z << endl;
	}
	k++;

	glNormal3f(-x, -y, -z);
	glVertex3f(x, y, z);
    	}
    	dptr ++;
    } 
	}
	
  glEnd();
  glPopMatrix();
  return this;
}

G2O_REGISTER_TYPE(RGBD_DATA, RGBDData);
G2O_REGISTER_ACTION(RGBDDataDrawAction);
