#include "pwn_tracker.h"

namespace pwn_tracker{

  using namespace std;
  using namespace pwn;
  using namespace boss;

  PwnTrackerFrame::PwnTrackerFrame(MapManager* manager, int id, IdContext* context):
    MapNode ( manager, id, context) {
    sensorOffset.setIdentity();
    scale=1;
  }
  
  void PwnTrackerFrame::serialize(ObjectData& data, IdContext& context) {
    MapNode::serialize(data,context);
    sensorOffset.matrix().toBOSS(data, "sensorOffset");
    cameraMatrix.toBOSS(data, "cameraMatrix");
    data.setInt("imageRows", imageRows);
    data.setInt("imageCols", imageCols);

    ObjectData* depthImageData=new ObjectData();
    data.setField("depthImage", depthImageData);
    depthImage.serialize(*depthImageData,context);

    ObjectData* depthThumbnailData=new ObjectData();
    data.setField("depthThumbnail", depthThumbnailData);
    depthThumbnail.serialize(*depthThumbnailData,context);


    ObjectData* normalThumbnailData=new ObjectData();
    data.setField("normalThumbnail", normalThumbnailData);
    normalThumbnail.serialize(*normalThumbnailData,context);

    // ObjectData* cloudData=new ObjectData();
    // data.setField("cloud", cloudData);
    // cloud.serialize(*cloudData,context);
  }

  
  void PwnTrackerFrame::deserialize(ObjectData& data, IdContext& context) {
    MapNode::deserialize(data,context);
    sensorOffset.matrix().fromBOSS(data,"sensorOffset");
    cameraMatrix.fromBOSS(data, "cameraMatrix");
    imageRows = data.getInt("imageRows");
    imageCols = data.getInt("imageCols");

    ObjectData* depthImageData = static_cast<ObjectData*>(data.getField("depthImage"));
    depthImage.deserialize(*depthImageData, context);

    
    ObjectData* normalThumbnailData = static_cast<ObjectData*>(data.getField("normalThumbnail"));
    normalThumbnail.deserialize(*normalThumbnailData,context);

    ObjectData* depthThumbnailData = static_cast<ObjectData*>(data.getField("depthThumbnail"));
    depthThumbnail.deserialize(*depthThumbnailData,context);

    // ObjectData* cloudData = static_cast<ObjectData*>(data.getField("cloud"));
    // cloud.deserialize(*cloudData,context);
  }

  PwnTrackerRelation::PwnTrackerRelation(MapManager* manager, int id, IdContext* context):
    MapNodeBinaryRelation(manager, id, context){
    _nodes.resize(2);
    inliers = 0;
    error = 0;
  }
  
  void PwnTrackerRelation::serialize(ObjectData& data, IdContext& context){
    MapNodeBinaryRelation::serialize(data,context);
    data.setInt("inliers", inliers);
    data.setFloat("error", error);
  }
    //! boss deserialization
  void PwnTrackerRelation::deserialize(ObjectData& data, IdContext& context){
    MapNodeBinaryRelation::deserialize(data,context);
    inliers = data.getInt("inliers");
    error = data.getFloat("error");
  }
    //! called when all links are resolved, adjusts the bookkeeping of the parents


  PwnTracker::PwnTracker(pwn::Aligner* aligner, pwn::DepthImageConverter* converter, boss_map::MapManager* manager) {
    _previousCloud = 0;
    _globalT.setIdentity();
    _previousCloudTransform.setIdentity();
    _scale = 4;
    _counter = 0;
    _numKeyframes = 0;
    this->_aligner = aligner;
    this->_converter = converter;
    this->_manager = manager;
  }

