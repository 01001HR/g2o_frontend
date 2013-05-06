#include "correspondencegenerator.h"
#include<omp.h>

void CorrespondenceGenerator::compute(HomogeneousPoint3fScene &referenceScene, HomogeneousPoint3fScene &currentScene, Eigen::Isometry3f T) {
  T.matrix().block<1,4>(3, 0) << 0,0,0,1;
  _numCorrespondences = 0;
  
  if((int)_correspondences.size() != _referenceIndexImage.rows() * _referenceIndexImage.cols())
    _correspondences.resize(_referenceIndexImage.rows() * _referenceIndexImage.cols());

  float minCurvatureRatio = 1./_inlierCurvatureRatioThreshold;
  float maxCurvatureRatio = _inlierCurvatureRatioThreshold;

  // construct an array of counters;
  int numThreads = omp_get_max_threads();
  
  int localCorrespondenceIndex[numThreads];
  int localOffset[numThreads];
  int columnsPerThread = _referenceIndexImage.cols()/numThreads;
  int iterationsPerThread = (_referenceIndexImage.rows() * _referenceIndexImage.cols())/numThreads;
  for (int i=0; i<numThreads; i++){
    localOffset[i] = i * iterationsPerThread;
    localCorrespondenceIndex[i] = localOffset[i];
  }
#pragma omp parallel 
  {
    int threadId = omp_get_thread_num();
    int cMin = threadId * columnsPerThread;
    int cMax = cMin + columnsPerThread;
    if (cMax > _referenceIndexImage.cols())
      cMax = _referenceIndexImage.cols();
    int& correspondenceIndex = localCorrespondenceIndex[threadId];
    for (int c = cMin;  c < cMax; c++) {
      for (int r = 0; r < _referenceIndexImage.rows(); r++) {
	const int referenceIndex = _referenceIndexImage(r, c);
	const int currentIndex = _currentIndexImage(r, c);
	if (referenceIndex < 0 || currentIndex < 0)
	  continue;

	const HomogeneousNormal3f& currentNormal = currentScene.normals()[currentIndex];      
	const HomogeneousPoint3f& _referenceNormal = referenceScene.normals()[referenceIndex];
	const HomogeneousPoint3f& currentPoint = currentScene.points()[currentIndex];
	const HomogeneousPoint3f& _referencePoint = referenceScene.points()[referenceIndex];
	
	// remappings
	HomogeneousPoint3f referencePoint = T * _referencePoint;
	HomogeneousNormal3f referenceNormal = T * _referenceNormal;
	// this condition captures the angluar offset, and is moved to the end of the loop
	if (currentNormal.dot(referenceNormal) < _inlierNormalAngularThreshold) 
	  continue;

	Eigen::Vector4f pointsDistance = currentPoint - referencePoint;
	// the condition below has moved to the increment, fill the pipeline, baby
	if (pointsDistance.squaredNorm() > _squaredThreshold)
	  continue;     	
	
	float referenceCurvature = referenceScene.stats()[referenceIndex].curvature();
	float currentCurvature = currentScene.stats()[currentIndex].curvature();
	if (referenceCurvature < _flatCurvatureThreshold)
	  referenceCurvature = _flatCurvatureThreshold;

	if (currentCurvature < _flatCurvatureThreshold)
	  currentCurvature = _flatCurvatureThreshold;

	float curvatureRatio = (referenceCurvature + 1e-5)/(currentCurvature + 1e-5);
	// the condition below has moved to the increment, fill the pipeline, baby
	if (curvatureRatio < minCurvatureRatio || curvatureRatio > maxCurvatureRatio)
	  continue;

	_correspondences[correspondenceIndex].referenceIndex = referenceIndex;
	_correspondences[correspondenceIndex].currentIndex = currentIndex;
	correspondenceIndex++;

      }
    }
  }
  // assemble the solution
  int k=0;
  for (int t=0; t<numThreads; t++){
    //cerr << "assembling from " << localOffset[t] << " until " << localCorrespondenceIndex[t] << endl;
    for (int i=localOffset[t]; i< localCorrespondenceIndex[t]; i++)
      _correspondences[k++] = _correspondences[i];
  }
  _numCorrespondences = k;

  //std::cerr << "dtot: " << dtot << " ndtot: " << ndtot << std::endl;
  for (size_t i = _numCorrespondences; i < _correspondences.size(); i++)
    _correspondences[i] = Correspondence();
}
