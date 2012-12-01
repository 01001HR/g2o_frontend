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


#ifndef LASERROBOTDATA_H
#define LASERROBOTDATA_H


#include <iosfwd>
#include <string>

#include "g2o/core/hyper_graph.h"
#include "g2o_frontend/thesis/SensorData.h"
#include "g2o/types/slam3d/types_slam3d.h"
#include "g2o/types/slam2d/types_slam2d.h"
#include "SensorLaserRobot.h"

/**
* \brief laser measurement obtained by a robot
*
* A laser measurement obtained by a robot. The measurement is equipped with a pose of the robot at which
* the measurement was taken. The read/write function correspond to the CARMEN logfile format.
*/
class LaserRobotData: public SensorData {
public:
	
	typedef std::vector<Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d> > Point2DVector;
	
	LaserRobotData();
	virtual ~LaserRobotData();
	//! read the data from a stream
	virtual bool read(std::istream& is);
	//! write the data to a stream
	virtual bool write(std::ostream& os) const;	
	virtual void writeOut(const std::string&) {}
	//void update();
	//void release();
	inline int paramIndex() const {return _paramIndex;}	
	inline void setParamIndex(int pi) {_paramIndex = pi;}	
	
	virtual Sensor* getSensor() const { return _laserRobotSensor; }
	virtual void setSensor(Sensor* laserRobotSensor_);
	/**
	* computes a cartesian view of the beams (x,y).
	* @return a vector with the points of the scan in cartesian coordinates.
	*/
	Point2DVector cartesian() const;
	const std::vector<float>& ranges() const {return _ranges;}
	std::vector<float>& ranges() {return _ranges;}	
	const std::vector<float>& intensities() const {return _intensities;}
	std::vector<float>& intensities() {return _intensities;}
	
	float minRange() const { return _minRange; }
	void  setMinRange(float r) { _minRange = r; }

	float maxRange() const { return _maxRange; }
	void  setMaxRange(float r)  { _maxRange = r; }

	float fov() const { return _fov; }
	void setFov(float f) { _fov = f; }

	float firstBeamAngle() const { return _firstBeamAngle; }
	void setFirstBeamAngle(float fba)  { _firstBeamAngle = fba; }

protected:
  SensorLaserRobot* _laserRobotSensor;
	//! velocities and safety distances of the robot
	double _laserTv, _laserRv, _forwardSafetyDist, _sideSaftyDist, _turnAxis;
	int _paramIndex;
	std::vector<float> _ranges;
	std::vector<float> _intensities;
	float _firstBeamAngle;
	float _fov;
	float _minRange;
	float _maxRange;
	float _accuracy;
  
};	

#ifdef G2O_HAVE_OPENGL

class LaserRobotDataDrawAction : public g2o::DrawAction{
public:
  LaserRobotDataDrawAction() : DrawAction(typeid(LaserRobotData).name()) {};
  virtual HyperGraphElementAction* operator()(g2o::HyperGraph::HyperGraphElement* element, 
					      g2o::HyperGraphElementAction::Parameters* params_ );
protected:
  virtual bool refreshPropertyPtrs(g2o::HyperGraphElementAction::Parameters* params_);
  g2o::IntProperty* _beamsDownsampling;
  g2o::FloatProperty* _pointSize;
  g2o::FloatProperty* _maxRange;
};

#endif

#endif // LASERROBOTDATA_H
