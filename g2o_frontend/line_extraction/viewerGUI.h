/*
 * viewerGUI.h
 *
 *  Created on: Dic 05, 2012
 *      Author: Martina
 */

#ifndef VIEWERGUI_H_
#define VIEWERGUI_H_

#include "viewer.h"
#include "IEdgesExtractor.h"
#include "line_extraction2d.h"
#include <Qt/qapplication.h>

//PCL
// #include <pcl/point_cloud.h>
// #include <pcl/ModelCoefficients.h>
// #include <pcl/io/pcd_io.h>
// #include <pcl/point_types.h>
// #include <pcl/sample_consensus/method_types.h>
// #include <pcl/sample_consensus/model_types.h>
// #include <pcl/segmentation/sac_segmentation.h>
// #include <pcl/filters/passthrough.h>


typedef std::pair<LaserRobotData*, LaserRobotData::Vector2fVector> LaserData;
typedef std::vector<LaserData> LaserDataVector;


class ViewerGUI : public QMainWindow, public Ui::MainWindow
{
	 Q_OBJECT

	public:
// 		ViewerGUI(LaserRobotData* theLaserData/*, Vector2fVector* theLinesPoints*/, Vector2fVector* theOriginalPoints, QWidget *parent=0);
		ViewerGUI(LaserDataVector* TheLdvector, QWidget *parent=0);

		int slider1value;
		int slider2value;
		int slider3value;
		
		std::string algotype;
		IEdgesExtractor* edgeExtr;
		Line2DExtractor* lineExtractor;
		Point2DClusterer* clusterer;
		/*Vector2fVector* linesFoundPoints;*/
// 		LaserRobotData* laserData;		
// 		Vector2fVector* originalPoints;
		LaserDataVector* ldvector;
		lineContainer lc;
		int numIteration;

	public slots:
	 void updateVal1(int val);
	 void updateVal2(int val);
	 void updateVal3(int val);
	 void showOriginal();
	 void lineExtraction();
	 void setAlgorithm();
	 void setIdIteration();
};

#endif
