#include"thinning.hpp"
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>

skeleton::skeleton( const cv::Mat_<uchar> &input ){
  input.copyTo(binary);
  return;
}

void skeleton::thinning( THINNING_METHOD method, bool inv, double param ){
  if( ZHANGSUEN != method && TRAPPEDBALL != method ){
    std::cerr << "[ERROR] in skeleton::thinning : Unknown method" << std::endl;
    exit(1);
  }
  // convert to 0/1 image.
  // 1 is foreground
  if( inv ){
    binary = (255 - binary) / 255;
  }else{
    binary = binary / 255;
  }
  thickness = cv::Mat_<uchar>::zeros( binary.size() );
  
  if( ZHANGSUEN == method ){
    ZhangSuenThinning();
  }else if( TRAPPEDBALL == method ){
    TrappedBallThinning( static_cast<int>(param) );
  }

  if( inv ){
    binary = (1 - binary) * 255;
  }else{
    binary = binary * 255;
  }
  return;
}

inline bool foreground( unsigned char v, bool rev ){
  return ( rev ? v==0 : v!=0 );
}

void skeleton::ZhangSuenIteration( int pattern ){
  cv::Mat_<uchar> mask = cv::Mat_<uchar>::ones( binary.size() );
  cv::Mat_<uchar> count = thickness.clone();
    
  int dx[9] = {  0,  0,  1,  1,  1,  0, -1, -1, -1};
  int dy[9] = {  0, -1, -1,  0,  1,  1,  1,  0, -1};
  
  for( int y = 1 ; y < binary.rows-1 ; ++y ){
    for( int x = 1 ; x < binary.cols-1 ; ++x ){
      // initialize v[]
      unsigned char v[9];
      for( int i = 0; i<9; ++i ){
        v[i] = binary(y+dy[i],x+dx[i]);
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
        mask(y,x) = 0;
      }
    }
  }

  // calc thickness
  cv::Mat_<uchar> temp;
  thickness.copyTo(temp);
  for( int y = 1 ; y < binary.rows-1 ; ++y ){
    for( int x = 1 ; x < binary.cols-1 ; ++x ){
      if( !mask(y,x) ){
        int min = 255;
        for( int i = 1; i < 9; ++i ){
          if( !binary(y+dy[i],x+dx[i]) )
            min = std::min<int>(min,thickness(y+dy[i],x+dx[i]));
        }
        temp(y,x) = (unsigned char)(min + 1);
      }
    }
  }
  temp.copyTo(thickness);
  binary &= mask;
  return;
}


void skeleton::ZhangSuenThinning(){

  cv::Mat_<uchar> prev = cv::Mat_<uchar>::zeros( binary.size() );
  cv::Mat_<uchar> diff;

  do{
    ZhangSuenIteration(0);
    ZhangSuenIteration(1);
    cv::absdiff( binary, prev, diff );
    binary.copyTo(prev);
  }while( cv::countNonZero(diff) );

  // calc thickness
  int dx[9] = {  0,  0,  1,  1,  1,  0, -1, -1, -1};
  int dy[9] = {  0, -1, -1,  0,  1,  1,  1,  0, -1};
  //  std::cout << thickness << std::endl;
  cv::Mat_<uchar> temp = cv::Mat_<uchar>::zeros( thickness.size() );
  for( int y = 1 ; y < binary.rows-1 ; ++y ){
    for( int x = 1 ; x < binary.cols-1 ; ++x ){
      if( binary(y,x) ){
        unsigned char max = 0;
        for( int i = 1; i < 9; ++i ){
          max = std::max<unsigned char>(max,thickness(y+dy[i],x+dx[i]));
        }
        temp(y,x) = (unsigned char)(2*max+1);
      }
    }
  }

  thickness = temp & (binary*255);
  
  return;
}

void skeleton::TrappedBallOpening( int radius ){
  cv::Mat_<uchar> ball = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(2*radius+1,2*radius+1));
  cv::morphologyEx(binary,
                   binary,
                   cv::MORPH_OPEN,
                   ball,
                   cv::Point(-1,-1),
                   1 // iteration
                   );

  return;
}

void skeleton::TrappedBallThinning( int radius ){
  binary = 1-binary;
  TrappedBallOpening(radius);
  // iteration
  // 0: foreground(drawing line)
  // 1: background
  // 2,3,.. : regions
  int region = 2;
  for( int y = 0 ; y < binary.rows ; ++y ){
    for( int x = 0 ; x < binary.cols ; ++x ){
      if( 1 == binary(y,x) ){
        cv::floodFill( binary,
                       cv::Point(x,y),
                       region,
                       0, // Rect*
                       0, // loDiff
                       0, // upDiff
                       4  // flags
                       );
        region += 1;
      }
    }
  }
  // information
  std::cerr << "[INFO] TrappedBallThinning : " << region << " regions detected" << std::endl;

  // dilation
  bool update;
  int num=0;
  // int dx[] = {  0,  1,  1,  1,  0, -1, -1, -1};
  // int dy[] = { -1, -1,  0,  1,  1,  1,  0, -1};
  // int dirs = 8;
  int dx[] = {  0,  1,  0, -1};
  int dy[] = { -1,  0,  1,  0};
  int dirs = 4;
  
  do{
    // initialize
    update = false;
    num += 1;
    // dilation
    for( int r = 2; r <= region; ++r ){
      // for each segment
      cv::Mat_<uchar> updateMask = cv::Mat_<uchar>::zeros( binary.size() );
      for( int y = 0 ; y < binary.rows ; ++y ){
        for( int x = 0 ; x < binary.cols ; ++x ){
          if( 0 == binary(y,x) ){
            // If foreground point P(x,y) is adjacent to only r
            int count_r=0;
            int count_others=0;
            for( int i = 0 ; i < dirs; ++i ){
              int px = x + dx[i];
              int py = y + dy[i];
              if( 0 <= px && px < binary.cols && 0<= py && py < binary.rows ){
                if( r == binary(py,px) ){
                  count_r += 1;
                }else if( 0 != binary(py,px) ){
                  count_others += 1;
                }
              }
            }
            if( count_others == 0 && count_r > 0 ){
              updateMask(y,x) = 1;
              update = true;
            }
          }
        }
      }
      thickness += updateMask * num;
      binary += updateMask * r;
    }
  }while( update );
  
  // finalinze
  for( int y = 0 ; y < binary.rows ; ++y ){
    for( int x = 0 ; x < binary.cols ; ++x ){
      if( 0 == binary(y,x) ){
        // line
        binary(y,x) = 1;
      }else{
        //background
        binary(y,x) = 0;
      }
    }
  }
  dilate(thickness,thickness,cv::Mat());
  thickness &= binary*255;
  return;
}
