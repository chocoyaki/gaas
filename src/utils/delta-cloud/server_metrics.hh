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

  std::string site_name;

  double alert_flag;

  double instant_energy;

  double cpu_avg;

  void flush();

public:

  std::string image_id;

  void init(std::string site_name = std::string(""));
  double check_img(std::string vm_name);

  double get_avg_cpu();
  /*double get_alert();
  double get_instant_energy();
   */
};



#endif
