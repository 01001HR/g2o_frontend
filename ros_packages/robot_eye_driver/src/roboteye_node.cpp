#include "roboteye_node.h"

using namespace ocular;
using namespace std;
using namespace Eigen;
using namespace roboteye;

roboteye_node::roboteye_node(double az_rate, double n_lines, double laser_freq, double averaging, bool intensity) : _sensor_IP("169.254.111.100")
{
    _az_rate = az_rate;
    _N_lines = n_lines;
    _laser_freq = laser_freq;
    _averaging = averaging;
    _intensity = intensity;

    _num_readings = 0;
    _desired_freq = 0.;
    _lastStamp = ros::Time::now().toSec();

    // connecting to RobotEye
    try {
        std::cout << "creating RE05Driver" << std::endl;

        laserone = new ocular::RE05Driver(_sensor_IP.c_str());
    }
    catch(std::exception  &e) {
        std::cout << "Something went wrong, caught exception: " << e.what() << std::endl;
        return;
    }

    cout << "[....] ip " << _sensor_IP << endl;

    _scan_pub = _handle.advertise<robot_eye_driver::RobotEyeScan>("roboteye_scan", 1024);

    _thrd = boost::thread(&roboteye_node::roboteyeRunning, this);
}

roboteye_node::~roboteye_node() {

    this->stopAndPrint();
//    _laser_callback._mutex_meas.lock();
//    _laser_callback._mutex_meas.~Mutex();
    _thrd.interrupt();
    _thrd.~thread();
}

void roboteye_node::roboteyeRunning(){

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

//    cout << "going to sleep for " << _seconds << " seconds." << endl;
//    int left_s = _seconds;
//    left_s = sleep(_seconds);

}


void roboteye_node::stopAndPrint() {

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

    this->printAndWriteLaserData();
}

void roboteye_node::printAndWriteLaserData() {


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
    cloud.width = _laser_callback._xyz_meas_all.size();
    cloud.height = 1;

    for(unsigned int i = 0; i < _laser_callback._xyz_meas_all.size(); i++){
        Eigen::Vector4f xyz_p = _laser_callback._xyz_meas_all[i];

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
//    pcl::io::savePCDFileASCII(outfilename.c_str(), cloud);

    plotting.flush();
    plotting.close();

    //    _mutex_meas.unlock();
}

/// todo: set the config parameter

//void roboteye_node::setConfig() {
//    _scan_config.min_azimuth = 0;
//    _scan_config.max_azimuth = 360;
//    _scan_config.azim_increment = 0.01; //to be checked
//    _scan_config.min_elevation = -35;
//    _scan_config.max_elevation = 35;
//    _scan_config.elev_increment = 0.004; //to be checked
//    _scan_config.min_range = 0.5; //see the documentation
//    _scan_config.max_range = 250.0; //see the documentation
//}
