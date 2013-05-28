#ifndef DRAWABLE_COVARIANCES
#define DRAWABLE_COVARIANCES

#include "../pwn2/pointstats.h"
#include "gl_parameter_covariances.h"
#include "drawable.h"

namespace pwn {

class DrawableCovariances : public Drawable {
 public:
  DrawableCovariances();
  DrawableCovariances(Eigen::Isometry3f transformation_, GLParameter *parameter_, PointStatsVector *covariances_);
  virtual ~DrawableCovariances() { glDeleteLists(_covarianceDrawList, 1); }

  virtual GLParameter* parameter() { return _parameter; }
  virtual PointStatsVector* covariances() { return _covariances; }
  inline GLuint covarianceDrawList() { return _covarianceDrawList; }

  virtual bool setParameter(GLParameter *parameter_);
  virtual void setCovariances(PointStatsVector *covariances_) { 
    _covariances = covariances_; 
    updateCovarianceDrawList();
  }
  void setStep(int step_) {
    _parameter->setStep(step_);
    updateCovarianceDrawList();
  }
  void setEllipsoidScale(float ellipsoidScale_) {
    _parameter->setEllipsoidScale(ellipsoidScale_);
    updateCovarianceDrawList();
  }

  virtual void draw();
  void updateCovarianceDrawList();

 protected:
  GLParameterCovariances *_parameter;
  PointStatsVector *_covariances;
  GLuint _covarianceDrawList; 
};  

}

#endif
