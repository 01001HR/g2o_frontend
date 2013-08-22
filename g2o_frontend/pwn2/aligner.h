#ifndef _PWN_ALINGER_H_
#define _PWN_ALINGER_H_

#include "linearizer.h"
#include "pointprojector.h"
#include "frame.h"
#include "correspondencefinder.h"
#include "se3_prior.h"

namespace pwn {

class Aligner {
 public:
  Aligner();

  inline void setProjector(PointProjector *projector_) { _projector = projector_; }
  inline void setReferenceFrame(Frame *referenceFrame_) { _referenceFrame = referenceFrame_; clearPriors();}
  inline void setCurrentFrame(Frame*currentFrame_) { _currentFrame = currentFrame_; clearPriors();}
  inline void setOuterIterations(const int outerIterations_) { _outerIterations = outerIterations_; }
  inline void setInnerIterations(const int innerIterations_) { _innerIterations = innerIterations_; }
  inline void setT(const Eigen::Isometry3f T_) { _T = T_; }
  inline void setInitialGuess(const Eigen::Isometry3f initialGuess_) { _initialGuess = initialGuess_; }
  inline void setSensorOffset(const Eigen::Isometry3f sensorOffset_) { 
    _referenceSensorOffset = sensorOffset_;
    _currentSensorOffset = sensorOffset_;
  }
  inline void setReferenceSensorOffset(const Eigen::Isometry3f referenceSensorOffset_) { _referenceSensorOffset = referenceSensorOffset_; }
  inline void setCurrentSensorOffset(const Eigen::Isometry3f currentSensorOffset_) { _currentSensorOffset = currentSensorOffset_; }

  inline const PointProjector* projector() const { return _projector; }
  inline Linearizer* linearizer() { return _linearizer; }
  inline void setLinearizer(Linearizer* linearizer_) { _linearizer = linearizer_; _linearizer->setAligner(this); }
  inline CorrespondenceFinder* correspondenceFinder() { return _correspondenceFinder; }
  inline void setCorrespondenceFinder(CorrespondenceFinder* correspondenceFinder_) { _correspondenceFinder = correspondenceFinder_; }
  inline const Frame* referenceFrame() const { return _referenceFrame; }
  inline const Frame* currentFrame() const { return _currentFrame; }
  inline int outerIterations() const { return _outerIterations; }
  inline int innerIterations() const { return _innerIterations; }
  inline const Eigen::Isometry3f& T() const { return _T; }
  inline const Eigen::Isometry3f& initialGuess() const { return _initialGuess; }
  inline const Eigen::Isometry3f& sensorOffset() const { return _referenceSensorOffset; }
  inline const Eigen::Isometry3f& referenceSensorOffset() const { return _referenceSensorOffset; }
  inline const Eigen::Isometry3f& currentSensorOffset() const { return _currentSensorOffset; }

  virtual void align();

  inline float error() const {return _error;}
  inline int inliers() const {return _inliers; }
  inline double totalTime() const {return _totalTime; }
  
  void addRelativePrior(const Eigen::Isometry3f& mean, const Matrix6f& informationMatrix);
  void addAbsolutePrior(const Eigen::Isometry3f& referenceTransform, const Eigen::Isometry3f& mean, const Matrix6f& informationMatrix);
  void clearPriors();

 protected:
  PointProjector *_projector;
  Linearizer *_linearizer;

  CorrespondenceFinder *_correspondenceFinder;

  Frame *_referenceFrame;
  Frame *_currentFrame;
  
  int _outerIterations, _innerIterations;

  Eigen::Isometry3f _T;
  Eigen::Isometry3f _initialGuess;
  //Eigen::Isometry3f _sensorOffset;
  Eigen::Isometry3f _referenceSensorOffset;
  Eigen::Isometry3f _currentSensorOffset;

  int _inliers;
  double _totalTime;
  float _error;

  std::vector<SE3Prior*> _priors;
};

}

#endif
