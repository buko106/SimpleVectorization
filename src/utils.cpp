#include"utils.hpp"

cv::Mat imread_as_grayscale( std::string filename ){
  int flag = 0; // 0 for grayscale, 1 for color
  return cv::imread( filename, flag );
}
