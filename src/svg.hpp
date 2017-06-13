#pragma once
#include"bezier.hpp"
#include<vector>

typedef std::pair<int,std::string> svg_config;

class svg{
  int viewBox[4];
  std::vector< std::pair<bezier,svg_config> > curves;
  std::vector< std::pair<std::pair<double,double>,svg_config> > circles;
public:
  void setViewBox( int, int, int, int );
  svg( int, int, int, int );
  void push( bezier, std::string = "", int = 3 );
  void push( double, double, std::string = "", int = 3 );
  friend std::ostream& operator<<( std::ostream&, const svg& );
};

std::ostream& operator<<( std::ostream&, const svg& );
