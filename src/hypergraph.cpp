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

    hyperedge tmp = { error, deg, e };
    h.push_back(tmp);
    
  }
}

double hypergraph::step( double lambda, double mu, size_t iteration , double C ){
  // automatic calc of C  
  if( C <= 0.0 ){
    C = std::pow( 0.999, 1.0/static_cast<double>(V.size()) );
  }
  // iteration
  for( size_t iteration_counter = 0 ; iteration_counter < iteration ; ++iteration_counter ){
    // generate x' from x with a random perturbation operator
    while( true ){
      int selected = random_generate_int(0,static_cast<int>(h.size()-1));
      double op = random_generate_uniform(0,1);
      if( op < 0.333 ){ // merge
        
        break;
      }else if( op < 0.666 ){ // split
        
        break;
      }else{ // degree switch
        BEZIER_DEG new_deg;
        while( h[selected].deg == (new_deg=static_cast<BEZIER_DEG>(random_generate_int(LINE,CUBIC)) ) )
          { ; }
        edge_t new_edge = to_edge_t( h[selected] );
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
        break;
      }
    }
    // fit bezier curves and calculate diff = U(x)-U(x')
    
    // draw a random value p \in [0,1]
    double p = random_generate_uniform(0,1);
    
    // if( p < std::exp(diff/T) ){
    //   // update x <- x'
    // } // else update x <- x

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
