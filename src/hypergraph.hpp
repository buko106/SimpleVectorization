#pragma once
#include"topology.hpp"
#include"bezier.hpp"
#include<set>

struct hyperedge{
public:
  double error;
  BEZIER_DEG deg;
  std::vector< std::pair<size_t,bool> > e;
};


class hypergraph{
private:
  std::vector<edge_t> edge;
  std::vector<hyperedge> h;
  std::set<pixel> V;
  double T;
  double w_max;
  edge_t to_edge_t( const hyperedge& ) const;
public:
  hypergraph();
  hypergraph( topology, BEZIER_DEG = CUBIC );
  void init( topology, BEZIER_DEG = CUBIC );
  std::vector< std::pair<edge_t,BEZIER_DEG> > to_bezier() const;
  double step( double , double , size_t = 1 , double = -1 );
};
