#include "voronoi_diagram.h"

using namespace std;
using namespace Eigen;


VoronoiDiagram* vd = 0;


cv::Mat eigen2cv(cv::Mat &out, const MatrixXf* in)
{
    for(int i = 0; i < in->rows(); ++i)
    {
        for(int j = 0; j < in->cols(); ++j)
        {
            out.at<uchar>(i, j) = (*in)(i, j);
        }
    }
    return out;
}


void mouseEvent(int evt, int x, int y, int flags, void *param)
{
    if(evt == CV_EVENT_LBUTTONDOWN)
    {
        vd->titSeeker(Vector2i(x, y));
        vd->spikeFinder();
        vd->savePGM("tit.pgm", *(vd->_tit));
        cv::Mat im2 = cv::imread("tit.pgm", CV_LOAD_IMAGE_UNCHANGED);
        cv::imshow("tit", im2);
        im2.release();
    }
}


int main(int argc, char** argv)
{
    if(argc < 2 || argc > 3)
    {
        cerr << "usage: " << argv[0] <<" <pgm map> " << endl;
        exit(-1);
    }

    const char* prova = argv[1];
    ifstream is(argv[1]);
    if(!is)
    {
        cerr << "Could not open map file for reading." << endl;
        exit(-1);
    }

    int res = 2500; // res = 2500 for MIT dataset
//    int res = 100; // res = 100 for DIS basement
    int dt = 256;
    float dr = 0.125;
    int max = 10;

    vd = new VoronoiDiagram(is, res, 256, (float) 0.125);

    vd->loadPGM();
    cout << "IMAGE LOADED" << endl;
    vd->queueFiller();
    cout << "FILLING QUEUE" << endl;
    vd->distmap();
    vd->distmap2image();
    vd->savePGM("distance_map.pgm", vd->_drawableDistmap);
    vd->distmap2voronoi();
    vd->cvmat2eigenmat();
    vd->savePGM("voronoi.pgm", vd->_drawableVoromap);
    vd->vmap2eigenmat();
    vd->savePGM("prova.pgm", vd->_testVMap);
    vd->eroded2eigen();
    vd->savePGM("eroded.pgm", vd->_drawableEroded);

    vd->fillLookUpTable(dt, dr, max);

    cv::namedWindow("MyWindow", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
    cv::setMouseCallback("MyWindow", mouseEvent, 0);
    cv::namedWindow("tit", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);

    //load and display an image
    cv::Mat img = cv::imread("eroded.pgm", CV_LOAD_IMAGE_UNCHANGED);
    cv::imshow("MyWindow", img);

    cv::waitKey(0);

    //cleaning up
    cv::destroyWindow("MyWindow");
    cv::destroyWindow("tit");
    img.release();

    exit(0);
}
