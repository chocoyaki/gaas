/****************************************************************************/
/* Specific client scheduling : to implement some specific scheduling       */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2011/03/02 00:18:17  bdepardo
 * Removed warnings: comparison is always true due to limited range of data type
 *
 * Revision 1.5  2010/07/30 14:44:25  glemahec
 * Temporary corrections for the new compilation process. Parallel compilation is still broken and there is a big mess on the CMakeLists files...
 *
 * Revision 1.4  2010/07/13 08:17:39  glemahec
 * DIET 2.5 beta 1 - Client Specific Scheduler compilation error correction
 *
 * Revision 1.3  2009/09/23 14:16:18  bdepardo
 * Burst request now checks whether or not the SeDs it knows are still in the
 * list of available SeD, and removes them if necessary.
 * This is still to do for burstlimit.
 *
 * Revision 1.2  2009/09/07 11:18:53  bdepardo
 * Added #include <cstdlib> for exit()
 *
 * Revision 1.1  2007/07/11 08:42:09  aamar
 * Adding "custom client scheduling" mode (known as Burst mode). Need to be
 * activated in cmake.
 *
 ****************************************************************************/
#include "SpecificClientScheduler.hh"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <list>
#include <string>

#include "ORBMgr.hh"
#include "debug.hh"

using namespace std;

SpecificClientScheduler * SpecificClientScheduler::myScheduler = NULL;

const char * SpecificClientScheduler::BurstRequest = "BURST_REQUEST";
const char * SpecificClientScheduler::BurstLimit   = "BURST_LIMIT";
string       SpecificClientScheduler::schedulerId  = "UNDEFINED";
bool         SpecificClientScheduler::enabled      = false;

unsigned int allowed_req_per_sed = 1;

void setAllowedReqPerSeD(unsigned ix) {
  allowed_req_per_sed = ix;
}

SpecificClientScheduler::SpecificClientScheduler() :
  mySem(1) {
  enabled = true;
}

SpecificClientScheduler::~SpecificClientScheduler() {
}

void
SpecificClientScheduler::setSchedulingId(const char * scheduling_name) {
  schedulerId = scheduling_name;
  enabled = true;
} // end setScheduling

void
SpecificClientScheduler::start(SeD_var& chosenServer,
                               corba_response_t * response) {
  if (myScheduler == NULL && !enabled ) {
    cerr << "FATAL ERROR : burst scheduler not enabled" << endl;
    exit(1);
  }
  if (myScheduler == NULL) {
    myScheduler = new SpecificClientScheduler();
  }
  myScheduler->schedule(schedulerId.c_str(), chosenServer, response);
} // end start

void
SpecificClientScheduler::schedule(const char * scheduling_name,
                                  SeD_var& chosenServer,
                                  corba_response_t * response) {
  if (!strcmp(SpecificClientScheduler::BurstRequest, scheduling_name) ) {
    burstRequest(chosenServer, response);
  }
  if (!strcmp(SpecificClientScheduler::BurstLimit, scheduling_name) ) {
    burstLimitRequest(chosenServer, response);
  }
} // end schedule

void
SpecificClientScheduler::setScheduler(SpecificClientScheduler * scheduler) {
  myScheduler = scheduler;
}

/**
 * Round Robbin sur les SeDs ou sur les services?
 *
 */
void
SpecificClientScheduler::burstRequest(SeD_var& chosenServer,
                                      corba_response_t * response) {
  static vector<string> availableSeDs;
  static vector<int> use;
  static int min = 0;
  unsigned int ix;//, jx;
  bool found;
  vector<string>::iterator i_s, i_r;
  vector<int>::iterator i_us, i_ur;

  // Remove obsolete SeDs
  for (i_s = availableSeDs.begin(), i_us = use.begin();
       i_s != availableSeDs.end() && i_us != use.end(); ++ i_s, ++ i_us) {
    found = false;
    for (ix = 0; ix < response->servers.length(); ++ ix) {
      if (*i_s == static_cast<char*>(response->servers[ix].loc.SeDName)) {
        found = true;
        break;
      }
    }

    if (!found) {
      i_r = i_s;
      -- i_s;
      i_ur = i_us;
      -- i_us;
      TRACE_FUNCTION(TRACE_ALL_STEPS, "Removing a SeD from the list");
      availableSeDs.erase(i_r);
      use.erase(i_ur);
    }
  }

  // Add news SeDs
  for (ix = 0; ix < response->servers.length(); ix++) {
    found = false;
    for (i_s = availableSeDs.begin(); i_s != availableSeDs.end(); ++ i_s) {
      if (*i_s == static_cast<char*>(response->servers[ix].loc.SeDName)) {
        found = true;
        break;
      }
    }
    if (!found) {
      availableSeDs.push_back(string(response->servers[ix].loc.SeDName));
      TRACE_FUNCTION(TRACE_ALL_STEPS, "Adding a new SeD to the list");
      use.push_back(min);
    }
  }

  // Search the SeD with the minimum value => Less used
  min = *use.begin();
  i_ur = use.begin();
  i_r = availableSeDs.begin();

  for (i_s = ++ availableSeDs.begin(), i_us = ++ use.begin();
       i_s != availableSeDs.end(); ++ i_s, ++ i_us) {
    if (*i_us < min) {
      i_ur = i_us;
      min = *i_us;
      i_r = i_s;
    }
  } // end for
  TRACE_FUNCTION(TRACE_ALL_STEPS, "Burst scheduler chooses a SeD used " <<
                 min << " times before." <<
                 " The SeDs vector contains " << availableSeDs.size() <<
                 " references");
  (*i_ur) += 1;
  chosenServer = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, *i_r);
}

