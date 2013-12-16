#include <list>
#include <set>
#include "g2o_frontend/boss/serializer.h"
#include "g2o_frontend/boss/deserializer.h"
#include "g2o_frontend/boss_map/reference_frame.h"
#include "g2o_frontend/boss_map/reference_frame_relation.h"
#include "g2o_frontend/boss_map/image_sensor.h"
#include "g2o_frontend/boss_map/laser_sensor.h"
#include "g2o_frontend/boss_map/imu_sensor.h"
#include "g2o_frontend/boss_map/sensor_data_synchronizer.h"
#include "g2o_frontend/boss_map/robot_configuration.h"
#include "g2o_frontend/boss_map/map_manager.h"
#include "g2o_frontend/boss_map/sensing_frame_node.h"
#include "g2o_frontend/boss_map/map_node_processor.h"
#include "g2o_frontend/basemath/bm_se3.h"

#define MARKUSED(X)  X=X

/*#include "g2o_frontend/pwn_boss/pwn_sensor_data.h"*/

using namespace boss_map;
using namespace boss;
using namespace std;

//pwn_boss::PWNSensorData data;

const char* banner[]={
  "boss_synchronizer: takes a raw boss log and aggregates messages into single frames",
  " based on time constraints.",
  " If you want to make a frame out of two topics that have a time distance of 0.01 sec you should use",
  " the -sync option.",
  "   -sync:topic1:topic2:time",
  " Adding multiple sync  options results in having frames containing multiple informations.",
  " Messages that do not match the constraints are dropped."
  "",
  "usage: boss_synchronizer [options] filein fileout",
  "example: boss_synchronizer \\",
  " -sync:/kinect/rgb/image_color:/kinect/depth_registered/image_raw:0.05 \\",
  " -sync:/kinect/rgb/image_color:/imu/data:0.1 \\",
  " -sync:/kinect/rgb/image_color:/front_scan:0.1 \\",
  " test.log test_sync.log", 0
};

void printBanner (){
  int c=0;
  while (banner[c]){
    cerr << banner [c] << endl;
    c++;
  }
}

struct CommandArg{
  CommandArg(const std::string s){
    size_t i = 0;
    std::string trimmed=s;
    do{
      i =trimmed.find_first_of(':');
      values.push_back(trimmed.substr(0,i));
      trimmed = trimmed.substr(i+1,std::string::npos);
    } while (i != std::string::npos);

    cerr << "arg" << endl;
    for (i=0; i<values.size(); i++)
      cerr <<" " << values[i] << endl; 
  }

  int asInt(int i) const {
    return atoi(values[i].c_str());
  }
  float asFloat(int i) const {
    return atof(values[i].c_str());
  }

  bool asBool(int i) const {
    if (values[i] == "true")
      return true;
    if (values[i] == "false")
      return false;
    throw std::runtime_error("illegal bool value");
  }

  std::string asString(int i) const{
    return values[i];
  }

  std::vector<std::string> values;
};

int parseArgs(std::list<CommandArg>& parsedArgs, int argc, char** argv){
  int c = 1;
  parsedArgs.clear();
  while (c<argc){
    if (*argv[c]=='-') {
      CommandArg arg(CommandArg(argv[c]));
      parsedArgs.push_back(arg);
      c++;
    } else {
      return c;
    }
  }
  return c;
}

void handleParsedArgs(Synchronizer* sync, std::list<CommandArg> args){
  for(std::list<CommandArg>::iterator it = args.begin(); it!=args.end(); it++){
    CommandArg& arg = *it;
    if (arg.asString(0)!="-sync")
      continue;
  // if (arg.values.size()!=4)
  //     throw runtime_error("options must be in the form -sync:topic1:topic2:time");
    std::string topic1 = arg.asString(1);
    std::string topic2 = arg.asString(2);
    double time = arg.asFloat(3);
    cerr << "sync: " << topic1 << " " << topic2 << " " << time << endl;
    sync->addSyncTimeCondition(topic1,topic2,time);
  }
 
}


