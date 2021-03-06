/*
 * @file CloudAPIConnection.hh
 *
 * @brief   Contains informations to log to a cloud
 *
 * @author  Lamiel Toch (lamiel.toch@ens-lyon.fr)
 *          Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _CLOUDAPICONNECTION_HH_
#define _CLOUDAPICONNECTION_HH_

#include <string>

class CloudAPIConnection{
public:
  CloudAPIConnection(const std::string& url, const std::string& usr, const std::string& passwd):
    username(usr), password(passwd), base_url(url) {
  }

  CloudAPIConnection() {};
  ~CloudAPIConnection(){};

  CloudAPIConnection(const CloudAPIConnection& api):
    username(api.username), password(api.password), base_url(api.base_url) {
  }

  std::string username; // eg : "oneadmin";
  std::string password; // eg : "mypassword";
  std::string base_url; // eg : "http://localhost:3001/api";
} ;

#endif /* _CLOUDAPICONNECTION_HH_ */
