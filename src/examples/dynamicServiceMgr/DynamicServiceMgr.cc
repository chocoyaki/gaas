/**
 * @file DynamicServiceMgr.cc
 *
 * @brief  DIET DynamicServiceMgr.
 *
 * @author  Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#include "DynamicServiceMgr.hh"
#ifndef __WIN32__
#include <dlfcn.h>
#endif
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

#ifndef __WIN32__
#include <dlfcn.h>
#define GetFunctionFromModule dlsym
#define CloseModule dlclose
#define LoadModule(value1,value2) (dlopen(value1,value2))
#define GetDLError dlerror
typedef void* LpHandleType;
#else /* __WIN32__ */
#include <windows.h>
#define GetFunctionFromModule GetProcAddress
#define LoadModule(value1,value2) (LoadLibrary(value1))
#define CloseModule FreeLibrary
const char* GetDLError(){

	const char* lpMsgBuf;
    DWORD dw = GetLastError();
	if(dw == 0) return NULL;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
	return lpMsgBuf;
}
typedef HINSTANCE LpHandleType;
#endif /* __WIN32__ */

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
    GetDLError();


    rmS = (removeService*) GetFunctionFromModule((LpHandleType)i_m->second, "removeService");
    error = GetDLError();
    if (error) {
      WARNING("Problem while searching for removeService");
    }

    rmS();
    CloseModule((LpHandleType)i_m->second);
  }

  this->services.clear();
}



int DynamicServiceMgr::addServiceMgr(const std::string & lib) {
  LpHandleType module = NULL;
  addService* addS;
  serviceName* serName;
  const char* error;
  std::string name;

  TRACE_TEXT(TRACE_ALL_STEPS, "*** Adding library '" << lib << "' ***" << std::endl);
  module = LoadModule(lib.c_str(), RTLD_NOW|RTLD_LOCAL);

  if (!module) {
    WARNING("Pb opening module");
    InstanciationError exception(GetDLError());
    throw exception;
  }

  GetDLError();

  serName = (serviceName*) GetFunctionFromModule(module, "serviceName");
  TRACE_TEXT(TRACE_ALL_STEPS, "*** Got serviceName() ***" << std::endl);

  error = GetDLError();
  if (error) {
    WARNING("Pb getting serviceName " << error);
    InstanciationError exception(error);
    throw exception;
  }

  name = serName();
  TRACE_TEXT(TRACE_ALL_STEPS, "*** Found service '" << name << "' ***"  << std::endl);

  if (this->services.find(name) != this->services.end()) {
    WARNING("*** Service '" << name << "' already exists ***");
    CloseModule(module);
    return -1;
  }

  this->services[name] = module;

  addS = (addService*) GetFunctionFromModule(module, "addService");
  error = GetDLError();
  if (error) {
    WARNING("Problem getting addService " << error);
    InstanciationError exception(error);
    throw exception;
  }

  return addS();
}


int DynamicServiceMgr::removeServiceMgr(const std::string & name) {
  LpHandleType module;
  removeService* rmS;
  const char* error;
  map_string_void_t::iterator i_m = this->services.find(name);

  if (i_m == this->services.end()) {
    WARNING("*** Service '" << name << "' does not exist! ***");
    return -1;
  }

  module = (LpHandleType)i_m->second;
  this->services.erase(i_m);
  TRACE_TEXT(TRACE_ALL_STEPS, "*** Will now remove a module ***"  << std::endl);

  if (!module) {
    WARNING("Pb opening module");
    InstanciationError exception(GetDLError());
    throw exception;
  }
  GetDLError();

  rmS = (removeService*) GetFunctionFromModule(module, "removeService");
  error = GetDLError();
  if (error) {
    WARNING("Pb getting removeService " << error);
    InstanciationError exception(error);
    throw exception;
  }
  return rmS();
}

