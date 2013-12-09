#include "merger.h"

namespace pwn {

  Merger::Merger() {
    _distanceThreshold = 0.1f;
    _normalThreshold = cosf(10 * M_PI / 180.0f);
    _maxPointDepth = 10.0f;
    _depthImageConverter = 0;
    _indexImage.create(0, 0);
    _depthImage.create(0, 0);
    _collapsedIndices.resize(0);
  }

  void Merger::merge(Frame *frame, Eigen::Isometry3f transform) {
    assert(_indexImage.rows > 0 && _indexImage.cols > 0 && "Merger: _indexImage has zero size");  
    assert(_depthImageConverter  && "Merger: missing _depthImageConverter");  
    assert(_depthImageConverter->projector()  && "Merger: missing projector in _depthImageConverter");  

    PointProjector *pointProjector = _depthImageConverter->projector();
    pointProjector->setTransform(transform);
    pointProjector->project(_indexImage, 
			    _depthImage, 
			    frame->points());
  
    // Scan all the points, 
    // if they fall in a cell not with -1, 
    //   skip
    // if they fall in a cell with n>1, 
    //   if distance is incompatible,
    //      skip
    // if normals are incompatible
    //      skip
    // accumulate the point in the cell i
    // set the target accumulator to i;
    int target = 0;
    int distance = 0;
    _collapsedIndices.resize(frame->points().size());
    std::fill(_collapsedIndices.begin(), _collapsedIndices.end(), -1);
    
    int killed = 0;
    int currentIndex = 0;
    for(size_t i = 0; i < frame->points().size(); currentIndex++ ,i++) {
      const Point currentPoint = frame->points()[i];
      const Normal currentNormal = frame->normals()[i];
    
      int r = -1, c = -1;
      float depth = 0.0f;
      pointProjector->project(r, c, depth, currentPoint);
      if(depth < 0 || depth > _maxPointDepth || 
	 r < 0 || r >= _depthImage.rows || 
	 c < 0 || c >= _depthImage.cols) {
	distance++;
	continue;
      }
        
      float &targetZ = _depthImage(r, c);
      int targetIndex = _indexImage(r, c);
      if(targetIndex < 0) {
	target++;
	continue;
      }
      const Normal &targetNormal = frame->normals().at(targetIndex);

      if(targetIndex == currentIndex) {
	_collapsedIndices[currentIndex] = currentIndex;
      } 
      else if(fabs(depth - targetZ) < _distanceThreshold && currentNormal.dot(targetNormal) > _normalThreshold) {
	Gaussian3f &targetGaussian = frame->gaussians()[targetIndex];
	Gaussian3f &currentGaussian = frame->gaussians()[currentIndex];
	targetGaussian.addInformation(currentGaussian);
	_collapsedIndices[currentIndex] = targetIndex;
	killed++;
      }
    }

    // Scan the vector of covariances.
    // if the index is -1
    //    copy into k
    //    increment k 
    // if the index is the same,
    //    update the point with normal
    //    copy into k
    //    increment k
    int murdered = 0;
    int k = 0;  
    for(size_t i = 0; i < _collapsedIndices.size(); i++) {
      int collapsedIndex = _collapsedIndices[i];
      if(collapsedIndex == (int)i) {
	frame->points()[i].head<3>() = frame->gaussians()[i].mean();
      }
      if(collapsedIndex < 0 || collapsedIndex == (int)i) {
	frame->points()[k] = frame->points()[i];
	frame->normals()[k] = frame->normals()[i];
	frame->stats()[k] = frame->stats()[i];
	frame->pointInformationMatrix()[k] = frame->pointInformationMatrix()[i];
	frame->normalInformationMatrix()[k] = frame->normalInformationMatrix()[i];
	frame->gaussians()[k] = frame->gaussians()[i];
	k++;
      } 
      else {
	murdered ++;
      }
    }    
    int originalSize = frame->points().size();
    
    // Kill the leftover points
    frame->points().resize(k);
    frame->normals().resize(k);
    frame->stats().resize(k);
    frame->pointInformationMatrix().resize(k);
    frame->normalInformationMatrix().resize(k);
    std::cerr << "Number of suppressed points: " << murdered  << std::endl;
    std::cerr << "Resized cloud from: " << originalSize << " to " << k << " points" <<std::endl;
    
    // Recompute the normals
    // pointProjector->project(_indexImage, _depthImage, frame->points());
    // _depthImageConverter->compute(*frame, _depthImage, transform);
  }

}
