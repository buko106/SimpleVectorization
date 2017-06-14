#include"bezier.hpp"
#include<iostream>

std::pair<double,bezier>
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


std::pair<double,bezier>
bezier_line_fitting( const edge_t &edge, double w_max ){
  size_t N = edge.size();
  if( N < 2 ){
    // few points to solve equation
    std::vector<std::pair<double,double> > ret;
    ret.push_back(std::make_pair<double,double>(edge[0].x,edge[0].y));
    ret.push_back(std::make_pair<double,double>(edge[N-1].x,edge[N-1].y));
    return std::make_pair(0.0,ret);
  }

  std::vector<double> wp(N);
  for( size_t i = 0 ; i < N ; ++i ){
    wp[i] = edge[i].w/w_max;
  }
  
  // generate equation
  double CX[2] = {};
  double CY[2] = {};
  double C[2][2] = {};

  for( size_t i = 0 ; i < N ; ++i ){
    double tp = static_cast<double>(i) / static_cast<double>(N-1);
    double weight = 1.0 - wp[i]/2.0;
    double px = edge[i].x;
    double py = edge[i].y;
    
    double t[2];
    for( int j = 0 ; j < 2 ; ++j ){
      t[j] = 1.0;
      for( int k = 0 ; k <    j  ; ++k )
        t[j] *=   tp;
      for( int k = 0 ; k < (1-j) ; ++k )
        t[j] *= 1-tp;
    }
    
    for( int j = 0 ; j < 2 ; ++j ){
      CX[j] += weight * t[j] * px;
      CY[j] += weight * t[j] * py;
      for( int k = 0 ; k < 2 ; ++k ){
        C[j][k] += weight * t[j] * t[k];
      }
    }
  }

  cv::Mat_<double> A(2,2);
  cv::Mat_<double> bx(2,1),by(2,1),x,y;
  for( int i = 0 ; i < 2 ; ++i ){
    bx(i,0) = CX[i];
    by(i,0) = CY[i];
    for( int j = 0; j < 2 ; ++j )
      A(i,j) = C[i][j];
  }

  cv::solve(A,bx,x);
  cv::solve(A,by,y);

  double opt_x[2],opt_y[2];
  for( int i = 0 ; i < 2 ; ++i ){
    opt_x[i] = x(i,0);
    opt_y[i] = y(i,0);
  }
  
  double err = 0.0;
  
  for( size_t i = 0 ; i < N ; ++i ){
    double tp = static_cast<double>(i) / static_cast<double>(N-1);
    double weight = 1.0 - wp[i]/2.0;
    double px = edge[i].x;
    double py = edge[i].y;

    double t[2];
    for( int j = 0 ; j < 2 ; ++j ){
      t[j] = 1.0;
      for( int k = 0 ; k <    j  ; ++k )
        t[j] *=   tp;
      for( int k = 0 ; k < (1-j) ; ++k )
        t[j] *= 1-tp;
    }

    double diff_x = -px;
    double diff_y = -py;

    for( int j = 0 ; j < 2 ; ++j ){
      diff_x += t[j] * opt_x[j];
      diff_y += t[j] * opt_y[j];
    }

    err += weight * ( diff_x * diff_x + diff_y * diff_y );
  }
  std::vector<std::pair<double,double> > ret;
  for( int j = 0 ; j < 2 ; ++j )
    ret.push_back(std::make_pair(opt_x[j],opt_y[j]));
  return std::make_pair(err,ret);
}

std::pair<double,bezier>
bezier_quadratic_fitting( const edge_t &edge, double w_max ){
  size_t N = edge.size();
  if( N < 3 ){
    // few points to solve equation
    std::vector<std::pair<double,double> > ret;
    ret.push_back(std::make_pair<double,double>(edge[0].x,edge[0].y));
    ret.push_back(std::make_pair<double,double>(edge[0].x,edge[0].y));
    ret.push_back(std::make_pair<double,double>(edge[N-1].x,edge[N-1].y));
    return std::make_pair(0.0,ret);
  }

  std::vector<double> wp(N);
  for( size_t i = 0 ; i < N ; ++i ){
    wp[i] = edge[i].w/w_max;
  }
  
  // generate equation
  double CX[3] = {};
  double CY[3] = {};
  double C[3][3] = {};
  double choose[3] = { 1.0, 2.0, 1.0 };

  for( size_t i = 0 ; i < N ; ++i ){
    double tp = static_cast<double>(i) / static_cast<double>(N-1);
    double weight = 1.0 - wp[i]/2.0;
    double px = edge[i].x;
    double py = edge[i].y;
    
    double t[3];
    for( int j = 0 ; j < 3 ; ++j ){
      t[j] = 1.0;
      for( int k = 0 ; k <    j  ; ++k )
        t[j] *=   tp;
      for( int k = 0 ; k < (2-j) ; ++k )
        t[j] *= 1-tp;
    }
    
    for( int j = 0 ; j < 3 ; ++j ){
      double weight_choose = weight * choose[j];
      CX[j] += weight_choose * t[j] * px;
      CY[j] += weight_choose * t[j] * py;
      for( int k = 0 ; k < 3 ; ++k ){
        C[j][k] += weight_choose * t[j] * choose[k] * t[k];
      }
    }
  }

  cv::Mat_<double> A(3,3);
  cv::Mat_<double> bx(3,1),by(3,1),x,y;
  for( int i = 0 ; i < 3 ; ++i ){
    bx(i,0) = CX[i];
    by(i,0) = CY[i];
    for( int j = 0; j < 3 ; ++j )
      A(i,j) = C[i][j];
  }

  cv::solve(A,bx,x);
  cv::solve(A,by,y);

  double opt_x[3],opt_y[3];
  for( int i = 0 ; i < 3 ; ++i ){
    opt_x[i] = x(i,0);
    opt_y[i] = y(i,0);
  }
  
  double err = 0.0;
  
  for( size_t i = 0 ; i < N ; ++i ){
    double tp = static_cast<double>(i) / static_cast<double>(N-1);
    double weight = 1.0 - wp[i]/2.0;
    double px = edge[i].x;
    double py = edge[i].y;

    double t[3];
    for( int j = 0 ; j < 3 ; ++j ){
      t[j] = 1.0;
      for( int k = 0 ; k <    j  ; ++k )
        t[j] *=   tp;
      for( int k = 0 ; k < (2-j) ; ++k )
        t[j] *= 1-tp;
    }

    double diff_x = -px;
    double diff_y = -py;

    for( int j = 0 ; j < 3 ; ++j ){
      diff_x += choose[j] * t[j] * opt_x[j];
      diff_y += choose[j] * t[j] * opt_y[j];
    }

    err += weight * ( diff_x * diff_x + diff_y * diff_y );
  }
  std::vector<std::pair<double,double> > ret;
  for( int j = 0 ; j < 3 ; ++j )
    ret.push_back(std::make_pair(opt_x[j],opt_y[j]));
  return std::make_pair(err,ret);
}

