#ifndef _OMEGAGENERATOR_H_
#define _OMEGAGENERATOR_H_

#include "homogeneouspoint3fstats.h"
#include "homogeneouspoint3fomega.h"

typedef Eigen::DiagonalMatrix<float, 4> Diagonal4f;

class OmegaGenerator {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  
  OmegaGenerator() {
    _flatOmega.setZero();
    _nonFlatOmega.setZero();
    _flatOmega.diagonal() = HomogeneousNormal3f(Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    _nonFlatOmega.diagonal() = HomogeneousNormal3f(Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    _curvatureThreshold = 1.0f;
  }

  inline HomogeneousPoint3fOmega flatOmega() { return _flatOmega; }
  inline void setFlatOmega(HomogeneousPoint3fOmega flatOmega_) { _flatOmega = flatOmega_; }
  inline HomogeneousPoint3fOmega nonFlatOmega() { return _nonFlatOmega; }
  inline void setNonFlatOmega(HomogeneousPoint3fOmega nonFlatOmega_) { _nonFlatOmega = nonFlatOmega_; }
  inline float curvatureThreshold() { return _curvatureThreshold; }
  inline void setCurvatureThreshold(float curvatureThreshold_) { _curvatureThreshold = curvatureThreshold_; }

  virtual void compute(HomogeneousPoint3fOmegaVector& omegas, 
		       HomogeneousPoint3fStatsVector& stats,
		       HomogeneousNormal3fVector& imageNormals) = 0;
  
 protected:
  HomogeneousPoint3fOmega _flatOmega;
  HomogeneousPoint3fOmega _nonFlatOmega;
  float _curvatureThreshold;
};

class PointOmegaGenerator : OmegaGenerator {
 public:
  PointOmegaGenerator() {
    _flatOmega.diagonal() = HomogeneousNormal3f(Eigen::Vector3f(1000.0f, 1.0f, 1.0f));
    _nonFlatOmega.diagonal() = HomogeneousNormal3f(Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    _curvatureThreshold = 0.02f;
  }

  virtual void compute(HomogeneousPoint3fOmegaVector& omegas, 
		       HomogeneousPoint3fStatsVector& stats,
		       HomogeneousNormal3fVector& imageNormals);
};

class NormalOmegaGenerator : OmegaGenerator {
 public:
  NormalOmegaGenerator() {
    _flatOmega.diagonal() = HomogeneousNormal3f(Eigen::Vector3f(100.0f, 100.0f, 100.0f));
    _nonFlatOmega.diagonal() = HomogeneousNormal3f(Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    _curvatureThreshold = 0.02f;
  }

  virtual void compute(HomogeneousPoint3fOmegaVector& omegas, 
		       HomogeneousPoint3fStatsVector& stats,
		       HomogeneousNormal3fVector& imageNormals);
};

#endif
