#include "pwn_closer.h"
#include "g2o_frontend/pwn_core/pwn_static.h"

namespace pwn_tracker {

  PwnCloserRelation::PwnCloserRelation(MapManager* manager, int id, IdContext* context):
    PwnTrackerRelation(manager, id, context){

    normalDifference = 0;
    depthDifference = 0;
    reprojectionDistance = 0;
    nonZeros = 0;
    outliers = 0;
    inliers = 0;
  }

  void PwnCloserRelation::serialize(ObjectData& data, IdContext& context){
    PwnTrackerRelation::serialize(data,context);
    data.setFloat("normalDifference", normalDifference);
    data.setFloat("depthDifference", depthDifference);
    data.setFloat("reprojectionDistance", reprojectionDistance);
    data.setInt("nonZeros", nonZeros);
    data.setInt("outliers", outliers);
    data.setInt("inliers", inliers);

    data.setBool("accepted", accepted);
    data.setInt("consensusCumInlier", consensusCumInlier);
    data.setInt("consensusCumOutlierTimes", consensusCumOutlierTimes);
    data.setInt("consensusTimeChecked", consensusTimeChecked);
  }

  void PwnCloserRelation::deserialize(ObjectData& data, IdContext& context){
    PwnTrackerRelation::deserialize(data,context);
    normalDifference = data.getFloat("normalDifference");
    depthDifference = data.getFloat("depthDifference");
    reprojectionDistance = data.getFloat("reprojectionDistance");
    nonZeros = data.getInt("nonZeros");
    outliers = data.getInt("outliers");
    inliers  = data.getInt("inliers");

    accepted = data.getBool("accepted");
    consensusCumInlier=data.getInt("consensusCumInlier");
    consensusCumOutlierTimes = data.getInt("consensusCumOutlierTimes");
    consensusTimeChecked = data.getInt("consensusTimeChecked");
  }


  PwnCloser::PwnCloser(pwn::Aligner* aligner_, 
		       pwn::DepthImageConverter* converter_,
		       MapManager* manager_,
		       PwnCache* cache_) : MapCloser(manager_), PwnMatcherBase(aligner_, converter_){
    //_aligner = aligner_;
    //_converter = converter_;
    _cache = cache_;
    //_frameInlierDepthThreshold = 50;
    _frameMinNonZeroThreshold = 3000;// was 3000
    _frameMaxOutliersThreshold = 100;
    _frameMinInliersThreshold = 1000; // was 1000
    _debug = false;
    _selector = new PwnCloserActiveRelationSelector(_manager);
    setScale(4);
    updateCache();
  }


  PwnCloserActiveRelationSelector::PwnCloserActiveRelationSelector(boss_map::MapManager* manager): MapRelationSelector(manager){}

  bool PwnCloserActiveRelationSelector::accept(MapNodeRelation* r) {
    if (!r)
      return false;
    {
      PwnCloserRelation* _r = dynamic_cast<PwnCloserRelation*>(r);
      if (_r){
	return _r->accepted;
      }
    }
    return dynamic_cast<PwnTrackerRelation*>(r);
  }
  


  void PwnCloser::processPartition(std::list<MapNodeBinaryRelation*>& newRelations, 
				   std::set<MapNode*>& otherPartition, 
				   MapNode* current_){
    PwnTrackerFrame* current = dynamic_cast<PwnTrackerFrame*>(current_);
    if (otherPartition.count(current)>0)
      return;
    Eigen::Isometry3d iT=current->transform().inverse();
    PwnCache::HandleType f_handle=_cache->get(current);
    current->cloud=f_handle.get();
    //cerr << "FRAME: " << current->seq << endl; 
    for (std::set <MapNode*>::iterator it=otherPartition.begin(); it!=otherPartition.end(); it++){
      PwnTrackerFrame* other = dynamic_cast<PwnTrackerFrame*>(*it);
      if (other==current)
	continue;

      PwnCache::HandleType f2_handle=_cache->get(other);
      other->cloud=f2_handle.get();
	
      Eigen::Isometry3d ig=iT*other->transform();
      PwnCloserRelation* rel = matchFrames(current, other, ig);
      //cerr << "  framesMatched: " << rel << " dc:"  << dc << " nc:" << nc << endl;
      if (rel) {
	rel->depthDifference = 0;
	rel->normalDifference = 0;

	cerr << "o";
	newRelations.push_back(rel);
      } else 
	cerr << ".";
    }
    cerr << endl;
  }

  
  
