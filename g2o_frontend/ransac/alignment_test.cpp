#include "alignment_horn2d.h"
#include "alignment_horn3d.h"
#include "alignment_se2.h"
#include "alignment_se3.h"
#include "alignment_line3d_linear.h"
#include "g2o/types/slam3d/isometry3d_mappings.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace Eigen;
using namespace g2o;
using namespace g2o_frontend;
using namespace std;
using namespace Slam3dAddons;


template <typename TypeDomain_, int dimension_>
struct EuclideanMapping{
  typedef TypeDomain_ TypeDomain;
  typedef typename Eigen::Matrix<double, dimension_, 1> VectorType;
  virtual int dimension() const {return dimension_;}
  virtual TypeDomain fromVector(const VectorType& v) const =  0;
  virtual VectorType toVector(const TypeDomain& t) const = 0;
};

template <int dimension_>
struct VectorMapping : public EuclideanMapping<Eigen::Matrix<double, dimension_, 1>, dimension_>{
  typedef typename EuclideanMapping<Eigen::Matrix<double, dimension_, 1>, dimension_>::TypeDomain TypeDomain;
  typedef typename EuclideanMapping<Eigen::Matrix<double, dimension_, 1>, dimension_>::VectorType VectorType;
  virtual TypeDomain fromVector(const VectorType& v) const {return v;}
  virtual VectorType toVector(const TypeDomain& t) const {return t;}
};

struct SE2Mapping: public EuclideanMapping<SE2, 3>{
  typedef typename EuclideanMapping<SE2, 3>::TypeDomain TypeDomain;
  typedef typename EuclideanMapping<SE2, 3>::VectorType VectorType;
  virtual TypeDomain fromVector(const VectorType& v) const {
    SE2 t;
    t.fromVector(v);
    return t;
  }
  virtual VectorType toVector(const TypeDomain& t) const {
    return t.toVector();
  }
};

struct SE3Mapping: public EuclideanMapping<Isometry3d,6>{
  typedef typename EuclideanMapping<Isometry3d, 6>::TypeDomain TypeDomain;
  typedef typename EuclideanMapping<Isometry3d, 6>::VectorType VectorType;
  virtual TypeDomain fromVector(const VectorType& v) const {
    return g2o::internal::fromVectorMQT(v);
  }
  virtual VectorType toVector(const TypeDomain& t) const {
    return g2o::internal::toVectorMQT(t);
  }
};

struct Line3DMapping: public EuclideanMapping<Line3D,6>{
  typedef typename EuclideanMapping<Line3D, 6>::TypeDomain TypeDomain;
  typedef typename EuclideanMapping<Line3D, 6>::VectorType VectorType;
  virtual TypeDomain fromVector(const VectorType& v) const {
    Line3D l(v);
    l.normalize();
    return l;
  }
  virtual VectorType toVector(const TypeDomain& t) const {
    return (VectorType)t;
  }
};



template <typename MappingType, typename AlignerType, typename EdgeCorrespondenceType>
bool testAligner(typename AlignerType::TransformType& result, 
		 int nPoints, 
		 const typename AlignerType::TransformType& transform,
		 const std::vector<double>& scales, 
		 const std::vector<double>& offsets,
		 const std::vector<double>& noises, bool debug = false){

  typedef typename AlignerType::PointVertexType PointVertexType;
  typedef typename AlignerType::PointEstimateType PointEstimateType;
  typedef typename AlignerType::TransformType TransformType;
  typedef typename MappingType::VectorType VectorType;

  OptimizableGraph graph;

  CorrespondenceVector correspondences;
  IndexVector indices(nPoints, 0);
  MappingType mapping;
  assert(scales.size()==mapping.dimension());
  double zeroVec[100];
  std::fill(zeroVec, zeroVec+100, 0); 
  ofstream os1;
  ofstream os2;
  if (debug) {
    os1.open("L1.dat");
    os2.open("L2.dat");
  }
  for (int i=0; i<nPoints; i++){
    VectorType randPoint;
    VectorType noisePoint;
    for(size_t k=0; k<scales.size(); k++){
      randPoint[k]=scales[k]*drand48()+offsets[k];
      noisePoint[k]=noises[k]*(drand48()-.5);
    }
    PointVertexType* v1=new PointVertexType();
    v1->setEstimate(mapping.fromVector(randPoint));
    v1->setId(i);
    graph.addVertex(v1);

    PointVertexType* v2 = new PointVertexType();
    PointEstimateType v2est=transform * v1->estimate();
    VectorType v2noise=mapping.toVector(v2est)+noisePoint;
    v2->setEstimate(mapping.fromVector(v2noise));
    v2->setId(i+nPoints);
    graph.addVertex(v2);    
    v2->updateCache();

    if (debug) {
      VectorType e1=mapping.toVector(v1->estimate());
      VectorType e2=mapping.toVector(v2->estimate());
      for (int i =0; i<mapping.dimension(); i++){
	os1 << e1[i] << " " ;
	os2 << e2[i] << " " ;
      }
      os1 << endl;
      os2 << endl;
    }
    EdgeCorrespondenceType* edge = new EdgeCorrespondenceType(); 
    edge->setMeasurementData(zeroVec);
    edge->vertices()[0]=v1;
    edge->vertices()[1]=v2;
    graph.addEdge(edge);
    Correspondence c(edge,100);
    correspondences.push_back(c);
    indices[i]=i;
  }
  AlignerType aligner;
  return aligner(result,correspondences,indices);
}

