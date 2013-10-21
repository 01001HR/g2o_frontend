#ifndef _ROS_IMU_DATA_MESSAGE_HANDLER_H_
#define _ROS_IMU_DATA_MESSAGE_HANDLER_H_
#include "g2o_frontend/boss_logger/bimusensor.h"
#include "ros_message_handler.h"
#include "sensor_msgs/Imu.h"

class RosIMUDataMessageHandler : public RosMessageHandler{
public:
  RosIMUDataMessageHandler(RosMessageContext* context, std::string topicName_);
  virtual ~RosIMUDataMessageHandler();
  virtual void subscribe();
  virtual void publish(boss_logger::BaseSensorData* sdata=0){}
  virtual void advertise(){}
  virtual bool configReady() const;
  void callback(const sensor_msgs::ImuConstPtr& imu);
  inline boss_logger::IMUSensor* sensor() {return _sensor;}
protected:
  ros::Subscriber _sub;
  std::string _topicName;
  boss_logger::IMUSensor* _sensor;
};

#endif
