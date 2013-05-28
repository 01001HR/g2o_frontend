#include "drawable_points.h"
#include "gl_parameter_points.h"
#include <iostream>

namespace pwn {

DrawablePoints::DrawablePoints() : Drawable() {
  _parameter = 0;
  _points = 0;
  _normals = 0;
  _traversabilityVector = 0;
  _pointDrawList = glGenLists(1);
  updatePointDrawList();
}

DrawablePoints::DrawablePoints(const Eigen::Isometry3f& transformation_, GLParameter *parameter_, PointVector *points_,  
			       NormalVector *normals_, std::vector<int> *traversabilityVector_) : Drawable(transformation_) {
  setParameter(parameter_);
  _points = points_;
  _normals = normals_;
  _traversabilityVector = traversabilityVector_;
  _pointDrawList = glGenLists(1);
  updatePointDrawList();
}

DrawablePoints::DrawablePoints(const Eigen::Isometry3f& transformation_, GLParameter *parameter_, PointVector *points_,  
			       NormalVector *normals_) : Drawable(transformation_) {
  setParameter(parameter_);
  _points = points_;
  _normals = normals_;
  _traversabilityVector = 0;
  _pointDrawList = glGenLists(1);
  updatePointDrawList();
}

bool DrawablePoints::setParameter(GLParameter *parameter_) {
  GLParameterPoints *pointsParameter = (GLParameterPoints*)parameter_;
  if(pointsParameter == 0) {
    _parameter = 0;
    return false;
  }
  _parameter = pointsParameter;
  return true;
}

void DrawablePoints::draw() {
  GLParameterPoints *pointsParameter = dynamic_cast<GLParameterPoints*>(_parameter);
  if(pointsParameter && 
     pointsParameter->show() && 
     pointsParameter->pointSize() > 0.0f) {
    glPushMatrix();
    glMultMatrixf(_transformation.data());
    pointsParameter->applyGLParameter();
    glCallList(_pointDrawList);
    glEnd();
    glPopMatrix();
  }
  // GLParameterPoints *pointsParameter = dynamic_cast<GLParameterPoints*>(_parameter);
  // if(_points && 
  //    _normals && 
  //    pointsParameter && 
  //    pointsParameter->show() && 
  //    pointsParameter->pointSize() > 0.0f) {
  //   glPushMatrix();
  //   glMultMatrixf(_transformation.data());
  //   pointsParameter->applyGLParameter();
  //   glBegin(GL_POINTS);
  //   for(size_t i = 0; i < _points->size(); i += pointsParameter->step()) {
  //     const Point &p = _points->at(i);
  //     const Normal &n = _normals->at(i);
  //     if(_traversabilityVector && _traversabilityVector->size() > 0) {
  //       if(_traversabilityVector->at(i) > 0)
  //         glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
  //       else if (_traversabilityVector->at(i) < 0)
  //         glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
  //       else
  //         glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
  //     }
  //     glNormal3f(n[0], n[1], n[2]);
  //     glVertex3f(p[0], p[1], p[2]);
  //   }
  //   glEnd();
  //   glPopMatrix();
  // }
}

void DrawablePoints::updatePointDrawList() {
  GLParameterPoints *pointsParameter = dynamic_cast<GLParameterPoints*>(_parameter);
  glNewList(_pointDrawList, GL_COMPILE);    
  if(_points && 
     _normals &&
     pointsParameter && 
     pointsParameter->show() && 
     pointsParameter->pointSize() > 0.0f) {
    glBegin(GL_POINTS);
    for(size_t i = 0; i < _points->size(); i += pointsParameter->step()) {
      const Point &p = _points->at(i);
      const Normal &n = _normals->at(i);
      if(_traversabilityVector && _traversabilityVector->size() > 0) {
        if(_traversabilityVector->at(i) > 0)
          glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        else if (_traversabilityVector->at(i) < 0)
          glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        else
          glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
      }
      glNormal3f(n[0], n[1], n[2]);
      glVertex3f(p[0], p[1], p[2]);
    }
    glEnd();
  }
  glEndList();
}

}
