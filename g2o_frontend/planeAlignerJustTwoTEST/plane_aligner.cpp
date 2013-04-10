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
#include "g2o_frontend/data/point_cloud_data.h"
#include "g2o_frontend/sensor_data/rgbd_data.h"
#include <Eigen/Geometry>
#include <pcl/ModelCoefficients.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/passthrough.h>
#include "g2o/types/slam3d_addons/vertex_plane.h"
#include "g2o/types/slam3d_addons/edge_plane.h"
#include "g2o/types/slam3d_addons/edge_se3_plane_calib.h"
#include "g2o/core/hyper_graph.h"
#include "g2o_frontend/ransac/ransac.h"
#include "g2o_frontend/ransac/alignment_plane_linear.h"
#include "g2o_frontend/basemath/bm_se2.h"
#include "g2o/types/slam3d/isometry3d_mappings.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "g2o_frontend/ransac/alignment_plane_linear.h"
#include "g2o_frontend/ransac/ransac.h"
#include <cstdio>
#include <iomanip>
#include <stdio.h>
#include <iostream>
#include "fileReading.h"
#include "printHelper.h"
#include "planesTransformations.h"
#include "graphUtils.h"


using namespace std;
using namespace g2o;
using namespace Slam3dAddons;
using namespace cv;
using namespace Eigen;
using namespace g2o_frontend;


volatile bool hasToStop;

//Signal Handling
//**************************************************************************************************************************************
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
//**************************************************************************************************************************************


//RANSAC
//**************************************************************************************************************************************
//**************************************************************************************************************************************
template <typename TypeDomain_, int dimension_>
struct EuclideanMapping{
    typedef TypeDomain_ TypeDomain;
    typedef typename Eigen::Matrix<double, dimension_, 1> VectorType;
    virtual int dimension() const {return dimension_;}
    virtual TypeDomain fromVector(const VectorType& v) const =  0;
    virtual VectorType toVector(const TypeDomain& t) const = 0;
};

template <int dimension_>
struct VectorMapping : public EuclideanMapping<Eigen::Matrix<double, dimension_, 1>, dimension_>{
    typedef typename EuclideanMapping<Eigen::Matrix<double, dimension_, 1>, dimension_>::TypeDomain TypeDomain;
    typedef typename EuclideanMapping<Eigen::Matrix<double, dimension_, 1>, dimension_>::VectorType VectorType;
    virtual TypeDomain fromVector(const VectorType& v) const {return v;}
    virtual VectorType toVector(const TypeDomain& t) const {return t;}
};


struct PlaneMapping: public EuclideanMapping<Plane3D,4>{
    typedef typename EuclideanMapping<Plane3D, 4>::TypeDomain TypeDomain;
    typedef typename EuclideanMapping<Plane3D, 4>::VectorType VectorType;
    virtual TypeDomain fromVector(const VectorType& v) const {
        Plane3D l(v);
        return l;
    }
    virtual VectorType toVector(const TypeDomain& t) const {
        return t.toVector();
    }
};

typedef std::vector<Correspondence> CorrespondenceVector;

template <typename MappingType, typename RansacType, typename EdgeCorrespondenceType>
bool testRansac(typename RansacType::TransformType& result,CorrespondenceVector& correspondences,IndexVector& iv){

    RansacType ransac;

    //ransac.correspondenceValidators()=validators;
    ransac.setCorrespondences(correspondences);
    ransac.setMaxIterations(2000);
    ransac.setInlierErrorThreshold(0.01);
    ransac.setInlierStopFraction(0.5);

    return ransac(result,iv);
}
//**************************************************************************************************************************************
//**************************************************************************************************************************************


Isometry3d odometry;    //the odometry ISOMETRY


