#include <signal.h>

#include "g2o/stuff/macros.h"
#include "g2o/stuff/color_macros.h"
#include "g2o/stuff/command_args.h"
#include "g2o/stuff/filesys_tools.h"
#include "g2o/stuff/string_tools.h"
#include "g2o/stuff/timeutil.h"

#include "g2o/core/sparse_optimizer.h"
#include "g2o/core/block_solver.h"
#include "g2o/core/factory.h"
#include "g2o/core/optimization_algorithm_gauss_newton.h"
#include "g2o/core/optimization_algorithm_levenberg.h"
#include "g2o/solvers/csparse/linear_solver_csparse.h"
#include "g2o/types/slam3d/types_slam3d.h"
#include "g2o/types/slam3d_addons/types_slam3d_addons.h"
#include "g2o_frontend/data/rgbd_image_data.h"


using namespace std;
using namespace g2o;
using namespace Slam3dAddons;

volatile bool hasToStop;
void sigquit_handler(int sig)
{
  if (sig == SIGINT) {
    hasToStop = 1;
    static int cnt = 0;
    if (cnt++ == 2) {
      cerr << __PRETTY_FUNCTION__ << " forcing exit" << endl;
      exit(1);
    }
  }
}


int main(int argc, char**argv){
  hasToStop = false;
  string filename;
  string outfilename;
  CommandArgs arg;
  float voxelSize;
  arg.param("o", outfilename, "otest.g2o", "output file name"); 
  arg.param("voxelSize", voxelSize, 0.01, "ghrid voxel size"); 
  arg.paramLeftOver("graph-input", filename , "", "graph file which will be processed", true);
  arg.parseArgs(argc, argv);
  
  // graph construction
  typedef BlockSolver< BlockSolverTraits<-1, -1> >  SlamBlockSolver;
  typedef LinearSolverCSparse<SlamBlockSolver::PoseMatrixType> SlamLinearSolver;
  SlamLinearSolver* linearSolver = new SlamLinearSolver();
  linearSolver->setBlockOrdering(false);
  SlamBlockSolver* blockSolver = new SlamBlockSolver(linearSolver);
  OptimizationAlgorithmGaussNewton* solverGauss   = new OptimizationAlgorithmGaussNewton(blockSolver);
  SparseOptimizer * graph = new SparseOptimizer();
  graph->setAlgorithm(solverGauss);
  graph->load(filename.c_str());
  
  // sort the vertices based on the id
  std::vector<int> vertexIds(graph->vertices().size());
  int k=0;
  for (OptimizableGraph::VertexIDMap::iterator it=graph->vertices().begin(); it!= graph->vertices().end(); it ++){
    vertexIds[k++] = (it->first);
  }
  std::sort(vertexIds.begin(), vertexIds.end());


  
  int planeNum = 0;
  if (vertexIds.size())
    planeNum = *vertexIds.rbegin() + 1;
  cerr << "loaded graph, now processing planes" << endl;
  signal(SIGINT, sigquit_handler);

  for (size_t i=0; i<vertexIds.size() && ! hasToStop; i++){
    OptimizableGraph::Vertex* _v=graph->vertex(vertexIds[i]);
    VertexSE3* v=dynamic_cast<VertexSE3*>(_v);
    if (!v)
      continue;
   
    OptimizableGraph::Data* d = v->userData();
    k = 0;
    while(d){
      RGBDImageData* imageData = dynamic_cast<RGBDImageData*>(d);
      d=d->next();
      if (imageData) {
	cerr << "image found" << endl;
	VertexPlane* vplane = new VertexPlane();
	vplane->setId(planeNum);
	double a=0, b=1, c=0, d=2;
	Vector4d coeffs;
	coeffs << a,b,c,d;
	vplane->setEstimate(Plane3D(coeffs));
	graph->addVertex(vplane);
      }
      
    }
  }
  cerr << endl;
 
  cerr << "saving.... " << endl;
  ofstream os (outfilename.c_str());
  graph->save(os);
  cerr << endl;
}
