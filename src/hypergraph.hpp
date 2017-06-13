#pragma once
#include"topology.hpp"
#include"bezier.hpp"

struct hyperedge{
public:
  double error;
  BEZIER_DIM dim;
  std::vector< std::pair<edge_t,bool> > e;
};

bool operator<( const hyperedge&, const hyperedge& );

