#include <exception>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <Eigen/Core>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <boost/thread/thread.hpp>
#include <g2o/stuff/command_args.h>
#include "roboteye.h"

#include <ros/ros.h>
#include <robot_eye_driver/RobotEyeScan.h>

//msg_gen/cpp/include/

using namespace ocular;
using namespace std;
using namespace roboteye;

// parameters
string outfilename;
string _sensor_IP = "169.254.111.100";
int _seconds;
double _az_rate;
double _N_lines;
double _laser_freq;
double _averaging;
bool _intensity;

// global variables
AngleCB _angle_callback;
LaserCB _laser_callback;
NotificationCB _notif_callback;
ocular::RE05Driver* laserone;

///inside a class??
//diagnostic_updater::DiagnosedPublisher<robot_eye_driver::RobotEyeScan> _scan_pub;
ros::Publisher _scan_pub;

robot_eye_driver::RobotEyeScan _scan_msg;
double _desired_freq;
roboteye::RobotEyeScan _scan;
roboteye::RobotEyeConfig _roboteye_config;


void printandwriteLaserData(){

    /// printing the laser data
    //to prevent possible later callbacks
    //    _mutex_meas.lock();

    /// debug
//    cerr << "output ALL" << endl;
//    for(unsigned int i = 0; i < _meas_all.size(); i++) {
//        ocular::ocular_rbe_obs_t meas = _meas_all[i];
//        cerr << "azim: " << meas.azimuth << ",\telev: " << meas.elevation << ",\trange: " << meas.range << ",\tintensity: " << meas.intensity << endl;
//    }
//    cerr << "output VECTOR" << endl;
//    for(unsigned int i = 0; i < _meas_all_vector.size(); i++) {
//        PolarMeasurements _meas_all_callback = _meas_all_vector[i];

//        for(unsigned int j = 0; j < _meas_all_callback.size(); j++) {
//            ocular::ocular_rbe_obs_t meas = _meas_all_callback[j];
//            cerr << "azim: " << meas.azimuth << ",\telev: " << meas.elevation << ",\trange: " << meas.range << ",\tintensity: " << meas.intensity << endl;
//        }
//    }


    cout << "writing Data for octave plotting and for PCD_viewer" << endl;
    ofstream plotting("plottami.txt");
    pcl::PointCloud<pcl::PointXYZI> cloud;
    cloud.width = _xyz_meas_all.size();
    cloud.height = 1;

    for(unsigned int i = 0; i < _xyz_meas_all.size(); i++){
        Eigen::Vector4d xyz_p = _xyz_meas_all[i];

        /// create file for octave plotting
        plotting << xyz_p[0] << " " << xyz_p[1] << " " << xyz_p[2] << " " << xyz_p[3];
        plotting << endl;

        /// creat a PCD file format TODO
        pcl::PointXYZI pcl_p;
        pcl_p.x = xyz_p[0];
        pcl_p.y = xyz_p[1];
        pcl_p.z = xyz_p[2];
        pcl_p.intensity = xyz_p[3];
        cloud.points.push_back(pcl_p);
    }

    cout << "Saving " << cloud.points.size() << " data points to outfilename.c_str()." << endl;
    pcl::io::savePCDFileASCII(outfilename.c_str(), cloud);

    plotting.flush();
    plotting.close();

    //    _mutex_meas.unlock();
}

