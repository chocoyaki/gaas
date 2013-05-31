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

//Packet handling
#include "types.h"
#include <pcap.h>
#include <pcap/sll.h>
#include <arpa/inet.h>

// DIET
#include "debug.hh"
#include "DietLogComponent.hh"
#include "ORBMgr.hh"

// CORBA
#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>

// Utils
#include <vector>
#include <algorithm> // for std::find
#include <boost/foreach.hpp>
#include <sstream>

// Singleton instance
Spy * Spy::instance = NULL;

char *
strToCharPtr(const std::string str) {
  char * result = new char[str.size() + 1];
  strcpy(result, str.c_str());
  return result;

}

Spy::Spy(int argc, char **argv) {
  initORB(argc, argv);

  dietLogComponent = new DietLogComponent("Spy", 0, 0, 0);
  ORBMgr::getMgr()->activate(dietLogComponent);

  updateSpiedComponents();
}

Spy::~Spy() {
  ORBMgr::kill();
}

std::vector<spy::Address> extractAddressList(omni::giopAddressList &list) {
  std::vector<spy::Address> addrs;
  omni::giopAddressList::const_iterator it = list.begin();
  omni::giopAddressList::const_iterator last = list.end();

  while (it != last) {

    ushort p;
    char * host = omni::omniURI::extractHostPort((*it)->address() + 9, p); // giop:tcp: or giop:scp: skipped

    spy::Address a = spy::Address(std::string(host), p);
    addrs.push_back(a);

    TRACE_TEXT(TRACE_MAIN_STEPS, (*it)->address()<< " -> " << a << std::endl);
    it++;
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

  IOP::IOR theIOR;
  omniIOR* ior = NULL;
  char * cName = NULL;
  cName = strToCharPtr(name);
  try {
    CORBA::Object_ptr o;

    ORBMgr::theMgr->getIOR(o);
    ORBMgr::theMgr->makeIOR(ORBMgr::theMgr->getIOR(o), theIOR);

    Agent_var agentTmp = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT,
                                                                     cName);
    ior = agentTmp->_getIOR();
  } catch (...) {
    SeD_var sedTmp = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, cName);
    ior = sedTmp->_getIOR();
  }

  delete[] cName;
  cName = NULL;

  if (!ior) {
    WARNING("cannot locate element " << name << std::endl);
  } else {
    std::cout << "Spying element : " << name << std::endl;

    omniIOR::IORInfo* iorInfo = ior->getIORInfo();
    if (iorInfo != NULL) {
      IIOP::ProfileBody body;



      if (ior->iopProfiles().length() == 0) {
        return;
      }

      IIOP::unmarshalProfile(ior->iopProfiles()[0], body);

      //Assume there is only one ssl port
      ushort sslPort;
      bool isSSL = false;

      for (_CORBA_ULong i = 0; i < body.components.length(); ++i) {
        IOP::ComponentId tag = body.components.get_buffer()[i].tag;
        if (tag == IOP::TAG_ALTERNATE_IIOP_ADDRESS) {
          char * addr = omniIOR::dump_TAG_ALTERNATE_IIOP_ADDRESS(
              body.components.get_buffer()[i]);
          std::cout << addr << std::endl;
        }
        if (tag == IOP::TAG_SSL_SEC_TRANS) {
          isSSL = true;
          char * addr = omniIOR::dump_TAG_SSL_SEC_TRANS(
              body.components.get_buffer()[i]);
          std::string prefix = std::string("TAG_SSL_SEC_TRANS port = ");
          std::istringstream ss(addr + prefix.length());
          ss >> sslPort;

          std::cout << sslPort << std::endl;
        }
      }

      omni::giopAddressList &list = iorInfo->addresses();
      std::vector<spy::Address>  addresses = extractAddressList(list);

      uint nbA = addresses.size();
      for (uint i = 0; i < nbA; ++i) {
        portOf.insert(std::make_pair(addresses[i].getPort(), name));
        if (isSSL) {

        spy::Address a(addresses[i].getIp(), sslPort);
        addresses.push_back(a);

          portOf.insert(std::make_pair(sslPort, name));
        }
      }

      BOOST_FOREACH(spy::Address a, addresses) {
        std::cout << a << std::endl;
      }
      watch[name] = addresses;
    }
  }
}

int Spy::initORB(int argc, char **argv) {
  try {
    ORBMgr::init(argc, argv);
  } catch (...) {
    ERROR_DEBUG("ORB initialization failed", 0);
  }
  return 0;
}

