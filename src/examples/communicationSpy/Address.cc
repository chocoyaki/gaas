/*
 * @file Address.cc
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include "Address.hh"

namespace spy {

Address::Address(std::string theIp, ushort thePort) :
  ip(theIp), port(thePort)
{
}

Address::~Address() {
  // TODO Auto-generated destructor stub
}

std::ostream&
operator<<(std::ostream& out,   Address& cAddress) {
  out << cAddress.ip << "[" << cAddress.port << "]";
    return out;
}

} /* namespace spy */
