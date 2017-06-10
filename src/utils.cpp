#include"utils.hpp"

cv::Mat imread_as_grayscale( std::string filename , bool inv ){
  int flag = 0; // 0 for grayscale, 1 for color
  cv::Mat image = cv::imread( filename, flag );
  if( inv )
    image = 255 - image;
  return image;
}

cv::Mat to_binary_image( cv::Mat gray, bool adaptive, int blockSize ){
  cv::Mat dst;
  double maxValue=255;
  if( adaptive ){
    double C=0; // constant offset of Threashold value
    cv::adaptiveThreshold(gray,
                          dst,
                          maxValue,
                          cv::ADAPTIVE_THRESH_MEAN_C,
                          cv::THRESH_BINARY,
                          blockSize,
                          C);
  }else{
    double dummy=0;
    cv::threshold(gray,dst,dummy,maxValue,cv::THRESH_BINARY|cv::THRESH_OTSU);
  }

  return dst;
}
