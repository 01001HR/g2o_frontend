#include "normalgenerator.h"
#include "omegagenerator.h"
#include "correspondencegenerator.h"

#include "g2o/stuff/command_args.h"
#include "g2o/stuff/timeutil.h"

#include <iostream>

#include "pointwithnormal.h"

using namespace std;

int main(int argc, char** argv) {
  /************************************************************************
   *                           Input Handling                             *
   ************************************************************************/
  
  
  // Depth image file (path+filename).
  string currentFilename, referenceFilename;

  // Variables for the input parameters. Just type on the command line
  // ./pwn_normal_extraction -h to have more details about them.
  float ng_scale = 1.0f;
  float ng_curvatureThreshold = 1.0f;

  // Define the camera matrix, place here the values for the particular 
  // depth camera used (Kinect, Xtion or any other type). This particular
  // matrix is the one related to the Kinect.
  Matrix3f cameraMatrix;
  cameraMatrix <<     
    525.0f, 0.0f, 319.5f,
    0.0f, 525.0f, 239.5f,
    0.0f, 0.0f, 1.0f;
  
  // Input parameters handling.
  g2o::CommandArgs arg;
  
  // Optional input parameters.
  arg.param("ng_scale", ng_scale, 1.0f, "Specify the scaling factor to apply on the depth image. [float]");
  arg.param("ng_curvatureThreshold", ng_curvatureThreshold, 1.0f, "Specify the max surface curvature threshold for which normals are discarded. [float]");
  
  // Last parameter has to be the depth image file.
  arg.paramLeftOver("depthImageFile1", referenceFilename, "./test1.pgm", "First depth image file (.pgm image) to analyze. [string]", true);
  arg.paramLeftOver("depthImageFile2", currentFilename, "./test2.pgm", "Secodn depth image file (.pgm image) to analyze. [string]", true);

  // Set parser input.
  arg.parseArgs(argc, argv);
  
  // The DepthImage object is used read a depth image from a .pgm image file. It is an extended Eigen 
  // matrix of unsigned char. 
  DepthImage referenceDepthImage, currentDepthImage;
  // Try to read the depth images given in input.
  if(!referenceDepthImage.load(referenceFilename.c_str())) {
    cout << "Failure while loading the depth image: " << referenceFilename<< ", quitting program!" << endl;
    exit(-1);
  }
  if(!currentDepthImage.load(currentFilename.c_str())) {
    cout << "Failure while loading the depth image: " << currentFilename << ", quitting program!" << endl;
    exit(-1);
  }

  // This is an hack since in the old code the images are loaded column-wise. 
  referenceDepthImage.transposeInPlace();
  currentDepthImage.transposeInPlace();
  cout << endl << "Loaded first depth image of size: " << referenceDepthImage.rows() << "x" << referenceDepthImage.cols() << endl;
  cout << endl << "Loaded second depth image of size: " << currentDepthImage.rows() << "x" << currentDepthImage.cols() << endl;
  
  /************************************************************************
   *                         Normal Computation                           *
   ************************************************************************/
  cout << "Computing normals...";
  // Creating the normal generator object and setting some parameters. 
  NormalGenerator referenceNormalGenerator, currentNormalGenerator;
  // Set the scale factor to apply on the depth image.
  referenceNormalGenerator.setScale(ng_scale);
  currentNormalGenerator.setScale(ng_scale);
  // Set the camera matrix
  referenceNormalGenerator.setCameraMatrix(cameraMatrix);
  currentNormalGenerator.setCameraMatrix(cameraMatrix);
  
  // Here will go the points.
  HomogeneousPoint3fVector referenceImagePoints; 
  HomogeneousPoint3fVector currentImagePoints; 
  // Here will go the normals.
  HomogeneousNormal3fVector referenceImageNormals;
  HomogeneousNormal3fVector currentImageNormals;

  // Normals computation.
  referenceNormalGenerator.compute(referenceImagePoints, referenceImageNormals, referenceDepthImage, ng_curvatureThreshold);
  currentNormalGenerator.compute(currentImagePoints, currentImageNormals, currentDepthImage, ng_curvatureThreshold);
  cout << " done." << endl;

  /************************************************************************
   *                         Omega Computation                            *
   ************************************************************************/
  cout << "Computing omegas...";
  PointOmegaGenerator pointOmegaGenerator;
  NormalOmegaGenerator normalOmegaGenerator;
  HomogeneousPoint3fOmegaVector referencePointOmega;
  HomogeneousPoint3fOmegaVector referenceNormalOmega;
  HomogeneousPoint3fOmegaVector currentPointOmega;
  HomogeneousPoint3fOmegaVector currentNormalOmega;

  pointOmegaGenerator.compute(referencePointOmega, referenceNormalGenerator.scaledStats, referenceImageNormals);
  normalOmegaGenerator.compute(referenceNormalOmega, referenceNormalGenerator.scaledStats, referenceImageNormals);
  pointOmegaGenerator.compute(currentPointOmega, currentNormalGenerator.scaledStats, currentImageNormals);
  normalOmegaGenerator.compute(currentNormalOmega, currentNormalGenerator.scaledStats, currentImageNormals);
  cout << " done." << endl;
  /************************************************************************
   *                         Correspondence Computation                   *
   ************************************************************************/
  cout << "Computing correspondences...";
  //CorrespondenceGenerator correspondenceGenerator;
  
  
  cout << " done." << endl;
  
  // This is just to check that the result is correct
  PointWithNormalVector referencePWNV(referenceImagePoints.size());
  for(size_t i = 0; i < referencePWNV.size(); ++i) {
    referencePWNV[i].head<3>() = referenceImagePoints[i].head<3>();
    referencePWNV[i].tail<3>() = referenceImageNormals[i].head<3>();
  }
  referencePWNV.save("reference.pwn", true);
  
  PointWithNormalVector currentPWNV(currentImagePoints.size());
  for(size_t i = 0; i < currentPWNV.size(); ++i) {
    currentPWNV[i].head<3>() = currentImagePoints[i].head<3>();
    currentPWNV[i].tail<3>() = currentImageNormals[i].head<3>();
  }
  currentPWNV.save("current.pwn", true);
  
  return 0;
}
