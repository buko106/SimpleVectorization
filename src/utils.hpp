#pragma once
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

cv::Mat_<uchar> imread_as_grayscale( std::string , bool=false );
cv::Mat_<uchar> to_binary_image( cv::Mat, double=-1 );
