/**
 * @file DynamicServiceMgr.hh
 *
 * @brief  DIET DynamicSeD header
 *
 * @author  Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _DYNAMICSED_HH_
#define _DYNAMICSED_HH_

#include <map>
#include <string>

#include "DIET_server.h"

/** Utility macro to simplify the user source code. */
#define DYNAMICSERVICEMGR_CLASS(T)                      \
  extern "C" DynamicServiceMgr * constructor() {         \
    return new(T) ();                                   \
  }                                                     \
  extern "C" void destructor(DynamicServiceMgr * ds) {   \
    delete ds;                                          \
  }


/** A simple exception when a module failed to be loaded. */
class InstanciationError {
public:
  explicit
  InstanciationError(const char *error)
    : errorCode(error) {
  }

  InstanciationError(const InstanciationError &error)
    : errorCode(error.errorCode) {
  }

  const char *
  message() const {
    return errorCode;
  }

private:
  const char *errorCode;
};


typedef std::map<std::string, void *> map_string_void_t;

/** The DynamicServiceMgr class declaration. */
class DynamicServiceMgr {
public:
  DynamicServiceMgr();

  ~DynamicServiceMgr();

  int
  addServiceMgr(const std::string &lib);

  int
  removeServiceMgr(const std::string &name);

private:
  typedef int service (diet_profile_t *);
  typedef int addService ();
  typedef int removeService ();
  typedef const char *const serviceName ();

  /** These two methods are obtained from the loaded module. */
  map_string_void_t services;
};

#endif  // _DYNAMICSED_HH_
