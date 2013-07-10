#include "pwn_loop_closer_controller.h"

#include <unistd.h>

#include "g2o/core/block_solver.h"
#include "g2o/core/factory.h"
#include "g2o/core/optimization_algorithm_gauss_newton.h"
#include "g2o/core/optimization_algorithm_levenberg.h"
#include "g2o/solvers/csparse/linear_solver_csparse.h"
#include "g2o/types/slam3d/types_slam3d.h"

using namespace std;
using namespace Eigen;
using namespace g2o;

namespace pwn {
  
  PWNLoopCloserController::PWNLoopCloserController(OptimizableGraph *graph_) {
    // Graph init
    _graph = graph_;
    
    // Projectors init
    _numProjectors = 4;
    _imageRows = 0;
    _imageCols = 0;
    _scaledImageRows = 0;
    _scaledImageCols = 0;
    _reduction = 2;
    _cameraMatrix << 
      525.0f, 0.0f, 319.5f,
      0.0f, 525.0f, 239.5f,
      0.0f, 0.0f, 1.0f;
    _scaledCameraMatrix << 
      525.0f, 0.0f, 319.5f,
      0.0f, 525.0f, 239.5f,
      0.0f, 0.0f, 1.0f;
    _sensorOffset = Isometry3f::Identity();
    _sensorOffset.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;
    _pinholePointProjector = new PinholePointProjector();
    _multiPointProjector = new MultiPointProjector();

    // Correspondence finder and linearizer init
    _correspondenceFinder = new CorrespondenceFinder();
     updateProjectors();
    _linearizer = new Linearizer();
    
    // Information matrix calculators init
    _curvatureThreshold = 0.1f;
    _pointInformationMatrixCalculator = new PointInformationMatrixCalculator();
    _normalInformationMatrixCalculator = new NormalInformationMatrixCalculator();
    _pointInformationMatrixCalculator->setCurvatureThreshold(_curvatureThreshold);
    _normalInformationMatrixCalculator->setCurvatureThreshold(_curvatureThreshold);

    // Aligner init
    _minNumInliers = 10000;
    _minError = 10.0f;
    _aligner = new Aligner();
    _aligner->setProjector(_multiPointProjector);
    _aligner->setLinearizer(_linearizer);
    _linearizer->setAligner(_aligner);
    _aligner->setCorrespondenceFinder(_correspondenceFinder);
    _aligner->setInnerIterations(1);
    _aligner->setOuterIterations(10);
  }

  bool PWNLoopCloserController::extractAbsolutePrior(Eigen::Isometry3f &priorMean, 
			    Matrix6f &priorInfo, 
			    G2OFrame *currentFrame) {
    VertexSE3 *currentVertex = currentFrame->vertex();
    ImuData *imuData = 0;
    OptimizableGraph::Data *d = currentVertex->userData();
    while(d) {
      ImuData *imuData_ = dynamic_cast<ImuData*>(d);
      if(imuData_) {
	imuData = imuData_;
      }
      d = d->next();
    }
    
    if(imuData) {
      Eigen::Matrix3d R = imuData->getOrientation().matrix();
      Eigen::Matrix3d Omega = imuData->getOrientationCovariance().inverse();
      priorMean.setIdentity();
      priorInfo.setZero();
      for(int c = 0; c < 3; c++)
	for(int r = 0; r < 3; r++)
	  priorMean.linear()(r, c) = R(r, c);
      
      for (int c = 0; c < 3; c++)
	for (int r = 0; r < 3; r++)
	  priorInfo(r + 3, c + 3) = Omega(r, c) * 100000.0f;
      return true;
    }
    return false;
  }

  bool PWNLoopCloserController::extractPWNData(G2OFrame *frame) const {
    // Get the list of data from the input vertex and check if one of them it's a PWNData
    OptimizableGraph::Data *d = frame->vertex()->userData();
    PWNData *pwnData = 0;
    while(d && !pwnData) {
      pwnData = dynamic_cast<PWNData*>(d);
      d = d->next();
    }
    
    if(!pwnData)
      return false;
  
    // Get the .pwn filename
    std::string fname = pwnData->filename();
    // Get the global transformation of the cloud
    Eigen::Isometry3f originPose = Eigen::Isometry3f::Identity();
    originPose.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;

    // Load cloud from the file
    if(frame->load(originPose, fname.c_str())) {      
      // Set frame parameters
      frame->cameraMatrix() = _cameraMatrix;
      frame->sensorOffset() = _sensorOffset;    
      frame->globalTransform() = originPose;
      frame->globalTransform().matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;
      frame->previousFrameTransform().setIdentity();
      frame->previousFrameTransform().matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;
      frame->setPreviousFrame(0);
    }
    else {
      return false;
    }

    // Computing information matrices
    frame->pointInformationMatrix().resize(frame->points().size());
    frame->normalInformationMatrix().resize(frame->points().size());
    _pointInformationMatrixCalculator->compute(frame->pointInformationMatrix(), frame->stats(), frame->normals());
    _normalInformationMatrixCalculator->compute(frame->normalInformationMatrix(), frame->stats(), frame->normals());
    
    return true;
  }

