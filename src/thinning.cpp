#include"thinning.hpp"
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>

skeleton::skeleton( cv::Mat input, bool binarize ){
  if( binarize ){
    int dummy  = 0;
    int maxVal = 255;
    cv::threshold( input, binary, dummy, maxVal, cv::THRESH_BINARY|cv::THRESH_OTSU);
  }else{
    binary = input;
  }
}

void skeleton::thinning( THINNING_METHOD method, bool reverse ){
  if( ZHANGSUEN != method ){
    std::cerr << "[ERROR] in skeleton::thinning : Unknown method" << std::endl;
    exit(1);
  }
  if( ZHANGSUEN == method ){
    skeleton::ZhangSuenThinning( reverse );
  }
  return;
}

inline bool foreground( unsigned char v, bool rev ){
  return ( rev ? v==0 : v!=0 );
}

void skeleton::ZhangSuenIteration( int pattern ){
  cv::Mat mask = cv::Mat::ones( binary.size(), CV_8UC1);

  int dx[9] = {  0,  0,  1,  1,  1,  0, -1, -1, -1};
  int dy[9] = {  0, -1, -1,  0,  1,  1,  1,  0, -1};
  
  for( int y = 1 ; y < binary.rows-1 ; ++y ){
    for( int x = 1 ; x < binary.cols-1 ; ++x ){
      // initialize v[]
      unsigned char v[9];
      for( int i = 0; i<9; ++i ){
        v[i] = binary.data[ (y+dy[i])*binary.step + (x+dx[i])*binary.elemSize() ];
      }
      // sum up S,N
      int S=0,N=0;
      for( int i = 0; i<8; ++i ){
        int fst =  i+1;
        int snd = (i+1)%8 + 1;
        if( v[fst] ) N++;
        if( v[fst] == 0 && v[snd] == 1 ) S++;
      }
      // check pattern
      bool m1,m2;
      if( pattern == 0 ){ m1 = v[1] && v[3] && v[5]; m2 = v[3] && v[5] && v[7]; }
      if( pattern == 1 ){ m1 = v[1] && v[3] && v[7]; m2 = v[1] && v[5] && v[7]; }
      // disable mask
      if( S==1 && ( 2 <= N && N <= 6) && !m1 && !m2 ){
        mask.data[ y*mask.step + x*mask.elemSize()] = 0;
      }
    }
  }

  binary &= mask;
  return;
}


void skeleton::ZhangSuenThinning( bool reverse ){
  if( reverse ){
    binary = (255 - binary) / 255;
  }else{
    binary = binary / 255;
  }

  thickness = cv::Mat::zeros( binary.size(),  CV_8UC1 );
  cv::Mat prev = cv::Mat::zeros( binary.size(), CV_8UC1 );
  cv::Mat diff;

  do{
    ZhangSuenIteration(0);
    ZhangSuenIteration(1);
    cv::absdiff( binary, prev, diff );
    binary.copyTo(prev);
  }while( cv::countNonZero(diff) );

                           
  if( reverse ){
    binary = (1 - binary) * 255;
  }else{
    binary = binary * 255;
  }
  return;
}
