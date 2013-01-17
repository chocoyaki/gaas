/*
 * SecurityManager.hh
 *
 *  Created on: 8 janv. 2013
 *      Author: gverger
 */

#ifndef SECURITYMANAGER_HH_
#define SECURITYMANAGER_HH_

#include "security_config.h"

#ifdef DIET_USE_SECURITY

#include <string>

class SecurityManager {
public:
	SecurityManager();
	virtual ~SecurityManager();

	bool enableSecureCommunicationsIfSet();

private:
	std::string cAFile;
	std::string keyFile;
	std::string passwordKey;
};

#endif /* DIET_USE_SECURITY */

#endif /* SECURITYMANAGER_HH_ */
