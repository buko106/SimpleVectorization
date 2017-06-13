#include"topology.hpp"
#include"bezier.hpp"
#include<iostream>
#include<queue>

// #include<opencv2/highgui/highgui.hpp>
bool operator<( const pixel& a, const pixel& b){
  return ( a.x<b.x ? true : ( a.y<b.y ? true : a.w < b.w ));
}

topology::topology( const skeleton &sk, bool inv ){
  create_topology( sk, inv );
  return;
}

edge_t edge_dfs( int init_x, int init_y, int x, int y,
                 cv::Mat_<uchar> &im, const cv::Mat_<uchar> &thickness, const cv::Mat_<uchar> &feature ){
  // only update im
  edge_t e;
  pixel init = { init_x, init_y, thickness.data[ init_y*thickness.step + init_x*thickness.elemSize() ] };
  e.push_back(init);

  // int dx[] = {  0,  1,  0, -1};
  // int dy[] = { -1,  0,  1,  0};
  // int dirs = 4;
  int dx[] = {  0,  1,  1,  1,  0, -1, -1, -1};
  int dy[] = { -1, -1,  0,  1,  1,  1,  0, -1};
  int dirs = 8;

  while( true ){
    pixel p = { x, y, thickness.at<uchar>(y,x) };
    e.push_back(p);
    if( feature.at<uchar>(y,x) )
      break;
    // next point ( feature point )
    bool next = false;
    for( int i = 0 ; i < dirs; ++i ){
      int nx = x + dx[i];
      int ny = y + dy[i];
      if( init_x-1 <= nx && nx <= init_x+1 && 
          init_y-1 <= ny && ny <= init_y+1 )
        continue;
      if( nx < 0 || nx >= im.cols || ny < 0 || ny >= im.rows )
        continue;
      
      if( im.at<uchar>(ny,nx) && feature.at<uchar>(ny,nx) ){
        im.at<uchar>(y,x) = 0;
        x = nx;
        y = ny;
        next = true;
        break;
      }
    }

    if( next ) 
      continue;
    // next point ( others )
    for( int i = 0 ; i < dirs; ++i ){
      int nx = x + dx[i];
      int ny = y + dy[i];
      if( init_x-1 <= nx && nx <= init_x+1 && 
          init_y-1 <= ny && ny <= init_y+1 )
        continue;
      if( nx < 0 || nx >= im.cols || ny < 0 || ny >= im.rows )
        continue;

      if( im.at<uchar>(ny,nx) ){
        im.at<uchar>(y,x) = 0;
        x = nx;
        y = ny;
        break;
      }
   
    }
    // cv::Mat temp = feature*64 | (im*32);
    // temp.at<uchar>(y,x) = 255;
    // cv::imshow("image", temp);
    // cv::waitKey(1);

  }
  return e;
}


void topology::create_topology( const skeleton &sk, bool inv ){
  cv::Mat_<uchar> im(sk.binary.clone());
  // convert to 0/1 Matrix
  if( inv ){
    im = (255-im)/255;
  }else{
    im = im/255;
  }
  
  cv::Mat_<uchar> feature = create_feature_map( im );
  edge.clear(); // create empty graph
  
  // int dx[] = {  0,  1,  0, -1};
  // int dy[] = { -1,  0,  1,  0};
  // int dirs = 4;
 
  int dx[] = {  0,  1,  1,  1,  0, -1, -1, -1};
  int dy[] = { -1, -1,  0,  1,  1,  1,  0, -1};
  int dirs = 8;
  
  // start DFS
  for( int y = 0 ; y < im.rows ; ++y ){
    for( int x = 0 ; x < im.cols ; ++x ){
      if( feature.data[ y*feature.step + x*feature.elemSize() ] ){
        for( int i = 0; i < dirs; ++i ){
          int nx = x + dx[i];
          int ny = y + dy[i];
          if( nx < 0 || nx >= im.cols || ny < 0 || ny >= im.rows )
            continue;

          if( im.data[ ny*im.step + nx*im.elemSize() ] ){
            edge_t e = edge_dfs( x, y, nx, ny, im, sk.thickness, feature ); // only update im
            edge.push_back(e);
          }
        }
      }
    }
  }
  // TODO: Detect circuit
  std::cerr << "[WARNING] create_topology : Simple circuit may not be detected( impletemnted in the future )" << std::endl;

  int max = std::numeric_limits<int>::min();
  int min = std::numeric_limits<int>::max();
  for( size_t i = 0 ; i < edge.size() ; ++i ){
    for( size_t j = 0 ; j < edge[i].size(); ++j ){
      max = std::max(max,edge[i][j].w);
      min = std::min(min,edge[i][j].w);
    }
  }

  w_max = static_cast<double>(max);
  std::cerr << "[INFO] create_topology : " << edge.size() << " edges detected" << std::endl;
  std::cerr << "[INFO] create_topology : max thickness = " << max << ", min thickness = " << min << std::endl;
  
  
  return;
}

