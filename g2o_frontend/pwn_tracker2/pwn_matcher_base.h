#pragma once

#include "g2o_frontend/pwn_core/cloud.h"
#include "g2o_frontend/pwn_core/pinholepointprojector.h"
#include "g2o_frontend/pwn_core/depthimageconverterintegralimage.h"
#include "g2o_frontend/pwn_core/aligner.h"
#include "g2o_frontend/boss/identifiable.h"
#include "g2o_frontend/pwn_boss/aligner.h"
#include "g2o_frontend/pwn_boss/depthimageconverterintegralimage.h"

namespace pwn_tracker {
  using namespace pwn;

  struct PwnMatcherBase : public boss::Identifiable {
    struct MatcherResult{
      EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
      Eigen::Isometry3d transform;
      Matrix6d informationMatrix;
      int cloud_inliers;
      int image_nonZeros;
      int image_outliers;
      int image_inliers;
      float image_reprojectionDistance;
    };

    PwnMatcherBase(pwn::Aligner* aligner_=0, pwn::DepthImageConverter* converter_=0,
		   int id=-1, boss::IdContext* context = 0);

    inline int scale() const {return _scale;}
    inline void setScale (int scale_) {_scale = scale_;}

    inline pwn::Aligner* aligner() { return _aligner;}
    inline void setAligner(pwn::Aligner* aligner_) { _aligner=aligner_;}

    inline pwn::DepthImageConverter* converter() { return _converter;}
    inline void setConverter(pwn::DepthImageConverter* converter_) { _converter=converter_;}


    pwn::Cloud* makeCloud(int& r, int& c,
			  Eigen::Matrix3f& cameraMatrix, 
			  const Eigen::Isometry3f& sensorOffset, 
			  const DepthImage& depthImage);

    void matchClouds(PwnMatcherBase::MatcherResult& result, 
		     pwn::Cloud* fromCloud, pwn::Cloud* toCloud,
		     const Eigen::Isometry3f& fromOffset_, const Eigen::Isometry3f& toOffset_, 
		     const Eigen::Matrix3f& toCameraMatrix_,
		     int toRows, int toCols,
		     const Eigen::Isometry3d& initialGuess=Eigen::Isometry3d::Identity());

    void clearPriors();
    void addRelativePrior(const Eigen::Isometry3d &mean, const Matrix6d &informationMatrix);
    void addAbsolutePrior(const Eigen::Isometry3d &referenceTransform, const Eigen::Isometry3d &mean, const Matrix6d &informationMatrix);

    
    //! boss serialization
    virtual void serialize(boss::ObjectData& data, boss::IdContext& context);
    //! boss deserialization
    virtual void deserialize(boss::ObjectData& data, boss::IdContext& context);

    //! boss deserialization
    virtual void deserializeComplete();



    float _frameInlierDepthThreshold;

    double cumTime;
    int numCalls;
  protected:
    pwn::Aligner* _aligner;
    pwn::DepthImageConverter* _converter;
    int _scale;

  private:
    pwn_boss::Aligner* _baligner;
    pwn_boss::DepthImageConverter* _bconverter;
  };



  template <typename T1, typename T2>
  void convertScalar(T1& dest, const T2& src){
    for (int i=0; i<src.matrix().cols(); i++)
      for (int j=0; j<src.matrix().rows(); j++)
	dest.matrix()(j,i) = src.matrix()(j,i);

  }


}