/**
 * Round Robbin sur les SeDs ou sur les services?
 *
 */
void
SpecificClientScheduler::burstLimitRequest(SeD_var& chosenServer,
                                           corba_response_t * response) {
  static vector<string> availableSeDs;
  static vector<int> use;

  // Add news SeDs
  for (unsigned int ix = 0; ix < response->servers.length(); ix++) {
    bool found = false;
    for (unsigned int jx = 0; jx < availableSeDs.size(); jx++) {
      if (availableSeDs[jx]==static_cast<char*>(response->servers[ix].loc.SeDName)) {
        found = true;
        break;
      }
    }
    if (!found) {
      availableSeDs.push_back(string(response->servers[ix].loc.SeDName));
      use.push_back(0);
      for (unsigned int ax = 0; ax < allowed_req_per_sed; ax++)
        mySem.post();
    }
  }

  // Search the SeD with the minimum value => Less used
  int min = use[0];
  int idx = 0;
  for (unsigned int ix = 0;
       ix < availableSeDs.size();
       ix++) {
    if (use[ix] < min) {
      idx = ix;
      min = use[ix];
    }
  } // end for
  cout << "Burst scheduler chooses a SeD used " <<
    use[idx] << "(" << min << ") times before." <<
    " The SeDs vector contains " << availableSeDs.size() <<
    " references" << endl;
  use[idx]++;
  chosenServer = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, availableSeDs[idx]);
}

void
SpecificClientScheduler::setSchedulingOptions(const char * schedOptions) {
  this->schedulingOptions = schedOptions;
  stringSplit(this->schedulingOptions, "|", this->myParams);
}

void
SpecificClientScheduler::pre_diet_call() {
  if (myScheduler == NULL) {
    myScheduler = new SpecificClientScheduler();
  }
  myScheduler->preCall();
} // end pre_diet_call

void
SpecificClientScheduler::post_diet_call() {
  if (myScheduler == NULL) {
    myScheduler = new SpecificClientScheduler();
  }
  myScheduler->postCall();
} // end post_diet_call


void
SpecificClientScheduler::preCall() {
  if (schedulerId == SpecificClientScheduler::BurstLimit) {
    this->mySem.wait();
  }
} // end preCall

void
SpecificClientScheduler::postCall() {
  if (schedulerId == SpecificClientScheduler::BurstLimit) {
    this->mySem.post();
  }
} // end postCall

bool
SpecificClientScheduler::isEnabled() {
  return enabled;
}

bool
SpecificClientScheduler::isOptionEnabled(string option, vector<string>& params) {
  for (unsigned int ix = 0; ix<params.size(); ix++)
    if (params[ix] == option) {
      return true;
    }
  return false;
}

void
SpecificClientScheduler::removeBlanks(string& token) {
  // remove blank
  size_t blankIt;
  while ( (blankIt = token.find_first_of(" ")) != token.npos) {
    token.erase(blankIt, blankIt + 1);
  }
}

void
SpecificClientScheduler::stringSplit(string str, string delim, vector<string>& results) {
  size_t cutAt;
  while( (cutAt = str.find_first_of(delim)) != str.npos ) {
    if(cutAt > 0) {
      string token = str.substr(0,cutAt);
      cout << "token " << token << endl;
      removeBlanks(token);
      results.push_back(token);
    }
    str = str.substr(cutAt+1);
  } // end while
  if(str.length() > 0) {
    removeBlanks(str);
    results.push_back(str);
  }
}

