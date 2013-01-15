#ifndef LINE2DEXTRACTOR_H
#define LINE2DEXTRACTOR_H

#include <iostream>
#include <Eigen/Core>
#include <Eigen/StdVector>
#include <vector>
#include <map>


using namespace Eigen;

//typedef Eigen::Matrix<double, 4, 1> Vector4f;

struct Line2D: public Vector4f{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  Line2D(){
    (*this) << 0,0,1,0;
  }
  Vector2f p() const { return head<2>();}
  Vector2f d() const { return tail<2>();}
  void setP(const Vector2f& p_)  { head<2>()=p_;}
  void setD(const Vector2f& d_)  { tail<2>()=d_;}

  void normalize(){
    tail<2>().normalize();
    head<2>()-=tail<2>()*(tail<2>().dot(head<2>()));
  }
  void fromPoints(const Vector2f& p0_, const Vector2f& p1_){
    head<2>()=p0_;
    tail<2>()=p1_-p0_;
    normalize();
  }
  float squaredDistance(const Vector2f& p0_) const{
    Vector2f dp=p()-p0_;
    float tmin = -dp.dot(d());
    dp+=d()*tmin;
    return dp.squaredNorm();
  }
  int p0Index, p1Index;
};
	

typedef std::vector<Line2D, Eigen::aligned_allocator<Line2D> > Line2DVector; //Vector of line
typedef std::vector<Vector2f, Eigen::aligned_allocator<Vector2f> > Vector2fVector; //vector of original points
// typedef std::vector<Vector2fVector, Eigen::aligned_allocator<Vector2f> > VectorOfSubsetsPoints; // vector of substets of points
	

class Point2DClusterer {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  typedef std::pair<int, int> Cluster;
  typedef std::vector<Cluster> ClusterVector;

	Point2DClusterer() {}
  inline const Cluster& cluster(int i) const {return _clusters[i];}
  inline int numClusters() const {return _clusters.size();};
  inline const Vector2fVector& points() const {return _points;}
  inline void setPoints(Vector2fVector& points_) {_points=points_;}
  void compute();
  
  Vector2fVector _points;
  ClusterVector _clusters;
  float _squaredDistance;
};

class Line2DExtractor: public Vector4f {
public:
  typedef std::map<int, Line2D, std::less<int>, Eigen::aligned_allocator<std::pair<const int, Line2D> > > IntLineMap;

  Line2DExtractor();
  virtual ~Line2DExtractor();

  inline const Vector2fVector& points() const {return _points;}
  inline void setPoints(Vector2fVector& points_) {_points=points_;}
  inline const IntLineMap& lines() const { return _lines;}
  
  void compute();

	//split thresholds
  float _splitThreshold;
  int _minPointsInLine;
	//merge thresholds
  int _maxPointDistance;
  float _rhoMergeThreshold;
  float _normalMergeThreshold;
	
protected:
  void initializeFromIndices(Line2D& l, int i0, int i1);
	int maxDistanceIndex(const Line2D& l);  
  bool split(int i);
  bool merge(int i);
//   bool step(Vector2fVector& /*points*/ ,
// 	    VectorOfSubsetsPoints& /*subset*/, 
// 	    Line2DVector& /*lines*/);
  Vector2fVector _points;
  IntLineMap _lines;
};

#endif // LINE2DEXTRACTOR_H
