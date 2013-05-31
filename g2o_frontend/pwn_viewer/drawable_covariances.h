#ifndef DRAWABLE_COVARIANCES
#define DRAWABLE_COVARIANCES

#include "../pwn2/stats.h"
#include "gl_parameter_covariances.h"
#include "drawable.h"

namespace pwn {

class DrawableCovariances : public Drawable {
 public:
  DrawableCovariances();
  DrawableCovariances(Eigen::Isometry3f transformation_, GLParameter *parameter_, StatsVector *covariances_);
  virtual ~DrawableCovariances() { 
    glDeleteLists(_covarianceDrawList, 1); 
    glDeleteLists(_sphereDrawList, 1); 
  }

  virtual GLParameter* parameter() { return _parameter; }
  virtual StatsVector* covariances() { return _covariances; }
  inline GLuint covarianceDrawList() { return _covarianceDrawList; }
  inline GLuint sphereDrawList() { return _sphereDrawList; }

  virtual bool setParameter(GLParameter *parameter_);
  virtual void setCovariances(StatsVector *covariances_) { 
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
  StatsVector *_covariances;
  GLuint _covarianceDrawList; 
  GLuint _sphereDrawList; 
};  

}

#endif
