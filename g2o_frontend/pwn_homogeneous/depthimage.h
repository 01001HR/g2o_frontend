/** \file depthimage.h
 *  \brief DepthImage class header file.
 *  This is the header file of the DepthImage class.
*/

#ifndef _DEPTH_IMAGE_H_
#define _DEPTH_IMAGE_H_
#include <Eigen/Core>
#include <Eigen/Geometry>

/** \typedef MatrixXus
 *  \brief Unsigned short matrix type.
 *
 *  The MatrixXus type it's an Eigen matrix of unsigned short with generic dimensions.
*/
typedef Eigen::Matrix<unsigned short, Eigen::Dynamic, Eigen::Dynamic> MatrixXus;

/**
 *  Base class for input/output operations on depth images. 
 *  Depth images are basically matrices of unsigned short, this class extends the 
 *  Eigen MatrixXf class so that it is possible to do the basic input/output operations with
 *  depth images. The DepthImage object will contain the depth image values expressed in 
 *  metrs.
 */

class DepthImage: public Eigen::MatrixXf{
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  
  /**
   *  Empty constructor.
   *  This constructor creates a DepthImage object with zero columns and zero rows. If the 
   *  optional input parameters are given and they are both different from zero the matrix 
   *  is filled with the maximum float representable.
   *  @param r is optional and can be used to define the number of columns of the matrix
   *  representing a depth image.
   *  @param c is optional and can be used to define the number of rows of the matrix
   *  representing a depth image.
   */
  DepthImage(int r=0, int c=0);
  
  /**
   *  Constructor using a matrix of unsigned short.
   *  This constructor creates a DepthImage object taking the values from a matrix of 
   *  unsigned short.
   *  @param m is a matrix of unsigned short containing the depth values expressed 
   *  in millimeters.
   */
  DepthImage(const MatrixXus& m);
  
  /**
   *  This method generates a matrix of unsigned short containing the values of the 
   *  DepthImage object expressed in millimeters. If the optional input parameter is given,
   *  then all the values above it will be pruned and setted to zero.
   *  @param m is a matrix of unsigned short where the output values are stored.
   *  @param dmax is optional and can be used to prune depth values above its value.
   */
  void toUnsignedShort(MatrixXus& m, float dmax = std::numeric_limits<float>::max()) const;
  
  /**
   *  This method updates the current depth values of the DepthImage object using the values
   *  inside the input unsigned short matrix.
   *  @param m is a matrix of unsigned short containing the depth values expressed 
   *  in millimeters.
   */
  void fromUnsignedShort(const MatrixXus& m);
  
  /**
   *  This method laods the values of a depth image file in the DepthImage object. 
   *  The file must be a .pgm image file.
   *  @param filename is a pointer to a string containing the filename of a .pgm depth image
   *  to load.
   */
  bool load(const char* filename); 
  
  /**
   *  This method saves the current depth values of the DepthImage object to an image file.
   *  The output image is saved in .pgm format. 
   *  @param filename is a pointer to a string containing the filename of the .pgm depth 
   *  image to save.
   */
  bool save(const char* filename) const;
};


#endif