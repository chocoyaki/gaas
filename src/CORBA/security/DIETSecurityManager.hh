/*
 * @file SecurityManager.hh
 *
 * @brief  Utility class for handling security for DIET
 *
 * @author   Guillaume Verger(guillaume.verger@inria.fr)
 */

#ifndef _DIETSECURITYMANAGER_HH_
#define _DIETSECURITYMANAGER_HH_

#include "security_config.h"

#ifdef DIET_USE_SECURITY

#include <string>
#include <vector>

class DIETSecurityManager {
public:
  DIETSecurityManager();

  virtual
  ~DIETSecurityManager();

  bool
  enableSecurity(int argc, char * argv[]);

  const std::vector<char *> &
  getORBOptions() const {return secuOptions; }

  bool
  isEnabled() const { return enabled; }

private:
  bool enabled;

  std::string cAFile;
  std::string keyFile;
  std::string passwordKey;

  std::vector<char *> secuOptions;

  bool
  initSSLContext();

  bool
  secureORBOptions(int argc, char * argv[]);
};


#endif /* DIET_USE_SECURITY */

#endif /* _DIETSECURITYMANAGER_HH_ */
