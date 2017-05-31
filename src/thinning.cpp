#include"thinning.hpp"
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>

skeleton::skeleton( cv::Mat input, bool binarize ){
  if( binarize ){
    int dummy  = 0;
    int maxVal = 255;
    cv::threshold( input, binary, dummy, maxVal, cv::THRESH_BINARY|cv::THRESH_OTSU);
  }else{
    binary = input;
  }
}

void skeleton::thinning( THINNING_METHOD method ){
  if( ZHANG != method ){
    std::cerr << "[ERROR] skeleton::thinning : Unknown method" << std::endl;
    exit(1);
  }
  thickness = cv::Mat::zeros( binary.size(),  CV_8UC1 );
  return;
}
