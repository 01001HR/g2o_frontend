#ifndef _ALINGER_H_
#define _ALINGER_H_
#include "linearizer.h"
#include "pointprojector.h"
#include "homogeneouspoint3fscene.h"
#include "correspondencegenerator.h"
#include "se3_prior.h"

class Aligner {
 public:
  Aligner();

  inline void setProjector(PointProjector *projector_) { _projector = projector_; }
  inline void setReferenceScene(HomogeneousPoint3fScene *referenceScene_) { _referenceScene = referenceScene_; clearPriors();}
  inline void setCurrentScene(HomogeneousPoint3fScene *currentScene_) { _currentScene = currentScene_; clearPriors();}
  inline void setOuterIterations(const int outerIterations_) { _outerIterations = outerIterations_; }
  inline void setInnerIterations(const int innerIterations_) { _innerIterations = innerIterations_; }
  inline void setT(const Eigen::Isometry3f T_) { _T = T_; }
  inline void setInitialGuess(const Eigen::Isometry3f initialGuess_) { _initialGuess = initialGuess_; }
  inline void setSensorOffset(const Eigen::Isometry3f sensorOffset_) { _sensorOffset = sensorOffset_; }

  inline const PointProjector* projector() const { return _projector; }
  inline Linearizer* linearizer() { return _linearizer; }
  inline void setLinearizer(Linearizer* linearizer_) { _linearizer = linearizer_; _linearizer->setAligner(this); }
  inline CorrespondenceGenerator* correspondenceGenerator() { return _correspondenceGenerator; } 
  inline void setCorrespondenceGenerator(CorrespondenceGenerator* correspondenceGenerator_) { _correspondenceGenerator = correspondenceGenerator_; } 
  inline const HomogeneousPoint3fScene* referenceScene() const { return _referenceScene; }
  inline const HomogeneousPoint3fScene* currentScene() const { return _currentScene; }  
  inline int outerIterations() const { return _outerIterations; }
  inline int innerIterations() const { return _innerIterations; }
  inline const Eigen::Isometry3f& T() const { return _T; }
  inline const Eigen::Isometry3f& initialGuess() const { return _initialGuess; }
  inline const Eigen::Isometry3f& sensorOffset() const { return _sensorOffset; }

  virtual void align();
  inline float error() const {return _error;}
  inline int inliers() const {return _inliers; }
  inline double totalTime() const {return _totalTime; }
  
  void addPrior(const Eigen::Isometry3f& mean, const Matrix6f& informationMatrix);
  void clearPriors();

 protected:
  PointProjector *_projector;
  Linearizer *_linearizer;

  CorrespondenceGenerator *_correspondenceGenerator;

  HomogeneousPoint3fScene *_referenceScene;
  HomogeneousPoint3fScene *_currentScene;
  
  int _outerIterations, _innerIterations;

  Eigen::Isometry3f _T;
  Eigen::Isometry3f _initialGuess;
  Eigen::Isometry3f _sensorOffset;
  int _inliers;
  double _totalTime;
  float _error;
  std::vector<SE3Prior, Eigen::aligned_allocator<SE3Prior> > _priors;
};

#endif