int main(int , char** ){
  { // ICP 2D
    cerr << "*************** TEST ICP 2D: *************** " <<endl;

    std::vector<double> scales;
    std::vector<double> offsets;
    std::vector<double> noises;
    for (int i=0; i<2; i++){
      scales.push_back(100);
      offsets.push_back(-50);
      noises.push_back(.5);
    }

    SE2 t0(100, 50, M_PI/4);
    SE2 tresult;
    bool result = testAligner<VectorMapping<2>, AlignmentAlgorithmHorn2D, EdgePointXY>(tresult, 100, t0, scales, offsets, noises);
    if (result){
      cerr << "ground truth: " <<endl;
      cerr << t0.toVector() << endl;
      cerr << "transform found: " <<endl;
      cerr << tresult.toVector() << endl;
      cerr << "transform error: " << endl;
      cerr << (t0*tresult).toVector() << endl;
    } else {
      cerr << "unable to find a transform" << endl;
    }
  }

  {  // ICP 3D
    cerr << "*************** TEST ICP 3D: *************** " <<endl;
    std::vector<double> scales;
    std::vector<double> offsets;
    std::vector<double> noises;
    for (int i=0; i<3; i++){
      scales.push_back(100);
      offsets.push_back(-50);
      noises.push_back(.5);
    }

    Vector6d _t;
    _t << 100, 50, 20, .3, .3, .3;
    Isometry3d t0=g2o::internal::fromVectorMQT(_t);
    Isometry3d tresult;
    bool result = testAligner<VectorMapping<3>, AlignmentAlgorithmHorn3D, EdgePointXYZ>(tresult, 100, t0, scales, offsets, noises);
    if (result){
      cerr << "ground truth: " <<endl;
      cerr << g2o::internal::toVectorMQT(t0)  << endl;
      cerr << "transform found: " <<endl;
      cerr << g2o::internal::toVectorMQT(tresult) << endl;
      cerr << "transform error: " << endl;
      cerr << g2o::internal::toVectorMQT((t0*tresult)) << endl;
    } else {
      cerr << "unable to find a transform" << endl;
    }
  }

  { // SE2
    cerr << "*************** TEST SE2  *************** " <<endl;
    std::vector<double> scales;
    std::vector<double> offsets;
    std::vector<double> noises;
    // translational part;
    for (int i=0; i<2; i++){
      scales.push_back(100);
      offsets.push_back(-50);
      noises.push_back(.1);
    }
    // rotational part
    scales.push_back((2*M_PI)*M_PI);
    offsets.push_back(-M_PI);
    noises.push_back(.1);

    SE2 t0(100, 50, M_PI/4);
    SE2 tresult;
    bool result = testAligner<SE2Mapping, AlignmentAlgorithmSE2, EdgeSE2>(tresult, 100, t0, scales, offsets, noises);
    if (result){
      cerr << "ground truth: " <<endl;
      cerr << t0.toVector() << endl;
      cerr << "transform found: " <<endl;
      cerr << tresult.toVector() << endl;
      cerr << "transform error: " << endl;
      cerr << (t0*tresult).toVector() << endl;
    } else {
      cerr << "unable to find a transform" << endl;
    }
  }

  { // SE3
    cerr << "*************** TEST SE3  *************** " <<endl;
    std::vector<double> scales;
    std::vector<double> offsets;
    std::vector<double> noises;
    // translational part;
    for (int i=0; i<3; i++){
      scales.push_back(100);
      offsets.push_back(50);
      noises.push_back(1.);
    }
    // rotational part
    for (int i=0; i<3; i++){
      scales.push_back(.2);
      offsets.push_back(-.1);
      noises.push_back(.01);
    }

    Vector6d _t;
    _t << 100, 50, 20, .3, .2, .1;
    Isometry3d t0=g2o::internal::fromVectorMQT(_t);

    Isometry3d tresult;
    bool result = testAligner<SE3Mapping, AlignmentAlgorithmSE3, EdgeSE3>(tresult, 100, t0, scales, offsets, noises);
    if (result){
      cerr << "ground truth: " <<endl;
      cerr << g2o::internal::toVectorMQT(t0)  << endl;
      cerr << "transform found: " <<endl;
      cerr << g2o::internal::toVectorMQT(tresult) << endl;
      cerr << "transform error: " << endl;
      cerr << g2o::internal::toVectorMQT((t0*tresult)) << endl;
    } else {
      cerr << "unable to find a transform" << endl;
    }
  }

  { // Line3D
    cerr << "*************** TEST Line3D  *************** " <<endl;
    std::vector<double> scales;
    std::vector<double> offsets;
    std::vector<double> noises;
    // translational part;
    for (int i=0; i<3; i++){
      scales.push_back(100);
      offsets.push_back(50);
      noises.push_back(1.);
    }
    // rotational part
    for (int i=0; i<3; i++){
      scales.push_back(2);
      offsets.push_back(-1);
      noises.push_back(0.1);
    }

    Vector6d _t;
    _t << 100, 50, 20, .3, .2, .1;
    Isometry3d t0=g2o::internal::fromVectorMQT(_t);

    Isometry3d tresult;
    bool result = testAligner<Line3DMapping, AlignmentAlgorithmLine3DLinear, EdgeLine3D>(tresult, 100, t0, scales, offsets, noises, true);
    if (result){
      cerr << "ground truth: " <<endl;
      cerr << g2o::internal::toVectorMQT(t0)  << endl;
      cerr << "transform found: " <<endl;
      cerr << g2o::internal::toVectorMQT(tresult) << endl;
      cerr << "transform error: " << endl;
      cerr << g2o::internal::toVectorMQT((t0*tresult)) << endl;
    } else {
      cerr << "unable to find a transform" << endl;
    }
  }



}
