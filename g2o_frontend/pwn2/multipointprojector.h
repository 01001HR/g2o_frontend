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
    Eigen::MatrixXf depthImage;
    Eigen::MatrixXi indexImage;

    ChildProjectorInfo(PointProjector *pointProjector_,
		       Eigen::Isometry3f sensorOffset_,
		       int width_, int height_) {
      pointProjector = pointProjector_;
      sensorOffset = sensorOffset_;
      width = width_;
      height = height_;
      if(indexImage.rows() != width || indexImage.cols() != height)
	indexImage.resize(width, height);    
    }
  };  

 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  MultiPointProjector() : PointProjector() {}

  virtual ~MultiPointProjector() {}

  void addPointProjector(PointProjector *pointProjector_, 
			 Eigen::Isometry3f sensorOffset_,
			 int width_, int height_) { 
    _pointProjectors.push_back(ChildProjectorInfo(pointProjector_, sensorOffset_, width_, height_)); 
  }
  
  void setPointProjector(PointProjector *pointProjector_, 
			 Eigen::Isometry3f sensorOffset_,
			 int width_, int height_,
			 int position) { 
    _pointProjectors[position].pointProjector = pointProjector_;
    _pointProjectors[position].sensorOffset = sensorOffset_;
    _pointProjectors[position].width = width_;
    _pointProjectors[position].height = height_;
  }

  virtual void size(int &rows, int &cols);

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
				const float worldRadius,
				const bool blackBorders=false) const;

  virtual void setTransform(const Eigen::Isometry3f &transform_);

 protected:
  mutable std::vector<ChildProjectorInfo> _pointProjectors;
};

}

#endif // _MULTIPOINTPROJECTOR_H_
