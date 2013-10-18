#ifndef _ROS_LASER_DATA_MESSAGE_HANDLER_H_
#define _ROS_LASER_DATA_MESSAGE_HANDLER_H_
#include "g2o_frontend/boss_logger/blasersensor.h"
#include "ros_message_handler.h"
#include "sensor_msgs/LaserScan.h"

class RosLaserDataMessageHandler : public RosMessageHandler{
public:
  RosLaserDataMessageHandler(RosMessageContext* context, std::string topicName_);
  virtual ~RosLaserDataMessageHandler();
  virtual void subscribe();
  virtual void publish(boss_logger::BaseSensorData* sdata=0){}
  virtual void advertise(){}
  virtual bool configReady() const;
  void callback(const sensor_msgs::LaserScanConstPtr& scan);
  inline boss_logger::LaserSensor* sensor() {return _sensor;}
protected:
  ros::Subscriber _sub;
  std::string _topicName;
  boss_logger::LaserSensor* _sensor;
};

#endif
