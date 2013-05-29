/*
 * @file Address.hh
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _ADDRESS_HH_
#define _ADDRESS_HH_

#include <string>
#include <iostream>

namespace spy {

class Address {
public:
  Address(std::string theIp, std::string thePort);
  virtual ~Address();

  const std::string& getIp() const {
    return ip;
  }

  const std::string& getPort() const {
    return port;
  }

private:

  std::string ip;
  std::string port;


public:
  friend std::ostream& operator<< (std::ostream &out, Address &cAddress);
};

} /* namespace spy */

#endif /* _ADDRESS_HH_ */
