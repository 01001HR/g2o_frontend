#ifndef G2O_FRONTEND_FEATURE_TRACKER
#define G2O_FRONTEND_FEATURE_TRACKER
#include <set>
#include "g2o_frontend/data/feature_data.h"
#include "g2o/core/optimizable_graph.h"
#include "g2o/types/slam2d/types_slam2d.h"

#include <Eigen/StdVector>

namespace g2o {

  struct BaseFrame;
  struct BaseTrackedFeature;
  struct BaseTrackedLandmark;
  struct FeatureTracker;

  typedef std::set<BaseTrackedFeature*> BaseTrackedFeatureSet;
  typedef std::set<BaseTrackedLandmark*> BaseTrackedLandmarkSet;
  typedef std::set<BaseFrame*> BaseFrameSet;
  typedef std::map<OptimizableGraph::Vertex*, BaseFrame*> VertexFrameMap;

  /** Basic structure that holds the oberrvations a robot takes from a give position.
      A frame is associated with:
      <ul>  
      <li>a previous frame in the trajectory (if not the first) </li>
      <li>a vertex of the graph indicating the robot pose from where the observations were taken </li>
      <li>an edge from the odometry (whose information matrix might be set to 0 if no odom is present</li>
      <li>a set of "tracked features"</li>
      <li>a set of "neighbor" frames that are those from which a substantial set of landmark has been seen in common</li>
      </ul>
   */
  struct BaseFrame {

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  
  BaseFrame(OptimizableGraph::Vertex* v,
	    OptimizableGraph::Edge* e,
	    BaseFrame* p,
	    BaseFrame* n);

    template<typename VertexType> 
    VertexType vertex() {return dynamic_cast<VertexType>(_vertex);}
 
    template<typename VertexType> 
    VertexType vertex() const {return dynamic_cast<VertexType>(_vertex);}
  
  
    virtual void setVertex(OptimizableGraph::Vertex* v);

    template<typename OdometryEdgeType> 
    OdometryEdgeType odometryEdge() {return dynamic_cast<OdometryEdgeType>(_odometryEdge);}

    template<typename OdometryEdgeType> 
    const OdometryEdgeType odometryEdge() const {return dynamic_cast<const OdometryEdgeType>(_odometryEdge);}

    inline void setOdometryEdge(OptimizableGraph::Edge* e) {
      _odometryEdge = e; 
    }

    inline BaseTrackedFeatureSet& features() {return _features;}
  
    inline BaseFrame* previous() { return _previousFrame;}
    inline void setPrevious(BaseFrame* p) { _previousFrame = p;   }

    inline BaseFrame* next() { return _nextFrame;}
    inline void setNext(BaseFrame* p) { _nextFrame = p; } 

    
    BaseFrameSet& neighbors () {return _neighbors;}
    const BaseFrameSet& neighbors () const {return _neighbors;}

    void landmarks(BaseTrackedLandmarkSet& landmarks_);
    
  protected:
    OptimizableGraph::Vertex* _vertex;
    OptimizableGraph::Edge* _odometryEdge;
    BaseFrame* _previousFrame;
    BaseFrame* _nextFrame;
    BaseTrackedFeatureSet _features;
    BaseFrameSet _neighbors;
  };
 

  /** Basic structure that holds a feature that was tracked between soem frames. 
      A tracked feature belongs to (only one!) framem can originate from another feature present in the
      previous frame (or not), and can lead to multiple tracked features in the subsequent frames.
      The feature only stores the pointers to the "prevous" features, while the successors are indirectly computed.
      A feature might be associated to a landmark or not.
      If a feature is associated to a landmark, it can also store the graph edge representing the observation
      made by the robot to the landmark, and rhe measurement of such an edge should be labeled with the measurement
      of the feature.
      </ul>
   */
  struct BaseTrackedFeature{
		
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  
  BaseTrackedFeature(BaseFrame* frame_, 
		     BaseFeatureData* featureData_, 
		     BaseTrackedFeature* previous_);

    virtual ~BaseTrackedFeature();
  
    inline BaseFrame* frame() { return _frame;}
    inline const BaseFrame* frame() const { return _frame;}
  
    template <typename FeatureDataType> 
    FeatureDataType featureData() {return _featureData ? dynamic_cast <FeatureDataType>(_featureData) : 0;}

    template <typename FeatureDataType> 
    FeatureDataType featureData() const {return _featureData ? dynamic_cast <FeatureDataType>(_featureData) : 0;}


    inline BaseTrackedFeature* previous() {return _previous;}
    inline const  BaseTrackedFeature* previous() const {return _previous;}

    void setPrevious(BaseTrackedFeature* previous_);

    inline BaseTrackedLandmark* landmark() {return _landmark;}
    inline const BaseTrackedLandmark* landmark() const {return _landmark;}
    inline void setLandmark(BaseTrackedLandmark* l) {_landmark = l;}

