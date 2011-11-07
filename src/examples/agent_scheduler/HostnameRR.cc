/**
 * @file HostnameRR.cc
 *
 * @brief  Hostname scheduler
 *
 * @author  David Loureiro (david.loureiro@sysfera.com)
 *
 * @section Licence
 *   |LICENCE|
 */

#include <map>
#include <string>

#include "GlobalSchedulers.hh"
#include "UserScheduler.hh"
#include "est_internal.hh"

std::map<std::string, unsigned int> hostCounter;

class HostnameRR: public UserScheduler {
public:
static const char *stName;

HostnameRR();
~HostnameRR();
void
init();

static char *
serialize(HostnameRR *GS);
static HostnameRR *
deserialize(const char *serializedScheduler);
/* Overriden aggregate method to schedule jobs with the SRA policy. */
int
aggregate(corba_response_t *aggrResp, size_t max_srv,
          const size_t nb_responses, const corba_response_t *responses);
};

using namespace std;

const char *HostnameRR::stName = "UserGS";

HostnameRR::~HostnameRR() {
}

HostnameRR::HostnameRR() {
  this->name = this->stName;
  this->nameLength = strlen(this->name);
}

int
HostnameRR::aggregate(corba_response_t *aggrResp, size_t max_srv,
                      const size_t nb_responses,
                      const corba_response_t *responses) {
  ServerList::iterator itSeD;
  unsigned int nbUsage = 0;
  corba_server_estimation_t selected;


  cout << "******************** HostnameRR ********************" << endl;
  ServerList candidates = CORBA_to_STL(responses, nb_responses);

  for (itSeD = candidates.begin(); itSeD != candidates.end(); ++itSeD)
    // We select the SeD by its host usage.
    if (hostCounter[HOSTNAME(*itSeD)] <= nbUsage) {
      selected = *itSeD;
    }

  aggrResp->servers.length(1);
  aggrResp->servers[0] = selected;

  return 0;
} // aggregate

SCHEDULER_CLASS(HostnameRR)
