#include "MultiCall.hh"
#include "est_internal.hh"
#include <iostream>

using namespace std;

corba_response_t * MultiCall::corba_response = NULL;

void MultiCall::set_response(corba_response_t *response) {
  corba_response = response;
}

corba_response_t* MultiCall::get_response() {
  return corba_response;
}

//computes the number of scenario per cluster
vector<int> MultiCall::cerfacsSchedule() {
  vector<int> nbDags;
  int nbClusters = corba_response->servers.length();
  int NS = (&(corba_response->servers[0]).estim)->estValues.length() - 1;
  int dag;
  int MSmin;
  int temp;
  int clusterMin;
  int i;

  for (i = 0; i < nbClusters; i++) {
    nbDags.push_back(0);
  }

  for (dag = 0; dag < NS; dag++) {
    MSmin = -1;
    clusterMin = 0;
    for (i = 0; i < nbClusters; i++) {
      temp = diet_est_get_internal((&(corba_response->servers[i]).estim), EST_USERDEFINED + (nbDags[i]), 0);
      if (MSmin == -1 || temp < MSmin) {
	MSmin = temp;
	clusterMin = i;
      }
    }
    nbDags[clusterMin]++;
  }
  return nbDags;
}

