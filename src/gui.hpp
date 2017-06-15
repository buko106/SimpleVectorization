#include<opencv2/core/core.hpp>
#include"bezier.hpp"

cv::Mat to_Mat( const std::vector<bezier>&, int, int, int, int, bool=false, size_t=20 );
