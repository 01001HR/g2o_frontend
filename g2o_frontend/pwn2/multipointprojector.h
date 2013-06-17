#ifndef _MULTIPOINTPROJECTOR_H_
#define _MULTIPOINTPROJECTOR_H_

#include "pointprojector.h"

namespace pwn {

class MultiPointProjector: public PointProjector {
  struct ChildProjectorInfo {
    PointProjector *pointProjector;
    Eigen::Isometry3f sensorOffset;
    int width;
    int height;
    
    ChildProjectorInfo() {
      pointProjector = 0;
      sensorOffset = Eigen::Isometry3f::Identity();
      width = 0;
      height = 0;
    }
    
    ChildProjectorInfo(PointProjector *pointProjector_, Eigen::Isometry3f sensorOffset_,
		       int width_, int height_) {
      pointProjector = pointProjector_;
      sensorOffset = sensorOffset_;
      width = width_;
      height = height_;
    }
  };  

 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  MultiPointProjector() : PointProjector() {}

  virtual ~MultiPointProjector() {}

  void addPointProjector(PointProjector *pointProjector_, Eigen::Isometry3f sensorOffset_,
			 int width_, int height_) { 
    _pointProjectors.push_back(ChildProjectorInfo(pointProjector_, sensorOffset_, width_, height_)); 
  }
  
  virtual void project(Eigen::MatrixXi &indexImage, 
		       Eigen::MatrixXf &depthImage, 
		       const PointVector &points) const;

  virtual void unProject(PointVector &points,
			 Eigen::MatrixXi &indexImage, 
                         const Eigen::MatrixXf &depthImage) const;

  virtual void unProject(PointVector &points,
  			 Gaussian3fVector &gaussians,
  			 Eigen::MatrixXi &indexImage,
                         const Eigen::MatrixXf &depthImage) const;
  
  virtual void projectIntervals(Eigen::MatrixXi& intervalImage, 
				const Eigen::MatrixXf& depthImage, 
				const float worldRadius) const;

 protected:
  std::vector<ChildProjectorInfo> _pointProjectors;
};

}

#endif // _MULTIPOINTPROJECTOR_H_