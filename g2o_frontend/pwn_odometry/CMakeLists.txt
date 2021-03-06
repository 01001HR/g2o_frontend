INCLUDE_DIRECTORIES(${EIGEN_INCLUDE_DIR} ${PCL_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${QT_INCLUDES})
LINK_DIRECTORIES(${PCL_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS})

FIND_PACKAGE(OpenMP)

if(OPENMP_FOUND)
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -D_PWN_USE_OPENMP_ -DEIGEN_DONT_PARALLELIZE")
 set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif(OPENMP_FOUND)

ADD_EXECUTABLE(pwn_odometry pwn_odometry.cpp
  odometry_controller.cpp odometry_controller.h
  pwn_odometry_controller.cpp pwn_odometry_controller.h
)
SET_TARGET_PROPERTIES(pwn_odometry PROPERTIES OUTPUT_NAME pwn_odometry)
TARGET_LINK_LIBRARIES(pwn_odometry
  pwn_core pwn_boss pwn_utils pwn_viewer
  ${QGLVIEWER_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} 
  ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} 
  ${G2O_STUFF_LIBRARY} ${G2O_OPENGL_HELPER_LIBRARY}
)

ADD_EXECUTABLE(pwn_odometry_sequential pwn_odometry_sequential.cpp
  odometry_controller.cpp odometry_controller.h
  pwn_odometry_sequential_controller.cpp pwn_odometry_sequential_controller.h
)
SET_TARGET_PROPERTIES(pwn_odometry_sequential PROPERTIES OUTPUT_NAME pwn_odometry_sequential)
TARGET_LINK_LIBRARIES(pwn_odometry_sequential
  pwn_core pwn_boss pwn_utils pwn_viewer
  ${QGLVIEWER_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} 
  ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} 
  ${G2O_STUFF_LIBRARY} ${G2O_OPENGL_HELPER_LIBRARY}
)

# ADD_EXECUTABLE(gicp_odometry gicp_odometry.cpp
#   odometry_controller.cpp odometry_controller.h
#   gicp_odometry_controller.cpp gicp_odometry_controller.h)
# SET_TARGET_PROPERTIES(gicp_odometry PROPERTIES OUTPUT_NAME gicp_odometry)
# TARGET_LINK_LIBRARIES(gicp_odometry
# 	pwn_core pwn_utils pwn_viewer
#   ${QGLVIEWER_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} 
#   ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} 
#   ${G2O_STUFF_LIBRARY} ${G2O_OPENGL_HELPER_LIBRARY}
#   ${PCL_LIBRARIES}
# )
