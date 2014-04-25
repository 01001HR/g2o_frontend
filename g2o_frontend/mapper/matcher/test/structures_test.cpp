#include <iostream>
#include <fstream>

#include "../matching/gradient_matcher.h"
#include "../matching/correlative_matcher.h"
#include "../utils/logReader.h"


using namespace std;
using namespace Eigen;
using namespace match_this;



int main(int argc, char** argv)
{
  float resolution = 0.3;
  float kernelRange = 0.5;
  int hV = 5;

  
  /**
   * GridMap c-tors tests
   */
  
  float radius = 50;
  GradientMatcher gm(resolution, radius, kernelRange/resolution, kernelRange);
  cout << "params:" << endl;
  cout << "ll: " << gm.getScanGrid().lowerLeft() << endl;
  cout << "up: " << gm.getScanGrid().upperRight() << endl;
  cout << "res: " << gm.getScanGrid().resolution() << endl;
  cout << "ires: " << 1./gm.getScanGrid().resolution() << endl;
  
  const char* logFile = argv[argc-1];
  LogReader lr(logFile, hV);

  vector<Vector2fVector> logScan = lr.getScans();
  vector<Vector3f> logPose = lr.getPoses();
  
  Vector2fVector previousScan;
  previousScan = logScan[0];  

  gm.convolveScan(previousScan);
  ofstream b("convolved_full_param.ppm");
  gm.saveConvolvedScanAsPPM(b, true);
  b.close();
  
  cout << "a" << endl;
  gm.integrateScan(previousScan, 1, Isometry2f::Identity());
  cout << "done" << endl;
  ofstream a("normal_full_param.ppm");
  gm.saveScanAsPPM(a, true);
  a.close();

  gm.clear();
  ofstream e("normal_full_param_cleared.ppm");
  gm.saveScanAsPPM(e, true);
  e.close();
  
  ofstream f("convolved_full_param_cleared.ppm");
  gm.saveConvolvedScanAsPPM(f, true);
  f.close();
  
  exit(1);
}
