#include "ros_message_context.h"
#include "ros_transform_message_handler.h"
#include "ros_pinholeimagedata_message_handler.h"
#include "ros_laser_message_handler.h"
#include "ros_imu_data_message_handler.h"

RosMessageContext::RosMessageContext(ros::NodeHandle* nh_) {
  _nh = nh_;
  _transformHandler = new RosTransformMessageHandler(this);
  _tfListener = new tf::TransformListener(*_nh, ros::Duration(30.0));
  _odomReferenceFrameId = "/odom";
  _baseReferenceFrameId = "/base_link";
}

RosMessageContext::~RosMessageContext(){
  delete _tfListener;
}

bool RosMessageContext::getOdomPose(Eigen::Isometry3d& _trans, double time){
  bool transformFound = true;
  _tfListener->waitForTransform(_odomReferenceFrameId, _baseReferenceFrameId,
				ros::Time(time), ros::Duration(1.0));
  try{
    tf::StampedTransform t;
    _tfListener->lookupTransform(_odomReferenceFrameId, _baseReferenceFrameId,
				 ros::Time(time), t);
    Eigen::Isometry3d transform;
    transform.translation().x()=t.getOrigin().x();
    transform.translation().y()=t.getOrigin().y();
    transform.translation().z()=t.getOrigin().z();
    Eigen::Quaterniond rot;
    rot.x()=t.getRotation().x();
    rot.y()=t.getRotation().y();
    rot.z()=t.getRotation().z();
    rot.w()=t.getRotation().w();
    transform.linear()=rot.toRotationMatrix();
    _trans = transform;
    transformFound = true;
  }
  catch (tf::TransformException ex){
    ROS_ERROR("%s",ex.what());
    transformFound = false;
  }
  return transformFound;
}


RosMessageHandler* RosMessageContext::handler(const std::string topic) {
  std::map<std::string, RosMessageHandler*>::iterator it = _handlers.find(topic);
  if (it==_handlers.end())
    return 0;
  return it->second;
}

bool RosMessageContext::addHandler(const std::string& type, const std::string& topic) {
  if (type == "laser"){
    cerr <<"added handler for topic" << topic << endl;
    _handlers.insert(make_pair(topic, new RosLaserDataMessageHandler(this,topic)));
    return true;
  }
  if (type == "image"){
    cerr << "added handler for topic" << topic << endl;
    _handlers.insert(make_pair(topic, new RosPinholeImageDataMessageHandler(this,topic)));
    return true;
  }
  if (type == "imu"){
    cerr << "added handler for topic" << topic << endl;
    _handlers.insert(make_pair(topic, new RosIMUDataMessageHandler(this,topic)));
    return true;
  }
  cerr << "unknown handler type [" << type << "]" <<  endl;
  return false;
}

bool RosMessageContext::configReady() const{
  bool allReady=true;
  for (std::map<std::string, RosMessageHandler*>::const_iterator it = _handlers.begin(); it!=_handlers.end(); it++){
    const RosMessageHandler* handler=it->second;
    allReady &= handler->configReady();
  }
  return allReady;
}

void RosMessageContext::init(){
  _transformHandler->subscribe();
  for (std::map<std::string, RosMessageHandler*>::iterator it = _handlers.begin(); it!=_handlers.end(); it++){
    RosMessageHandler* handler=it->second;
    handler->subscribe();
  }
}
