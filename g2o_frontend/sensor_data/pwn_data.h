#ifndef PWNDATA_H
#define PWNDATA_H

#include "g2o/core/hyper_graph.h"
#include "g2o/types/slam3d/types_slam3d.h"

#include "g2o_frontend/pwn_core/cloud.h"

using namespace std;
using namespace g2o;
using namespace pwn;

class PWNData : public HyperGraph::Data {
 public:  
  PWNData(Cloud *cloud = 0);
  virtual ~PWNData();

  //! read the data from a stream
  virtual bool read(std::istream &is);
  //! write the data to a stream
  virtual bool write(std::ostream &os) const;
  //! write the images (if changed)
  virtual void writeOut() const;

  void update();
  void release();

  inline const std::string& filename() const { return _filename; };
  inline const Cloud* cloud() const { return _cloud; }
  inline const Eigen::Isometry3f& originPose() const { return _originPose; }
  
  inline void setFilename(const std::string filename_) { _filename = filename_; };  
  void setCloud(Cloud *cloud_) {
    if (_cloud) 
      delete _cloud;
    _cloud = cloud_;
    _cloudModified = true;
  }
  inline void setOriginPose(const Eigen::Isometry3f originPose_) { _originPose = originPose_; }  

 protected:
  std::string _filename;
  Cloud *_cloud;
  Eigen::Isometry3f _originPose;

 private:
  mutable bool _cloudModified;
};

#ifdef G2O_HAVE_OPENGL

class PWNDataDrawAction : public g2o::DrawAction {
 public:
  PWNDataDrawAction() : DrawAction(typeid(PWNData).name()) {};
  virtual HyperGraphElementAction* operator()(g2o::HyperGraph::HyperGraphElement *element, 
					      g2o::HyperGraphElementAction::Parameters *params_);
 
 protected:
  virtual bool refreshPropertyPtrs(g2o::HyperGraphElementAction::Parameters *params_);
  g2o::IntProperty *_beamsDownsampling;
  g2o::FloatProperty *_pointSize;
};

#endif // G2O_HAVE_OPENGL

#endif // PWNDATA_H
