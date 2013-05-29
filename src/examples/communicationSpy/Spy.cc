/*
 * @file Spy.cpp
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include "Spy.hh"

#include "Address.hh"

#include "debug.hh"
#include "DietLogComponent.hh"
#include "ORBMgr.hh"

#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>

#include <vector>
#include <algorithm> // for std::find
#include <boost/foreach.hpp>
#include <sstream>


char *
strToCharPtr(const std::string str) {
  char * result = new char[str.size()+1];
  strcpy(result, str.c_str());
  return result;

}


Spy::Spy(int argc, char **argv) {
  initORB(argc,argv);

  dietLogComponent =  new DietLogComponent("Spy", 0,0,0);
  ORBMgr::getMgr()->activate(dietLogComponent);

  updateSpiedComponents();
}

Spy::~Spy() {

}

void
Spy::listenToPort(int port) {
  ports.push_back(port);
}

void Spy::stopListeningPort(int port) {
  size_t idx = std::find(ports.begin(), ports.end(), port) - ports.begin();

  if (idx < ports.size()) {
    ports.erase(ports.begin() + idx);
  }
}

std::vector<spy::Address>
extractAddressList(omni::giopAddressList &list) {
  std::vector<spy::Address> addrs;
  omni::giopAddressList::const_iterator it = list.begin();
  omni::giopAddressList::const_iterator last = list.end();

  while (it != last) {

    ushort p ;
    char * host = omni::omniURI::extractHostPort((*it)->address()+9, p); // giop:tcp: or giop:scp: skipped

    std::ostringstream port;
    port << p;

    spy::Address a = spy::Address(std::string(host), port.str());
    addrs.push_back(a);

    TRACE_TEXT(TRACE_MAIN_STEPS, (*it)->address()<< " -> " << a << std::endl);
    it ++;
  }
  return addrs;
}


void Spy::spyOn(std::string & name) {
  spiedComponents.find(name);
  const bool isIn = spiedComponents.find(name) != spiedComponents.end();
  if (isIn) {
    return;
  }

  spiedComponents.insert(name);

  omniIOR* ior = NULL;
  char * cName = NULL;
  cName = strToCharPtr(name);
  try {

    Agent_var agentTmp = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, cName);
    ior= agentTmp->_getIOR();
  } catch (...) {
    SeD_var sedTmp = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, cName);
    ior = sedTmp->_getIOR();
  }

  delete cName;
  cName = NULL;


  if (!ior) {
    WARNING("cannot locate element " << name << std::endl);
  } else {
    std::cout << "Spying element : "<< name << std::endl;

   omniIOR::IORInfo* iorInfo = ior->getIORInfo();
    if (iorInfo != NULL) {
      IIOP::ProfileBody body;

        if (ior->iopProfiles().length() == 0) {
          return ;
        }

        IIOP::unmarshalProfile(ior->iopProfiles()[0], body);

        //Assume there is only one ssl port
        ushort sslPort;

        for (_CORBA_ULong i = 0; i < body.components.length(); ++i) {
          IOP::ComponentId tag = body.components.get_buffer()[i].tag ;
          if (tag == IOP::TAG_ALTERNATE_IIOP_ADDRESS) {
            char * addr = omniIOR::dump_TAG_ALTERNATE_IIOP_ADDRESS(body.components.get_buffer()[i]);
            std::cout << addr << std::endl;
          }
          if (tag == IOP::TAG_SSL_SEC_TRANS) {
            char * addr = omniIOR::dump_TAG_SSL_SEC_TRANS(body.components.get_buffer()[i]);
            std::string prefix = std::string("TAG_SSL_SEC_TRANS port = ");
            std::istringstream ss( addr+prefix.length());
            ss >> sslPort;

            std::cout << sslPort<< std::endl;
          }
        }



        omni::giopAddressList &list = iorInfo->addresses();
        std::vector<spy::Address> addresses = extractAddressList(list);

        std::ostringstream portS;
        portS << sslPort;

        uint nbA = addresses.size();
        for (uint i = 0; i < nbA; ++i) {
          spy::Address a(addresses[i].getIp(), portS.str());
          addresses.push_back(a);
        }

       BOOST_FOREACH(spy::Address a, addresses) {
         std::cout << a << std::endl;
       }

      }
  }
}


int Spy::initORB(int argc, char **argv) {
  try {
    ORBMgr::init(argc,argv);
  } catch (...) {
    ERROR_DEBUG("ORB initialization failed", 0);
  }
  return 0;
}

void Spy::updateSpiedComponents() {

  std::string contexts[] = { AGENTCTXT, SEDCTXT, CLIENTCTXT };
  BOOST_FOREACH(std::string ctx, contexts) {
    std::list<std::string> result = ORBMgr::getMgr()->localObjects(ctx);
    BOOST_FOREACH(std::string s, result) {
      spyOn(s);
    }
  }

}
