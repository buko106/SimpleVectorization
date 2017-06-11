#pragma once
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

cv::Mat imread_as_grayscale( std::string , bool=false );
cv::Mat to_binary_image( cv::Mat, double=-1 );
