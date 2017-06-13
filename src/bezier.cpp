#include"bezier.hpp"
#include<iostream>

std::pair<double,std::vector<std::pair<double,double> > > 
bezier_fittting( const edge_t &edge, double w_max, BEZIER_DIM dim ){
  if( dim == LINE ){
    return bezier_line_fitting( edge, w_max );
  }else if( dim == QUAD ){
    return bezier_quadratic_fitting( edge, w_max );
  }else if( dim == CUBIC ){
    return bezier_cubic_fitting( edge, w_max );
  }
  
  std::cerr << "[ERROR] bezier_fitting : dim = " << dim << " is not supported" << std::endl;
  exit(1);
}


std::pair<double,std::vector<std::pair<double,double> > >
bezier_line_fitting( const edge_t &edge, double w_max ){
  size_t N = edge.size();

  double px0 = edge[  0].x; double py0 = edge[  0].y;
  double px1 = edge[N-1].x; double py1 = edge[N-1].y;
  std::vector<double> wp(N);
  for( size_t i = 0 ; i < N ; ++i ){
    wp[i] = edge[i].w/w_max;
  }

  double err = 0.0;
  for( size_t i = 0 ; i < N ; ++i ){
    double t = static_cast<double>(i) / static_cast<double>(N);
    double weight = 1.0 - wp[i]/2.0;
    double t0 = (1.0-t) ;
    double t1 =      t  ;
    double px = edge[i].x;
    double py = edge[i].y;
    double diff_x = t0 * px0 + t1 * px1 - px;
    double diff_y = t0 * py0 + t1 * py1 - py;
    err += weight * ( diff_x * diff_x + diff_y * diff_y );
  }
  
  std::vector<std::pair<double,double> > ret;
  ret.push_back(std::make_pair(px0,py0));
  ret.push_back(std::make_pair(px1,py1));
  return std::make_pair(err,ret);
}

std::pair<double,std::vector<std::pair<double,double> > >
bezier_quadratic_fitting( const edge_t &edge, double w_max ){
  size_t N = edge.size();

  double px0 = edge[  0].x; double py0 = edge[  0].y;
  double px2 = edge[N-1].x; double py2 = edge[N-1].y;
  std::vector<double> wp(N);
  for( size_t i = 0 ; i < N ; ++i ){
    wp[i] = edge[i].w/w_max;
  }

  // generate equation
  double C  = 0.0;
  double CX = 0.0;
  double CY = 0.0;
  for( size_t i = 0 ; i < N ; ++i ){
    double t = static_cast<double>(i) / static_cast<double>(N);
    double weight = 1.0 - wp[i]/2.0;
    double t0 = (1.0-t) * (1.0-t) ;
    double t1 =      t  * (1.0-t) ;
    double t2 =      t  *      t  ;
    double px = edge[i].x;
    double py = edge[i].y;
    C  += weight * t1 * 2 * t1;
    CX += weight * t1 * ( t0 * px0 + t2 * px2 - px );
    CY += weight * t1 * ( t0 * py0 + t2 * py2 - py );
  }

  double px1 = -CX/C;
  double py1 = -CY/C;

  double err = 0.0;

  for( size_t i = 0 ; i < N ; ++i ){
    double t = static_cast<double>(i) / static_cast<double>(N);
    double weight = 1.0 - wp[i]/2.0;
    double t0 = (1.0-t) * (1.0-t) ;
    double t1 =      t  * (1.0-t) ;
    double t2 =      t  *      t  ;
    double px = edge[i].x;
    double py = edge[i].y;
    double diff_x = t0 * px0 + 2 * t1 * px1 + t2 * px2 - px;
    double diff_y = t0 * py0 + 2 * t1 * py1 + t2 * py2 - py;
    err += weight * ( diff_x * diff_x + diff_y * diff_y );
  }
  
  std::vector<std::pair<double,double> > ret;
  ret.push_back(std::make_pair(px0,py0));
  ret.push_back(std::make_pair(px1,py1));
  ret.push_back(std::make_pair(px2,py2));
  return std::make_pair(err,ret);
}