void stopandprint() {

    /// stopping laser callback
    cout << endl << "stopping laser" << endl;
    int attempts = 0;
    ocular::ocular_error_t err_ = laserone->StopLaser();
    while(err_ != 0 && attempts < 10){
        err_ = laserone->StopLaser();
        attempts++;
    }
    if(err_ != 0){
        cout << "got an error(StopLaser): " << laserone->GetErrorString(err_) << endl;
        exit(1);
    }

    /// stopping laser movements
    cout << "stopping laser motors" << endl;
    attempts = 0;
    err_ = laserone->Stop();
    while(err_ != 0 && attempts < 10){
        err_ = laserone->Stop();
        attempts++;
    }
    if(err_ != 0){
        cout << "got an error(Stop): " << laserone->GetErrorString(err_) << endl;
        exit(1);
    }

    //  /// stopping aperture angles callback
    //  cout << "stopping aperture angles streaming" << endl;
    //  attempts = 0;
    //  err_ = laserone->StopStreamingApertureAngles();
    //  while(err_ != 0 && attempts < 10){
    //    err_ = laserone->StopStreamingApertureAngles();
    //    attempts++;
    //  }
    //  if(err_ != 0){
    //    cout << "got an error: " << laserone->GetErrorString(err_) << endl;
    //    exit(1);
    //  }

    /// setting a desired final position
    cout << "setting a desired final position" << endl;
    attempts = 0;
    //  err_ = laserone->SetApertureAngles(0.0f, -35.0f, 1, &_notif_callback); // look down
    err_ = laserone->Home(); //home position should be az=0, el=0;
    while(err_ != 0 && attempts < 10){
        //    err_ = laserone->SetApertureAngles(0.0f, -35.0f, 1, &_notif_callback);
        err_ = laserone->Home();
        attempts++;
    }
    if(err_ != 0){
        std::cout << "got an error(setting home): " << laserone->GetErrorString(err_) << std::endl;
        exit(1);
    }

    printandwriteLaserData();

//    cout << endl;
//    cout << "=== END ===" << endl;
//    exit(0);
}


void init_parameters(int argc, char ** argv){

    // initialize parameters
    _sensor_IP = "169.254.111.100";
    g2o::CommandArgs arg;
    arg.param("t", _seconds, 15, "choose for how many seconds you want to exec this routine");
    arg.param("az_rate", _az_rate, 10, "set the number of rounds per second [max:15(5400 degrees/sec)]");
    arg.param("nlines", _N_lines, 100, "set the number of horizontal lines (vertical resolution) [min:_az_rate]");
    arg.param("avg", _averaging, 1, "set the 'averaging' value range:[1,5]");
    arg.param("lfreq", _laser_freq, 10000, "set the measurement frequency [range:[1, 30000]]");
    arg.param("intensity", _intensity, 0, "enable the streaming of intensity values [can be 'on' only if lfreq <= 10000] default off");
    arg.param("o", outfilename, "Eyebot_pcd.pcd", "output filename of a pcd format file");

    arg.parseArgs(argc, argv);

    // check parameters consistency
    if(_averaging > 5){
        cout << "-avg must be an integer value from 1 to 5" << endl << endl;
        exit(1);
    }

    if(_laser_freq > 30000 || _laser_freq == 0){
        cout << "-lfreq must be an integer value in [1, 30000]" << endl << endl;
        exit(1);
    }

    if(_az_rate > 15 || _az_rate == 0){
        cout << "-az_rate must be a value in (0, 15]" << endl << endl;
        exit(1);
    }

    if(_N_lines < _az_rate){
        cout << "-nlines must be at least equal to az_rate" << endl << endl;
        exit(1);
    }

    if(_intensity && _laser_freq > 10000){
        cout << "-intensity can't be specified for lfreq values higher than 10000" << endl << endl;
        exit(1);
    }

    if(_seconds < 1){
        cout << "-t must be an integer positive non zero value" << endl << endl;
        exit(1);
    }

    // print parameters
    cout << "SENSOR IP:\t" << _sensor_IP << endl;
    cout << "LASER FREQUENCY:\t" << _laser_freq << endl;
    cout << "AVERAGING:\t" << _averaging << endl;
    cout << "GET INTENSITY:\t" << (_intensity? "YES" : "NO") << endl;
    cout << "AZIMUTH RATE:\t" << _az_rate << endl;
    cout << "N_LINES:\t" << _N_lines << endl;
    cout << "RUNNING FOR:\t" << _seconds << " SECONDS" << endl;

}