void analysePacket(u_char* args, const struct pcap_pkthdr* pkthdr,
    const u_char* packet) {

  const struct nread_ip* ip; /* packet structure         */
  const struct nread_tcp* tcp; /* tcp structure            */

  int len; /* length holder         */

  ip = (struct nread_ip*) (packet + sizeof(struct sll_header));
  tcp = (struct nread_tcp*) ((const u_char *) ip + IP_HL(ip) * 4);

  len = ntohs(ip->ip_len); /* get packet length */

  len -= IP_HL(ip) * 4;
  len -= tcp->th_off * 4;

  if (len > 0) {
    printf("\n");

    ushort srcPort = ntohs(tcp->th_sport);
    ushort dstPort = ntohs(tcp->th_dport);

    Spy * theSpy = Spy::getSpy();
    std::string src = "??";
    if (theSpy->isListeningToPort(srcPort)) {
      src = theSpy->isBindedToPort(srcPort);
    }

    std::string dst = "??";
    if (theSpy->isListeningToPort(dstPort)) {
      dst = theSpy->isBindedToPort(dstPort);
    }

    printf("%s --> %s : \n", src.c_str(), dst.c_str());
    const u_char * data = (const u_char *) tcp + tcp->th_off * 4;
    for (int i = 0; i < len; i++) {
      if (isprint(data[i])) /* Check if the packet data is printable */
        printf("%c", data[i]); /* Print it */
      else
        printf("."); /* If not print a . */
    }
  }

}

int Spy::run() {
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t* descr;
  struct bpf_program filter;

  /* open device for reading */
  descr = pcap_open_live("any", BUFSIZ, 0, -1, errbuf);
  if (descr == NULL) {
    printf("pcap_open_live(): %s\n", errbuf);
    exit(1);
  }

  const char * filter_str = createFilter().c_str();
  std::cout << filter_str << std::endl;
  if (pcap_compile(descr, &filter, filter_str, 1, PCAP_NETMASK_UNKNOWN) == -1) {
    fprintf(stderr, "Error compiling pcap\n");
    return (1);
  }

  if (pcap_setfilter(descr, &filter)) {
    fprintf(stderr, "Error setting pcap filter\n");
    return (1);
  }

  /* allright here we call pcap_loop(..) and pass in our callback function */
  /* int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)*/
  /* If you are wondering what the user argument is all about, so am I!!   */
  pcap_loop(descr, -1, analysePacket, NULL);

  fprintf(stdout, "\nDone processing packets... wheew!\n");
  return 0;
}

Spy *
Spy::getSpy() {
  if (!instance) {
    std::cerr << "Spy is not initialized ! Did you forget to call Spy::init()?"
        << std::endl;
  }
  return instance;
}

void Spy::init(int argc, char** argv) {
  if (!instance) {
    instance = new Spy(argc, argv);
  } else {
    std::cerr << "Spy is already initialized ! Multiple calls to Spy::init()."
        << std::endl;
  }
}

void Spy::kill() {
  if (instance) {
    delete instance;
    instance = NULL;
  }
}

std::string
Spy::isBindedToPort(ushort port) {
  return portOf[port];
}

bool Spy::isListeningToPort(ushort port) {
  return (portOf.find(port) != portOf.end());
}

void Spy::updateSpiedComponents() {

  std::string contexts[] = { AGENTCTXT, SEDCTXT, CLIENTCTXT };
  BOOST_FOREACH(std::string & ctx, contexts) {
    std::list<std::string> result = ORBMgr::getMgr()->localObjects(ctx);
    BOOST_FOREACH(std::string s, result) {
      spyOn(s);
    }
  }

}

std::string Spy::createFilter() {
  typedef std::map<std::string, std::vector<ushort> > mapAddressPorts;

  mapAddressPorts addresses;

  BOOST_FOREACH(mapAgentAddresses::value_type & p ,watch) {
    BOOST_FOREACH(spy::Address& a , p.second) {
      if (addresses.find(a.getIp()) == addresses.end()) {
        addresses[a.getIp()] = std::vector<ushort>();
      }
      addresses[a.getIp()].push_back(a.getPort());
    }
  }

  std::ostringstream b;
  b << "tcp and (";

  BOOST_FOREACH(mapAddressPorts::value_type & p, addresses ) {
    std::vector<ushort>::iterator portIt = p.second.begin();
    b << "(host " << p.first << " and (port " << (*portIt);
    ++portIt;
    while (portIt != p.second.end()) {
      b << " or port " << (*portIt);
      ++portIt;
    }
    b << "))";
  }
  b << ")";
  return b.str();
}
