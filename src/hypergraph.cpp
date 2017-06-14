#include"hypergraph.hpp"
#include"bezier.hpp"
#include"utils.hpp"
#include<set>
#include<algorithm>
hypergraph::hypergraph(){ return; }
hypergraph::hypergraph( topology tp, BEZIER_DEG deg ){
  init( tp, deg );
}

void hypergraph::init( topology tp, BEZIER_DEG deg ){
  edge.resize(0);
  h.resize(0);
  V.clear();
  T = 1.0;
  w_max = tp.w_max;
    
  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    edge.push_back( tp.edge[i] );

    V.insert(tp.edge[i][0]);
    V.insert(tp.edge[i][tp.edge[i].size()-1]);

    bool reverse = false;
    std::pair<double,bezier> curve = bezier_fittting( tp.edge[i], tp.w_max, deg );
    double error = curve.first;
    std::vector< std::pair<size_t,bool> > e(1,std::make_pair(i,reverse));
    
    hyperedge tmp = { error, deg, e, { tp.edge[i][0], tp.edge[i][tp.edge[i].size()-1] }};
    h.push_back(tmp);
    
  }
}

bool hypergraph::merge( double lambda, double mu ){
  size_t selected = random_generate_int(0,static_cast<int>(h.size()-1));
  
  int tail = random_generate_int(0,1);
  std::vector< std::pair<size_t,int> > adjacent;
  for( size_t i = 0 ; i < h.size() ; ++i ){
    if( i == selected ) continue;
    for( int b = 0 ; b <= 1 ; ++b ){
      if( h[i].ends[b] == h[selected].ends[tail] )
        adjacent.push_back( std::make_pair(i,b) );
    }
  }
  if( adjacent.empty() ) return false;
  std::pair<size_t,int>
    target = adjacent[random_generate_int(0,static_cast<int>(adjacent.size()-1))];
  BEZIER_DEG new_deg = std::max(h[target.first].deg,h[selected].deg);
  hyperedge new_hyper_edge;
  if( tail ){ // simply copy to e
    new_hyper_edge.e.insert( new_hyper_edge.e.end(),
                             h[selected].e.begin(),
                             h[selected].e.end() );
  }else{ // reversibly copy ( change direction flag )
    for( size_t i = 0 ; i < h[selected].e.size() ; ++i ){
      size_t idx = h[selected].e.size() - 1 - i;
      new_hyper_edge.e.push_back( std::make_pair(h[selected].e[idx].first,
                                                 !h[selected].e[idx].second));
    }
  }
  
  if( target.second ){ // reversibly copy ( change direction flag )
    for( size_t i = 0 ; i < h[target.first].e.size() ; ++i ){
      size_t idx = h[target.first].e.size() - 1 - i;
      new_hyper_edge.e.push_back( std::make_pair(h[target.first].e[idx].first,
                                                 !h[target.first].e[idx].second));
    }
  }else{ // simply copy to e
    new_hyper_edge.e.insert( new_hyper_edge.e.end(),
                             h[target.first].e.begin(),
                             h[target.first].e.end() );
  }
  
  edge_t tmp = to_edge_t( new_hyper_edge );
  // fit bezier curves and calculate diff = U(x)-U(x')
  std::pair<double,bezier> result = bezier_fittting( tmp, w_max, new_deg );
  double dUf =  h[selected].error + h[target.first].error - result.first;
  double dUs =  (2.0 - 1.0) + mu * static_cast<double>( h[selected].deg + h[target.first].deg - new_deg );
  double diff = ( 1.0 - lambda ) * dUf + lambda * dUs ;
  // draw a random value p \in [0,1]
  double p = random_generate_uniform(0,1);
  if( p < std::exp(diff/T) ){
    // update x <- x'
    new_hyper_edge.error = result.first;
    new_hyper_edge.deg = new_deg;
    new_hyper_edge.ends[0] = h[selected].ends[ 1-tail ];
    new_hyper_edge.ends[1] = h[target.first].ends[ 1-target.second ];
    h[selected] = new_hyper_edge;
    h.erase(h.begin() + target.first);
  } // else update x <- x
  return true;
}

