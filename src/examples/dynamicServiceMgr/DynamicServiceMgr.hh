/****************************************************************************/
/* DIET DynamicSeD header.                                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (benjamin.depardon@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2011/01/21 17:31:29  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 * The function 'InstanciationError::message' can be const
 *
 * Revision 1.1  2008/12/08 15:32:43  bdepardo
 * Added an example to dynamically load a service given a library:
 * the library is sent by the client, and the SeD loads it and uses the new
 * service(s) added.
 *
 *
 ****************************************************************************/

#ifndef _DYNAMICSED_HH_
#define _DYNAMICSED_HH_

#include <map>
#include <string>

#include "DIET_server.h"

/** Utility macro to simplify the user source code. */
#define DYNAMICSERVICEMGR_CLASS(T) \
  extern "C" DynamicServiceMgr* constructor() { \
    return new (T)(); \
  } \
  extern "C" void destructor(DynamicServiceMgr* ds) { \
    delete ds; \
  }


/** A simple exception when a module failed to be loaded. */
class InstanciationError
{
public:
  InstanciationError(const char* error):errorCode(error) {}
  InstanciationError(const InstanciationError &error) :
    errorCode(error.errorCode) {};
  const char* message() const { return errorCode; }
private:
  const char* errorCode;
};


typedef std::map<std::string, void*> map_string_void_t;

/** The DynamicServiceMgr class declaration. */
class DynamicServiceMgr
{
  typedef int service(diet_profile_t*);
  typedef int addService();
  typedef int removeService();
  typedef const char * const serviceName();

public:
  DynamicServiceMgr();
  ~DynamicServiceMgr();

  int addServiceMgr(const std::string & lib);
  int removeServiceMgr(const std::string & name);

private:
  /** These two methods are obtained from the loaded module. */
  map_string_void_t services;
};

#endif // _DYNAMICSED_HH_
