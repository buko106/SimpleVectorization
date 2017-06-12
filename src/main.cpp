#include"utils.hpp"
#include"thinning.hpp"
#include"topology.hpp"
#include"bezier.hpp"
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

  cv::imwrite((output/"binary.png").generic_string(),binary);
  cv::imwrite((output/"skeleton.png").generic_string(),sk.binary);
  cv::imwrite((output/"thickness.png").generic_string(), 255-(sk.thickness * (255./maxVal)));
  topology tp(sk,false);

  double w_max = -DBL_MAX;
  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    for( size_t j = 0 ; j < tp.edge[i].size() ; ++j ){
      w_max = std::max<double>(w_max,tp.edge[i][j].w);
    }
  }

  fs::ofstream graph(output / "graph.svg");
  //   <path d="C 125 5E+1 175 50 200 0" stroke="blue" stroke-width="3"  fill="none" />
  graph << "<svg viewBox=\"0 0 " << im.cols << " " << im.rows << "\">" << std::endl;
  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    auto result = bezier_cubic_fitting( tp.edge[i], w_max );
    auto curve = result.second;

    graph << "\t<path d=\"" ;
    for( size_t p = 0 ; p < curve.size() ; ++p ){
      if( p == 0 ) graph << " M ";
      if( p == 1 ) graph << " C ";
      graph << " " << curve[p].first << " " << curve[p].second;
    }
    graph << "\" stroke=\"blue\" stroke-width=\"3\"  fill=\"none\" />\n" ;
  }

  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    graph << "\t<circle cx=\"" << tp.edge[i][0].x 
          << "\" cy=\"" << tp.edge[i][0].y 
          << "\" r=\"3\" stroke=\"none\" fill=\"red\"/>\n";
    graph << "\t<circle cx=\"" << tp.edge[i][tp.edge[i].size()-1].x 
          << "\" cy=\"" << tp.edge[i][tp.edge[i].size()-1].y 
          << "\" r=\"3\" stroke=\"none\" fill=\"red\"/>\n";
  }
  graph <<  "</svg>" << std::endl;

  std::cout << "w_max=" << w_max << std::endl;
  
  graph.close();

  tp.refine(0.5);
  graph.open(output / "refined.svg");
  //   <path d="C 125 5E+1 175 50 200 0" stroke="blue" stroke-width="3"  fill="none" />
  graph << "<svg viewBox=\"0 0 " << im.cols << " " << im.rows << "\">" << std::endl;
  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    auto result = bezier_cubic_fitting( tp.edge[i], w_max );
    auto curve = result.second;

    graph << "\t<path d=\"" ;
    for( size_t p = 0 ; p < curve.size() ; ++p ){
      if( p == 0 ) graph << " M ";
      if( p == 1 ) graph << " C ";
      graph << " " << curve[p].first << " " << curve[p].second;
    }
    graph << "\" stroke=\"blue\" stroke-width=\"3\"  fill=\"none\" />\n" ;
  }

  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    graph << "\t<circle cx=\"" << tp.edge[i][0].x 
          << "\" cy=\"" << tp.edge[i][0].y 
          << "\" r=\"3\" stroke=\"none\" fill=\"red\"/>\n";
    graph << "\t<circle cx=\"" << tp.edge[i][tp.edge[i].size()-1].x 
          << "\" cy=\"" << tp.edge[i][tp.edge[i].size()-1].y 
          << "\" r=\"3\" stroke=\"none\" fill=\"red\"/>\n";
  }
  graph <<  "</svg>" << std::endl;

  std::cout << "w_max=" << w_max << std::endl;
  
  graph.close();

  return 0;
}
