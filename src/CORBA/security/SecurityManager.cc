/*
 * SecurityManager.cpp
 *
 *  Created on: 8 janv. 2013
 *      Author: gverger
 */
#include "security_config.h"

#ifdef DIET_USE_SECURITY
#include "SecurityManager.hh"
#include "configuration.hh"



#include <omniORB4/CORBA.h>
#include <omniORB4/sslContext.h> // include <omniORB4/CORBA.h> before this one
#include <string>
#include <iostream>

using namespace std;

SecurityManager::SecurityManager() {
	this->cAFile = "UNDEFINED";
	this->keyFile = "UNDEFINED";
	this->passwordKey = "";
	CONFIG_STRING(diet::SSLROOTCERTIFICATE, this->cAFile);
	CONFIG_STRING(diet::SSLPRIVATEKEY, this->keyFile);
	CONFIG_STRING(diet::SSLPRIVATEKEYPASSWORD, this->passwordKey);
}

SecurityManager::~SecurityManager() {
}

bool SecurityManager::enableSecureCommunicationsIfSet() {
// TODO Add configuration test to see if secure com is needed
	sslContext::certificate_authority_file = cAFile.c_str();
	sslContext::key_file = keyFile.c_str();
	sslContext::key_file_password = passwordKey.c_str();

	sslContext::verify_mode = SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

	cout << "Security enabled : " << sslContext::certificate_authority_file << endl;
	return true;
}
#endif