class SequentialMapNode: public boss_map::MapNodeAlias {
public:
  SequentialMapNode(MapNode* original_=0, MapManager* manager_=0, int id_=-1, IdContext* context_ = 0):
    MapNodeAlias(original_, manager_, id_, context_){
    _odometry = 0;
    _imu = 0;
    _previousNode = 0;
  }
  virtual void serialize(ObjectData& data, IdContext& context){
    MapNodeAlias::serialize(data,context);
    data.setPointer("imu",_imu);
    data.setPointer("odometry", _odometry);
    data.setPointer("previousNode", _previousNode);
  }

  //! boss deserialization
  virtual void deserialize(ObjectData& data, IdContext& context) {
    MapNodeAlias::deserialize(data, context);
    data.getReference("imu").bind(_imu);
    data.getReference("odometry").bind(_odometry);
    data.getReference("previousNode").bind(_previousNode);
  }

  //! odometry getter
  MapNodeBinaryRelation* odometry() const { return _odometry;}

  //! odometry setter
  void setOdometry(MapNodeBinaryRelation* odometry_)  { _odometry = odometry_;}

    //! imu getter
  MapNodeUnaryRelation* imu() const { return _imu;}

  //! imu setter
  void setImu(MapNodeUnaryRelation* imu_) { _imu = imu_;}

  // !previous node getter
  SequentialMapNode* previousNode() const {return _previousNode;}

  // !previous node setter
  void setPreviousNode(SequentialMapNode* previousNode_)  {_previousNode = previousNode_;}

protected:
  MapNodeBinaryRelation* _odometry;
  MapNodeUnaryRelation* _imu;
  SequentialMapNode* _previousNode;
};


class BaseTracker: public MapNodeProcessor {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  BaseTracker(MapManager* manager_,   RobotConfiguration* config_): MapNodeProcessor(manager_, config_){
    _keyNode = 0;
    _currentNode = 0;
    _globalTransform.setIdentity();
    _keyNodeTransform.setIdentity();
    _currentNodeTransform.setIdentity();
    useOdometry = true;
    useImu = false;
    _counter = 0;
  }

