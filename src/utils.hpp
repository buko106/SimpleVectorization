#pragma once
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

cv::Mat_<uchar> imread_as_grayscale( std::string , bool=false );
cv::Mat_<uchar> to_binary_image( cv::Mat, double=-1 );

double random_generate_uniform( double, double ); // x \in (a,b) \subset R
int random_generate_int( int, int ); // x \in [a,b] \subset Z
std::string random_generate_rgb();
