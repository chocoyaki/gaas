/****************************************************************************/
/* DIET DynamicServiceMgr.                                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (benjamin.depardon@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2011/01/23 19:27:47  bdepardo
 * Fixed: Throwing exception in destructor
 *
 * Revision 1.3  2011/01/21 17:31:29  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 * The function 'InstanciationError::message' can be const
 *
 * Revision 1.2  2010/03/03 14:05:16  bdepardo
 * Definition for CYGWIN compilation
 *
 * Revision 1.1  2008/12/08 15:32:43  bdepardo
 * Added an example to dynamically load a service given a library:
 * the library is sent by the client, and the SeD loads it and uses the new
 * service(s) added.
 *
 *
 ****************************************************************************/

#include "DynamicServiceMgr.hh"
#include <dlfcn.h>
#include "debug.hh"

#include <iostream>
/*
* When compiling the example on cygwin the variable RTLD_LOCAL
* was not declared and thus it is now setted to 0
*
* Perhaps the test would better be on the cygwin flag but
* this problem could also appear on other platforms
*
*/
#ifndef RTLD_LOCAL
#define RTLD_LOCAL 0
#endif

/** The default constructor. */
DynamicServiceMgr::DynamicServiceMgr() {
}


/** The default destructor. */
DynamicServiceMgr::~DynamicServiceMgr() {
  map_string_void_t::iterator i_m;
  removeService* rmS;
  const char* error;

  for (i_m = this->services.begin(); i_m != this->services.end(); ++ i_m) {
    if (!(i_m->second)) {
      WARNING("Problem while dealing with an \"opened\" module");
    }
    dlerror();


    rmS = (removeService*) dlsym(i_m->second, "removeService");
    error = dlerror();
    if (error) {
      WARNING("Problem while searching for removeService");
    }

    rmS();
    dlclose(i_m->second);
  }

  this->services.clear();
}



int DynamicServiceMgr::addServiceMgr(const std::string & lib) {
  void* module = NULL;
  addService* addS;
  serviceName* serName;
  const char* error;
  std::string name;

  TRACE_TEXT(TRACE_ALL_STEPS, "*** Adding library '" << lib << "' ***" << std::endl);
  module = dlopen(lib.c_str(), RTLD_NOW|RTLD_LOCAL);

  if (!module) {
    WARNING("Pb opening module");
    InstanciationError exception(dlerror());
    throw exception;
  }
  dlerror();

  serName = (serviceName*) dlsym(module, "serviceName");
  TRACE_TEXT(TRACE_ALL_STEPS, "*** Got serviceName() ***" << std::endl);

  error = dlerror();
  if (error) {
    WARNING("Pb getting serviceName " << error);
    InstanciationError exception(error);
    throw exception;
  }

  name = serName();
  TRACE_TEXT(TRACE_ALL_STEPS, "*** Found service '" << name << "' ***"  << std::endl);

  if (this->services.find(name) != this->services.end()) {
    WARNING("*** Service '" << name << "' already exists ***");
    dlclose(module);
    return -1;
  }

  this->services[name] = module;

  addS = (addService*) dlsym(module, "addService");
  error = dlerror();
  if (error) {
    WARNING("Problem getting addService " << error);
    InstanciationError exception(error);
    throw exception;
  }

  return addS();
}


int DynamicServiceMgr::removeServiceMgr(const std::string & name) {
  void* module;
  removeService* rmS;
  const char* error;
  map_string_void_t::iterator i_m = this->services.find(name);

  if (i_m == this->services.end()) {
    WARNING("*** Service '" << name << "' does not exist! ***");
    return -1;
  }

  module = i_m->second;
  this->services.erase(i_m);
  TRACE_TEXT(TRACE_ALL_STEPS, "*** Will now remove a module ***"  << std::endl);

  if (!module) {
    WARNING("Pb opening module");
    InstanciationError exception(dlerror());
    throw exception;
  }
  dlerror();

  rmS = (removeService*) dlsym(module, "removeService");
  error = dlerror();
  if (error) {
    WARNING("Pb getting removeService " << error);
    InstanciationError exception(error);
    throw exception;
  }
  return rmS();
}



