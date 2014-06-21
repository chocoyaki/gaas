#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include "server_metrics.hh"
#include "server_utils.hh"
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <istream>
//#include "server.hh"

using namespace std;

void MetricsAggregator::init(std::string node_name) {
  if (node_name == "") {
    const int buf_size = 256;
    char buf[buf_size];
    buf[buf_size-1] = 0;
    if (0 != gethostname(buf, buf_size-1)) {
      throw std::exception();
    }
    node_name = std::string(buf);
  }
  this->node_name = node_name;
  //cluster_name = get_cluster(node_name);
  //site_name = get_site(node_name);
  //node_flops = ::get_node_flops(node_name, cluster_name, site_name);
  //core_flops = ::get_core_flops(node_name, cluster_name, site_name);
  //num_cores = ::get_num_cores(node_name, cluster_name, site_name);
  //conso_job = ::get_bench_conso();
  //image_id = ::get_image_id(node_name);
  //std::cout << "node: " << node_name << std::endl;
  //std::cout << "cluster: " << cluster_name << std::endl;
  //std::cout << "site: " << site_name << std::endl;
  //std::cout << "node_flops: " << node_flops << std::endl;
  //std::cout << "core_flops: " << core_flops << std::endl;
  //std::cout << "num_cores: " << num_cores << std::endl;
  //std::cout << "conso_job(user_defined_benchmark): " << conso_job << std::endl;
  //std::cout << "ID Image:" << image_id << std::endl;
}
