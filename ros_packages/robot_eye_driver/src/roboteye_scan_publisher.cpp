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
#include "myCallbacks.h"

#include <ros/ros.h>
#include <robot_eye_driver/RobotEyeScan.h>

//msg_gen/cpp/include/

using namespace ocular;
using namespace std;

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

void printandwriteLaserData(){

    /// printing the laser data
    //to prevent possible later callbacks
    //    _mutex_meas.lock();
    //    for(unsigned int i = 0; i < _measurements.size(); i++) {
    //        ocular::ocular_rbe_obs_t meas = _measurements[i];
    //        cout << "azim: " << meas.azimuth << ",\telev: " << meas.elevation << ",\trange: " << meas.range << ",\tintensity: " << meas.intensity << endl;
    //    }

    cout << "writing Data for octave plotting and for PCD_viewer" << endl;
    ofstream plotting("plottami.txt");
    pcl::PointCloud<pcl::PointXYZI> cloud;
    cloud.width = _xyz_meas.size();
    cloud.height = 1;

    for(unsigned int i = 0; i < _xyz_meas.size(); i++){
        Eigen::Vector4d xyz_p = _xyz_meas[i];

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

void quit(){

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

    cout << endl;
    cout << "=== END ===" << endl;
    exit(0);
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

int main(int argc, char** argv){

    init_parameters(argc,argv);
    ros::init(argc, argv, "roboteye_scan_publisher");

    ros::NodeHandle n;
    ros::Publisher scan_pub = n.advertise<robot_eye_driver::RobotEyeScan>("roboteye_scan", 50);

    /*.......to be modified from now on basing on laser_drivers/hokuyo_node/node/hokuyo_node.cpp and/or laser_scan_publisher_tutorial.......*/

    unsigned int num_readings = _measurements.size();
    double laser_frequency = _laser_freq;
    std::vector<Eigen::Vector3d> measurements[num_readings];
    double intensities[num_readings];

    int count = 0;
    ros::Rate r(1.0);
    while(n.ok()){
        //generate some fake data for our laser scan
//        for(unsigned int i = 0; i < num_readings; ++i){
//            measurements[i] = count;
//            intensities[i] = 100 + count;
//        }
        ros::Time scan_time = ros::Time::now();

        //populate the LaserScan message
        robot_eye_driver::RobotEyeScan scan;
//        sensor_msgs::LaserScan scan;
        scan.header.stamp = scan_time;
        scan.header.frame_id = "laser_frame";
        scan.azimuth_min = deg2rad(0);
        scan.azimuth_max = deg2rad(360);
        scan.azimuth_increment = deg2rad(90) / num_readings; //to be checked
        scan.elevation_min = deg2rad(-35);
        scan.elevation_max = deg2rad(35);
        scan.elevation_increment = 3.14 / num_readings;
        scan.time_increment = (1 / laser_frequency) / (num_readings);
        scan.range_min = 0.0; //see the documentation
        scan.range_max = 100.0; //see the documentation

        scan.measurements.resize(num_readings);
        scan.intensities.resize(num_readings);
//        for(unsigned int i = 0; i < num_readings; ++i){
//            scan.measurements[i] = measurements[i];
//            scan.intensities[i] = intensities[i];
//        }

        scan_pub.publish(scan);
        ++count;
        r.sleep();
    }
}
