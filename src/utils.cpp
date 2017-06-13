#include"utils.hpp"

cv::Mat_<uchar> imread_as_grayscale( std::string filename , bool inv ){
  int flag = 0; // 0 for grayscale, 1 for color
  cv::Mat_<uchar> image = cv::imread( filename, flag );
  if( inv )
    image = 255 - image;
  return image;
}

cv::Mat_<uchar> to_binary_image( cv::Mat gray, double thresh ){
  cv::Mat_<uchar> dst;
  double maxValue=255;
  double dummy=0;
  
  if( (thresh<0) | (thresh>255) ){
    cv::threshold(gray,dst,dummy,maxValue,cv::THRESH_BINARY|cv::THRESH_OTSU);
  }else{
    cv::threshold(gray,dst,thresh,maxValue,cv::THRESH_BINARY);
  }
  
  return dst;
}
