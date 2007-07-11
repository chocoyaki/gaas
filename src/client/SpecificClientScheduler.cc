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
 * Revision 1.1  2007/07/11 08:42:09  aamar
 * Adding "custom client scheduling" mode (known as Burst mode). Need to be
 * activated in cmake.
 *
 ****************************************************************************/
#include "SpecificClientScheduler.hh"
#include <iostream>
#include <vector>

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
  static vector<SeD_var> availableSeDs;
  static vector<int> use;

  // Add news SeDs
  for (unsigned int ix=0; ix < response->servers.length(); ix++) {
    bool found = false;
    for (unsigned int jx=0; jx < availableSeDs.size(); jx++) {
      if (availableSeDs[jx]->_is_equivalent(response->servers[ix].loc.ior._ptr)) {
        found = true;
        break;
      }
    }
    if (!found) {
      availableSeDs.push_back(response->servers[ix].loc.ior);
      use.push_back(0);
    }
  }

  // Search the SeD with the minimum value => Less used
  int min = use[0];
  int idx = 0;
  for (unsigned int ix=0;
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
  chosenServer = availableSeDs[idx];  
}

/**
 * Round Robbin sur les SeDs ou sur les services?
 * 
 */
void
SpecificClientScheduler::burstLimitRequest(SeD_var& chosenServer,
                                           corba_response_t * response) {
  static vector<SeD_var> availableSeDs;
  static vector<int> use;

  // Add news SeDs
  for (unsigned int ix=0; ix < response->servers.length(); ix++) {
    bool found = false;
    for (unsigned int jx=0; jx < availableSeDs.size(); jx++) {
      if (availableSeDs[jx]->_is_equivalent(response->servers[ix].loc.ior._ptr)) {
        found = true;
        break;
      }
    }
    if (!found) {
      availableSeDs.push_back(response->servers[ix].loc.ior);
      use.push_back(0);
      for (unsigned int ax = 0; ax < allowed_req_per_sed; ax++)
        mySem.post();
    }
  }

  // Search the SeD with the minimum value => Less used
  int min = use[0];
  int idx = 0;
  for (unsigned int ix=0;
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
  chosenServer = availableSeDs[idx];  
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
  for (unsigned int ix=0; ix<params.size(); ix++)
    if (params[ix] == option) {
      return true;
    }
  return false;
}

void
SpecificClientScheduler::removeBlanks(string& token) {
  // remove blank
  int blankIt;
  while ( (blankIt = token.find_first_of(" ")) != token.npos) {
    token.erase(blankIt, blankIt + 1);
  }
}

void
SpecificClientScheduler::stringSplit(string str, string delim, vector<string>& results) {
  int cutAt;
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