bool hypergraph::split( double lambda, double mu ){
  size_t selected = random_generate_int(0,static_cast<int>(h.size()-1));
  if( h[selected].e.size() <= 1 ) return false;
  int split_pos = random_generate_int(0,static_cast<int>(h[selected].e.size()-2));
  hyperedge new_hyper_edge[2];
  BEZIER_DEG new_deg = h[selected].deg;
  new_hyper_edge[0].e.insert(new_hyper_edge[0].e.end(),
                             h[selected].e.begin(),
                             h[selected].e.begin()+(split_pos+1));
  new_hyper_edge[1].e.insert(new_hyper_edge[1].e.end(),
                             h[selected].e.begin()+(split_pos+1),
                             h[selected].e.end());
  // fit bezier curves and calculate diff = U(x)-U(x')
  std::pair<double,bezier> result[2];
  for( int i = 0 ; i < 2 ; ++i ){
    edge_t new_edge;
    new_edge = to_edge_t( new_hyper_edge[i] );
    result[i] = bezier_fittting( new_edge, w_max, new_deg );
  }
  double dUf =  h[selected].error - result[0].first - result[1].first ;
  double dUs =  (1.0 - 2.0 ) + mu * static_cast<double>( h[selected].deg - 2 * new_deg );
  double diff = ( 1.0 - lambda ) * dUf + lambda * dUs ;
  // draw a random value p \in [0,1]
  double p = random_generate_uniform(0,1);
  if( p < std::exp(diff/T) ){
    // update x <- x'
    for( int i = 0 ; i < 2 ; ++i ){
      new_hyper_edge[i].error = result[i].first;
      new_hyper_edge[i].deg = new_deg;
    }
    new_hyper_edge[0].ends[0] = h[selected].ends[0];
    new_hyper_edge[1].ends[1] = h[selected].ends[1];
    new_hyper_edge[0].ends[1] = edge[h[selected].e[split_pos+1].first][0];
    new_hyper_edge[1].ends[0] = edge[h[selected].e[split_pos+1].first][0];
    h.erase(h.begin()+selected);
    for( int i = 0 ; i < 2 ; ++i ){
      h.push_back(new_hyper_edge[i]);
    }
  } // else update x <- x
  return true;
}

bool hypergraph::degree_switch( double lambda, double mu ){
  size_t selected = random_generate_int(0,static_cast<int>(h.size()-1));
  BEZIER_DEG new_deg;
  while( h[selected].deg == (new_deg=static_cast<BEZIER_DEG>(random_generate_int(LINE,CUBIC)) ) )
    { ; }
  edge_t new_edge = to_edge_t( h[selected] );
  // fit bezier curves and calculate diff = U(x)-U(x')
  std::pair<double,bezier> result = bezier_fittting( new_edge, w_max, new_deg );
  double dUf =  h[selected].error - result.first;
  double dUs =  mu * static_cast<double>( h[selected].deg - new_deg );
  double diff = ( 1.0 - lambda ) * dUf + lambda * dUs ;
  // draw a random value p \in [0,1]
  double p = random_generate_uniform(0,1);
  if( p < std::exp(diff/T) ){
    // update x <- x'
    h[selected].deg = new_deg;
    h[selected].error = result.first;
  } // else update x <- x
  return true;
}

bool hypergraph::overlap( double lambda, double mu ){
  size_t selected = random_generate_int(0,static_cast<int>(h.size()-1));
  int tail = random_generate_int(0,1);

  size_t end_edge = h[selected].e[ tail * (h[selected].e.size()-1) ].first;
  pixel end_pixel = h[selected].ends[tail];
  std::vector< std::pair<size_t,int> > adjacent;
  for( size_t i = 0 ; i < edge.size() ; ++i ){
    for( int b = 0 ; b <= 1 ; ++b ){
      if( end_edge == i ) continue;
      if( end_pixel == edge[i][ b*(edge[i].size()-1) ] )
        adjacent.push_back( std::make_pair(i,b) );
    }
  }
  if( adjacent.empty() ) return false;
  
  std::pair<size_t,int>
    target = adjacent[random_generate_int(0,static_cast<int>(adjacent.size()-1))];
  
  hyperedge new_hyper_edge;
  new_hyper_edge.e = h[selected].e;
  if( tail ){ // add to end()
    new_hyper_edge.e.insert( new_hyper_edge.e.end()  , std::make_pair(target.first,tail!=target.second) );
  }else{ // add to begin()
    new_hyper_edge.e.insert( new_hyper_edge.e.begin(), std::make_pair(target.first,tail!=target.second) );
  }
  edge_t tmp = to_edge_t( new_hyper_edge );
  // fit bezier curves and calculate diff = U(x)-U(x')
  std::pair<double,bezier> result = bezier_fittting( tmp, w_max, h[selected].deg );
  double dUf =  h[selected].error - result.first;
  double dUs = 0.0 + mu * 0.0;
  double diff = ( 1.0 - lambda ) * dUf + lambda * dUs ;
  // draw a random value p \in [0,1]
  double p = random_generate_uniform(0,1);
  if( p < std::exp(diff/T) ){
    // update x <- x'
    new_hyper_edge.error = result.first;
    new_hyper_edge.deg = h[selected].deg;
    new_hyper_edge.ends[0] = tmp[0];
    new_hyper_edge.ends[1] = tmp[tmp.size()-1];
    h[selected] = new_hyper_edge;
  } // else update x <- x
  return true;
}

