ADD_LIBRARY(slam_library ${G2O_FRONTEND_LIB_TYPE}
  feature_tracker.cpp feature_tracker.h
  feature_tracker_closure.cpp feature_tracker_closure.h
  feature_tracker_pointxy.cpp feature_tracker_pointxy.h
 )

SET_TARGET_PROPERTIES(slam_library PROPERTIES OUTPUT_NAME ${LIB_PREFIX}_slam)

IF(CMAKE_BUILD_TYPE MATCHES Release)
  TARGET_LINK_LIBRARIES(slam_library
    types_data
    ${G2O_TYPES_SLAM2D}
    ${G2O_TYPES_SLAM3D}
    ${G2O_TYPES_DATA}
    ${G2O_CORE_LIBRARY}
    ${G2O_STUFF_LIBRARY}
  )
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  TARGET_LINK_LIBRARIES(slam_library
    types_data
    ${G2O_TYPES_SLAM2D_DEBUG}
    ${G2O_TYPES_SLAM3D_DEBUG}
    ${G2O_TYPES_DATA_DEBUG}
    ${G2O_CORE_LIBRARY_DEBUG}
    ${G2O_STUFF_LIBRARY_DEBUG}
  )
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)
IF(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  
ENDIF(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
IF(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
  
ENDIF(CMAKE_BUILD_TYPE MATCHES MinSizeRel)

ADD_EXECUTABLE(tracker_test_application
  tracker_test.cpp
)

INCLUDE_DIRECTORIES(tracker_test_application ${CSPARSE_INCLUDE_DIR})

IF(CMAKE_BUILD_TYPE MATCHES Release)
  TARGET_LINK_LIBRARIES(tracker_test_application
    slam_library
    types_data
    ${G2O_TYPES_SLAM3D}
    ${G2O_TYPES_SLAM2D}
    ${G2O_TYPES_DATA}
    ${G2O_CORE_LIBRARY}
    ${G2O_SOLVER_CSPARSE}
    ${G2O_STUFF_LIBRARY}
    ${G2O_SOLVER_CSPARSE_EXTENSION}
    ${CSPARSE_LIBRARY}
  )
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)
IF(CMAKE_BUILD_TYPE MATCHES Debug)
  TARGET_LINK_LIBRARIES(tracker_test_application
    slam_library
    types_data
    ${G2O_TYPES_SLAM3D_DEBUG}
    ${G2O_TYPES_SLAM2D_DEBUG}
    ${G2O_TYPES_DATA_DEBUG}
    ${G2O_CORE_LIBRARY_DEBUG}
    ${G2O_STUFF_LIBRARY_DEBUG}
    ${G2O_SOLVER_CSPARSE_DEBUG}
    ${G2O_SOLVER_CSPARSE_EXTENSION_DEBUG}
    ${CSPARSE_LIBRARY}
  )
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)


SET_TARGET_PROPERTIES(tracker_test_application PROPERTIES OUTPUT_NAME tracker_test${EXE_POSTFIX})

ADD_EXECUTABLE(g2o_anonymize_observations_application   g2o_anonymize_observations.cpp
)
TARGET_LINK_LIBRARIES(g2o_anonymize_observations_application
  types_data
  ${G2O_TYPES_SLAM3D}
  ${G2O_TYPES_SLAM2D}
  ${G2O_TYPES_DATA}
  ${G2O_CORE_LIBRARY} 
  ${G2O_STUFF_LIBRARY}
)
SET_TARGET_PROPERTIES(g2o_anonymize_observations_application PROPERTIES OUTPUT_NAME g2o_anonymize_observations${EXE_POSTFIX})

# ADD_EXECUTABLE(slam_simulator3d_application
#   test_simulator3d.cpp
# )
# TARGET_LINK_LIBRARIES(slam_simulator3d_application  
#   slam_simulator_library 
#   types_data
#   ${G2O_TYPES_SLAM3D}
#   ${G2O_TYPES_SLAM2D}
#   ${G2O_TYPES_DATA}
#   ${G2O_CORE_LIBRARY}
#   ${G2O_STUFF_LIBRARY}
# )

# SET_TARGET_PROPERTIES(slam_simulator3d_application PROPERTIES OUTPUT_NAME slam_simulator3d${EXE_POSTFIX})


# INSTALL(TARGETS g2o_simulator_library g2o_simulator2d_application g2o_simulator3d_application
#   RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
#   LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#   ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
# )

# FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
# INSTALL(FILES ${headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/g2o/apps/g2o_simulator)
