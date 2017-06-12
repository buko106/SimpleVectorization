#include"bezier.hpp"
#include<Eigen/Dense>
#include<iostream>

std::pair<double,std::vector<std::pair<double,double> > > 
bezier_fittting( edge_t edge, double w_max, BEZIER_DIM dim ){
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
bezier_line_fitting( edge_t edge, double w_max ){
  exit(1);
}

std::pair<double,std::vector<std::pair<double,double> > >
bezier_quadratic_fitting( edge_t edge, double w_max ){
  exit(1);
}

std::pair<double,std::vector<std::pair<double,double> > >
bezier_cubic_fitting( edge_t edge, double w_max ){
  exit(1);
}