  virtual void process(Serializable*s){
    MapNode* node = dynamic_cast<MapNode*> (s);
    MapNodeRelation* rel = dynamic_cast<MapNodeRelation*>(s);

    if (! node && ! rel){
      _pendingObjects.push_back(s);
      return;
    }


    if (node){ 
      // flush all pending objects in the queue
      while (! _pendingObjects.empty()){
	Serializable* s = _pendingObjects.front();
	_pendingObjects.pop_front();
	put(s);
      }
      _pendingObjects.push_back(node);
      // get ready to assemble a new node
      _tempNode = node;
      _tempNodeImu = 0;
      _tempNodeOdometry = 0;
      _tempNodeTransform = _tempNode->transform();
    }

    if (rel) {
      if(useImu){
	MapNodeUnaryRelation* r = dynamic_cast<MapNodeUnaryRelation*>(s);
	if (r && r->nodes()[0] == _tempNode){
	  _tempNodeImu = r;
	}
      }
      if (useOdometry){
	MapNodeBinaryRelation* r = dynamic_cast<MapNodeBinaryRelation*>(s);
	if (r && r->nodes()[0] == _currentNode && r->nodes()[1] == _tempNode){
	  _tempNodeOdometry = r;
	  cerr << "odom1: " << _tempNodeOdometry << " odomTransform: " << t2v(_tempNodeOdometry->transform()).transpose() << endl;
	}
      }
      _pendingObjects.push_back(rel);
    }

    bool nodeReady = 
      // we need to have one node to process
      _tempNode &&
      // that should be different from the previous one
      _tempNode!=_currentNode &&
      // if we require the odometry, there should be one or we need to be in the first frame
      (!useOdometry|| (_tempNodeOdometry || !_currentNode)) && 
      // if we require the imu, we need to have one
      (!useImu||_tempNodeImu);
    
    if (!nodeReady)
      return;

    Eigen::Isometry3f currentGuess;
    currentGuess.setIdentity();
    // now we need to initialize the estimate of the node based on the odometry, if available
    if (_currentNode) {
      cerr << endl;
      Eigen::Isometry3d dt = _currentNodeTransform.inverse()*_tempNodeTransform;
      cerr << "dt: " << t2v(dt).transpose() << endl;
      if (_tempNodeOdometry && useOdometry) {
	dt = _tempNodeOdometry->transform();
      }
      _currentGuess = _globalTransform*dt
    } 

    // we can refine the estimate with the imu, if available
    if (_tempNodeImu && useImu){
      _currentGuess.linear() = _tempNodeImu->transform().linear();
    }
 
    _currentNode = _tempNode;
    _currentNodeTransform = _tempNodeTransform;
    _currentNodeOdometry = _tempNodeOdometry;
    _currentNodeImu = _tempNodeImu;


    // now we need to initialize the estimate of the node based on something
    if (_keyNode){
      Eigen::Isometry3d t;
      Eigen::Matrix<double, 6,6> info;
      MapNodeBinaryRelation* newRel;
      bool trackOk = track(newRel, t, info, _keyNode, _currentNode, _currentNodeOdometry, _keyNodeImu, _currentNodeImu);
      
      if (trackOk){
	//_globalTransform = _globalTransform * t;

	if (! (_counter%50) ) {
	  Eigen::Matrix3d R = _globalTransform.linear();
	  Eigen::Matrix3d E = R.transpose() * R;
	  E.diagonal().array() -= 1;
	  _globalTransform.linear() -= 0.5 * R * E;
	}
	_globalTransform.matrix().row(3) << 0,0,0,1;
	_counter ++;

	cerr << "_globalTransform: " << t2v(_globalTransform).transpose() << endl;
      } 

      if (newRel || !trackOk){
	_keyNode = _currentNode;
	_keyNodeTransform = _currentNodeTransform;
	_keyNodeOdometry = _currentNodeOdometry;
	_keyNodeImu = _currentNodeImu;
      }
      if (newRel){
	_pendingObjects.push_back(newRel);
	_manager->addRelation(newRel);
      }
    } else {
      _keyNode = _currentNode;
      _keyNodeTransform = _currentNodeTransform;
      _keyNodeOdometry = _currentNodeOdometry;
      _keyNodeImu = _currentNodeImu;
    }  

  }

protected:
  Eigen::Isometry3d _globalTransform;
  virtual bool track(MapNodeBinaryRelation*& rel,
		     Eigen::Isometry3d& transform, Eigen::Matrix<double, 6,6>& info,
		     MapNode* keyNode,
		     Eigen::Isometry3f* keyNodeTransform,
		     MapNode* currentNode,
		     Eigen::Isometry3f* currentNodeGuess,
		     const MapNodeBinaryRelation* currentNodeOdometry,
		     const MapNodeUnaryRelation* keyNodeImu,
		     const MapNodeUnaryRelation* currentNodeImu) {
    cerr << endl;
    cerr << __PRETTY_FUNCTION__ << endl;
    cerr << "keyNode: " << keyNode->seq() << " t:" << t2v(keyNode->transform()).transpose() << endl;
    cerr << "currentNode: " << currentNode->seq() << " t:" << t2v(currentNode->transform()).transpose() << endl;
    transform=_keyNode->transform().inverse()*_currentNode->transform();
    cerr << "transform"  << t2v(transform).transpose() << endl;
    if (_currentNodeOdometry){
      //cerr << "odom: " << t2v(currentNodeOdometry->transform()).transpose() << endl;
    }
    if (keyNodeImu){
      //cerr << "imu1: " << t2v(keyNodeImu->transform()).transpose() << endl;
    }
    if (currentNodeImu){
      //cerr << "imu1: " << t2v(currentNodeImu->transform()).transpose() << endl;
    }
    rel = new MapNodeBinaryRelation(_manager);
    rel->nodes()[0]=keyNode;
    rel->nodes()[1]=currentNode;
    info.setIdentity();
    rel->setInformationMatrix(info);
    rel->setTransform(transform);
    return true;
  }
  MapNode* _keyNode, *_currentNode, *_tempNode;
  Eigen::Isometry3d _keyNodeTransform, _currentNodeTransform, _tempNodeTransform;
  const MapNodeBinaryRelation* _keyNodeOdometry, *_currentNodeOdometry, *_tempNodeOdometry;
  const MapNodeUnaryRelation* _keyNodeImu, *_currentNodeImu, *_tempNodeImu;
  int _counter;
  bool useOdometry;
  bool useImu;
  std::deque<Serializable*> _pendingObjects;
};



