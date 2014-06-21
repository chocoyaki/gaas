#ifndef _SERVER_METRICS_HH_
#define _SERVER_METRICS_HH_

#include <stdint.h>
#include <deque>
#include <vector>
#include <utility>
#include <string>
#include "boost/tuple/tuple.hpp"
#include <iostream>
#include <fstream>
#include <istream>
#include <cstdlib>
#include <string>

using namespace std;

typedef boost::tuple<time_t, time_t, double> measure;
typedef std::vector<measure> measures;
typedef boost::tuple<uint64_t, uint64_t, boost::tuple<measures, measures> > exec_record;
typedef std::deque<exec_record> exec_records;


// aggregate metrics of a sed execution
class MetricsAggregator {

protected:

  exec_records execs;
  std::string node_name;
  std::string cluster_name;
  std::string site_name;
  double core_flops;
  double node_flops;
  int num_cores;
  int current_jobs;
  double conso_job;


  void flush();

public:

  std::string image_id;

  void init(std::string node_name = std::string(""));
};



#endif
