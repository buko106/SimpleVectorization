#pragma once
#include"topology.hpp"
#include<vector>

enum BEZIER_DIM{
  LINE  = 1,
  QUAD  = 2,
  CUBIC = 3
};

typedef std::vector<std::pair<double,double> > bezier;

std::pair<double,bezier> bezier_line_fitting( const edge_t& , double );
std::pair<double,bezier> bezier_quadratic_fitting( const edge_t& , double );
std::pair<double,bezier> bezier_cubic_fitting( const edge_t& , double );
std::pair<double,bezier> bezier_fittting( const edge_t& , double, BEZIER_DIM );
