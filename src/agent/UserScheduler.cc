/****************************************************************************/
/* DIET UserScheduler class source code.                                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael LE MAHEC (lemahec@clermont.in2p3.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

#include "UserScheduler.hh"
#include <dlfcn.h>

/** To avoid multiple load of the chosen scheduler, "instance" is static and
 *  won't be changed after the first call to the "getInstance" method.
 */
UserScheduler* UserScheduler::instance = NULL;
const char* UserScheduler::stName = "UserGS";

UserScheduler* UserScheduler::getInstance(const char * moduleName) {
  if (instance==NULL) instance = new UserScheduler(moduleName);
  return instance;
}

/** The default constructor. */
UserScheduler::UserScheduler() {
  this->name = this->stName;
  this->nameLength = strlen(this->name);
}

/** The private constructor. "moduleName" is the path to the scheduler module. */
UserScheduler::UserScheduler(const char* moduleName) {
  module = dlopen(moduleName, RTLD_LAZY);

  if (!module) {
    InstanciationError exception(dlerror());
    throw exception;
  }
  dlerror();

  constructs = (constructor*) dlsym(module, "constructor");
  const char* error = dlerror();
  if (error) {
    InstanciationError exception(error);
    throw exception;
  }
  destroys = (destructor*) dlsym(module, "destructor");
  error = dlerror();
  if (error) {
    InstanciationError exception(error);
    throw exception;
  }
}

/** Needed for the compilation. */
UserScheduler::~UserScheduler() {
}

/** To get a new instance of the chosen scheduler class. */
GlobalScheduler* UserScheduler::instanciate(const char * moduleName) {
  return getInstance(moduleName)->constructs();
}

/** Calls the destructor of the loaded class. */
void UserScheduler::destroy(GlobalScheduler* scheduler) {
  getInstance(NULL)->destroys(dynamic_cast<UserScheduler*>(scheduler));
}

/** Deserialization: returns a new instance of the chosen scheduler class. */
GlobalScheduler*
UserScheduler::deserialize(const char* serializedScheduler,
                           const char* moduleName) {
  return getInstance(moduleName)->constructs();
}

char*
UserScheduler::serialize(GlobalScheduler* GS) {
  return strdup(stName);
}

/** Should never been called. Overridden in the user scheduler class. */
int
UserScheduler::aggregate(corba_response_t* aggrResp,
			 size_t max_srv,
			 const size_t nb_responses,
			 const corba_response_t* responses) {
  return GlobalScheduler::aggregate(aggrResp, max_srv, nb_responses, responses);
}

/* Usefull function to simplify the scheduler development. */

/* Utility function converting the responses given by the children to a STL
   list of servers. */
std::list<corba_server_estimation_t> CORBA_to_STL(const corba_response_t* responses,
                                                  int nb_responses) {
  std::list<corba_server_estimation_t> result;

  for (int i=0; i<nb_responses; ++i)
    for (unsigned int j=0; j<responses[i].servers.length(); ++j)
      result.push_back(responses[i].servers[j]);

  return result;
}

/* Utility function converting a STL list of servers to a CORBA sequence. */
void STL_to_CORBA(std::list<corba_server_estimation_t> &servers,
                  corba_response_t* &aggrResp) {
  std::list<corba_server_estimation_t>::iterator it;
  unsigned int i=0;

  aggrResp->servers.length(servers.size());
  for (it=servers.begin(); it!=servers.end(); ++it)
    aggrResp->servers[i++]=*it;
}