bool hypergraph::dissociation( double lambda, double mu ){
  size_t selected = random_generate_int(0,static_cast<int>(h.size()-1));
  int tail = random_generate_int(0,1);

  if( h[selected].e.size() < 2 ) return false;
  size_t end_edge = h[selected].e[ tail * (h[selected].e.size()-1) ].first;
  
  bool found=false;
  for( size_t i = 0 ; i < h.size() ; ++i ){
    if( i == selected ) continue;
    for( size_t j = 0 ; j < h[i].e.size() ; ++j ){
      if( h[i].e[j].first == end_edge )
        found = true;
    }
  }
  if( !found ) return false;
    
  hyperedge new_hyper_edge;
  new_hyper_edge.e = h[selected].e;
  if( tail ){ // erase from end()
    new_hyper_edge.e.pop_back();
  }else{ // erase from begin()
    new_hyper_edge.e.erase(new_hyper_edge.e.begin());
  }

  edge_t tmp = to_edge_t( new_hyper_edge );
  // fit bezier curves and calculate diff = U(x)-U(x')
  std::pair<double,bezier> result = bezier_fittting( tmp, w_max, h[selected].deg );
  double dUf =  h[selected].error - result.first;
  double dUs = 0.0 + mu * 0.0;
  double diff = ( 1.0 - lambda ) * dUf + lambda * dUs ;
  // draw a random value p \in [0,1]
  double p = random_generate_uniform(0,1);
  if( p < std::exp(diff/T) ){
    // update x <- x'
    new_hyper_edge.error = result.first;
    new_hyper_edge.deg = h[selected].deg;
    new_hyper_edge.ends[0] = tmp[0];
    new_hyper_edge.ends[1] = tmp[tmp.size()-1];
    h[selected] = new_hyper_edge;
  } // else update x <- x
  return true;
}

double hypergraph::step( double lambda, double mu, size_t iteration , double C ){
  // automatic calc of C  
  if( C <= 0.0 ){
    C = std::pow( 0.999, 1./static_cast<double>(V.size()) );
  }
  // iteration
  for( size_t iteration_counter = 0 ; iteration_counter < iteration ; ++iteration_counter ){
    // generate x' from x with a random perturbation operator
    while( true ){
      double op = random_generate_uniform(0,1);
      if( op < 0.2 ){
        if( merge(lambda,mu) ) break;
      }else if( op < 0.4 ){
        if( split(lambda,mu) ) break;
      }else if( op < 0.6 ){
        if( degree_switch(lambda,mu) ) break;
      }else if( op < 0.8 ){
        if( overlap(lambda,mu) )break;
      }else{
        if( dissociation(lambda,mu) )break;
      }
    }
    
    T *= C;
  }
  // calculation of U(x)
  double Us = 0.0;
  double Uf = 0.0;
  for( size_t i = 0 ; i < h.size() ; ++i ){
    Uf += h[i].error;
    Us += ( 1.0 + mu * h[i].deg );
  }
  return (1-lambda) * Uf + lambda * Us ;
}

edge_t hypergraph::to_edge_t( const hyperedge& hyper ) const{
  size_t N = hyper.e.size();
  edge_t ret;
  for( size_t i = 0; i < N; ++i ){
    size_t index = hyper.e[i].first;
    bool reverse = hyper.e[i].second;
    if( reverse ){ // reversed
      std::reverse_copy(edge[index].begin(),edge[index].end(), back_inserter(ret) );
    }else{ // not reversed
      std::copy(edge[index].begin(),edge[index].end(), back_inserter(ret) );
    }

    if( i != N-1 ) ret.pop_back();
  }
  return ret;
}

std::vector< std::pair<edge_t,BEZIER_DEG> > hypergraph::to_bezier() const{
  std::vector< std::pair<edge_t,BEZIER_DEG> >
    ret;
  for( size_t i = 0 ; i < h.size() ; ++i ){
    edge_t tmp = to_edge_t(h[i]);
    ret.push_back( std::make_pair( to_edge_t(h[i]), h[i].deg ) );
  }
  return ret;
}
