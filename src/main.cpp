#include"utils.hpp"
#include"thinning.hpp"
#include<iostream>
#include<boost/program_options.hpp>

using namespace boost::program_options;

options_description set_options(){
  options_description opt("Options");
  opt.add_options()
    ("help,h",                       "Show help")
    ("input" , value<std::string>(),      "Input image file");
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
    std::cerr << "Unknown command line option: " << ex.what() << std::endl << opt << std::endl;
    throw;
  }
  notify(argmap);
  // show help
  if( argmap.count("help") || !argmap.count("input") ) {
    std::cerr << opt << std::endl;
    exit(1);
  }
  // processing
  std::string input = argmap["input"].as<std::string>();
  cv::Mat im = imread_as_grayscale(input);
  skeleton sk(im,true);
  cv::Mat binary = sk.binary.clone();
  sk.thinning(ZHANGSUEN,true);
  
  cv::namedWindow("gray", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  cv::imshow("gray", im);
  cv::waitKey(0);
  cv::namedWindow("binary", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  cv::imshow("binary", binary);
  cv::waitKey(0);
  cv::namedWindow("thinning", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  cv::imshow("thinning", sk.binary);
  cv::waitKey(0);

  double maxVal;
  cv::minMaxLoc(sk.thickness, NULL, &maxVal, NULL, NULL);
  std::cout << maxVal << std::endl;
  cv::namedWindow("thickness", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  cv::imshow("thickness", sk.thickness * (255./maxVal));
  cv::waitKey(0);
  
  return 0;
}
