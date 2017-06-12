#pragma once
#include"topology.hpp"
#include<vector>

enum BEZIER_DIM{
  LINE  = 1,
  QUAD  = 2,
  CUBIC = 3
};

std::pair<double,std::vector<std::pair<double,double> > > bezier_line_fitting( edge_t );
std::pair<double,std::vector<std::pair<double,double> > > bezier_quadratic_fitting( edge_t );
std::pair<double,std::vector<std::pair<double,double> > > bezier_cubic_fitting( edge_t );
std::pair<double,std::vector<std::pair<double,double> > > bezier_fittting( edge_t, BEZIER_DIM );