  void PwnTracker::init(){
    if (_previousTrackerFrame)
      delete _previousTrackerFrame;
    if (_previousCloud)
      delete _previousCloud;
    
    _previousCloud = 0;
    _globalT.setIdentity();
    _previousCloudTransform.setIdentity();
    _scale = 4;
    _counter = 0;
    _numKeyframes = 0;
    initCallback();
  }
  void PwnTracker::makeThumbnails(cv::Mat& depthThumbnail, cv::Mat& normalThumbnail, 
				  Frame* f, int r, int c, 
				  const Eigen::Isometry3f& offset, 
				  const Eigen::Matrix3f& cameraMatrix,
				  float scale){

    PinholePointProjector proj;
    proj.setImageSize(r,c);
    proj.setCameraMatrix(cameraMatrix);
    proj.scale(scale);
    pwn::IntImage indices(proj.imageRows(), proj.imageCols());
    pwn::DepthImage depthBuffer(proj.imageRows(), proj.imageCols());
    proj.setTransform(offset);
    proj.project(indices, depthBuffer, f->points());
    normalThumbnail = cv::Mat(proj.imageCols(), proj.imageRows(), CV_8UC3);
    depthThumbnail = cv::Mat(proj.imageCols(), proj.imageRows(), CV_16UC1);
    depthBuffer.toCvMat(depthThumbnail);
    for (int i = 0; i<indices.cols(); i++)
      for (int j = 0; j<indices.rows(); j++){
    	cv::Vec3b& pixel = normalThumbnail.at<cv::Vec3b>(i,j);
    	int idx = indices(j,i);
    	pixel[0] = 0;
    	pixel[1] = 0;
    	pixel[2] = 0;
    	if (idx==0)
    	  continue;
    	const pwn::Normal& n = f->normals()[idx];
    	if (n.squaredNorm()<0.1)
    	  continue;
    	pixel[0] = 127*(n.x()-0.5);
    	pixel[1] = 127*(n.y()-0.5);
    	pixel[2] = 127*(n.z()-0.5);
      }
  }

  pwn::Frame* PwnTracker::makeCloud(int& r, int& c, Eigen::Matrix3f& cameraMatrix,
				    const Eigen::Isometry3f& sensorOffset,  const DepthImage& depthImage) {
    PinholePointProjector* projector=dynamic_cast<PinholePointProjector*>(_converter->_projector);

    cameraMatrix(2,2)=1;
    projector->setCameraMatrix(cameraMatrix);
    projector->setImageSize(depthImage.rows(), depthImage.cols());

    DepthImage scaledImage;
    DepthImage::scale(scaledImage,depthImage,_scale);
    projector->scale(1.0/_scale);
    cameraMatrix=projector->cameraMatrix();
    r = projector->imageRows();
    c = projector->imageCols();
    pwn::Frame* cloud = new pwn::Frame;
    _converter->compute(*cloud,scaledImage, sensorOffset);
    // scale image and camera matrix
    return cloud;
  }