int publishScan(const roboteye::RobotEyeScan &scan) {

/// !!!!!TODO: MODIFY WITH THE roboteye _scan and _scan_config

    unsigned int num_readings = _meas_all.size();

    ros::Time scan_time = ros::Time::now();
    //ros::Time().fromNSec((uint64_t)scan.system_time_stamp) + ros::Duration(driver_.config_.time_offset);

    //populate the RobotEyeScan message
    _scan_msg.header.stamp = scan_time;
    _scan_msg.header.frame_id = "roboteye_frame";
    _scan_msg.azimuth_min = deg2rad(0);
    _scan_msg.azimuth_max = deg2rad(360);
    _scan_msg.azimuth_increment = deg2rad(0.01); //to be checked
    _scan_msg.elevation_min = deg2rad(-35);
    _scan_msg.elevation_max = deg2rad(35);
    _scan_msg.elevation_increment = deg2rad(0.004);
    _scan_msg.time_increment = (1 / _laser_freq) / (num_readings);
    _scan_msg.range_min = 0.5; //see the documentation
    _scan_msg.range_max = 250.0; //see the documentation

    _scan_msg.measurements.resize(num_readings);
    _scan_msg.intensities.resize(num_readings);

    for(unsigned int i = 0; i < num_readings; ++i){
        geometry_msgs::Vector3 v_ranges;
        v_ranges.x = scan.ranges[i].x();
        v_ranges.y = scan.ranges[i].y();
        v_ranges.z = scan.ranges[i].z();

        _scan_msg.measurements[i] = v_ranges;
        _scan_msg.intensities[i] = scan.intensities[i];
    }

    _desired_freq = (1. / scan.config.scan_time);
    _scan_pub.publish(_scan_msg);

    //ROS_DEBUG("publishScan done");

    return(0);
}

int main(int argc, char** argv) {

    cout << "=== LASERONE ===" << endl << endl;

    init_parameters(argc,argv);

    //attention! one measurements one scan(to be modified)
    try {
        std::cout << "creating RE05Driver" << std::endl;
        // connecting to RobotEye
        laserone = new ocular::RE05Driver(_sensor_IP.c_str());
    }
    catch(std::exception  &e) {
        std::cout << "Something went wrong, caught exception: " << e.what() << std::endl;
        return 1;
    }

    /// homing the RobotEye, no need to call a sleep(), because Home() is blocking
    ocular::ocular_error_t err0 = laserone->Home();
    if(err0 != 0) {
        cout << "home error err: " << laserone->GetErrorString(err0) << endl;
        exit(1);
    }
    double azimuth, elevation;
    err0 = laserone->GetApertureAngles(azimuth, elevation);
    cout << "the initial home position is:\tazimuth = " << azimuth << ",\televation = " << elevation << ", err: " << laserone->GetErrorString(err0) << endl;

    /// start a Full Field Scan
    err0 = laserone->StartFullFieldScan(_az_rate, _N_lines); // e.g. 3600 degrees/sec (10 revolution/sec) with NLines_min = 3600/360
    if(err0 != 0) {
        cout << "Start FullField scan err: " << laserone->GetErrorString(err0) << endl;
        exit(1);
    }
    err0 = laserone->StartLaser(_laser_freq, _averaging, _intensity, &_laser_callback);
    cout << "...starting streaming laser measurements" << endl;
    if(err0 != 0) {
        cout << "Start laser err: " << laserone->GetErrorString(err0) << endl;
        exit(1);
    }
//    ocular::ocular_error_t err2 = laserone->StreamApertureAngles(5, &_angle_callback);
//    cout << "...streaming aperture position" << ", err: " << laserone->GetErrorString(err2) << std::endl;

    cout << "going to sleep for " << _seconds << " seconds." << endl;
    sleep(_seconds);

    stopandprint();

    ros::init(argc, argv, "roboteye_scan_publisher");
    ros::NodeHandle n;
    _scan_pub = n.advertise<robot_eye_driver::RobotEyeScan>("roboteye_scan", 100);

    /*...to be modified from now on basing on
      laser_drivers/hokuyo_node/node/hokuyo_node.cpp
      and/or laser_scan_publisher_tutorial...*/

    /// TODO
    //create the struct roboteye::RobotEyeScan _scan
    //use threads --> change the sleep routine

    ros::Rate r(1.0);
    while(n.ok()){

        publishScan(_scan);
        r.sleep();
    }

    cout << endl;
    cout << "=== END ===" << endl;
    exit(0);
}