  bool PWNLoopCloserController::alignVertexWithPWNData(Isometry3f &transform, 
						       G2OFrame *referenceFrame, 
						       G2OFrame *currentFrame) {

    cerr << "Aligning vertex " << referenceFrame->vertex()->id() << " and " << currentFrame->vertex()->id() << endl;
  
    // Extract initial guess
    Eigen::Isometry3f initialGuess;
    Eigen::Isometry3d delta = referenceFrame->vertex()->estimate().inverse() * currentFrame->vertex()->estimate();
    for(int c = 0; c < 4; c++)
      for(int r = 0; r < 3; r++)
	initialGuess.matrix()(r, c) = delta.matrix()(r, c);
    
    Eigen::Isometry3f imuMean;
    Matrix6f imuInfo;
    bool hasImu = this->extractAbsolutePrior(imuMean, imuInfo, currentFrame);    
    initialGuess = Isometry3f::Identity();
    initialGuess.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;    

    // Setting aligner
    _aligner->clearPriors();
    _aligner->setReferenceFrame(referenceFrame);
    _aligner->setCurrentFrame(currentFrame);
    _aligner->setInitialGuess(initialGuess);
    _aligner->setSensorOffset(Isometry3f::Identity());
    if(hasImu)
      _aligner->addAbsolutePrior(referenceFrame->globalTransform(), imuMean, imuInfo);
    
    // Align
    _aligner->align();  
    transform = _aligner->T();
    if(_aligner->outerIterations() != 0 && 
       (_aligner->inliers() < _minNumInliers || 
	_aligner->error() / _aligner->inliers() > _minError)) {
      cerr << "ALIGNER FAILURE!!!!!!!!!!!!!!!" << endl;
      cerr << "inliers/minimum number of inliers: " << _aligner->inliers() << " / " << _minNumInliers << endl;
      cerr << "error/minimum error: " << _aligner->error() / _aligner->inliers() << " / " << _minError << endl;
      transform.matrix().setZero();
      return false;
    }
  
    // Recondition the rotation to prevent roundoff to accumulate
    Eigen::Matrix3f R = transform.linear();
    Eigen::Matrix3f E = R.transpose() * R;
    E.diagonal().array() -= 1;
    transform.linear() -= 0.5 * R * E;

    if(_aligner->outerIterations() != 0) {
      cout << "Initial guess: " << t2v(initialGuess).transpose() << endl;
      cout << "Transform: " << t2v(_aligner->T()).transpose() << endl;
    }

    // Add edge

    return true;
  }

  void PWNLoopCloserController::updateProjectors() {
    // Clear the list of projectors
    _multiPointProjector->clearProjectors();

    // Compute the reduced camera matrix and image size
    float scale = 1.0f / _reduction;
    _scaledCameraMatrix = _cameraMatrix * scale;
    _scaledCameraMatrix(2, 2) = 1.0f;
    _scaledImageRows = _imageRows / _reduction;
    _scaledImageCols = _imageCols / _reduction;
    
    // Set the camera matrix to the pinhole point projector
    _pinholePointProjector->setCameraMatrix(_scaledCameraMatrix);
    
    // Create the projectors for the multi projector
    float angleStep = 2.0f * M_PI / _numProjectors;

    for(int i = 0; i < _numProjectors; i++) {
      Isometry3f currentSensorOffset = _sensorOffset;
      if(i > 0)
	currentSensorOffset.linear() =  AngleAxisf(i * angleStep, Vector3f::UnitZ()) * _sensorOffset.linear();
      currentSensorOffset.matrix().row(3) << 0.0f, 0.0f, 0.0f, 1.0f;
    
      PinholePointProjector *currentPinholePointProjector = new PinholePointProjector();
      currentPinholePointProjector->setCameraMatrix(_scaledCameraMatrix);
      _multiPointProjector->addPointProjector(currentPinholePointProjector, currentSensorOffset, 
					      _scaledImageRows, _scaledImageCols);
    }
    
    _correspondenceFinder->setSize(_scaledImageRows, _scaledImageCols);
  }
}
