#include "drawable_frame.h"

#include <iostream>

using namespace g2o;
using namespace std;

namespace pwn {

DrawableFrame::DrawableFrame(const Eigen::Isometry3f &transformation_, GLParameter *parameter_, 
			     Frame *frame_) : Drawable(transformation_) {
  setParameter(parameter_);
  _frame = frame_;
  _drawablePoints = 0;
  _drawableCorrespondences = 0;
  _drawableCovariances = 0;
  _drawableCorrespondences = 0;
  _previousDrawableFrame = 0;
  constructDrawableObjects();
}

bool DrawableFrame::setParameter(GLParameter *parameter_) {
  GLParameterFrame *frameParameter = (GLParameterFrame*)parameter_;
  if(frameParameter == 0) {
    _parameter = 0;
    return false;
  }
  _parameter = frameParameter;
  return true;
}

void DrawableFrame::clearDrawableObjects() {
  if(_drawablePoints)
    delete _drawablePoints;
  if(_drawableNormals)
    delete _drawableNormals;
  if(_drawableCovariances)
    delete _drawableCovariances;
  if(_drawableCorrespondences)
    delete _drawableCorrespondences;
  _drawablePoints = 0;
  _drawableCorrespondences = 0;
  _drawableCovariances = 0;
  _drawableCorrespondences = 0;
}

void DrawableFrame::constructDrawableObjects(){
  if(_frame) {
    _drawablePoints = new DrawablePoints(Isometry3f::Identity(), 
					 (GLParameter*)_parameter->parameterPoints(), &_frame->points(), &_frame->normals(), &_frame->traversabilityVector());
    _drawableNormals = new DrawableNormals(Isometry3f::Identity(), 
					   (GLParameter*)_parameter->parameterNormals(), &_frame->points(), &_frame->normals());
    _drawableCovariances = new DrawableCovariances(Isometry3f::Identity(), 
						   (GLParameter*)_parameter->parameterCovariances(), &_frame->stats());
    _drawableCorrespondences = new DrawableCorrespondences();
    _drawableCorrespondences->setParameter((GLParameter*)_parameter->parameterCorrespondences());
  }
}

void DrawableFrame::setFrame(Frame *f) {
  if(f != _frame) {
    clearDrawableObjects();
    _frame = f;
    constructDrawableObjects();
  }
}

void DrawableFrame::draw() {
  if(_parameter->show() && _frame) {
    glPushMatrix();
    glMultMatrixf(_transformation.data());
    if(_drawablePoints)
      _drawablePoints->draw();
    if(_drawableNormals)
      _drawableNormals->draw();
    if(_drawableCovariances)
      _drawableCovariances->draw();
    if(_drawableCorrespondences)
      _drawableCorrespondences->draw();
    glPopMatrix();
  }
}

}
