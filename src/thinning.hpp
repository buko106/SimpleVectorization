#pragma once
#include<opencv2/core/core.hpp>

enum THINNING_METHOD{
  ZHANG,
  TRAPPEDBALL
};

class skeleton{
public: 
  cv::Mat binary;
  cv::Mat thickness;
  skeleton( cv::Mat, bool = false );
  void thinning( THINNING_METHOD = TRAPPEDBALL );
};
