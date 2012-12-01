/*
  <one line to give the program's name and a brief idea of what it does.>
  Copyright (C) 2012  <copyright holder> <email>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "LaserRobotData.h"
#include "g2o/stuff/macros.h"
#include "g2o/core/factory.h"
#include <Eigen/Dense>
#include <fstream>
#include <iomanip>

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

LaserRobotData::LaserRobotData(): 
  _laserTv(0.), _laserRv(0.), _forwardSafetyDist(0.), _sideSaftyDist(0.), _turnAxis(0.)
{
  _paramIndex = -1;
  _laserRobotSensor = 0;
  _firstBeamAngle = -M_PI/2;
  _fov = M_PI;
  _minRange = 0;
  _maxRange = 30;
  _accuracy = 0.1;
	
}

LaserRobotData::~LaserRobotData()
{
}

bool LaserRobotData::read(istream& is){
  float res, remissionMode;
	
  is >> _paramIndex >> _firstBeamAngle >> _fov >> res >> _maxRange >> _accuracy >> remissionMode;

  int beams;
  is >> beams;
  _ranges.resize(beams);
  for (int i=0; i<beams; i++)
    is >> _ranges[i];

  is >> beams;
  _intensities.resize(beams);
  for (int i = 0; i < beams; i++)
    is >> _intensities[i];

  // special robot laser stuff
  double x,y,theta;
  //odom pose of the robot: no need
  is >> x >> y >> theta;
  //laser pose wrt to the world: no need
  is >> x >> y >> theta;
  is >> _laserTv >>  _laserRv >>  _forwardSafetyDist >> _sideSaftyDist >> _turnAxis;

  // timestamp + hostname
  string hostname;
  double ts;
  is >> ts;
  is >> hostname;
  setTimeStamp(ts);
  is >> ts;
  return true;
}

Eigen::Vector3d to2D(const Eigen::Isometry3d& iso) {
	
  Eigen::Vector3d rv;
  rv[0] = iso.translation().x();
  rv[1] = iso.translation().y();
  Eigen::AngleAxisd aa(iso.linear());
  rv[2] = aa.angle();	
  return rv;
}

bool LaserRobotData::write(ostream& os) const {

  const HyperGraph::DataContainer* container = dataContainer();
  const VertexSE3* v = dynamic_cast<const VertexSE3*> (container);
  if (! v) {
    cerr << "dataContainer = " << container << endl;
    cerr << "dynamic cast failed" << endl;
    return false;
  }
		
  const OptimizableGraph* g = v->graph();
  if (! g) {
    cerr << "no graph" << endl;
    return false;
  }
		
  const Parameter* p = g->parameters().getParameter(_paramIndex);
  if (! p) {
    cerr << "no param" << endl;
    return false;
  }

		
  const ParameterSE3Offset* oparam = dynamic_cast<const g2o::ParameterSE3Offset*> (p);

  if (! oparam) {
    cerr << "no good param" << endl;
    return false;
  }
		
  Eigen::Isometry3d offset = oparam->offset();
		
  float angularStep = _fov / _ranges.size();
  int remissionMode = 0;
  os << _paramIndex << " " << _firstBeamAngle << " " << _fov << " " << angularStep << " " << _maxRange << " " << _accuracy << " "
     << remissionMode << " ";
  os << _ranges.size();
  for (size_t i = 0; i < _ranges.size(); ++i) 
    os << " " << _ranges[i];
  os << " " << _intensities.size();
  for (size_t i = 0; i < _intensities.size(); ++i) 
    os << " " << _intensities[i];

  // odometry pose
  Eigen::Vector3d pose = to2D(v->estimate());
  os << " " << pose.x() << " " << pose.y() << " " << pose.z();
  // laser pose wrt the world
  pose =  to2D(v->estimate()*offset);
  os << " " << pose.x() << " " << pose.y() << " " << pose.z();

  // crap values
  os << FIXED(" " <<  _laserTv << " " <<  _laserRv << " " << _forwardSafetyDist << " "
	      << _sideSaftyDist << " " << _turnAxis);
  //the second timestamp is the logged timestamp
  string hn = "hostname";
  os << FIXED(" " << _timeStamp << " " << hn << " " << _timeStamp);

  return os.good();
}


void LaserRobotData::setSensor(Sensor* laserRobotsensor_)
{
  _laserRobotSensor = dynamic_cast<SensorLaserRobot*>(laserRobotsensor_);
}


LaserRobotData::Point2DVector LaserRobotData::cartesian() const
{
  Point2DVector points;
  points.reserve(_ranges.size());
  float angularStep = _fov / _ranges.size();
  float alpha=_firstBeamAngle;
  for (size_t i = 0; i < _ranges.size(); ++i) {
    const float& r = _ranges[i];
    if (r < _maxRange) {
      points.push_back(Eigen::Vector2d(cos(alpha) * r, sin(alpha) * r));
    }
    alpha+=angularStep;
  }
  return points;
}



bool LaserRobotDataDrawAction::refreshPropertyPtrs(g2o::HyperGraphElementAction::Parameters* params_)
{
  if (!DrawAction::refreshPropertyPtrs(params_))
    return false;
  if (_previousParams)
    {
      _beamsDownsampling = _previousParams->makeProperty<IntProperty>(_typeName + "::BEAMS_DOWNSAMPLING", 1);
      _pointSize = _previousParams->makeProperty<FloatProperty>(_typeName + "::POINT_SIZE", 1.0f);
      _maxRange = _previousParams->makeProperty<FloatProperty>(_typeName + "::MAX_RANGE", -1.);
    } 
  else 
    {
      _beamsDownsampling = 0;
      _pointSize = 0;
      _maxRange = 0;
    }
  return true;
}


HyperGraphElementAction* LaserRobotDataDrawAction::operator()(HyperGraph::HyperGraphElement* element, 
							      HyperGraphElementAction::Parameters* params_)
{
  if (typeid(*element).name()!=_typeName)
    return 0;

  refreshPropertyPtrs(params_);
  if (! _previousParams){
    return this;
  }
  if (_show && !_show->value())
    return this;
  LaserRobotData* that = static_cast<LaserRobotData*>(element);

  LaserRobotData::Point2DVector points = that->cartesian();

  int k=0;
  if (_maxRange && _maxRange->value() >= 0 ) {
    float r2=_maxRange->value();
    r2 *= r2;
    for (size_t i=0; i<points.size(); i++){
      if (points[i].squaredNorm()<r2)
	points[k++]=points[i];
    }
    points.resize(k);
  }
  
  const HyperGraph::DataContainer* container = that->dataContainer();
  const VertexSE3* v = dynamic_cast<const VertexSE3*> (container);
  if (! v) return false;

  const OptimizableGraph* g = v->graph();
  const Parameter* p = g->parameters().getParameter(that->paramIndex());
  const ParameterSE3Offset* oparam = dynamic_cast<const ParameterSE3Offset*> (p);
		
  const Eigen::Isometry3d& offset = oparam->offset();
  glPushMatrix();
  glMultMatrixd(offset.data());
  glColor4f(1.f,0.f,0.f,0.5f);
  int step = 1;
  if (_beamsDownsampling )
    step = _beamsDownsampling->value();
  if (_pointSize) {
    glPointSize(_pointSize->value());
  }
  glBegin(GL_POINTS);
  for (size_t i=0; i<points.size(); i+=step){
    glVertex3f((float)points[i].x(), (float)points[i].y(), 0.f);
  }
  glEnd();
  glPopMatrix();
  return this;
  return 0;
}


G2O_REGISTER_TYPE(LASER_ROBOT_DATA, LaserRobotData);
G2O_REGISTER_ACTION(LaserRobotDataDrawAction);
