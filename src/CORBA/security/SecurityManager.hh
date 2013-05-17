/*
 * @file SecurityManager.hh
 *
 * @brief  Utility class for handling security for DIET
 *
 * @author   Guillaume Verger(guillaume.verger@inria.fr)
 */

#ifndef _SECURITYMANAGER_HH_
#define _SECURITYMANAGER_HH_

#include "security_config.h"

#ifdef DIET_USE_SECURITY

#include <string>
#include <vector>

class SecurityManager {
public:
  SecurityManager();

  virtual
  ~SecurityManager();

  bool
  enableSecurity(int argc, char * argv[]);

  std::vector<char *>
  getORBOptions() const {return secuOptions; }

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

#endif /* _SECURITYMANAGER_HH_ */
