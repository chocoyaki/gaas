#include <UserScheduler.hh>
#include <boost/foreach.hpp>
#include <iostream>
#include "DIETSchedTestsCommon.hpp"

class DIETSchedTestScheduler : public UserScheduler {

public:

  static const char* stName;
  DIETSchedTestScheduler();
  ~DIETSchedTestScheduler();
  void init();
  static char* serialize(DIETSchedTestScheduler* scheduler);
  static DIETSchedTestScheduler* deserialize(const char* serializedScheduler);
  int aggregate(corba_response_t* aggrResp, size_t max_srv,
                const size_t nb_responses, const corba_response_t* responses);
};

const char* DIETSchedTestScheduler::stName="UserGS";

DIETSchedTestScheduler::~DIETSchedTestScheduler() {}

DIETSchedTestScheduler::DIETSchedTestScheduler() {
  this->name = this->stName;
  this->nameLength = strlen(this->name);
}

int DIETSchedTestScheduler::aggregate(corba_response_t* aggrResp, size_t max_srv,
                                      const size_t nb_responses,
                                      const corba_response_t* responses) {
  std::cout << "DIETSchedTestScheduler::aggregate called" << std::endl;
  ServerList candidates = CORBA_to_STL(responses, nb_responses);
  BOOST_FOREACH(corba_server_estimation_t &e, candidates) {
    double m1 = diet_est_get(&(e.estim), EST_CUSTOMMETRIC1, 0.0);
    double m2 = diet_est_get(&(e.estim), EST_CUSTOMMETRIC2, 0.0);
    double m3 = diet_est_get(&(e.estim), EST_CUSTOMMETRIC3, 0.0);
    double m4 = diet_est_get(&(e.estim), EST_CUSTOMMETRIC4, 0.0);
    double m5 = diet_est_get(&(e.estim), EST_CUSTOMMETRIC5, 0.0);
  }
  STL_to_CORBA(candidates, aggrResp);
  return 0;
}

SCHEDULER_CLASS(DIETSchedTestScheduler)
