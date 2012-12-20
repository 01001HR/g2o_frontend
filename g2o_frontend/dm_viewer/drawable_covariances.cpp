#include "drawable_covariances.h"
#include "gl_parameter_covariances.h"
#include "dm_qglviewer.h"

DrawableCovariances::DrawableCovariances() : Drawable() {
  GLParameterCovariances* covariancesParameter = new GLParameterCovariances();
  _parameter = (GLParameter*)covariancesParameter;
  _covariances = 0;
}

DrawableCovariances::DrawableCovariances(Eigen::Isometry3f transformation_, GLParameter *parameter_, int step_, CovarianceSVDVector *covariances_) : Drawable(transformation_, step_){
  setParameter(parameter_);
  _covariances = covariances_;
}

bool DrawableCovariances::setParameter(GLParameter *parameter_) {
  GLParameterCovariances* covariancesParameter = (GLParameterCovariances*)parameter_;
  if (covariancesParameter == 0) {
    _parameter = 0;
    return false;
  }
  _parameter = parameter_;
  return true;
}

// Drawing function of the class object.
void DrawableCovariances::draw() {
  GLParameterCovariances* covariancesParameter = (GLParameterCovariances*)_parameter;
  if (_covariances && covariancesParameter && covariancesParameter->ellipsoidScale() > 0.0f) {
    glPushMatrix();
    glMultMatrixf(_transformation.data());
    
    covariancesParameter->applyGLParameter();
    float ellipsoidScale = covariancesParameter->ellipsoidScale();
    Eigen::Vector4f colorLowCurvature = covariancesParameter->colorLowCurvature();
    Eigen::Vector4f colorHighCurvature = covariancesParameter->colorHighCurvature();
    float curvatureThreshold = covariancesParameter->curvatureThreshold();
    for (size_t i = 0; i < _covariances->size(); i += _step) {
      const SVDMatrix3f& covSVD = (*_covariances)[i];
      const Eigen::Vector3f& lambda = covSVD.lambda;
      const Eigen::Isometry3f& I = covSVD.isometry;
      if (covSVD.lambda.squaredNorm() == 0.0f)
	continue;
      float sx = sqrt(lambda[0])*ellipsoidScale;
      float sy = sqrt(lambda[1])*ellipsoidScale;
      float sz = sqrt(lambda[2])*ellipsoidScale;
      float curvature = lambda[0] / (lambda[0] + lambda[1] + lambda[2]);
      glPushMatrix();
      glMultMatrixf(I.data());
      if (curvature > curvatureThreshold) {
	glColor4f(colorHighCurvature[0] - curvature, colorHighCurvature[1], colorHighCurvature[2], colorHighCurvature[3]);
      }
      else {
	glColor4f(colorLowCurvature[0], colorLowCurvature[1] - curvature, colorLowCurvature[2], colorLowCurvature[3]);
	sx = 1e-03;
	sy = ellipsoidScale;
	sz = ellipsoidScale;
      }
      glScalef(sx, sy, sz);
      glCallList(_dmQGLViewer->ellipsoidDrawList());
      glPopMatrix();
    }
    glPopMatrix();
  }
}
