#ifndef DRAWABLE_POINTS
#define DRAWABLE_POINTS

#include "../pwn/homogeneousvector4f.h"
#include "drawable.h"

class DrawablePoints : public Drawable {
 public:
  DrawablePoints();
  DrawablePoints(const Eigen::Isometry3f& transformation_, GLParameter *parameter_, HomogeneousPoint3fVector *points_, HomogeneousNormal3fVector *normals_);

  virtual void setPoints(HomogeneousPoint3fVector *points_) { _points = points_; }
  virtual void setNormals(HomogeneousNormal3fVector *normals_) { _normals = normals_; }
  virtual bool setParameter(GLParameter *parameter_);
  
  virtual HomogeneousPoint3fVector* points() { return _points; }
  virtual HomogeneousNormal3fVector* normals() { return _normals; }
  virtual GLParameter* parameter() { return _parameter; };
  
  virtual void draw();

 protected:
  HomogeneousPoint3fVector *_points;
  HomogeneousNormal3fVector *_normals;
};  

#endif
