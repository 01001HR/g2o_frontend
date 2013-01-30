#ifndef DM_MATH_H
#define DM_MATH_H

#include "dm_defs.h"

template <typename OtherDerived>
inline Eigen::Matrix<typename OtherDerived::Scalar,3,3> quat2mat(const Eigen::MatrixBase< OtherDerived >& q)
{
  const typename OtherDerived::Scalar& qx = q.x();
  const typename OtherDerived::Scalar& qy = q.y();
  const typename OtherDerived::Scalar& qz = q.z();
  typename  OtherDerived::Scalar qw = sqrt(1.f - q.squaredNorm());
  Eigen::Matrix<typename  OtherDerived::Scalar,3,3> R;
  R << qw*qw + qx*qx - qy*qy - qz*qz, 2*(qx*qy - qw*qz) , 2*(qx*qz + qw*qy),
    2*(qx*qy + qz*qw) , qw*qw - qx*qx + qy*qy - qz*qz, 2*(qy*qz - qx*qw),
    2*(qx*qz - qy*qw) , 2*(qy*qz + qx*qw), qw*qw - qx*qx - qy*qy + qz*qz;

    return R;
}

template <typename OtherDerived>
inline Eigen::Matrix<typename OtherDerived::Scalar,3,1> mat2quat(const Eigen::MatrixBase< OtherDerived > & R)
{
  Eigen::Quaternion<typename OtherDerived::Scalar> q(R); 
  q.normalize();
  Eigen::Matrix<typename OtherDerived::Scalar,3,1> rq;
  rq << q.x(), q.y(), q.z();
  if (q.w()<0){
    rq = -rq;
  }
  return rq;
}

template <typename OtherDerived>
inline Eigen::Transform<typename OtherDerived::Scalar, 3, Eigen::Isometry> v2t(const Eigen::MatrixBase< OtherDerived >& x)
{
  Eigen::Transform<typename OtherDerived::Scalar, 3, Eigen:: Isometry> X;
  X.linear() = quat2mat(x.tail(3));
  X.translation() = x.head(3);
  return X;
}

template <typename Scalar>
inline Eigen::Matrix<Scalar, 6, 1> t2v(const Eigen::Transform<Scalar, 3, Eigen::Isometry>& X)
{
    Eigen::Matrix<Scalar, 6, 1> v;
    v.head(3) = X.translation();
    v.tail(3) = mat2quat(X.linear());

    return v;
}

template <typename Scalar>
inline Eigen::Matrix<Scalar,3,3> skew(const Eigen::Matrix<Scalar,3,1>& v)
{
    const float& tx = v.x();
    const float& ty = v.y();
    const float& tz = v.z();
    Eigen::Matrix<Scalar,3,3> S;
    S << 0, (2*tz), (-2*ty),
            (-2*tz), 0, (2*tx),
            (2*ty),  (-2*tx),0;
    return S;
}

inline Vector6f remapPoint(const Eigen::Isometry3f& X, const Vector6f p)
{
    Vector6f p2;
    p2.head<3>() = X.linear()*p.head<3>() + X.translation();
    p2.tail<3>() = X.linear()*p.tail<3>();

    return p2;
}

typedef Eigen::Matrix<double, 12, 12> Matrix12d;
typedef Eigen::Matrix<double, 6, 12>  Matrix6x12d;
typedef Eigen::Matrix<double, 12, 1> Vector12d;

inline Vector12d homogeneous2vector(const Eigen::Matrix4d& transform){
  Vector12d x;
  x.block<3,1>(0,0)=transform.block<1,3>(0,0).transpose();
  x.block<3,1>(3,0)=transform.block<1,3>(1,0).transpose();
  x.block<3,1>(6,0)=transform.block<1,3>(2,0).transpose();
  x.block<3,1>(9,0)=transform.block<3,1>(0,3);
  return x;
}
  
inline Eigen::Matrix4d vector2homogeneous(const Vector12d x){
  Eigen::Isometry3d transform=Eigen::Isometry3d::Identity();
  transform.matrix().block<1,3>(0,0)=x.block<3,1>(0,0).transpose();
  transform.matrix().block<1,3>(1,0)=x.block<3,1>(3,0).transpose();
  transform.matrix().block<1,3>(2,0)=x.block<3,1>(6,0).transpose();
  transform.translation()=x.block<3,1>(9,0);
  return transform.matrix();
}

typedef Eigen::Matrix<float, 12, 12> Matrix12f;
typedef Eigen::Matrix<float, 6, 12>  Matrix6x12f;
typedef Eigen::Matrix<float, 12, 1> Vector12f;

inline Vector12f homogeneous2vector(const Eigen::Matrix4f& transform){
  Vector12f x;
  x.block<3,1>(0,0)=transform.block<1,3>(0,0).transpose();
  x.block<3,1>(3,0)=transform.block<1,3>(1,0).transpose();
  x.block<3,1>(6,0)=transform.block<1,3>(2,0).transpose();
  x.block<3,1>(9,0)=transform.block<3,1>(0,3);
  return x;
}
  
inline Eigen::Matrix4f vector2homogeneous(const Vector12f x){
  Eigen::Isometry3f transform=Eigen::Isometry3f::Identity();
  transform.matrix().block<1,3>(0,0)=x.block<3,1>(0,0).transpose();
  transform.matrix().block<1,3>(1,0)=x.block<3,1>(3,0).transpose();
  transform.matrix().block<1,3>(2,0)=x.block<3,1>(6,0).transpose();
  transform.translation()=x.block<3,1>(9,0);
  return transform.matrix();
}

#endif
