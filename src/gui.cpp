#include"gui.hpp"
#include"bezier.hpp"

cv::RNG rng( time(NULL) );

std::pair<double,double>bezier_to_point( const bezier& curve, double tp ){
  const double choose[4][4] =
    { { 0.0, 0.0, 0.0, 0.0 },
      { 1.0, 1.0, 0.0, 0.0 },
      { 1.0, 2.0, 1.0, 0.0 },
      { 1.0, 3.0, 3.0, 1.0 } };
  size_t deg = curve.size() - 1 ;
  double t[4] = {};
  for( size_t i = 0 ; i < (deg+1) ; ++i ){
    t[i] = choose[deg][i] ;
    for( size_t j = 0 ; j < i ; ++j )
      t[i] *= (1-tp);
    for( size_t j = 0 ; j < (deg-i) ; ++j )
      t[i] *= tp;
  }

  double x = 0.0;
  double y = 0.0;
  for( size_t i = 0 ; i < (deg+1) ; ++i ){
    x += t[i] * curve[i].first;
    y += t[i] * curve[i].second;
  }
  return std::make_pair(x,y);
}

cv::Mat to_Mat( const std::vector<bezier>& curves, int x0, int y0, int x1, int y1, bool random, size_t sample ){
  cv::Scalar blue(255,0,0),red(0,0,255),white(255,255,255);
  cv::Mat img = cv::Mat(y1-y0,x1-x0,CV_8UC3,white);
  int thickness=1;
  int lineType=8;
  int radius=2;
  cv::Point* p = new cv::Point[sample];
  
  for( size_t i = 0 ; i < curves.size() ; ++i ){
    cv::Scalar line_color = ( random ? cv::Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255)) : blue );
    cv::Scalar circle_color = ( random ? cv::Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255)) : red );
    for( size_t j = 0 ; j < sample ; ++j ){
      double tp = static_cast<double>(j) / static_cast<double>(sample-1);
      std::pair<double,double> res = bezier_to_point( curves[i], tp );
      p[j].x = cv::saturate_cast<int>(res.first);
      p[j].y = cv::saturate_cast<int>(res.second);
    }
    for( size_t j = 0 ; j < sample-1 ; ++j ){
      cv::line( img, p[j], p[j+1], line_color, thickness, lineType );
    }
    cv::circle(img,p[0],radius,circle_color,-1);
    cv::circle(img,p[sample-1],radius,circle_color,-1);
  }

  return img;
}
