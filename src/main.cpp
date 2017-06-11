#include"utils.hpp"
#include"thinning.hpp"
#include<iostream>
#include<boost/program_options.hpp>
#include<boost/filesystem.hpp>
#include<boost/filesystem/fstream.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

po::options_description set_options(){
  po::options_description opt("Options");
  opt.add_options()
    ("help,h"   ,                            "Show help")
    ("input,i"  , po::value<fs::path>(),  "Input image file")
    ("output,o" , po::value<fs::path>(),  "Output directory");
  return opt;
}

int main( int argc, char* argv[] ){
  // set options
  po::options_description opt = set_options();
  // parse command line
  po::variables_map argmap;
  try{
    store(parse_command_line(argc, argv, opt), argmap);
  }catch (std::exception& ex){
    std::cerr << "[ERROR] " << ex.what() << std::endl << opt << std::endl;
    exit(1);
  }
  notify(argmap);
  // show help
  if( argmap.count("help") || !argmap.count("input") || !argmap.count("output") ){
    std::cerr << opt << std::endl;
    exit(1);
  }

  // error handling of file I/O
  fs::path input = argmap["input"].as<fs::path>();
  fs::path output = argmap["output"].as<fs::path>();
  if( !fs::exists(input) ){
    std::cerr << "[ERROR] No such file " << input <<std::endl;
    exit(1);
  }
  cv::Mat im = imread_as_grayscale(input.generic_string(),true);
  if( im.empty() ){
    std::cerr << "[ERROR] Unable to read image " << input << std::endl;
    exit(1);
  }

  if( fs::exists(output) ){
    std::cerr<< "[WARNING] Directory " << output << " already exists" << std::endl;
  }else if( !fs::create_directories(output) ){
    std::cerr<< "[ERROR] Unable to create directory "<< output <<std::endl;
    exit(1);
  }

  fs::ofstream log(output / "log");

  // processing
  cv::Mat binary = to_binary_image(im,15);

  skeleton sk(binary);

  double radius = 3;
  sk.thinning(TRAPPEDBALL,false,radius);
  
  double maxVal;
  cv::minMaxLoc(sk.thickness, NULL, &maxVal, NULL, NULL);

  cv::namedWindow("image", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  cv::imshow("image", im);
  cv::waitKey(0);
  cv::imshow("image", binary);
  cv::waitKey(0);
  cv::imshow("image", sk.binary);
  cv::waitKey(0);
  cv::imshow("image", 255-(sk.thickness * (255./maxVal)));
  cv::waitKey(0);

  
  cv::Mat feature = cv::Mat::zeros(sk.binary.size(),CV_8UC1);

  int dx[] = {  0,  1,  1,  1,  0, -1, -1, -1};
  int dy[] = { -1, -1,  0,  1,  1,  1,  0, -1};
  int dirs = 8;
  
  for( int y = 0 ; y < sk.binary.rows ; ++y ){
    for( int x = 0 ; x < sk.binary.cols ; ++x ){
      if( 0 != sk.binary.data[ y*sk.binary.step + x*sk.binary.elemSize()] ){
        int c=0;
        for( int i = 0 ; i < dirs ; ++i ){
          int nxt = (i+1)%dirs;
          int px = x + dx[i];
          int py = y + dy[i];
          int nx = x + dx[nxt];
          int ny = y + dy[nxt];
          if( 0 == sk.binary.data[ py*sk.binary.step + px*sk.binary.elemSize()] &&
              0 != sk.binary.data[ ny*sk.binary.step + nx*sk.binary.elemSize()] )
            c += 1;
        }
        if( c==1 || c==3 || c==4 ){
          feature.data[ y*feature.step + x*feature.elemSize() ] = 1;
        }
      }
    }
  }
  
  sk.binary = (sk.binary & ((1-feature)*31) ) | feature*255;
  cv::imshow("image", sk.binary);
  cv::imwrite( (output/"feature.pgm").generic_string() , sk.binary );
  cv::waitKey(0);

  return 0;
}
