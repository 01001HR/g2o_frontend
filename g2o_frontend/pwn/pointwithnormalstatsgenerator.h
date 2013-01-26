#ifndef POINT_WITH_NORMAL_STATISTICS_GENERATOR
#define POINT_WITH_NORMAL_STATISTICS_GENERATOR
#include "pointwithnormal.h"
#include "integralpointimage.h"

struct PointWithNormalSVD{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  friend class PointWithNormalStatistcsGenerator;
  friend class PointWithNormalMerger;
  friend PointWithNormalSVD operator * (const Eigen::Isometry3f& t, const PointWithNormalSVD& svd_);
  PointWithNormalSVD(): 
    _mean(Eigen::Vector3f::Zero()),
    _singularValues(Eigen::Vector3f::Zero()),
    _U(Eigen::Matrix3f::Zero()),
    _curvature(-1),
    _z(0),
    _n(0)
  {}
	
  inline const Eigen::Vector3f& singularValues() const {return _singularValues;}
  inline const Eigen::Matrix3f& U() const {return _U;}
  inline const Eigen::Vector3f& mean() const {return _mean;}
  inline int n() const {return _n;}
  inline float curvature() const {return _curvature;}
  inline float z() const {return _z;}

protected:
  inline void updateCurvature() {
    if (_singularValues.squaredNorm()==0) 
      _curvature = -1; 
    else
      _curvature = _singularValues(0)/(_singularValues(0) + _singularValues(1) + _singularValues(2) );
  }	

  Eigen::Vector3f _mean;
  Eigen::Vector3f _singularValues;
  Eigen::Matrix3f _U;
  float _curvature;
  float _z;
  int _n;
};

inline PointWithNormalSVD operator * (const Eigen::Isometry3f& t, const PointWithNormalSVD& svd_){
  PointWithNormalSVD rsvd(svd_);
  rsvd._mean = t*svd_.mean();
  rsvd._U = t.linear() * svd_._U;
  return rsvd;
}

typedef std::vector<PointWithNormalSVD, Eigen::aligned_allocator<PointWithNormalSVD> > PointWithNormalSVDVector;

class PointWithNormalStatistcsGenerator{
public:
  PointWithNormalStatistcsGenerator();

  void computeNormalsAndSVD(PointWithNormalVector& points, PointWithNormalSVDVector& svds, const Eigen::MatrixXi& indices, 
			    const Eigen::Matrix3f& cameraMatrix, const Eigen::Isometry3f& cameraPose=Eigen::Isometry3f::Identity() );

  // void computeNormals(PointWithNormalVector& points, const Eigen::MatrixXi indices, const Eigen::Matrix3f& cameraMatrix);


  inline int step() const {return _step;}
  inline void setStep(int step_)  {_step=step_;}
  inline int minPoints() const {return _minPoints;}
  inline void setMinPoints(int minPoints_)  {_minPoints=minPoints_;}
  inline int imageRadius() const {return _imageRadius;}
  inline void setImageRadius(int imageRadius_)  {_imageRadius=imageRadius_;}
  inline float worldRadius() const {return _worldRadius;}
  inline void setWorldRadius(float worldRadius_)  {_worldRadius=worldRadius_;}
  inline float maxCurvature() const {return _maxCurvature;}
  inline void setMaxCurvature(float maxCurvature_)  {_maxCurvature=maxCurvature_;}
  inline const Eigen::Matrix3f& cameraMatrix() const {return _cameraMatrix;}
  inline void setCameraMatrix(Eigen::Matrix3f cameraMatrix_)  {_cameraMatrix=cameraMatrix_;}

#ifdef _PWN_USE_OPENMP_
  inline int numThreads() const { return _numThreads; }
  inline void setNumThreads(int numThreads_)  { _numThreads = numThreads_; }
#endif //_PWN_USE_OPENMP_

protected:
  Eigen::Vector2i _range(int r, int c) const;

  int _step;
  int _imageRadius;
  int _minPoints;
  float _worldRadius;
  float _maxCurvature;
  Eigen::Matrix3f _cameraMatrix;
  IntegralPointImage _integralImage;
  int _numThreads;
};

#endif
