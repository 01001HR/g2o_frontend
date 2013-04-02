#include <Eigen/Eigenvalues>
#include "homogeneouspoint3fstatsgenerator.h"
#include <iostream>
#include <omp.h>
using namespace std;
HomogeneousPoint3fStatsGenerator::HomogeneousPoint3fStatsGenerator(){
  _worldRadius = 0.1;
  _maxImageRadius = 30;
  _minImageRadius = 10;
  _minPoints = 50;
}

void HomogeneousPoint3fStatsGenerator::compute(HomogeneousPoint3fStatsVector& stats, 
					       const HomogeneousPoint3fIntegralImage& integralImage,
					       const Eigen::MatrixXi& intervalImage,
					       /*const Eigen::MatrixXf& depthImage,*/
					       const Eigen::MatrixXi& indexImage)
{
  assert(integralImage.rows()==intervalImage.rows());
  assert(integralImage.cols()==intervalImage.cols());
  assert(integralImage.rows()==indexImage.rows());
  assert(integralImage.cols()==indexImage.cols());

  #pragma omp parallel for
  for (int c=0; c<indexImage.cols(); ++c){
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigenSolver;
    const int* index = &indexImage.coeffRef(0,c);
    const int* interval = &intervalImage.coeffRef(0,c);
    /*const float* depth = &depthImage.coeffRef(0,c);*/
    for (int r=0; r<indexImage.rows(); ++r, ++index, ++interval/*, ++depth*/){
     
      // is the point valid, is its range valid?
      if (*index<0 || *interval<0)
	continue;
      
      assert(*index<(int)stats.size());
      int imageRadius = *interval;
      if (imageRadius < _minImageRadius)
	imageRadius = _minImageRadius;
      if (imageRadius > _maxImageRadius)
	imageRadius = _maxImageRadius;

      HomogeneousPoint3fAccumulator acc=integralImage.getRegion(r-imageRadius, r+imageRadius, 
								c-imageRadius, c+imageRadius);
      if (acc.n()< _minPoints)
	continue;
      Eigen::Matrix4f covariance4f = acc.covariance();
      Eigen::Matrix3f covariance3f = covariance4f.block<3,3>(0,0);
      eigenSolver.computeDirect(covariance3f, Eigen::ComputeEigenvectors);
      covariance4f.setZero();
      covariance4f.block<3,3>(0,0) = eigenSolver.eigenvectors();
      covariance4f.block<3,1>(0,3) = eigenSolver.eigenvalues();
      if (covariance4f.coeffRef(0,3)<0)
	covariance4f.coeffRef(3,0)=0;
      stats[*index] = covariance4f;
      
      // cerr << 
      // 	"idx:"<< *index << 
      // 	" r: " << r << " c:" << c << " n:" << acc.n() <<  
      // 	/*" d:" << *depth <<"interval: " << *interval <<*/
      // 	" imR:" << imageRadius << 
      // 	" mean:" << acc.mean().transpose() << 
      // 	"  ev2:" << stats[*index].eigenValues().transpose() << endl;

      // if (stats[*index].eigenValues().transpose().norm() ==0) {
      // 	cerr << "cov:" << endl;
      // 	cerr << covariance3f << endl;
	 
      //}

    }
  }

}