  PwnCloserRelation* PwnCloser::matchFrames(PwnTrackerFrame* from, PwnTrackerFrame* to, 
					    const Eigen::Isometry3d& initialGuess){
    if (from == to)
      return 0; 
    MatcherResult result;			
    // Eigen::Isometry3d fromOffset, toOffset;
    // convertScalar(fromOffset, from->sensorOffset);
    // convertScalar(toOffset, to->sensorOffset);
    
    // Eigen::Matrix3d fromCamera, toCamera;
    // convertScalar(fromCamera, from->cameraMatrix);
    // convertScalar(toCamera, to->cameraMatrix);


    matchClouds(result, 
		from->cloud, to->cloud, 
		from->sensorOffset, to->sensorOffset, 
		to->cameraMatrix,
		to->imageRows,to->imageCols,
		initialGuess);
    if(result.image_nonZeros < _frameMinNonZeroThreshold ||
       result.image_outliers > _frameMaxOutliersThreshold || 
       result.image_inliers  < _frameMinInliersThreshold)
      return 0;

    PwnCloserRelation* rel = new PwnCloserRelation(_manager);
    rel->setFrom(from);
    rel->setTo(to);
    rel->setTransform(result.transform);
    rel->setInformationMatrix(result.informationMatrix);
    rel->nonZeros = result.image_nonZeros;
    rel->outliers = result.image_outliers;
    rel->inliers  = result.image_inliers;
    return rel;
    /*

    //cerr <<  "  matching  frames: " << from->seq << " " << to->seq << endl;
    
  
    Eigen::Isometry3f fromOffset, toOffset;
    Eigen::Matrix3f fromCameraMatrix, toCameraMatrix;

    convertScalar(fromOffset, from->sensorOffset);
    convertScalar(fromCameraMatrix, from->cameraMatrix);
    convertScalar(toOffset, to->sensorOffset);
    convertScalar(toCameraMatrix, to->cameraMatrix);
    
    PinholePointProjector* projector = dynamic_cast<PinholePointProjector*>(_aligner->projector());
    int r, c;
    
    PwnCloserRelation* rel = new PwnCloserRelation(_manager);
    _aligner->setReferenceSensorOffset(fromOffset);
    _aligner->setCurrentSensorOffset(toOffset);
    Eigen::Isometry3f ig;
    convertScalar(ig, initialGuess);
    ig.translation().z() = 0;
    _aligner->setInitialGuess(ig);
    //cerr << "initialGuess: " << t2v(ig).transpose() << endl;
    projector->setCameraMatrix(toCameraMatrix);
    projector->setImageSize(to->imageRows,to->imageCols);
    projector->scale(1./_scale);
    
    // cerr << "cameraMatrix: " << endl;
    // cerr << projector->cameraMatrix() << endl;
    r = projector->imageRows();
    c = projector->imageCols();
    // char dbgName[1024];
    // sprintf(dbgName, "match-%06d-%06d",from->seq, to->seq);
    // _aligner->debugPrefix()=dbgName;
    _aligner->correspondenceFinder()->setImageSize(r,c);
    _aligner->setReferenceFrame(fromCloud);
    _aligner->setCurrentFrame(toCloud);
    _aligner->align();
    //_aligner->debugPrefix()=""; FICSMI

    // cerr << "_fromCloud.points():" << fromCloud->points().size() << endl;
    // cerr << "_toCloud.points():" << toCloud->points().size() << endl;
    cerr << "AlInliers: " << _aligner->inliers() << endl;
    rel->setFrom(from);
    rel->setTo(to);
    Eigen::Isometry3d relationMean;
    convertScalar(relationMean, _aligner->T());
    rel->setTransform(relationMean);

    Matrix6d omega;
    convertScalar(omega, _aligner->omega());
    omega.setIdentity(); //HACK
    omega*=100;
    rel->setInformationMatrix(omega);
    rel->setTo(to);
    rel->setFrom(from);
    scoreMatch(rel);
    if ((rel->nonZeros<_frameMinNonZeroThreshold) || 
	(rel->outliers>_frameMaxOutliersThreshold) || 
	(rel->inliers<_frameMinInliersThreshold)) {
      delete rel;
      return 0;
    }
    return rel;
    */
  }

  void PwnCloser::updateCache(){
    _cache->setConverter(_converter);
    _cache->setScale(_scale);
  }

  float  PwnCloser::compareNormals(cv::Mat& m1, cv::Mat& m2){
    if (m1.rows!=m2.rows || m1.cols != m2.cols)
      return 0;
    return norm(abs(m1-m2));
  }
  
  float PwnCloser::compareDepths(cv::Mat& m1, cv::Mat& m2){
    if (m1.rows!=m2.rows || m1.cols != m2.cols)
      return 0;
    // consides only the pixels that are >0;
    cv::Mat mask = (m1>0) & (m2>0);
    //cerr << "mask!" << endl;
    
    mask.convertTo(mask, m1.type());
    //cerr << "convert" << endl;

    cv::Mat diff = (m1-m2)&mask;

    //cerr << "diffMask" << endl;

    diff.convertTo(diff, CV_32FC1);
    return norm(diff);
  }

// closure actions

  NewFrameCloserAdder::NewFrameCloserAdder(PwnCloser* closer, PwnTracker* tracker):
    PwnTracker::NewFrameAction(tracker){
    _closer = closer;
  }
  void NewFrameCloserAdder::compute (PwnTrackerFrame* frame) {
    _closer->addKeyNode(frame);
  }


  CloserRelationAdder::CloserRelationAdder(std::list<Serializable*>& objects_,
		      PwnCloser* closer, 
		      MapG2OReflector* optimizer_, 
		      PwnTracker* tracker):
    PwnTracker::NewRelationAction(tracker),
    _objects(objects_) {
    _closer = closer;
    _optimizer = optimizer_;
  }

  void CloserRelationAdder::compute (PwnTrackerRelation* relation) {
    _closer->addRelation(relation);
    cerr << "CLOSER PARTITIONS: " << _closer->partitions().size() << endl;
    int cr=0;
    for(std::list<MapNodeBinaryRelation*>::iterator it=_closer->committedRelations().begin();
	it!=_closer->committedRelations().end(); it++){
      _objects.push_back(*it);
      cr++;
    }
    if (cr){
      cerr << "COMMITTED RELATIONS: " << cr << endl;
      _optimizer->optimize();
      // char fname[100];
      // sprintf(fname, "out-%05d.g2o", lastFrameAdded->seq);
      // optimizer->save(fname);
    }
  }

  BOSS_REGISTER_CLASS(PwnCloserRelation);
}