  void PwnTracker::processFrame(const DepthImage& depthImage, const Eigen::Isometry3f& sensorOffset, const Eigen::Matrix3f& cameraMatrix){
    int r,c;
    Eigen::Matrix3f scaledCameraMatrix = cameraMatrix;
    pwn::Frame* cloud = makeCloud(r,c,scaledCameraMatrix,sensorOffset,depthImage);

    bool newFrame = false;
    bool newRelation = false;
    PwnTrackerFrame* currentTrackerFrame=0;
    Eigen::Isometry3d relationMean;
    if (_previousCloud){
      _aligner->setCurrentSensorOffset(sensorOffset);
      _aligner->setCurrentFrame(cloud);
      
      _aligner->correspondenceFinder()->setImageSize(r,c);
      PinholePointProjector* alprojector=(PinholePointProjector*)(_aligner->projector());
      alprojector->setCameraMatrix(scaledCameraMatrix);
      alprojector->setImageSize(r,c);

      Eigen::Isometry3f guess=_previousCloudTransform.inverse()*_globalT;
      _aligner->setInitialGuess(guess);
      _aligner->align();
 
      // cerr << "inliers: " << _aligner->inliers() << endl;
      // cerr << "chi2: " << _aligner->error() << endl;
      // cerr << "chi2/inliers: " << _aligner->error()/_aligner->inliers() << endl;
      // cerr << "initialGuess: " << t2v(guess).transpose() << endl;
      // cerr << "transform   : " << t2v(_aligner->T()).transpose() << endl;
      if (_aligner->inliers()>-1){
    	_globalT = _previousCloudTransform*_aligner->T();
    	//cerr << "TRANSFORM FOUND" <<  endl;
      } else {
    	//cerr << "FAILURE" <<  endl;
    	_globalT = _globalT*guess;
      }
      convertScalar(relationMean, _aligner->T());
      if (! (_counter%50) ) {
    	Eigen::Matrix3f R = _globalT.linear();
    	Eigen::Matrix3f E = R.transpose() * R;
    	E.diagonal().array() -= 1;
    	_globalT.linear() -= 0.5 * R * E;
      }
      _globalT.matrix().row(3) << 0,0,0,1;
 
      newAlignmentCallback(_globalT, _aligner->T(), _aligner->inliers(), _aligner->error());
			   
      int maxInliers = r*c;
      float inliersFraction = (float) _aligner->inliers()/(float) maxInliers;
      if (inliersFraction<0.6){
	newFrame = true;
	newRelation = true;

	// char filename[1024];
	// sprintf (filename, "frame-%05d.pwn", _numKeyframes);
	// frame->save(filename,1,true,_globalT);

	_numKeyframes ++;
	delete _previousCloud;
	_aligner->setReferenceSensorOffset(sensorOffset);
	_aligner->setReferenceFrame(cloud);
	_previousCloud = cloud;
	_previousCloudTransform = _globalT;
	// cerr << "new frame added (" << _numKeyframes <<  ")" << endl;
	// cerr << "inliers: " << _aligner->inliers() << endl;
	// cerr << "maxInliers: " << maxInliers << endl;
	// cerr << "chi2: " << _aligner->error() << endl;
	// cerr << "chi2/inliers: " << _aligner->error()/_aligner->inliers() << endl;
	// cerr << "initialGuess: " << t2v(guess).transpose() << endl;
	// cerr << "transform   : " << t2v(_aligner->T()).transpose() << endl;
	// cerr << "globalTransform   : " << t2v(_globalT).transpose() << endl;
      } else { // previous frame but offset is small
	delete cloud;
      }
    } else { // first frame
      //ser.writeObject(*manager);
      newFrame = true;
      _aligner->setReferenceSensorOffset(sensorOffset);
      _aligner->setReferenceFrame(cloud);
      _previousCloud = cloud;
      _previousCloudTransform = _globalT;

      _numKeyframes ++;
      /*Eigen::Isometry3f t = _globalT;
	geometry_msgs::Point p;
	p.x = t.translation().x();
	p.y = t.translation().y();
	p.z = t.translation().z();
	m_odometry.points.push_back(p);
      */
    }
    _counter++;

    if (newFrame) {
      //cerr << "maing new frame, previous: " << _previousTrackerFrame << endl;
      currentTrackerFrame = new PwnTrackerFrame(_manager);
      //currentTrackerFrame->cloud.set(cloud);

      boss_logger::ImageBLOB* depthBLOB=new boss_logger::ImageBLOB();
      depthImage.toCvMat(depthBLOB->cvImage());
      depthBLOB->adjustFormat();
      currentTrackerFrame->depthImage.set(depthBLOB);
      
      boss_logger::ImageBLOB* normalThumbnailBLOB=new boss_logger::ImageBLOB();
      boss_logger::ImageBLOB* depthThumbnailBLOB=new boss_logger::ImageBLOB();
      makeThumbnails(depthThumbnailBLOB->cvImage(), normalThumbnailBLOB->cvImage(), cloud, 
		    depthImage.rows(), 
		    depthImage.cols(),
		    sensorOffset,
		    cameraMatrix, 
		    0.1);
      normalThumbnailBLOB->adjustFormat();
      depthThumbnailBLOB->adjustFormat();
      
      currentTrackerFrame->depthThumbnail.set(depthThumbnailBLOB);
      currentTrackerFrame->normalThumbnail.set(normalThumbnailBLOB);
      currentTrackerFrame->cameraMatrix = cameraMatrix;
      currentTrackerFrame->imageRows = depthImage.rows();
      currentTrackerFrame->imageCols = depthImage.cols();
      Eigen::Isometry3d _iso;
      convertScalar(_iso, _globalT);
      currentTrackerFrame->setTransform(_iso);
      convertScalar(currentTrackerFrame->sensorOffset, sensorOffset);
      _manager->addNode(currentTrackerFrame);
      newFrameCallback(currentTrackerFrame);
      //ser.writeObject(*currentTrackerFrame);
      //cerr << "saved frame" << currentTrackerFrame << endl;
      delete depthThumbnailBLOB;
      delete normalThumbnailBLOB;
      delete depthBLOB;
    }

    if (newRelation) {
      PwnTrackerRelation* rel = new PwnTrackerRelation(_manager);
      rel->setTransform(relationMean);
      rel->setInformationMatrix(Eigen::Matrix<double, 6,6>::Identity());
      rel->setTo(currentTrackerFrame);
      rel->setFrom(_previousTrackerFrame);
      //cerr << "saved relation" << _previousTrackerFrame << " " << currentTrackerFrame << endl;
      _manager->addRelation(rel);
      newRelationCallback(rel);
      //ser.writeObject(*rel);
    }

    if (currentTrackerFrame) {
      _previousTrackerFrame = currentTrackerFrame;
    }
  }