std::pair<double,bezier>
bezier_cubic_fitting( const edge_t &edge, double w_max ){
  size_t N = edge.size();
  if( N < 4 ){
    // few points to solve equation
    std::vector<std::pair<double,double> > ret;
    ret.push_back(std::make_pair<double,double>(edge[0].x,edge[0].y));
    ret.push_back(std::make_pair<double,double>(edge[0].x,edge[0].y));
    ret.push_back(std::make_pair<double,double>(edge[N-1].x,edge[N-1].y));
    ret.push_back(std::make_pair<double,double>(edge[N-1].x,edge[N-1].y));
    return std::make_pair(0.0,ret);
  }

  std::vector<double> wp(N);
  for( size_t i = 0 ; i < N ; ++i ){
    wp[i] = edge[i].w/w_max;
  }
  
  // generate equation
  double CX[4] = {};
  double CY[4] = {};
  double C[4][4] = {};
  double choose[4] = { 1.0, 3.0, 3.0, 1.0 };

  for( size_t i = 0 ; i < N ; ++i ){
    double tp = static_cast<double>(i) / static_cast<double>(N-1);
    double weight = 1.0 - wp[i]/2.0;
    double px = edge[i].x;
    double py = edge[i].y;
    
    double t[4];
    for( int j = 0 ; j < 4 ; ++j ){
      t[j] = 1.0;
      for( int k = 0 ; k <    j  ; ++k )
        t[j] *=   tp;
      for( int k = 0 ; k < (3-j) ; ++k )
        t[j] *= 1-tp;
    }
    
    for( int j = 0 ; j < 4 ; ++j ){
      double weight_choose = weight * choose[j];
      CX[j] += weight_choose * t[j] * px;
      CY[j] += weight_choose * t[j] * py;
      for( int k = 0 ; k < 4 ; ++k ){
        C[j][k] += weight_choose * t[j] * choose[k] * t[k];
      }
    }
  }

  cv::Mat_<double> A(4,4);
  cv::Mat_<double> bx(4,1),by(4,1),x,y;
  for( int i = 0 ; i < 4 ; ++i ){
    bx(i,0) = CX[i];
    by(i,0) = CY[i];
    for( int j = 0; j < 4 ; ++j )
      A(i,j) = C[i][j];
  }

  cv::solve(A,bx,x);
  cv::solve(A,by,y);

  double opt_x[4],opt_y[4];
  for( int i = 0 ; i < 4 ; ++i ){
    opt_x[i] = x(i,0);
    opt_y[i] = y(i,0);
  }
  
  double err = 0.0;
  
  for( size_t i = 0 ; i < N ; ++i ){
    double tp = static_cast<double>(i) / static_cast<double>(N-1);
    double weight = 1.0 - wp[i]/2.0;
    double px = edge[i].x;
    double py = edge[i].y;

    double t[4];
    for( int j = 0 ; j < 4 ; ++j ){
      t[j] = 1.0;
      for( int k = 0 ; k <    j  ; ++k )
        t[j] *=   tp;
      for( int k = 0 ; k < (3-j) ; ++k )
        t[j] *= 1-tp;
    }

    double diff_x = -px;
    double diff_y = -py;

    for( int j = 0 ; j < 4 ; ++j ){
      diff_x += choose[j] * t[j] * opt_x[j];
      diff_y += choose[j] * t[j] * opt_y[j];
    }

    err += weight * ( diff_x * diff_x + diff_y * diff_y );
  }
  std::vector<std::pair<double,double> > ret;
  for( int j = 0 ; j < 4 ; ++j )
    ret.push_back(std::make_pair(opt_x[j],opt_y[j]));
  return std::make_pair(err,ret);
}
