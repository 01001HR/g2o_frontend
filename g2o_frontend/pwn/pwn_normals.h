#ifndef PWN_NORMALS_H
#define PWN_NORMALS_H

#include "pwn_defs.h"
#include <Eigen/Dense>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>

using namespace Eigen;

typedef Matrix<Vector3f, Dynamic, Dynamic> Matrix3D;
typedef Matrix<float, 9, 1> Vector9f;
typedef Matrix<Vector9f, Dynamic, Dynamic> MatrixVector9f;

typedef std::queue<Vector2i>  Vector2iQueue;
typedef std::vector<Vector2i> Vector2iVector;

void computeIterativeCovariance(Matrix3f &covariance, 
				const Vector6fPtrMatrix &points, const Vector2i &imgPoint, 
				char **visitedMask, Vector2iVector &visited, 
				float rw2, int ri2);
void computeIntegralCovariance(Matrix3f &covariance, 
			       const Vector6fPtrMatrix &points, const Vector2i &imgPoint,  
			       MatrixVector9f &integImage, MatrixXi &integMask, 
			       const Matrix3f &cameraMatrix, float r);
void computeNormalAndCurvature(Vector3f normal, float &curvature, 
			       const Matrix3f covariance);
void computeIntegralImage(MatrixVector9f &integImage, MatrixXi &integMask, 
			  const Vector6fPtrMatrix &points);
void computeNormals(Vector6fPtrMatrix &cloud, MatrixXf &curvature, 
		    const Matrix3f &cameraMatrix, float r, float d);
void getOffset(int &offset, 
	       Vector3f point, const Matrix3f &cameraMatrix, 
	       float r, int col);
void getRegion(Vector9f &region, int &regionMask, int &numValidPoints,
	       const MatrixVector9f &integImage, const MatrixXi &integMask, 
	       int offset, Vector2i p0i);

#endif
