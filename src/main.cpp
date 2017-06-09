#include"utils.hpp"
#include"thinning.hpp"
#include<iostream>
#include<boost/program_options.hpp>

using namespace boost::program_options;

options_description set_options(){
  options_description opt("Options");
  opt.add_options()
    ("help,h"   ,                         "Show help")
    ("input,i"  , value<std::string>(),   "Input image file")
    ("output,o" , value<std::string>(),   "Output directory");
  return opt;
}

int main( int argc, char* argv[] ){
  // set options
  options_description opt = set_options();
  // parse command line
  variables_map argmap;
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
  // processing
  std::string input = argmap["input"].as<std::string>();
  cv::Mat im = imread_as_grayscale(input);
  if( im.empty() ){
    std::cerr << "[ERROR] Unable to read image(\"" << input << "\")" << std::endl;
    exit(1);
  }

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