int main(int argc, char** argv) {
  std::list<CommandArg> parsedArgs;
  if (argc == 1) {
    printBanner();
    return 0;
  }

  int c = parseArgs(parsedArgs, argc, argv);
  if (c<argc-2) {
    printBanner();
    return 0;
  }
    
  

  // create a synchronizer
  Synchronizer sync;

  handleParsedArgs(&sync, parsedArgs);
  std::string filein = argv[c];
  std::string fileout = argv[c+1];
  Deserializer des;
  des.setFilePath(filein.c_str());

  sync.addSyncTopic("/camera/depth_registered/image_rect_raw");

  Serializer ser;
  ser.setFilePath(fileout.c_str());
  
  cerr <<  "running logger with arguments: filein[" << filein << "] fileout: [" << fileout << "]" << endl;

  std::vector<BaseSensorData*> sensorDatas;
  RobotConfiguration* conf = readLog(sensorDatas, des);
  cerr << "# frames: " << conf->frameMap().size() << endl;
  cerr << "# sensors: " << conf->sensorMap().size() << endl;
  cerr << "# sensorDatas: " << sensorDatas.size() << endl;

  conf->serializeInternals(ser);
  ser.writeObject(*conf);
  TSCompare comp;
  std::sort(sensorDatas.begin(), sensorDatas.end(), comp);

  MapManager* manager = new MapManager();
  ser.writeObject(*manager);

  
  SensingFrameNodeMaker* nodeMaker = new SensingFrameNodeMaker();
  nodeMaker->init(manager,conf);
  StreamProcessor::PropagatorOutputHandler* sync2nm=new StreamProcessor::PropagatorOutputHandler(&sync, nodeMaker);

  OdometryRelationAdder* odometryAdder = new OdometryRelationAdder(manager,conf);
  StreamProcessor::PropagatorOutputHandler* nm2odom=new StreamProcessor::PropagatorOutputHandler(nodeMaker, odometryAdder);
  
  ImuRelationAdder* imuAdder = new ImuRelationAdder(manager,conf);
  StreamProcessor::PropagatorOutputHandler* odom2imu=new StreamProcessor::PropagatorOutputHandler(odometryAdder, imuAdder);

  
  BaseTracker* baseTracker = new BaseTracker(manager,conf);
  StreamProcessor::PropagatorOutputHandler* imu2tracker=new StreamProcessor::PropagatorOutputHandler(imuAdder, baseTracker);
  MARKUSED(imu2tracker);
  
  StreamProcessor::WriterOutputHandler* writer = new StreamProcessor::WriterOutputHandler(baseTracker, &ser);

  MARKUSED(sync2nm);
  MARKUSED(nm2odom);
  MARKUSED(odom2imu);
  MARKUSED(writer);

  for (size_t i = 0; i< sensorDatas.size(); i++){
    BaseSensorData* data = sensorDatas[i];
    sync.process(data);
  }

}