std::pair<double,std::vector<std::pair<double,double> > >
bezier_cubic_fitting( const edge_t &edge, double w_max ){
  size_t N = edge.size();
  
  double px0 = edge[  0].x; double py0 = edge[  0].y;
  double px3 = edge[N-1].x; double py3 = edge[N-1].y;
  std::vector<double> wp(N);
  for( size_t i = 0 ; i < N ; ++i ){
    wp[i] = edge[i].w/w_max;
  }
  
  // generate equation
  double C11 = 0.0; 
  double C22 = 0.0; 
  double C12 = 0.0;
  // C12 = C21
  double CX1 = 0.0; double CX2 = 0.0;
  double CY1 = 0.0; double CY2 = 0.0;
  
  for( size_t i = 0 ; i < N ; ++i ){
    double t = static_cast<double>(i) / static_cast<double>(N);
    double weight = 1.0 - wp[i]/2.0;
    double t0 = (1.0-t) * (1.0-t) * (1.0-t);
    double t1 =      t  * (1.0-t) * (1.0-t);
    double t2 =      t  *      t  * (1.0-t);
    double t3 =      t  *      t  *      t ;
    double px = edge[i].x;
    double py = edge[i].y;
    C11 += weight * t1 * 3 * t1;
    C22 += weight * t2 * 3 * t2;
    C12 += weight * t1 * 3 * t2;
    CX1 += weight * t1 * ( t0 * px0 + t3 * px3 - px );
    CX2 += weight * t2 * ( t0 * px0 + t3 * px3 - px );
    CY1 += weight * t1 * ( t0 * py0 + t3 * py3 - py );
    CY2 += weight * t2 * ( t0 * py0 + t3 * py3 - py );
  }

  // Eigen::Matrix2d A;
  // A  << (C11+0.000001), C12, C12, (C22+0.000001) ;
  // Eigen::Vector2d bx,by;
  // bx << -CX1, -CX2;
  // by << -CY1, -CY2;
  // Eigen::Vector2d x12,y12;
  // x12 = A.fullPivLu().solve(bx);
  // y12 = A.fullPivLu().solve(by);
  // double px1 = x12(0); double py1 = y12(0);
  // double px2 = x12(1); double py2 = y12(1);

  cv::Mat_<double> A(2,2);
  A << (C11+0.00001), C12, C12, (C22+0.00001);
  cv::Mat_<double> bx(2,1),by(2,1),x12,y12;
  bx << -CX1, -CX2;
  by << -CY1, -CY2;

  cv::solve(A,bx,x12);
  cv::solve(A,by,y12);

  double px1 = x12(0,0); double py1 = y12(0,0);
  double px2 = x12(1,0); double py2 = y12(1,0);

  
  double err = 0.0;
  
  for( size_t i = 0 ; i < N ; ++i ){
    double t = static_cast<double>(i) / static_cast<double>(N);
    double weight = 1.0 - wp[i]/2.0;
    double t0 = (1.0-t) * (1.0-t) * (1.0-t);
    double t1 =      t  * (1.0-t) * (1.0-t);
    double t2 =      t  *      t  * (1.0-t);
    double t3 =      t  *      t  *      t ;
    double px = edge[i].x;
    double py = edge[i].y;
    double diff_x = t0 * px0 + 3 * t1 * px1 + 3 * t2 * px2 + t3 * px3 - px;
    double diff_y = t0 * py0 + 3 * t1 * py1 + 3 * t2 * py2 + t3 * py3 - py;
    err += weight * ( diff_x * diff_x + diff_y * diff_y );
  }
  
  std::vector<std::pair<double,double> > ret;
  ret.push_back(std::make_pair(px0,py0));
  ret.push_back(std::make_pair(px1,py1));
  ret.push_back(std::make_pair(px2,py2));
  ret.push_back(std::make_pair(px3,py3));
  return std::make_pair(err,ret);
}