int main(int argc, char**argv)
{
    cout << endl << endl <<"================================================================"<<endl<<endl;

    //Globals
    //**************************************************************************************************************************************
    hasToStop = false;
    string filename;
    string outfilename;
    CommandArgs arg;
    int theError;
    int vertex1;
    int ransac;
    int size;       //number of planes to read from the .dat file
    //**************************************************************************************************************************************

    //Inits arguments
    //**************************************************************************************************************************************
    arg.param("o", outfilename, "otest.g2o", "output file name");
    arg.param("e",theError,5,"errore");
    arg.param("r",ransac,0,"ransac");
    arg.param("s",size,1,"size");
    arg.param("v1",vertex1,1,"primo vertice");
    arg.paramLeftOver("graph-input", filename , "", "graph file which will be processed", true);
    arg.parseArgs(argc, argv);
    //**************************************************************************************************************************************

    //Reading graph
    //**************************************************************************************************************************************
    OptimizableGraph graph;
    OptimizableGraph outgraph;
    //**************************************************************************************************************************************

    //Stuff
    //**************************************************************************************************************************************
    signal(SIGINT, sigquit_handler);
    //**************************************************************************************************************************************


    //Ransac Things
    //**************************************************************************************************************************************
    CorrespondenceVector mycorrVector;
    //**************************************************************************************************************************************

    //GENERAZIONE GRAFO ON FLY
    //  leggo da due file
    //- p1.dat contiene i piani nel frame 0
    //- p2.dat contiene i piani nel frame 1
    //- transform.dat contiene la trasformata relativa tra i due

    //piani non normalizzati da leggere nel file
    Vector4d piano[5];
    //piani non normalizzati transformati, verranno salvati qui
    Vector4d pianoR[5];
    //la trasformata relativa tra i piani, verra' letta dal file e salvata qui
    Isometry3d trasformata;


    fillPlanes("p1.dat",5,piano);
    fillPlanes("p2.dat",5,pianoR);
    fillTransform("transform.dat",trasformata);


    //stampe di debug
    cout << "piani nel frame 0"<<endl;
    for(int i =0;i<size;i++)
    {
        printVector4dAsRow(piano[i],1);
    }
    cout << endl;

    cout << "piani nel frame 1"<<endl;
    for(int i=0;i<size;i++)
    {
        printVector4dAsRow(pianoR[i],1);
    }
    cout << endl;

    cout << "trasformata"<<endl;
    Vector6d trasformataV=g2o::internal::toVectorMQT(trasformata);
    printVector6dAsRow(trasformataV,1);
    cout << endl;

    cout << endl << endl <<"================================================================"<<endl<<endl;

    //************************************************************************************************
    //************************************************************************************************

    VertexSE3* v1 = new VertexSE3;
    VertexSE3* v2 = new VertexSE3;

    v1->setId(0);
    v2->setId(1);

    Isometry3d v1Est;
    v1Est.setIdentity();
    v1->setEstimate(v1Est);

    EdgeSE3* v1TOv2=new EdgeSE3;
    v1TOv2->setMeasurement(trasformata);
    v1TOv2->setVertex(0,v1);
    v1TOv2->setVertex(1,v2);

    v2->setEstimate(v1->estimate()*v1TOv2->measurement()); // v1->estimate()*v1TOv2->measurement() ====>>> LA TRASFORMATA

    //PARAMETER OFFSET
    VertexSE3* offset=new VertexSE3;
    offset->setId(2545);
    offset->setEstimate(Isometry3d::Identity());
    graph.addVertex(offset);
    graph.addVertex(v1);
    graph.addVertex(v2);
    graph.addEdge(v1TOv2);

    int planeID=666;
    //VERTICE 1
    for(int i=0;i<size;i++)
    {
        VertexPlane* vPlane=new VertexPlane;
        Plane3D plane;
        plane.fromVector(piano[i]);
        vPlane->setEstimate(plane);
        vPlane->setId(planeID);

        cout << "[1] Adding normalized vertex plane: "<<endl;
        graph.addVertex(vPlane);
        printPlaneCoeffsAsRow(plane,1);

        EdgeSE3PlaneSensorCalib* eSE3calib= new EdgeSE3PlaneSensorCalib;
        eSE3calib->setVertex(0,v1);
        eSE3calib->setVertex(1,vPlane);
        eSE3calib->setVertex(2,offset);
        eSE3calib->color=Vector3d(0,1,0);
        eSE3calib->setMeasurement(plane);

        cout <<"Adding edge plane"<<endl;
        graph.addEdge(eSE3calib);
        planeID++;
    }
    //VERTICE 2

    for(int i=0;i<size;i++)
    {
        VertexPlane* vPlane=new VertexPlane;
        Plane3D plane;
        plane.fromVector(pianoR[i]);
        vPlane->setEstimate(plane);
        vPlane->setId(planeID);

        cout << "[2] Adding normalized vertex plane: "<<endl;
        graph.addVertex(vPlane);
        printPlaneCoeffsAsRow(plane,1);
        EdgeSE3PlaneSensorCalib* eSE3calib= new EdgeSE3PlaneSensorCalib;
        eSE3calib->setVertex(0,v2);
        eSE3calib->setVertex(1,vPlane);
        eSE3calib->setVertex(2,offset);

        //--------------------------------> IMPORTANT <-----------------------------------------
        eSE3calib->setMeasurement(v2->estimate().inverse()*plane);
        //--------------------------------> IMPORTANT <-----------------------------------------

        eSE3calib->color=Vector3d(1,0,0);
        cout <<"Adding edge plane"<<endl;
        graph.addEdge(eSE3calib);
        planeID++;
    }


    cout << "salvo grafo intermedio...";
    ofstream grafino ("grafene.g2o");
    graph.save(grafino);
    cout << "salvato"<<endl;

    cout << endl <<"================================================================"<<endl<<endl;


    //*****************************************************************************************
    //**                                                                                      *
    //**               A questo punto un grafo salvato in memoria e su file                   *
    //**                                                                                      *
    //*****************************************************************************************
    //-----------------------------------------------------------------------------------------
    //Struttura del grafo:
    // verticeSE3(0) in (0,0,0,0,0,0)
    // verticeSE3(1) nella posa determinata dalla trasformata
    // EdgeSE3(0,1)  trasformata relativa (letta dal file
    //-----------------------------------------------------------------------------------------


    //recupero il vertice richiesto in input
    OptimizableGraph::Vertex* _v=graph.vertex(0);
    _v->setFixed(true);

    v1=dynamic_cast<VertexSE3*>(_v);
    v2=new VertexSE3;

    //v è un vertex SE3
    if(v1)
    {

        //accedo ad ogni edge di quel vertice
        OptimizableGraph::EdgeSet e = v1->edges();

        //di quei edge accedo ad ogni vertice successivo
        for (HyperGraph::EdgeSet::iterator it = e.begin(); it!=e.end(); it++)
        {

            HyperGraph::Edge* _e = *it;
            //accedo solo a quelli SE3 per sapere su quale vertice spostarmi
            EdgeSE3 * eSE3 =dynamic_cast< EdgeSE3*>(_e);

            if(eSE3)
            {
                //accedo al vertice successivo sul quale andare
                VertexSE3* nextVertex = dynamic_cast<  VertexSE3*>(eSE3->vertex(1));

                //verifico che il vertice che ho recuperato dall'Edge è effettivamente
                //- di tipo EdgeSE3 (verificato in precedenza)
                //- il suo id è differente da quello di partenza

                if(nextVertex->id()!=v1->id() && nextVertex)
                {
                    cout << "mi muovo da - a"<<endl;
                    cout <<"V[" <<v1->id() << "] -> V[" << nextVertex->id() <<"] "<< endl;
                    _v=graph.vertex(nextVertex->id());
                    //se va tutto bene a questo punto l'odometria deve essere la stessa della trasformata iniziale
                    odometry=eSE3->measurement();
                    cout << "Odometria letta dal grafo:"<<endl;
                    printVector6dAsRow(g2o::internal::toVectorMQT(odometry),1);
                    cout << "Trasformata letta da file:"<<endl;
                    printVector6dAsRow(g2o::internal::toVectorMQT(trasformata),1);

                    outgraph.addEdge(eSE3);
                    //_v è il nuovo vertice su cui mi devo spostare
                    v2=dynamic_cast<VertexSE3*>(_v);
                }
            }
        }
    }

    //a questo punto:
    //  v1   vertice iniziale
    //  v2   vertice successivo

    cout<<endl;
    outgraph.addVertex(v1);
    outgraph.addVertex(v2);
    cout<<endl;

    vector<Plane3D> plane_1_container;
    vector<Plane3D> plane_2_container;
    vector<Plane3D> plane_2_container_REMAPPED;


    Matrix3d info= Matrix3d::Identity();
    info*=1000;
    info(2,2)=10; //il vincolo in traslazione avrà un peso minore

    getCalibPlanes(v1,&plane_1_container,Vector3d(1,0,0),info);
    getCalibPlanes(v2,&plane_2_container,Vector3d(0,1,0),info);


    cout << "Il primo   container è composta da " <<plane_1_container.size()<<" elemento"<< endl;

    for(int i=0;i<plane_1_container.size();i++)
    {
        Plane3D tmpPlane=plane_1_container.at(i);
        printPlaneCoeffsAsRow(tmpPlane,1);
    }

    cout << "Il secondo container è composta da " <<plane_2_container.size()<<" elemento"<<endl;

    for(int i=0;i<plane_2_container.size();i++)
    {
        Plane3D tmpPlane=plane_2_container.at(i);
        printPlaneCoeffsAsRow(tmpPlane,1);
    }

    cout << "Il secondo container di piani rimappati" <<endl;

    for(int i=0;i<plane_2_container.size();i++)
    {
        Plane3D tmpPlane=plane_2_container.at(i);
        tmpPlane=odometry*tmpPlane;
        plane_2_container_REMAPPED.push_back(tmpPlane);
        printPlaneCoeffsAsRow(tmpPlane,1);
    }

    //    cout << "Tento di rimappare i piani visti nel frame 2 nel frame 1"<<endl;

    //    for(int i=0;i<plane_2_container.size();i++ )
    //    {
    //        Plane3D piano_V2=plane_2_container.at(i);
    //        Plane3D piano_rimappato=odometry*piano_V2;
    //        printPlaneCoeffsAsRow(piano_rimappato,1);
    //    }


    cout <<"--------------------------------------------------"<<endl;
    cout <<"--------------------------------------------------"<<endl;
    cout <<"Computazione Errore"<<endl;
    cout <<"--------------------------------------------------"<<endl;
    cout <<"--------------------------------------------------"<<endl;




    //Creazione del vettore delle corrispondenze e calcolo degli errori.

    for(int i=0;i<plane_1_container.size();i++)
    {
        Plane3D p1=plane_1_container.at(i);

        for(int j=0;j<plane_2_container.size();j++)
        {

            Plane3D p2=plane_2_container.at(j);
            double error = computeError(p1,p2);

            //DEBUG INFO ----
            printPlaneCoeffsAsRow(p1);
            cout << " <> ";
            printPlaneCoeffsAsRow(p2);
            cout <<" ["<<error<<"]"<<endl;
            //DEBUG INFO ----

            //FILLING CORRESPONDANCE VECTOR
            EdgePlane* eplane = new EdgePlane;

            VertexPlane* vPlane1=new VertexPlane;
            VertexPlane* vPlane2=new VertexPlane;

            vPlane1->setEstimate(p1);
            vPlane2->setEstimate(plane_2_container_REMAPPED.at(j));

            eplane->setVertex(0,vPlane1);
            eplane->setVertex(1,vPlane2);
            Correspondence corr(eplane,error);
            mycorrVector.push_back(corr);

        }
        cout << endl;
    }

    cout << "Starting ransac... press a key"<<endl;

    //***************************************************************************************************************************

    if(ransac)
    {
        Isometry3d tresult;
        tresult.setIdentity();
        IndexVector iv;
        RansacPlaneLinear ransac;

        ransac.setCorrespondences(mycorrVector);
        ransac.setMaxIterations(2000);
        ransac.setInlierErrorThreshold(0.0001);
        ransac.setInlierStopFraction(0.1);

        ransac(tresult,iv);

        bool result = testRansac<PlaneMapping, RansacPlaneLinear, EdgePlane>(tresult, mycorrVector,iv);

        Vector6d ttt=g2o::internal::toVectorMQT(tresult);
        Vector6d ttt2=g2o::internal::toVectorMQT(tresult.inverse());
        cout << "ERROR THRESHOLD "<<ransac.inlierErrorThreshold()<<endl;
        cerr << "Transformation result from ransac"<<endl<<ttt<<endl;
        cerr << "Transformation result from ransac"<<endl<<ttt2<<endl;
        cerr << "Odometry from robot"<<endl<<g2o::internal::toVectorMQT(odometry)<<endl;
        cout << "SIZE INLIERS "<<iv.size()<<endl;

        for(int i=0;i<ransac.errors().size();i++)
        {
            cout << "ERR "<<ransac.errors().at(i)<<endl;
        }
    }
    exit(0);
}

