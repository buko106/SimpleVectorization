#include"utils.hpp"
#include"thinning.hpp"
#include"topology.hpp"
#include"bezier.hpp"
#include"svg.hpp"
#include"hypergraph.hpp"
#include"gui.hpp"
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
    ("disp,d"   ,                            "Always display drawing")
    ("input,i"  , po::value<fs::path>(),  "Input image file")
    ("output,o" , po::value<fs::path>(),  "Output directory")
    ("radius,r" , po::value<int>()->default_value(3), "Radius of trapped-ball")
    ("tolerance,t", po::value<double>()->default_value(2.), "Tolarable average fitting error")
    ("mu,m" , po::value<double>()->default_value(3.0), "Balance parameter for degree of bezier curves. High value means less dgree")
    ("lambda,l" , po::value<double>()->default_value(0.984375), "Balance parameter for fidelity and simplicity. Must be in range [0,1]. High value means high simplicity");
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

  double lambda = argmap["lambda"].as<double>();
  double mu = argmap["mu"].as<double>();
  // show help
  if( argmap.count("help") || !argmap.count("input") || !argmap.count("output")
      || lambda < 0.0 || lambda > 1.0 ){
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

  if( fs::exists(output/"iter") ){
    std::cerr<< "[WARNING] Directory " << output/"iter" << " already exists" << std::endl;
  }else if( !fs::create_directories(output/"iter") ){
    std::cerr<< "[ERROR] Unable to create directory "<< output/"iter" <<std::endl;
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
      result = bezier_cubic_fitting( tp.edge[i], tp.w_max );
    bezier curve = result.second;
    svg_graph.push(curve);
    svg_graph.push(curve[0].first,curve[0].second);
    svg_graph.push(curve[curve.size()-1].first,curve[curve.size()-1].second);
  }

  graph << svg_graph << std::endl;

  graph.close();

  double tolerance = argmap["tolerance"].as<double>();
  tp.refine(tolerance);
  graph.open(output / "refined.svg");


  svg svg_refined(0,0,im.cols,im.rows);
  for( size_t i = 0 ; i < tp.edge.size() ; ++i ){
    std::pair<double,bezier>
      result = bezier_cubic_fitting( tp.edge[i], tp.w_max );
    bezier curve = result.second;
    svg_refined.push(curve);
    svg_refined.push(curve[0].first,curve[0].second);
    svg_refined.push(curve[curve.size()-1].first,curve[curve.size()-1].second);
  }

  graph << svg_refined << std::endl;
  
  graph.close();

  // iter/iter0.svg
  graph.open(output /"iter"/"iter0.svg");
  graph << svg_refined << std::endl;  
  graph.close();

  hypergraph hyper( tp, LINE );
  int iter = 100000;
  int num  =   1000;
  fs::ofstream log(output/"log");

  double normaled_lambda = lambda /( (1.-lambda)*(1.+1.5*mu) + lambda );
  for( int i = 1 ; i <= iter ; ++i ){
    double U = hyper.step( normaled_lambda, mu );
    log << i << " " << U <<std::endl;

    if( i%num==0 || argmap.count("disp") ){
      std::vector<bezier> v;
      svg svg_iter(0,0,im.cols,im.rows);
      
      std::vector< std::pair<edge_t,BEZIER_DEG> >
        curve = hyper.to_bezier();
      for( size_t c = 0 ; c < curve.size() ; ++c ){
        std::pair<double,bezier> res = bezier_fittting( curve[c].first, tp.w_max, curve[c].second );
        if( i%num == 0 ){
          svg_iter.push(res.second);
          svg_iter.push(res.second[0].first,res.second[0].second);
          svg_iter.push(res.second[res.second.size()-1].first,res.second[res.second.size()-1].second);
        }
        if( argmap.count("disp") ){
          v.push_back(res.second);
        }
      }

      if( argmap.count("disp") ){
        cv::namedWindow("image", CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
        cv::imshow("image", to_Mat(v,0,0,im.cols,im.rows) );
        cv::waitKey(1);
      }
      if( i%num == 0 ){
        // output graph
        graph.open(output /"iter"/("iter"+std::to_string(i)+".svg"));
        graph << svg_iter << std::endl;
        graph.close();
      }
    }
  }
  return 0;
}
