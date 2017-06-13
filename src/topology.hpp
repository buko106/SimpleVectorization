#pragma once
#include"thinning.hpp"
#include<vector>

struct pixel{
public:
  int x;
  int y;
  int w;
};

bool operator<( const pixel&, const pixel& );
typedef std::vector<pixel> edge_t;

class topology{
private:
public:
  cv::Mat_<uchar> create_feature_map( const cv::Mat_<uchar>& );
  std::vector<edge_t> edge;
  topology( const skeleton& , bool=false );
  void create_topology( const skeleton& , bool=false );
  void refine( double );
};

