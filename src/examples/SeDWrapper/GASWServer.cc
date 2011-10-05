/****************************************************************************/
/* SeD based on GASW description of executable program                      */
/* Generic Application Service Wrapper (GASW)                               */
/* http://modalis.polytech.unice.fr/softwares/moteur/gasw                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/04/06 15:02:37  bdepardo
 * Added SeDWrapper example. This example is compiled when workflows are activated.
 *
 */


#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/unistd.h>
#include <time.h>
#include "omnithread.h"
#include "boost/filesystem.hpp"

#include "SeDDescrParser.hh"
#include "SeDService.hh"
#include "DIET_server.h"
#include "DagWfParser.hh" // for XMLParsingException

#define TRACE(mess) {				\
    cout << "[GASW] " << mess << endl;		\
  }
#define WARN(mess) {				\
    cerr << "[GASW] " << mess << endl;		\
  }

using namespace std;
namespace fs = boost::filesystem;

map<string, SeDService*> serviceTable;
omni_mutex myLock;

/* MUST BE REENTRANT */
SeDService*
getService(const string& serviceName) {
  myLock.lock();
  map<string, SeDService*>::iterator iter = serviceTable.find(serviceName);
  myLock.unlock();
  if (iter != serviceTable.end())
    return (SeDService*) iter->second;
  else {
    WARN("Cannot find service name " << serviceName);
    exit(1);
  }
}

/* MUST BE REENTRANT */
void
performance_Exec_Time(diet_profile_t* pb ,estVector_t perfValues )
{
  SeDService *serv = getService(string(pb->pb_name));
  double tcomp = serv->getAvgComputationTime();
  TRACE("Estimation of computation time for '" << serv->getName()
	<< "' = " << tcomp << " ms ");
  double eft = 0;
  /* Set the job duration and compute SeD's EFT (results stored in EV) */
  diet_estimate_comptime(perfValues, tcomp);
  diet_estimate_eft(perfValues, tcomp, pb);
  /* Get the values from EV */
  tcomp = diet_est_get_system(perfValues, EST_TCOMP, 10000000);
  eft = diet_est_get_system(perfValues, EST_EFT, 10000000);
  TRACE("TCOMP=" << tcomp <<  "/ EFT=" << eft);
}

void
set_up_scheduler(diet_profile_desc_t* profile)
{
  diet_aggregator_desc_t *agg = NULL;
  agg = diet_profile_desc_aggregator(profile);
  diet_service_use_perfmetric(performance_Exec_Time);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
  diet_aggregator_priority_min(agg, EST_EFT);
}

/* MUST BE REENTRANT */
int
serviceRun(diet_profile_t* pb)
{
  // Make copy of original service object
  SeDService *serv = new SeDService(*getService(string(pb->pb_name)));

  // Generate unique request ID
  serv->genUniqueReqId();
  TRACE("Service request unique id: " << serv->getReqId());

  // Current directory
  fs::path  initialPath = fs::initial_path();
  TRACE("Current directory: " << initialPath);

  // Working directory
  if (serv->createWorkingDirectory(initialPath.string()) != 0)
    return 1;
  TRACE("Working directory: " << serv->getWorkingDirectory());

  // Service dependencies (check and download if not already present)
  if (serv->cpyDependencies() != 0)
    return 1;

  // Copy input data from profile to the service request arguments
  serv->cpyProfileToArgs(pb);

  // Store start time
  struct timeval t1;
  gettimeofday(&t1, NULL);

  // Launch executable
  string cmdLine;
  serv->genCommandLine(cmdLine);
  TRACE("Launch command: " << cmdLine);
  cmdLine = "cd " + serv->getWorkingDirectory() + " && " + cmdLine;
  int res = system(cmdLine.c_str());

  // Store finish time
  struct timeval t2;
  gettimeofday(&t2, NULL);

  // Copy output data (files) to profile
  if (!serv->cpyArgsToProfile(pb)) {
    WARN("Outputs processing failed - JOB FAILED");
    res = -1;
  }

  // Cleanup
  //   if (!serv->removeWorkingDirectory()) {
  //     cout << "Removed working directory: done." << endl;
  //   }

  // delete the current instance of service
  delete serv;

  // update reference service with value of computation time for this request
  //   double time_us = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
  //   myLock.lock();
  //   getService(string(pb->pb_name))->setComputationTimeMeasure(time_us / 1000);
  //   myLock.unlock();

  return res;
}

int main(int argc, char * argv[])
{
  if (argc != 4) {
    WARN("Usage: " << argv[0] << " <diet_config> <gasw_file.xml> <compTime>");
    exit(1);
  }
  string fileName = argv[2];
  string avgCompTime = argv[3]; /* in ms */

  TRACE("Parsing file : " << fileName);
  SeDDescrParser * parser = new GASWParser(fileName);
  try {
    parser->parseXml();
  } catch (XMLParsingException& e) {
    WARN("XML Parser exception: " << e.ErrorMsg());
    exit(1);
  }
  const list<SeDService*>& serviceList = parser->getServices();

  // Declare services
  TRACE("Declare services to DIET");
  diet_service_table_init(serviceList.size());

  for (list<SeDService*>::const_iterator servIter = serviceList.begin();
       servIter != serviceList.end();
       ++servIter)
    {
      SeDService *currServ = (SeDService*) *servIter;
      serviceTable[currServ->getName()] = currServ;

      // Set estimation of computation time
      currServ->setAvgComputationTime(atoi(avgCompTime.c_str()));

      diet_profile_desc_t * profile = currServ->createAndDeclareProfile();

      set_up_scheduler(profile);
      if (diet_service_table_add(profile, NULL, serviceRun)) return 1;
      diet_profile_desc_free(profile);
    }

  diet_print_service_table();
  int res = diet_SeD(argv[1], argc, argv);
  delete parser;
  return res;
}
