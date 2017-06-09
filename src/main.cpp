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
    std::cerr << "[ERROR] No such file( " << input << " )" <<std::endl;
    exit(1);
  }
  cv::Mat im = imread_as_grayscale(input.generic_string());
  if( im.empty() ){
    std::cerr << "[ERROR] Unable to read image( " << input << " )" << std::endl;
    exit(1);
  }

  if( fs::exists(output) ){
    std::cerr<< "[WARNING] " << output << " already exists" << std::endl;
  }else if( !fs::create_directories(output) ){
    std::cerr<< "[ERROR] Unable to create directory( "<< output << ")" <<std::endl;
    exit(1);
  }

  fs::ofstream log(output / "log");

  // processing
  skeleton sk(im,true);
  cv::Mat binary = sk.binary.clone();
  sk.thinning(ZHANGSUEN,true);
  
  double maxVal;
  cv::minMaxLoc(sk.thickness, NULL, &maxVal, NULL, NULL);

  cv::namedWindow("image", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  cv::imshow("image", im);
  cv::waitKey(0);
  cv::imshow("image", binary);
  cv::waitKey(0);
  cv::imshow("image", sk.binary);
  cv::waitKey(0);
  cv::imshow("image", sk.thickness * (255./maxVal));
  cv::waitKey(0);
  
  return 0;
}
