#include "myscheduler.hh"
#include <scheduler/est_internal.hh>
#include <boost/foreach.hpp>
#include <iostream>
#include "common.hh"
#include <map>
#include <list>

struct is_busy {
  bool operator() (const corba_server_estimation_t& e) { return diet_est_get_internal(&(e.estim), EST_CURRENTJOBS, 0.0) > 0; }
};

struct is_busy_multicores {
 bool operator() (const corba_server_estimation_t& e) { return diet_est_get_internal(&(e.estim), EST_CURRENTJOBS, 0.0) >= diet_est_get_internal(&(e.estim), EST_NUMCORES, 0.0); }
};


class MyScheduler : public UserScheduler {

public:

  static const char* stName;
  MyScheduler();
  ~MyScheduler();
  void init();
  static char* serialize(MyScheduler* GS);
  static MyScheduler* deserialize(const char* serializedScheduler);
  int aggregate(corba_response_t* aggrResp, size_t max_srv,
                const size_t nb_responses, const corba_response_t* responses);
};

const char* MyScheduler::stName="UserGS";

MyScheduler::~MyScheduler() {}

MyScheduler::MyScheduler() {
  this->name = this->stName;
  this->nameLength = strlen(this->name);
}

int MyScheduler::aggregate(corba_response_t* aggrResp, size_t max_srv,
                           const size_t nb_responses,
                           const corba_response_t* responses) {
  std::cout << "MyScheduler::aggregate called" << std::endl;
  
  /* Convert the corba response to a list */

  ServerList candidates = CORBA_to_STL(responses, nb_responses);
  
  BOOST_FOREACH(corba_server_estimation_t &e, candidates) {
    double cpu_idle = diet_est_get_internal(&(e.estim), EST_CPUIDLE, 0.0);
    double conso = diet_est_get_internal(&(e.estim), EST_CONSOJOB, 0.0);
    double node_flops = diet_est_get_internal(&(e.estim), EST_NODEFLOPS, 0.0);
    double core_flops = diet_est_get_internal(&(e.estim), EST_COREFLOPS, 0.0);
    double num_cores = diet_est_get_internal(&(e.estim), EST_NUMCORES, 0.0);
    int current_jobs = diet_est_get_internal(&(e.estim), EST_CURRENTJOBS, 0.0);
    std::cout << "metrics for server " << e.loc.hostName << std::endl;
    std::cout << "  cpu_idle   = " << cpu_idle << std::endl;
    std::cout << "  conso      = " << conso << std::endl;
    std::cout << "  node_flops = " << node_flops << std::endl;
    std::cout << "  core_flops = " << core_flops << std::endl;
    std::cout << "  num_cores  = " << num_cores << std::endl;
    std::cout << "  current_jobs = " << current_jobs << std::endl;
  }
  
  // Exclude servers that already working
  candidates.remove_if (is_busy());
    
  /* We select the SeD by determined criteria (see myscheduler.hh) */
  SORT(candidates, compFunction);
  
  /* Convert the sorted list to a corba sequence*/
  STL_to_CORBA(candidates, aggrResp);

  /* Display the result of the sorting*/
  BOOST_FOREACH(corba_server_estimation_t &e, candidates) {
    double conso = diet_est_get_internal(&(e.estim), EST_CONSOJOB, 0.0);
    std::cout << "metrics CONSOJOB for server " << e.loc.hostName << " = " << conso << std::endl;
  }
    
  return 0;
}

SCHEDULER_CLASS(MyScheduler)
