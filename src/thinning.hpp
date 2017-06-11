#pragma once
#include<opencv2/core/core.hpp>

enum THINNING_METHOD{
  ZHANGSUEN,
  TRAPPEDBALL
};

class skeleton{
private:
  void ZhangSuenIteration( int );
  void ZhangSuenThinning();
  void TrappedBallOpening( int );
  void TrappedBallThinning( int );
public: 
  cv::Mat binary;
  cv::Mat thickness;
  skeleton( cv::Mat );
  void thinning( THINNING_METHOD = TRAPPEDBALL, bool = false, double = -1 );
};
