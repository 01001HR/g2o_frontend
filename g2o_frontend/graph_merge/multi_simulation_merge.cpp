#include <iostream>
#include <cmath>

#include "g2o/types/slam2d/vertex_se2.h"
#include "g2o/types/slam2d/edge_se2.h"

#include "g2o/core/sparse_optimizer.h"
#include "g2o/core/block_solver.h"
#include "g2o/core/factory.h"
#include "g2o/core/optimization_algorithm_factory.h"
#include "g2o/core/optimization_algorithm_gauss_newton.h"
#include "g2o/solvers/csparse/linear_solver_csparse.h"

#include "graph_simulator.h"


using namespace Eigen;
using namespace g2o;
using namespace std;



int main(int argc, char** argv)
{
    float nx = 0.05;
    float ny = 0.01;
    float nz = 2.;
    if(argc == 4)
    {
        nx = atof(argv[1]);
        ny = atof(argv[2]);
        nz = atof(argv[3]);
    }
    int samples = 1500;
    int trajectories = 2;
    bool interGraphClosures = true;
    bool loopClosures = true;
    Vector3d noise(nx, ny, DEG2RAD(nz));
    GraphSimulator gs(noise);
    gs.simulate(samples, trajectories, interGraphClosures, loopClosures);

    typedef BlockSolver< BlockSolverTraits<-1, -1> >  SlamBlockSolver;
    typedef LinearSolverCSparse<SlamBlockSolver::PoseMatrixType> SlamLinearSolver;

    // allocating the optimizer
    SparseOptimizer optimizer;
    SlamLinearSolver* linearSolver = new SlamLinearSolver();
    linearSolver->setBlockOrdering(false);
    SlamBlockSolver* blockSolver = new SlamBlockSolver(linearSolver);
    OptimizationAlgorithmGaussNewton* solver = new OptimizationAlgorithmGaussNewton(blockSolver);

    optimizer.setAlgorithm(solver);

    SparseOptimizer merged;
    SlamLinearSolver* linearSolver1 = new SlamLinearSolver();
    linearSolver1->setBlockOrdering(false);
    SlamBlockSolver* blockSolver1 = new SlamBlockSolver(linearSolver1);
    OptimizationAlgorithmGaussNewton* solver1 = new OptimizationAlgorithmGaussNewton(blockSolver1);
    merged.setAlgorithm(solver1);

    cout << "Trajectories size: " << gs.trajectories().size() << endl;

    int offset = samples + 100000;
    for(size_t k = 0; k < gs.trajectories().size(); ++k)
    {
        const SimGraph& traj = gs.trajectories()[k];
        for(size_t i = 0; i < traj.poses().size(); ++i)
        {
            const SimNode& pose = traj.poses()[i];
            const Isometry2d& transform = pose.noisy_pose;
            VertexSE2* robot = new VertexSE2;
            robot->setId(pose.id);
            robot->setEstimate(transform);
            optimizer.addVertex(robot);

            VertexSE2* copy = new VertexSE2;
            int new_id = pose.id + k * offset;
            copy->setId(new_id);
            copy->setEstimate(transform);
            merged.addVertex(copy);
        }

        for(Edges::iterator it = traj.edges().begin(); it != traj.edges().end(); ++it)
        {
            SimEdge* edge = *it;

            EdgeSE2* odometry = new EdgeSE2;
            odometry->vertices()[0] = optimizer.vertex(edge->from_id);
            odometry->vertices()[1] = optimizer.vertex(edge->to_id);
            odometry->setMeasurement(edge->noisy_transform);
            odometry->setInformation(edge->information);

            optimizer.addEdge(odometry);

            EdgeSE2* e1 = new EdgeSE2;
            int new_from_id = edge->from_id + k * offset;
            int new_to_id = edge->to_id + k * offset;
            e1->vertices()[0] = merged.vertex(new_from_id);
            e1->vertices()[1] = merged.vertex(new_to_id);
            e1->setMeasurement(edge->noisy_transform);
            e1->setInformation(edge->information);

            merged.addEdge(e1);
        }

        ostringstream before;
        before << "before-" << k << ".g2o";
        optimizer.save(before.str().c_str());

        VertexSE2* firstRobotPose = dynamic_cast<VertexSE2*>(optimizer.vertex(0));
        firstRobotPose->setFixed(true);
        optimizer.setVerbose(true);

        optimizer.initializeOptimization();
        optimizer.optimize(10);

        ostringstream after;
        after << "after-" << k << ".g2o";
        optimizer.save(after.str().c_str());

        optimizer.clear();
    }

    Edges closures = gs.closures();
    ofstream ofs("closures.g2o");
    cout << "Number of Inter Graph Closures: " << closures.size() << endl;
    for(Edges::iterator cit = closures.begin(); cit != closures.end(); cit++)
    {
        SimEdge* simClosure = *cit;

        EdgeSE2* closure = new EdgeSE2;
        int new_from_id = simClosure->from_id + offset;
        closure->vertices()[0] = merged.vertex(new_from_id);
        closure->vertices()[1] = merged.vertex(simClosure->to_id);
        closure->setMeasurement(simClosure->real_transform);
        closure->setInformation(simClosure->information);

        merged.addEdge(closure);

        ofs << "EDGE_SE2 " << closure->vertex(0)->id() << " " << closure->vertex(1)->id();
        Vector3d p = closure->measurement().toVector();
        ofs << " " << p.x() << " " << p.y() << " " << p.z();
        for(int i = 0; i < 3; ++i)
            for(int j = i; j < 3; ++j)
                ofs << " " << closure->information()(i, j);
        ofs << endl;
    }
    ofs.close();

    ostringstream merged_pre;
    merged_pre << "merged_before.g2o";
    merged.save(merged_pre.str().c_str());

    VertexSE2* fixedVertex = dynamic_cast<VertexSE2*>(merged.vertex(0));
    fixedVertex->setFixed(true);
    merged.setVerbose(true);

    merged.initializeOptimization();
    merged.optimize(10);

    ostringstream merged_after;
    merged_after << "merged_after.g2o";
    merged.save(merged_after.str().c_str());

    return 0;
}
