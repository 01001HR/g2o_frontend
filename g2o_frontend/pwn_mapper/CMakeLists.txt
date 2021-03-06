IF(CUDA_FOUND)  
  set(BUILD_SHARED_LIBS OFF) 
endif(CUDA_FOUND)

FIND_PACKAGE(OpenMP)

if(OPENMP_FOUND)
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -D_PWN_USE_OPENMP_ -DEIGEN_DONT_PARALLELIZE")
 set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif(OPENMP_FOUND)

INCLUDE_DIRECTORIES(${EIGEN_INCLUDE_DIR} ${CSPARSE_INCLUDE_DIR})

ADD_LIBRARY(pwn_mapper_library
  g2o_frame.h
  pwn_mapper_controller.cpp pwn_mapper_controller.h
  pwn_loop_closer_controller.cpp pwn_loop_closer_controller.h
)

SET_TARGET_PROPERTIES(pwn_mapper_library PROPERTIES OUTPUT_NAME pwn_mapper)


TARGET_LINK_LIBRARIES(pwn_mapper_library
  pwn_core
  sensor_data_library
  ${G2O_TYPES_DATA}
  ${G2O_TYPES_SLAM2D}
  ${G2O_TYPES_SLAM3D}
  ${G2O_STUFF_LIBRARY}
  ${G2O_CORE_LIBRARY}
  ${G2O_SOLVER_CSPARSE}
  ${G2O_STUFF_LIBRARY}
  ${G2O_SOLVER_CSPARSE_EXTENSION}
  ${CSPARSE_LIBRARY}
)

IF(TRAVERSABILITY_FOUND)
TARGET_LINK_LIBRARIES(pwn_mapper_library
  traverse_library
)
endif(TRAVERSABILITY_FOUND)
  

#ADD_EXECUTABLE(pwn_mapper_test
#  pwn_mapper_test.cpp 
#)

#SET_TARGET_PROPERTIES(pwn_mapper_test PROPERTIES OUTPUT_NAME pwn_mapper_test)

#TARGET_LINK_LIBRARIES(
#  pwn_mapper_test
#  pwn_mapper_library
#  pwn_core
#  sensor_data_library
#  ${G2O_TYPES_DATA}
#  ${G2O_TYPES_SLAM2D}
#  ${G2O_TYPES_SLAM3D}
#  ${G2O_STUFF_LIBRARY}
#  ${G2O_CORE_LIBRARY}
#  ${G2O_SOLVER_CSPARSE}
#  ${G2O_STUFF_LIBRARY}
#  ${G2O_SOLVER_CSPARSE_EXTENSION}
#  ${CSPARSE_LIBRARY})



IF(CUDA_FOUND)  
  set(BUILD_SHARED_LIBS OFF) 
  TARGET_LINK_LIBRARIES(pwn_mapper_library pwn_cuda_library ${CUDA_CUDART_LIBRARY}) 
#  TARGET_LINK_LIBRARIES(pwn_mapper_test pwn_cuda_library ${CUDA_CUDART_LIBRARY})
ENDIF(CUDA_FOUND)