  PwnTracker::~PwnTracker() {
    init();
  }  


  std::vector<Serializable*> readConfig(Aligner*& aligner, DepthImageConverter*& converter, const std::string& configFile){
    aligner = 0;
    converter = 0;
    Deserializer des;
    des.setFilePath(configFile);
    Serializable* s;
    std::vector<Serializable*> instances;
    cerr << "Reading" << endl;
    while ((s=des.readObject())){
      instances.push_back(s);
      Aligner* al=dynamic_cast<Aligner*>(s);
      if (al) {
	cerr << "got aligner" << endl;
	aligner = al;
      }
      DepthImageConverter* conv=dynamic_cast<DepthImageConverter*>(s);
      if  (conv) {      
	cerr << "got converter" << endl;
	converter = conv;
      }
    }
    if (aligner) {
      cerr << "alpp: " << aligner->projector() << endl;
      cerr << "allz: " << aligner->linearizer() << endl;
      if (aligner->linearizer())
	cerr << "lzal: " << aligner->linearizer()->aligner() << endl;
    
    }

    return instances;
  }

BOSS_REGISTER_CLASS(PwnTrackerFrame);
BOSS_REGISTER_CLASS(PwnTrackerRelation);


}

#if 0


bool PwnTracker::retrieveImageParameters(Eigen::Isometry3f& sensorOffset, 
					 Eigen::Matrix3f& cameraMatrix, 
					 const sensor_msgs::Image::ConstPtr& img,
					 const sensor_msgs::CameraInfo::ConstPtr& info){
   sensorOffset = Eigen::Isometry3f::Identity();
    try{
      tf::StampedTransform t;
      _tfListener->lookupTransform(_base_frame_id, img->header.frame_id, img->header.stamp, t);
      sensorOffset.translation().x()=t.getOrigin().x();
      sensorOffset.translation().y()=t.getOrigin().y();
      sensorOffset.translation().z()=t.getOrigin().z();
      Eigen::Quaternionf rot;
      rot.x()=t.getRotation().x();
      rot.y()=t.getRotation().y();
      rot.z()=t.getRotation().z();
      rot.w()=t.getRotation().w();
      sensorOffset.linear()=rot.toRotationMatrix();
    }
    catch (tf::TransformException ex){
      ROS_ERROR("%s",ex.what());
      return false;
    }
    int i=0;
    for (int r=0; r<3; r++)
      for (int c=0; c<3; c++, i++)
    	cameraMatrix(r,c) = info->K[i];
    cameraMatrix(2,2) = 1;
    return true;
  }


void PwnTracker::broadcastTransform(const sensor_msgs::Image::ConstPtr& img){
  geometry_msgs::TransformStamped st;
  {
    st.header.frame_id = "/world";
    st.child_frame_id =  _base_frame_id;
    st.header.stamp = img->header.stamp;
    st.transform.translation.x = _globalT.translation().x();
    st.transform.translation.y = _globalT.translation().y();
    st.transform.translation.z = _globalT.translation().z();
    Eigen::Quaternionf rot(_globalT.linear());
    st.transform.rotation.x = rot.x();
    st.transform.rotation.y = rot.y();
    st.transform.rotation.z = rot.z();
    st.transform.rotation.w = rot.w();
    _tfBroadcaster->sendTransform(st);
  }
}
  void PwnTracker::subscribe(){
    _cameraSubscriber = new image_transport::CameraSubscriber(_imageTransport->subscribeCamera(_topic,1,boost::bind(&PwnTracker::callback,this, _1, _2)));
    cerr << "subscribed image: [" <<  _topic << "]" << endl;
  }


#endif