    int trackLenght() const;

    template <typename EdgeDataType> 
    EdgeDataType edge() {return _edge ? dynamic_cast <EdgeDataType>(_edge) : 0;}

    template <typename EdgeDataType> 
    EdgeDataType edge() const {return _edge ? dynamic_cast <EdgeDataType>(_edge) : 0;}

    inline void setEdge(OptimizableGraph::Edge* e) {_edge = e;}

    inline int numChildren() const {
      return _children.size();
    }

    inline BaseTrackedFeatureSet& children() {return _children;}
    inline const BaseTrackedFeatureSet& children() const {return _children;}
    OptimizableGraph::Edge* _edge;
    
  protected:
    BaseFrame* _frame;
    BaseFeatureData* _featureData;
    BaseTrackedFeature* _previous;
    BaseTrackedLandmark* _landmark;
    BaseTrackedFeatureSet _children;
  };


  /** Basic structure that represents a tracked landmark. A landmark can arise from many tracked features and corresponds
      to a vertex in the graph.
   */
  struct BaseTrackedLandmark{
	  
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    BaseTrackedLandmark(OptimizableGraph::Vertex* v);
 
    template <typename LandmarkVertexType>
    LandmarkVertexType vertex() {return dynamic_cast <LandmarkVertexType>(_vertex);}

    template <typename LandmarkVertexType>
    const LandmarkVertexType vertex() const {return dynamic_cast < const LandmarkVertexType>(_vertex);}

    inline BaseTrackedFeatureSet& features() {return _trackedFeatures;}

  protected:
    OptimizableGraph::Vertex* _vertex;
    BaseTrackedFeatureSet _trackedFeatures;
  };


  /** Basic structure that represents a correspondence between a pair of features. 
      The distance is 0 if the features "look" the same, according to the metric used to compute the correspondence.
   */
  struct Correspondence {
	 
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  
  Correspondence(BaseTrackedFeature* f1_=0, BaseTrackedFeature* f2_=0, double distance_=0 ): 
    f1(f1_), f2(f2_), distance(distance_) {}
  
    inline bool operator<(const Correspondence& c2) const {
      return distance<c2.distance;
    }

    BaseTrackedFeature* f1;
    BaseTrackedFeature* f2;
    double distance;
  };
  typedef std::vector<Correspondence,Eigen::aligned_allocator<Correspondence> > CorrespondenceVector;



  /** Function object that instantiates a landmark object starting from a sequence of observations.
      The track should be at least long as minNumObervations(), otherwise the landmark creation fails.
      Overridden for specific feature and landmark types.
   */
  struct LandmarkConstructor {
	
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
   
		// instantiates a landmark given a set of measurements that are supposed to come from the same landmark. If null, it returns false;
    //! returns a landmark object constructed from the sequence of features given as argument, and initialized accordingly
    virtual BaseTrackedLandmark* constructLandmark(std::vector<BaseTrackedFeature*> &) = 0;
    //! returns an graph edge that contains the measure of the landmark's vertex ad is labeled with the feature's measurement andn information matrix.
    virtual OptimizableGraph::Edge* constructEdge(BaseTrackedLandmark* l, BaseTrackedFeature* f) = 0;
    virtual int minNumObservations() const = 0;
  };

  /** Function object that computes the correspondence of two sets of features.
      Overridden in specific implementations
   */
  struct CorrespondenceFinder {
		
	  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    // determines candidate correspondences between two sets of features;
    virtual void compute(BaseTrackedFeatureSet& s1, BaseTrackedFeatureSet& s2) = 0;
    inline const CorrespondenceVector& correspondences() {return _correspondences;}
  protected:
    CorrespondenceVector _correspondences;
  };


  /** Function object that computes deterimines the aligment of two frames or two sets of features,
      given a set of (noisy) correspondences. read RANSAC. Overridden.
   */
  struct Matcher {
		
	  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    // computes the best match between a set of correspondences;
    Matcher();
    virtual void compute(const CorrespondenceVector& correspondences) = 0;
    inline const CorrespondenceVector& matches() {return _matches;}
    virtual bool applyCandidateTransform();
    // if the two sets of features have a non empty intersection of unique landmarks, this returns how many they are
    inline int landmarkIdentityMatches() const {return _landmarkIdentityMatches;}

  protected:
    CorrespondenceVector _matches;
    int _landmarkIdentityMatches;
  };


  /** Function object that rejects the correspondences based on some criteria.
   */
  struct CorrespondenceFilter{
		
	  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
		
    virtual void compute(const CorrespondenceVector& correspondences)=0;
    inline const CorrespondenceVector& filtered() const {return _filtered;}
  protected:
    CorrespondenceVector _filtered;
  };

