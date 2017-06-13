#include"svg.hpp"

void svg::setViewBox( int x0, int y0, int x1, int y1 ){
  viewBox[0] = x0;
  viewBox[1] = y0;
  viewBox[2] = x1;
  viewBox[3] = y1;
  return;
}

svg::svg( int x0, int y0, int x1, int y1 ){
  setViewBox( x0, y0, x1, y1 );
  return;
}

void svg::push( bezier curve, std::string color, int thickness ){
  if( color == "" ) color = "blue";
  svg_config config = std::make_pair(thickness,color);
  curves.push_back(std::make_pair(curve,config));
  return;
}

void svg::push( int cx, int cy, std::string color, int radius ){
  if( color == "" ) color = "red";
  svg_config config = std::make_pair(radius,color);
  circles.push_back(std::make_pair(std::make_pair(cx,cy),config));
  return;
}

std::ostream& operator<<( std::ostream& os, const svg& svg ){
  os << "<svg viewBox=\""
     << svg.viewBox[0] << " "
     << svg.viewBox[1] << " "
     << svg.viewBox[2] << " "
     << svg.viewBox[3] << "\">\n";
  
  for( size_t i = 0 ; i < svg.curves.size() ; ++i ){
    
    bezier c = svg.curves[i].first;
    svg_config config = svg.curves[i].second;

    os << "\t<path d=\"" ;
    for( size_t p = 0 ; p < c.size() ; ++p ){
      if( p == 0 ) os << " M ";
      if( p == 1 ){
        if(      c.size() == 4 ) os << " C ";
        else if( c.size() == 3 ) os << " Q ";
        else if( c.size() == 2 ) os << " L ";
      }
      os << " " << c[p].first << " " << c[p].second;
    }
    os << "\" stroke=\"" << config.second
       << "\" stroke-width=\"" << config.first
       <<"\"  fill=\"none\" />\n" ;
  }

  for( size_t i = 0 ; i < svg.circles.size() ; ++i ){
    std::pair<int,int> c = svg.circles[i].first;
    svg_config config = svg.circles[i].second;


    os << "\t<circle cx=\"" << c.first
       << "\" cy=\"" << c.second
       << "\" r=\"" << config.first
       << "\" stroke=\"none\" fill=\"" << config.second << "\"/>\n";
  }
  os << "</svg>";
  return os;
}
