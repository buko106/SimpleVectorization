#include"utils.hpp"
#include"thinning.hpp"
#include"topology.hpp"
#include"bezier.hpp"
#include"svg.hpp"
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
    ("output,o" , po::value<fs::path>(),  "Output directory")
    ("radius,r" , po::value<int>()->default_value(3), "Radius of trapped-ball")
    ("tolerance,t", po::value<double>()->default_value(1.), "tolarable average fitting error");
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
  cv::Mat_<uchar> im = imread_as_grayscale(input.generic_string(),true);
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

  // processing
  cv::Mat_<uchar> binary = to_binary_image(im,15);

  skeleton sk(binary);

  double radius = argmap["radius"].as<int>();
  sk.thinning(TRAPPEDBALL,false,radius);
  
  double maxVal;
  cv::minMaxLoc(sk.thickness, NULL, &maxVal, NULL, NULL);

  cv::imwrite((output/"binary.png").generic_string(),binary);
  cv::imwrite((output/"skeleton.png").generic_string(),sk.binary);
  cv::imwrite((output/"thickness.png").generic_string(), 255-(sk.thickness * (255./maxVal)));
  topology tp(sk,false);


  fs::ofstream graph(output / "graph.svg");

  svg svg_graph(0,0,im.cols,im.rows);
  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    std::pair<double,bezier>
      result = bezier_quadratic_fitting( tp.edge[i], tp.w_max );
    bezier curve = result.second;
    svg_graph.push(curve);
  }

  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    size_t n = tp.edge[i].size();
    svg_graph.push(tp.edge[i][0].x,tp.edge[i][0].y);
    svg_graph.push(tp.edge[i][n-1].x,tp.edge[i][n-1].y);
  }
  graph << svg_graph << std::endl;

  graph.close();

  double tolerance = argmap["tolerance"].as<double>();
  tp.refine(tolerance);
  graph.open(output / "refined.svg");


  svg svg_refined(0,0,im.cols,im.rows);
  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    std::pair<double,bezier>
      result = bezier_quadratic_fitting( tp.edge[i], tp.w_max );
    bezier curve = result.second;
    svg_refined.push(curve);
  }

  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    size_t n = tp.edge[i].size();
    svg_refined.push(tp.edge[i][0].x,tp.edge[i][0].y);
    svg_refined.push(tp.edge[i][n-1].x,tp.edge[i][n-1].y);
  }
  graph << svg_refined << std::endl;
  
  graph.close();

  return 0;
}
