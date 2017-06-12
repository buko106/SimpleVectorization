#include"topology.hpp"
#include<iostream>

#include<opencv2/highgui/highgui.hpp>
topology::topology( const skeleton &sk, bool inv ){
  create_topology( sk, inv );
  return;
}

edge_t edge_dfs( int init_x, int init_y, int x, int y, cv::Mat &im, const cv::Mat &thickness, const cv::Mat &feature ){
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
  cv::Mat im(sk.binary.clone());
  // convert to 0/1 Matrix
  if( inv ){
    im = (255-im)/255;
  }else{
    im = im/255;
  }
  
  cv::Mat feature = create_feature_map( im );
  edge.clear(); // create empty graph
  
  // int dx[] = {  0,  1,  0, -1};
  // int dy[] = { -1,  0,  1,  0};
  // int dirs = 4;
 
  int dx[] = {  0,  1,  1,  1,  0, -1, -1, -1};
  int dy[] = { -1, -1,  0,  1,  1,  1,  0, -1};
  int dirs = 8;
  
  int R = im.rows;
  int C = im.cols;
  // start DFS
  for( int y = 0 ; y < R ; ++y ){
    for( int x = 0 ; x < C ; ++x ){
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

  int max = -100000;
  int min =  100000;
  for( size_t i = 0 ; i < edge.size() ; ++i ){
    for( size_t j = 0 ; j < edge[i].size(); ++j ){
      max = std::max(max,edge[i][j].w);
      min = std::min(min,edge[i][j].w);
    }
  }
  
  std::cerr << "[INFO] create_topology : " << edge.size() << " edges detected" << std::endl;
  std::cerr << "[INFO] create_topology : max thickness = " << max << ", min thickness = " << min << std::endl;
  
  
  return;
}

void topology::refine( double tolerance ){
  return;
}

cv::Mat topology::create_feature_map( const cv::Mat& im ){
  
  cv::Mat feature = cv::Mat::zeros(im.size(),CV_8UC1);

  int dx[] = {  0,  1,  1,  1,  0, -1, -1, -1};
  int dy[] = { -1, -1,  0,  1,  1,  1,  0, -1};
  int dirs = 8;

  int R = im.rows;
  int C = im.cols;

  for( int y = 0 ; y < R ; ++y ){
    for( int x = 0 ; x < C ; ++x ){
      if( 0 != im.data[ y*im.step + x*im.elemSize()] ){
        int c=0;
        for( int i = 0 ; i < dirs ; ++i ){
          int nxt = (i+1)%dirs;
          int px = x + dx[i];
          int py = y + dy[i];
          int nx = x + dx[nxt];
          int ny = y + dy[nxt];
          if( 0 == ( py<0 || py>=R || px<0 || px>=C ? 0 : im.at<uchar>(py,px)) &&
              0 != ( ny<0 || ny>=R || nx<0 || nx>=C ? 0 : im.at<uchar>(ny,nx) ))
            c += 1;
        }
        if( c==1 || c==3 || c==4 ){
          feature.data[ y*feature.step + x*feature.elemSize() ] = 1;
        }
      }
    }
  }
  
  return feature;
}
