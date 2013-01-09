/********************************************************************************
** Form generated from reading UI file 'pwn_gui_base_main_window.ui'
**
** Created: Wed Jan 9 09:33:41 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PWN_GUI_BASE_MAIN_WINDOW_H
#define UI_PWN_GUI_BASE_MAIN_WINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGraphicsView>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QStatusBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "QGLViewer/qglviewer.h"

QT_BEGIN_NAMESPACE

class Ui_PWNGuiBaseMainWindow
{
public:
    QAction *actionQuit;
    QAction *actionPoints;
    QAction *actionNormals;
    QAction *actionCovariances;
    QAction *actionCorrespndences;
    QAction *actionStep;
    QAction *actionAbout;
    QAction *actionMinimize;
    QAction *actionMaximize;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QCheckBox *checkBox_normals;
    QSpinBox *spinBox_step;
    QCheckBox *checkBox_correspondences;
    QCheckBox *checkBox_step;
    QCheckBox *checkBox_points;
    QCheckBox *checkBox_covariances;
    QDoubleSpinBox *doubleSpinBox_points;
    QDoubleSpinBox *doubleSpinBox_normals;
    QDoubleSpinBox *doubleSpinBox_covariances;
    QDoubleSpinBox *doubleSpinBox_correspondences;
    QCheckBox *checkBox_step_by_step;
    QPushButton *pushButton_initial_guess;
    QPushButton *pushButton_optimize;
    QGraphicsView *graphicsView1_2d;
    QGraphicsView *graphicsView2_2d;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *horizontalSpacer;
    QGLViewer *viewer_3d;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuView;
    QMenu *menuHelp;
    QMenu *menuWindow;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *PWNGuiBaseMainWindow)
    {
        if (PWNGuiBaseMainWindow->objectName().isEmpty())
            PWNGuiBaseMainWindow->setObjectName(QString::fromUtf8("PWNGuiBaseMainWindow"));
        PWNGuiBaseMainWindow->resize(789, 600);
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PWNGuiBaseMainWindow->sizePolicy().hasHeightForWidth());
        PWNGuiBaseMainWindow->setSizePolicy(sizePolicy);
        actionQuit = new QAction(PWNGuiBaseMainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionPoints = new QAction(PWNGuiBaseMainWindow);
        actionPoints->setObjectName(QString::fromUtf8("actionPoints"));
        actionNormals = new QAction(PWNGuiBaseMainWindow);
        actionNormals->setObjectName(QString::fromUtf8("actionNormals"));
        actionCovariances = new QAction(PWNGuiBaseMainWindow);
        actionCovariances->setObjectName(QString::fromUtf8("actionCovariances"));
        actionCorrespndences = new QAction(PWNGuiBaseMainWindow);
        actionCorrespndences->setObjectName(QString::fromUtf8("actionCorrespndences"));
        actionStep = new QAction(PWNGuiBaseMainWindow);
        actionStep->setObjectName(QString::fromUtf8("actionStep"));
        actionAbout = new QAction(PWNGuiBaseMainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionMinimize = new QAction(PWNGuiBaseMainWindow);
        actionMinimize->setObjectName(QString::fromUtf8("actionMinimize"));
        actionMaximize = new QAction(PWNGuiBaseMainWindow);
        actionMaximize->setObjectName(QString::fromUtf8("actionMaximize"));
        centralwidget = new QWidget(PWNGuiBaseMainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy);
        centralwidget->setMaximumSize(QSize(16777215, 16777215));
        centralwidget->setLayoutDirection(Qt::LeftToRight);
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        checkBox_normals = new QCheckBox(centralwidget);
        checkBox_normals->setObjectName(QString::fromUtf8("checkBox_normals"));
        checkBox_normals->setChecked(false);

        gridLayout->addWidget(checkBox_normals, 3, 0, 1, 1);

        spinBox_step = new QSpinBox(centralwidget);
        spinBox_step->setObjectName(QString::fromUtf8("spinBox_step"));
        spinBox_step->setEnabled(true);
        spinBox_step->setMinimum(1);

        gridLayout->addWidget(spinBox_step, 1, 1, 1, 1);

        checkBox_correspondences = new QCheckBox(centralwidget);
        checkBox_correspondences->setObjectName(QString::fromUtf8("checkBox_correspondences"));
        checkBox_correspondences->setEnabled(true);

        gridLayout->addWidget(checkBox_correspondences, 5, 0, 1, 1);

        checkBox_step = new QCheckBox(centralwidget);
        checkBox_step->setObjectName(QString::fromUtf8("checkBox_step"));
        checkBox_step->setChecked(true);

        gridLayout->addWidget(checkBox_step, 1, 0, 1, 1);

        checkBox_points = new QCheckBox(centralwidget);
        checkBox_points->setObjectName(QString::fromUtf8("checkBox_points"));
        checkBox_points->setChecked(true);

        gridLayout->addWidget(checkBox_points, 2, 0, 1, 1);

        checkBox_covariances = new QCheckBox(centralwidget);
        checkBox_covariances->setObjectName(QString::fromUtf8("checkBox_covariances"));
        checkBox_covariances->setEnabled(true);

        gridLayout->addWidget(checkBox_covariances, 4, 0, 1, 1);

        doubleSpinBox_points = new QDoubleSpinBox(centralwidget);
        doubleSpinBox_points->setObjectName(QString::fromUtf8("doubleSpinBox_points"));
        doubleSpinBox_points->setEnabled(true);
        doubleSpinBox_points->setSingleStep(0.25);
        doubleSpinBox_points->setValue(1);

        gridLayout->addWidget(doubleSpinBox_points, 2, 1, 1, 1);

        doubleSpinBox_normals = new QDoubleSpinBox(centralwidget);
        doubleSpinBox_normals->setObjectName(QString::fromUtf8("doubleSpinBox_normals"));
        doubleSpinBox_normals->setEnabled(false);
        doubleSpinBox_normals->setSingleStep(0.01);

        gridLayout->addWidget(doubleSpinBox_normals, 3, 1, 1, 1);

        doubleSpinBox_covariances = new QDoubleSpinBox(centralwidget);
        doubleSpinBox_covariances->setObjectName(QString::fromUtf8("doubleSpinBox_covariances"));
        doubleSpinBox_covariances->setEnabled(false);
        doubleSpinBox_covariances->setSingleStep(0.01);

        gridLayout->addWidget(doubleSpinBox_covariances, 4, 1, 1, 1);

        doubleSpinBox_correspondences = new QDoubleSpinBox(centralwidget);
        doubleSpinBox_correspondences->setObjectName(QString::fromUtf8("doubleSpinBox_correspondences"));
        doubleSpinBox_correspondences->setEnabled(false);
        doubleSpinBox_correspondences->setSingleStep(0.25);

        gridLayout->addWidget(doubleSpinBox_correspondences, 5, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        checkBox_step_by_step = new QCheckBox(centralwidget);
        checkBox_step_by_step->setObjectName(QString::fromUtf8("checkBox_step_by_step"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(checkBox_step_by_step->sizePolicy().hasHeightForWidth());
        checkBox_step_by_step->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(checkBox_step_by_step);

        pushButton_initial_guess = new QPushButton(centralwidget);
        pushButton_initial_guess->setObjectName(QString::fromUtf8("pushButton_initial_guess"));
        sizePolicy1.setHeightForWidth(pushButton_initial_guess->sizePolicy().hasHeightForWidth());
        pushButton_initial_guess->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(pushButton_initial_guess);

        pushButton_optimize = new QPushButton(centralwidget);
        pushButton_optimize->setObjectName(QString::fromUtf8("pushButton_optimize"));
        sizePolicy1.setHeightForWidth(pushButton_optimize->sizePolicy().hasHeightForWidth());
        pushButton_optimize->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(pushButton_optimize);

        graphicsView1_2d = new QGraphicsView(centralwidget);
        graphicsView1_2d->setObjectName(QString::fromUtf8("graphicsView1_2d"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(graphicsView1_2d->sizePolicy().hasHeightForWidth());
        graphicsView1_2d->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(graphicsView1_2d);

        graphicsView2_2d = new QGraphicsView(centralwidget);
        graphicsView2_2d->setObjectName(QString::fromUtf8("graphicsView2_2d"));
        sizePolicy2.setHeightForWidth(graphicsView2_2d->sizePolicy().hasHeightForWidth());
        graphicsView2_2d->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(graphicsView2_2d);


        horizontalLayout->addLayout(verticalLayout);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalSpacer = new QSpacerItem(10000, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout_2->addItem(horizontalSpacer);

        viewer_3d = new QGLViewer(centralwidget);
        viewer_3d->setObjectName(QString::fromUtf8("viewer_3d"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(viewer_3d->sizePolicy().hasHeightForWidth());
        viewer_3d->setSizePolicy(sizePolicy3);

        verticalLayout_2->addWidget(viewer_3d);


        horizontalLayout->addLayout(verticalLayout_2);

        PWNGuiBaseMainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(PWNGuiBaseMainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 789, 25));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuView = new QMenu(menubar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        menuWindow = new QMenu(menubar);
        menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
        PWNGuiBaseMainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(PWNGuiBaseMainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        PWNGuiBaseMainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuWindow->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuView->addSeparator();

        retranslateUi(PWNGuiBaseMainWindow);
        QObject::connect(checkBox_step, SIGNAL(toggled(bool)), spinBox_step, SLOT(setEnabled(bool)));
        QObject::connect(checkBox_points, SIGNAL(toggled(bool)), doubleSpinBox_points, SLOT(setEnabled(bool)));
        QObject::connect(checkBox_normals, SIGNAL(toggled(bool)), doubleSpinBox_normals, SLOT(setEnabled(bool)));
        QObject::connect(checkBox_covariances, SIGNAL(toggled(bool)), doubleSpinBox_covariances, SLOT(setEnabled(bool)));
        QObject::connect(checkBox_correspondences, SIGNAL(toggled(bool)), doubleSpinBox_correspondences, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(PWNGuiBaseMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *PWNGuiBaseMainWindow)
    {
        PWNGuiBaseMainWindow->setWindowTitle(QApplication::translate("PWNGuiBaseMainWindow", "PWN Gui", 0, QApplication::UnicodeUTF8));
        actionQuit->setText(QApplication::translate("PWNGuiBaseMainWindow", "Quit", 0, QApplication::UnicodeUTF8));
        actionPoints->setText(QApplication::translate("PWNGuiBaseMainWindow", "Points", 0, QApplication::UnicodeUTF8));
        actionNormals->setText(QApplication::translate("PWNGuiBaseMainWindow", "Normals", 0, QApplication::UnicodeUTF8));
        actionCovariances->setText(QApplication::translate("PWNGuiBaseMainWindow", "Covariances", 0, QApplication::UnicodeUTF8));
        actionCorrespndences->setText(QApplication::translate("PWNGuiBaseMainWindow", "Correspndences", 0, QApplication::UnicodeUTF8));
        actionStep->setText(QApplication::translate("PWNGuiBaseMainWindow", "Step", 0, QApplication::UnicodeUTF8));
        actionAbout->setText(QApplication::translate("PWNGuiBaseMainWindow", "About DM Viewer", 0, QApplication::UnicodeUTF8));
        actionMinimize->setText(QApplication::translate("PWNGuiBaseMainWindow", "Minimize", 0, QApplication::UnicodeUTF8));
        actionMaximize->setText(QApplication::translate("PWNGuiBaseMainWindow", "Maximize", 0, QApplication::UnicodeUTF8));
        checkBox_normals->setText(QApplication::translate("PWNGuiBaseMainWindow", "Normals", 0, QApplication::UnicodeUTF8));
        checkBox_correspondences->setText(QApplication::translate("PWNGuiBaseMainWindow", "Correspondences", 0, QApplication::UnicodeUTF8));
        checkBox_step->setText(QApplication::translate("PWNGuiBaseMainWindow", "Step", 0, QApplication::UnicodeUTF8));
        checkBox_points->setText(QApplication::translate("PWNGuiBaseMainWindow", "Points", 0, QApplication::UnicodeUTF8));
        checkBox_covariances->setText(QApplication::translate("PWNGuiBaseMainWindow", "Covariances", 0, QApplication::UnicodeUTF8));
        checkBox_step_by_step->setText(QApplication::translate("PWNGuiBaseMainWindow", "Step-By-Step", 0, QApplication::UnicodeUTF8));
        pushButton_initial_guess->setText(QApplication::translate("PWNGuiBaseMainWindow", "Initial Guess", 0, QApplication::UnicodeUTF8));
        pushButton_optimize->setText(QApplication::translate("PWNGuiBaseMainWindow", "Optimize", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("PWNGuiBaseMainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuView->setTitle(QApplication::translate("PWNGuiBaseMainWindow", "View", 0, QApplication::UnicodeUTF8));
        menuHelp->setTitle(QApplication::translate("PWNGuiBaseMainWindow", "Help", 0, QApplication::UnicodeUTF8));
        menuWindow->setTitle(QApplication::translate("PWNGuiBaseMainWindow", "Window", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class PWNGuiBaseMainWindow: public Ui_PWNGuiBaseMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PWN_GUI_BASE_MAIN_WINDOW_H
