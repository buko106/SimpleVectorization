#include"utils.hpp"
#include<random>
#include<iomanip>
cv::Mat_<uchar> imread_as_grayscale( std::string filename , bool inv ){
  int flag = 0; // 0 for grayscale, 1 for color
  cv::Mat_<uchar> image = cv::imread( filename, flag );
  if( inv )
    image = 255 - image;
  return image;
}

cv::Mat_<uchar> to_binary_image( cv::Mat gray, double thresh ){
  cv::Mat_<uchar> dst;
  double maxValue=255;
  double dummy=0;
  
  if( (thresh<0) | (thresh>255) ){
    cv::threshold(gray,dst,dummy,maxValue,cv::THRESH_BINARY|cv::THRESH_OTSU);
  }else{
    cv::threshold(gray,dst,thresh,maxValue,cv::THRESH_BINARY);
  }
  
  return dst;
}

std::random_device rd;
std::mt19937 mt(rd());

int random_generate_int( int a, int b ){
  std::uniform_int_distribution<int> uniform_int(a,b);
  return uniform_int(mt);
}

double random_generate_uniform( double a, double b ){
  std::uniform_real_distribution<double> uniform_double(a,b);
  return uniform_double(mt);
}

std::string random_generate_rgb(){
  int full = random_generate_int(0,2);
  int zero;
  while( full == ( zero = random_generate_int(0,2))){;}
  int color[3];
  for( int i = 0 ; i < 3 ; ++i ){
    if( i == zero )      color[i] = 0x0;
    else if( i == full ) color[i] = 0xf;
    else                 color[i] = random_generate_int(0,5)*3;
  }
    
  std::stringstream stream;
  stream << "#"
         << std::setfill ('0')
         // << std::setw(2)
         << std::hex
         << color[0] << color[1] << color[2];
  return stream.str();
}