void topology::refine( double tolerance ){
  double w_max = -std::numeric_limits<double>::max();
  for( size_t i = 0 ; i < edge.size() ; ++i ){
    for( size_t j = 0 ; j < edge[i].size() ; ++j ){
      w_max = std::max<double>(w_max,edge[i][j].w);
    }
  }

  double total_error = 0.0;
  int    total_pixel = 0;
  std::priority_queue< std::pair<double,edge_t> > pq;
  for( size_t i = 0; i < edge.size(); ++i ){
    std::pair<double,std::vector<std::pair<double,double> > >
      result = bezier_cubic_fitting( edge[i], w_max );
    double err = result.first;
    total_error += err;
    total_pixel += static_cast<int>(edge[i].size());
    pq.push(make_pair(err,edge[i]));
  }

  while( total_error/static_cast<double>(total_pixel) >= tolerance){
    std::pair<double,edge_t> top = pq.top();
    double err = top.first;
    edge_t curve = top.second;

    pq.pop();

    int left  = 0;
    int right = static_cast<int>(curve.size());
    while( true ){
      int med = (left+right)/2;
      edge_t l_curve(curve.begin(),curve.begin()+med);
      std::pair<double,std::vector<std::pair<double,double> > >
        l_result = bezier_cubic_fitting( l_curve, w_max );
      double l_err = l_result.first;

      edge_t r_curve(curve.begin()+(med-1),curve.end());
      std::pair<double,std::vector<std::pair<double,double> > >
        r_result = bezier_cubic_fitting( r_curve, w_max );
      double r_err = r_result.first;
      
      if( l_err < r_err ) left  = med;
      else                right = med;

      if( left >= right-1 ){
        pq.push(make_pair(l_err,l_curve));
        pq.push(make_pair(r_err,r_curve));
        total_error += l_err + r_err - err;
        break;
      }
    }
  }
  // finalize
  edge.resize(0);
  while( !pq.empty() ){
    std::pair<double,edge_t> top = pq.top();
    pq.pop();
    edge.push_back(top.second);
  }
  std::cerr << "[INFO] topology::refine : " << edge.size() << " curves in refined topology" << std::endl;
  return;
}

cv::Mat_<uchar> topology::create_feature_map( const cv::Mat_<uchar>& im ){
  
  cv::Mat_<uchar> feature = cv::Mat_<uchar>::zeros(im.size());

  int dx[] = {  0,  1,  1,  1,  0, -1, -1, -1};
  int dy[] = { -1, -1,  0,  1,  1,  1,  0, -1};
  int dirs = 8;

  int R = im.rows;
  int C = im.cols;

  for( int y = 0 ; y < R ; ++y ){
    for( int x = 0 ; x < C ; ++x ){
      if( 0 != im(y,x) ){
        int c=0;
        for( int i = 0 ; i < dirs ; ++i ){
          int nxt = (i+1)%dirs;
          int px = x + dx[i];
          int py = y + dy[i];
          int nx = x + dx[nxt];
          int ny = y + dy[nxt];
          if( 0 == ( py<0 || py>=R || px<0 || px>=C ? 0 : im(py,px)) &&
              0 != ( ny<0 || ny>=R || nx<0 || nx>=C ? 0 : im(ny,nx) ))
            c += 1;
        }
        if( c==1 || c==3 || c==4 ){
          feature(y,x) = 1;
        }
      }
    }
  }
  
  return feature;
}
