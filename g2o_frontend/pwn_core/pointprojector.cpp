#include "pointprojector.h"
#include "g2o_frontend/basemath/bm_se3.h"

namespace pwn {

  PointProjector::PointProjector() {
    _transform.setIdentity();
    _minDistance = 0.01;
    _maxDistance = 6.0f;
    _imageRows = 0;
    _imageCols = 0;
  }

  PointProjector::~PointProjector() {}

  void PointProjector::project(IntImage &indexImage, 
			       DepthImage &depthImage, 
			       const PointVector &points) {
    assert(indexImage.rows > 0 && indexImage.cols > 0 && "PointProjector: Index image has zero dimensions");
    depthImage.create(indexImage.rows, indexImage.cols);
    depthImage.setTo(std::numeric_limits<float>::max());
    indexImage.setTo(cv::Scalar(-1));
    const Point *point = &points[0];
    for(size_t i = 0; i < points.size(); i++, point++) {
      int x, y;
      float d;
      if(!project(x, y, d, *point) ||
	  x < 0 || x >= indexImage.rows ||
	  y < 0 || y >= indexImage.cols)
	continue;
      float &otherDistance = depthImage(x, y);
      int &otherIndex = indexImage(x, y);
      if(otherDistance > d) {
	otherDistance = d;
	otherIndex = i;
      }
    }
  }

  void PointProjector::unProject(PointVector &points,
				 IntImage &indexImage,
				 const DepthImage &depthImage) const {
    assert(depthImage.rows > 0 && depthImage.cols > 0 && "PointProjector: Depth image has zero dimensions");
    points.resize(depthImage.rows * depthImage.cols);
    int count = 0;
    indexImage.create(depthImage.rows, depthImage.cols);
    Point *point = &points[0];
    for(int r = 0; r < depthImage.rows; r++){
      const float* f = &depthImage(r, 0);
      int *i = &indexImage(r, 0);
      for(int c = 0; c < depthImage.cols; c++, f++, i++) {
	if(!unProject(*point, r, c, *f)) {
	  *i = -1;
	  continue;
	}
	point++;
	*i = count;
	count++;
      }
    }

    points.resize(count);
  }

  void PointProjector::unProject(PointVector &points,
				 Gaussian3fVector &gaussians,
				 IntImage &indexImage,
				 const DepthImage &depthImage) const {
    assert(depthImage.rows > 0 && depthImage.cols > 0 && "PointProjector: Depth image has zero dimensions");
    points.resize(depthImage.rows * depthImage.cols);
    int count = 0;
    indexImage.create(depthImage.rows, depthImage.cols);
    Point *point = &points[0];
    for(int r = 0; r < depthImage.rows; r++) {
      const float *f = &depthImage(r, 0);
      int *i = &indexImage(r, 0);
      for(int c = 0; c < depthImage.cols; c++, f++, i++) {
	if(!unProject(*point, r, c, *f)) {
	  *i = -1;
	  continue;
	}
	point++;
	*i = count;
	count++;
      }
    }

    gaussians.resize(count);
    points.resize(count);
  }
  
  void PointProjector::projectIntervals(IntImage &intervalImage, 
					const DepthImage &depthImage, 
					const float worldRadius,
					const bool /*blackBorders*/) const {
    assert(depthImage.rows > 0 && depthImage.cols > 0 && "PointProjector: Depth image has zero dimensions");
    intervalImage.create(depthImage.rows, depthImage.cols);
    for(int r = 0; r < depthImage.rows; r++) {
      const float *f = &depthImage(r, 0);
      int *i = &intervalImage(r, 0);
      for(int c = 0; c < depthImage.cols; c++, f++, i++) {
	*i = projectInterval(r, c, *f, worldRadius);
      }
    }
  }
   
  inline bool PointProjector::project(int &, int &, float &, const Point &) const { return false; }
  
  inline bool PointProjector::unProject(Point &, const int, const int, const float) const { return false; }
  
  inline int PointProjector::projectInterval(const int, const int, const float, const float) const { return 0; }
  
 }
