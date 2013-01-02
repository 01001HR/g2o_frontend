#ifndef DM_MATH_H
#define DM_MATH_H

#include "dm_defs.h"

inline Eigen::Matrix3f quat2mat(const Eigen::Vector3f& q)
{
    const float& qx = q.x();
    const float& qy = q.y();
    const float& qz = q.z();
    float qw = sqrt(1.f - q.squaredNorm());
    Eigen::Matrix3f R;
    R << qw*qw + qx*qx - qy*qy - qz*qz, 2*(qx*qy - qw*qz) , 2*(qx*qz + qw*qy),
         2*(qx*qy + qz*qw) , qw*qw - qx*qx + qy*qy - qz*qz, 2*(qy*qz - qx*qw),
         2*(qx*qz - qy*qw) , 2*(qy*qz + qx*qw), qw*qw - qx*qx - qy*qy + qz*qz;

    return R;
}

inline Eigen::Vector3f mat2quat(const Eigen::Matrix3f& R)
{
    float n = 1./(2*sqrt(1 + R(0, 0) + R(1, 1) + R(2, 2)));

    return Eigen::Vector3f(n*(R(2, 1) - R(1, 2)),
                    n*(R(0, 2) - R(2, 0)),
                    n*(R(1, 0) - R(0, 1)));
}

inline Eigen::Isometry3f v2t(const Vector6f& x)
{
    Eigen::Isometry3f X;
    X.linear() = quat2mat(x.tail<3>());
    X.translation() = x.head<3>();

    return X;
}

inline Vector6f t2v(const Eigen::Isometry3f& X)
{
    Vector6f v;
    v.head<3>() = X.translation();
    v.tail<3>() = mat2quat(X.linear());

    return v;
}

inline Eigen::Matrix3f skew(const Eigen::Vector3f& v)
{
    const float& tx = v.x();
    const float& ty = v.y();
    const float& tz = v.z();
    Eigen::Matrix3f S;
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
