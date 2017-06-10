#pragma once
#include<opencv2/core/core.hpp>

enum THINNING_METHOD{
  ZHANGSUEN,
  TRAPPEDBALL
};

class skeleton{
private:
  void ZhangSuenIteration( int );
  void ZhangSuenThinning( bool );
public: 
  cv::Mat binary;
  cv::Mat thickness;
  skeleton( cv::Mat );
  void thinning( THINNING_METHOD = TRAPPEDBALL, bool = false );
};