  /** This rejects the correspondences of the matches where the distance between the best match for a feature
      and the second best match for the second feature is too small. (read the features are ambiguous).
   */
  struct CorrespondenceSecondMatchFilter : public CorrespondenceFilter {
    CorrespondenceSecondMatchFilter(double distanceDifferenceThreshold = 0.5, double distanceRatioThreshold=-1.);
    virtual void compute(const CorrespondenceVector& correspondences);
    inline double distanceDifferenceThreshold() const { return _distanceDifferenceThreshold; }
    inline void setDistanceDifferenceThreshold(double distanceDifferenceThreshold_) { _distanceDifferenceThreshold = distanceDifferenceThreshold_; }
    inline double distanceRatioThreshold() const { return _distanceRatioThreshold; }
    inline void setDistanceRatioThreshold(double distanceRatioThreshold_) { _distanceRatioThreshold = distanceRatioThreshold_; }

  protected:
    double _distanceDifferenceThreshold;
    double _distanceRatioThreshold;
  };


  /**
     Tracker, manages the incremental tracking of features, frames and so on.
     It stores the state and  "owns" the internal structures.
   */
  struct FeatureTracker{
	  
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    FeatureTracker(OptimizableGraph* graph_, 
		   LandmarkConstructor* landmarkConstructor_, 
		   CorrespondenceFinder* correspondenceFinder_,
		   Matcher* matcher_);
  
    //! this adds a landmark to the pool, not to the graph
    void addLandmark(BaseTrackedLandmark* l);

    //! this removes a landmark from the pool and from all tracked features that were referencing it;
    void removeLandmark(BaseTrackedLandmark* l);

    void mergeLandmarks(BaseTrackedLandmark* l1, BaseTrackedLandmark* l2);

    //! this adds a landmark and all edges to the graph
    void confirmLandmark(BaseTrackedLandmark* /*f*/);

    //! this adds e new frame at the end of the history
    void addFrame(OptimizableGraph::Vertex* v, OptimizableGraph::Edge* odometry);

    //! this adds e new feature to the pool
    void addTrackedFeature(BaseTrackedFeature* feature);

    //! this removes a track of features from the pool
    // if they reference a landmark, also the landmark is removed
    // returns the numnber of tracked features removed
    int removeTrackedFeature(BaseTrackedFeature* feature, bool recursive, bool removeLandmarks);

    inline BaseFrame* lastFrame() {return _lastFrame;}
    inline const BaseFrame* lastFrame() const {return _lastFrame;}

    OptimizableGraph* graph() {return _graph;}
    const OptimizableGraph* graph() const {return _graph;}
    
    // look for the correspondences with odometry
    void searchInitialCorrespondences(CorrespondenceVector& correspondences, int numFrames=1);
    
    // do side effect by creating landmarks and connecting tracked features based on the set of correspondences passed
    void updateTracksAndLandmarks(const CorrespondenceVector& correspondences);


    // min num of times a feature should be seen and matched to instantiate a landmark
    inline int minLandmarkCreationFrames() const { return _minLandmarkCreationFrames; }
    inline void setMinLandmarkCreationFrames(int minLandmarkCreationFrames_) {_minLandmarkCreationFrames = minLandmarkCreationFrames_;}

    // not used :)
    inline int minLandmarkCommitFrames() const { return _minLandmarkCommitFrames; }
    inline void setMinLandmarkCommitFrames(int minLandmarkCommitFrames_) {_minLandmarkCommitFrames = minLandmarkCommitFrames_;}
    
    
    inline const VertexFrameMap& frames() const {return _frames;}
    inline VertexFrameMap& frames() {return _frames;}

    inline const BaseTrackedLandmarkSet& landmarks() const { return _landmarks; };
    inline BaseTrackedLandmarkSet& landmarks() { return _landmarks; };
    
    bool setNeighborFrames(BaseFrame* f1, BaseFrame* f2, bool connect);
    int refineConnectivity(BaseFrameSet& frames, int minCommonLandmarks, bool odometryIsGood=false);


    // utlities
    static void commonLandmarks(BaseTrackedLandmarkSet& common, BaseFrame* f1, BaseFrame* f2);
    static void selectFeaturesWithLandmarks(BaseTrackedFeatureSet& featureSet, BaseFrameSet& frameSet);
    static void selectFeaturesWithLandmarks(BaseTrackedFeatureSet& featureSet, BaseFrame* frame);
    BaseFrame* lastNFrames(BaseFrameSet& fset, int nFramesBack) ;
  protected:
    BaseFrame* _lastFrame;
    std::set<BaseTrackedLandmark*> _landmarks;
    VertexFrameMap _frames;
    OptimizableGraph* _graph;
  
    LandmarkConstructor*  _landmarkConstructor;
    CorrespondenceFinder* _correspondenceFinder;
    Matcher* _matcher;

    int _minLandmarkCreationFrames;
    int _minLandmarkCommitFrames;
    int _runningLandmarkId;
  };

}// end namespace

#endif
