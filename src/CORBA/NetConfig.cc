/****************************************************************************/
/* DIET forwarder implementation - Network configuration                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2011/04/07 17:07:25  bdepardo
 * Really add all local addresses when using "localhost" in the configuration
 * file.
 * Currently, this code has only been tested on Linux and Mac OS X, but it
 * should also work on freebsd.
 * Still need to test on AIX, Cygwin and Windows.
 *
 * Revision 1.7  2011/02/24 16:55:41  bdepardo
 * Code cleanup.
 * Use TRACE_TEXT instead of cout
 *
 * Revision 1.6  2011/02/16 18:54:02  bdepardo
 * Accept both the old and the new configuration file format.
 *
 * Revision 1.5  2011/02/16 18:09:18  bdepardo
 * Configuration files of forwarders now use "accept = " and "reject = ".
 * This is a syntax similar to the one used in the MA, SeD and client
 * configuration files.
 *
 * Revision 1.4  2010/07/27 10:24:32  glemahec
 * Improve robustness & general performance
 *
 * Revision 1.3  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 * Revision 1.2  2010/07/13 15:24:13  glemahec
 * Warnings corrections and some robustness improvements
 *
 * Revision 1.1  2010/07/12 16:11:03  glemahec
 * DIET 2.5 beta 1 - New ORB manager; dietForwarder application
 ****************************************************************************/

#include "NetConfig.hh"

#include <string>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <regex.h>
#include <unistd.h> // For gethostname()
#include <netdb.h> // For gethostent()
/* For local address management. */
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/* For netmask retrieving. */
#include <netinet/in.h>


#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <cstring>


#if defined __darwin__ || defined __freebsd__
// required for getifaddrs
#include <ifaddrs.h>
#endif

//#include "debug.hh"

#ifndef HOST_NAME_MAX
#ifdef APPLE
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#else
#define HOST_NAME_MAX 255
#endif
#endif

using namespace std;

NetConfig::NetConfig() {
  char hostname[HOST_NAME_MAX+1];
	
  gethostname(hostname, HOST_NAME_MAX);
  hostname[HOST_NAME_MAX]='\0';
  myHostname = hostname;
}

NetConfig::NetConfig(const list<string>& accept,
                     const list<string>& reject,
                     const string& hostname)
{
  this->accept = accept;
  this->reject = reject;
  this->myHostname = hostname;
}

NetConfig::NetConfig(const string& filePath) {
  char hostname[HOST_NAME_MAX+1];
	
  gethostname(hostname, HOST_NAME_MAX);
  hostname[HOST_NAME_MAX]='\0';
  myHostname = hostname;
	
  this->filePath = filePath;
  parseFile();
}

NetConfig::NetConfig(const NetConfig& cfg) {
  operator=(cfg);
}

NetConfig& NetConfig::operator=(const NetConfig& cfg) {
  filePath = cfg.filePath;
  accept = cfg.accept;
  reject = cfg.reject;
  myHostname = cfg.myHostname;
  return *this;
}

void NetConfig::addLocalHost(std::list<string>& l) const {
  struct hostent* hp;
  char** it;
  char buffer[INET6_ADDRSTRLEN+1];
  string hostname = myHostname;
  
  size_t sp=0;
  while ((sp=hostname.find('.', sp))!=string::npos) {
    hostname.insert(sp,"\\");
    sp+=2;
  }
  
  l.push_back(myHostname);
  l.push_back("localhost");
  l.push_back("127\\.0\\.0\\.1");
  
  hp = gethostbyname(myHostname.c_str());
  if (hp != NULL) {
    for (it=hp->h_aliases; *it!=NULL; ++it) {
      string hostname = *it;
      size_t sp=0;
      while ((sp=hostname.find('.', sp))!=string::npos) {
	hostname.insert(sp,"\\");
	sp+=2;
      }
      l.push_back(hostname);
      cout << "## " << hostname << endl;
    }
    
    for (it=hp->h_addr_list; *it!=NULL; ++it) {
      if (inet_ntop(hp->h_addrtype, *it, buffer, INET6_ADDRSTRLEN)!=NULL) {
	string hostname = buffer;
	size_t sp=0;
	while ((sp=hostname.find('.', sp))!=string::npos) {
	  hostname.insert(sp,"\\");
	  sp+=2;
	}
	l.push_back(hostname);
      }
    }
  }
  

#ifdef __linux__
  /* FIXME: Dirty hack for getting all IP addresses of the machine
   * Currently this has only been tested on linux
   * TODO: test on other systems
   * TODO: add ipv6 addresses, currently only ipv4 addresses are handled
   */
  int sock;
  static struct ifreq ifreqs[100]; // 100 should be enough
  struct ifconf ifconf;
  ifconf.ifc_buf = (char*) (ifreqs);
  ifconf.ifc_len = sizeof(ifreqs);
  
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock >= 0) {
    ioctl(sock, SIOCGIFCONF, (char*) &ifconf);
    
    unsigned int nifaces =  ifconf.ifc_len / sizeof(struct ifreq);
    for (unsigned int i = 0; i < nifaces; ++ i) {
      string eth = ifreqs[i].ifr_name;
      struct ifreq ifr;
      int sock2 = socket(AF_INET, SOCK_DGRAM, 0);
      if (sock2 >= 0) {
	strncpy(ifr.ifr_name, eth.c_str(), IFNAMSIZ-1);
	ioctl(sock2, SIOCGIFADDR, &ifr);
	
	/* Get IP, and insert backslashes before '.' */
	hostname = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
	size_t sp=0;
	while ((sp=hostname.find('.', sp))!=string::npos) {
	  hostname.insert(sp,"\\");
	  sp+=2;
	}
	
	/* Only add addresses that are not yet in the list */
	if (find(l.begin(), l.end(), hostname) == l.end()) {
	  l.push_back(hostname);
	}
      } // end if (sock2 >= 0)
      close(sock2);
    } // end for (...)
  } // end if (sock >= 0)
  close(sock);

#elif defined __darwin__ || defined __freebsd__
  /* Works only if getifaddrs exists */
  struct ifaddrs *if_addrs = NULL;
  struct ifaddrs *if_addr = NULL;
  void *tmp = NULL;
  char buf[INET6_ADDRSTRLEN];
  if (0 == getifaddrs(&if_addrs)) {    
    for (if_addr = if_addrs; if_addr != NULL; if_addr = if_addr->ifa_next) {
      // Address
      if (if_addr->ifa_addr->sa_family == AF_INET) {
        tmp = &((struct sockaddr_in *)if_addr->ifa_addr)->sin_addr;
      } else {
        tmp = &((struct sockaddr_in6 *)if_addr->ifa_addr)->sin6_addr;
      }

      /* Get IP, and insert backslashes before '.' */
      const char * hostname_c = inet_ntop(if_addr->ifa_addr->sa_family,
                                          tmp, buf, sizeof(buf));
      if (hostname_c) {
        hostname = hostname_c;
        size_t sp = 0;
        while ((sp=hostname.find('.', sp))!=string::npos) {
          hostname.insert(sp,"\\");
          sp+=2;
        }
        
        /* Only add addresses that are not yet in the list */
        if (find(l.begin(), l.end(), hostname) == l.end()) {
          l.push_back(hostname);
        }
      }
    }
    freeifaddrs(if_addrs);
  } // end: if (0 == getifaddrs(&if_addrs))
#endif // __linux__

}

void NetConfig::parseFile() {
  ifstream file(filePath.c_str());
	
  if (!file.is_open())
    throw runtime_error("Unable to open "+filePath);
  while (!file.eof()) {
    char buffer[1024];
    size_t pos;
    file.getline(buffer, 1024);
    string line(buffer);
		
    /* Remove comments. */
    if ((pos = line.find('#')) != string::npos)
      line = line.substr(0, pos);

    /* Remove blank characters. */
    if ((pos = line.find_last_not_of(' ')) != string::npos) {
      line.erase(pos+1);
    }
    if ((pos = line.find_last_not_of('\t')) != string::npos) {
      line.erase(pos+1);
    }

    /* Void line. */
    if (line == "") {
      continue;
    }

    /* Manage accepted networks. */
    if (line.find("accept = ") == 0) {
      string network = line.substr(10);
      if (network=="localhost") {
        addLocalHost(accept);
      } else {
        addAcceptNetwork(network);
      }
    }
    if (line.find("accept:") == 0) {
      string network = line.substr(7);
      if (network=="localhost") {
        addLocalHost(accept);
      } else {
        addAcceptNetwork(network);
      }
    }
    if (line.find("reject = ") == 0) {
      string network = line.substr(10);
      if (network=="localhost") {
        addLocalHost(reject);
      } else {
        addRejectNetwork(network);
      }
    }
    if (line.find("reject:") == 0) {
      string network = line.substr(7);
      if (network=="localhost") {
        addLocalHost(reject);
      } else {
        addRejectNetwork(network);
      }
    }
  }
}

void NetConfig::reset() {
  accept.clear();
  reject.clear();
}

void NetConfig::addAcceptNetwork(const std::string& pattern) {
  accept.push_back(pattern);
}

void NetConfig::remAcceptNetwork(const std::string& pattern) {
  accept.remove(pattern);
}

void NetConfig::addRejectNetwork(const std::string& pattern) {
  reject.push_back(pattern);
}

void NetConfig::remRejectNetwork(const std::string& pattern) {
  reject.remove(pattern);
}


bool match(const string& hostname, const list<string> l) {
  list<string>::const_iterator it;
	
  for (it=l.begin(); it!=l.end(); ++it) {
    regex_t reg;
    if (regcomp(&reg, it->c_str(), REG_EXTENDED | REG_ICASE)) {
      regfree(&reg);
      continue;
    }
    if (!regexec(&reg, hostname.c_str(), 0, NULL, 0)) {
      regfree(&reg);
      return true;
    }
    regfree(&reg);
  }
  return false;
}

bool NetConfig::manage(const std::string& hostname) const {
  if (!match(hostname, accept)) {
    //cout << "The hostname " << hostname << " does not match "
    //     << "any element of the accept list" << endl;
    return false;
  }
  if (match(hostname, reject)) {
    //cout << "The hostname " << hostname << "matches "
    //		 << "an element of the reject list" << endl;
    return false;
  }
  return true;
}

void NetConfig::updateConfig() {
  reset();
  parseFile();
}

const list<string>& NetConfig::getAcceptList() const {
  return accept;
}

const list<string>& NetConfig::getRejectList() const {
  return reject;
}

/*int main(int argc, char* argv[]) {
  if (argc<2) {
  cerr << "Usage: " << argv[0] << " <cfg file>" << endl;
  return EXIT_FAILURE;
  }
  NetConfig net(argv[1]);
	
  cout << "reach(localhost): " << net.canReach("localhost") << endl;
  cout << "reach(192.168.0.1): " << net.canReach("192.168.0.1") << endl;
  cout << "reach(graal.ens-lyon.fr): " << net.canReach("graal.ens-lyon.fr") << endl;
  cout << "reach(capricorne-25.lyon.grid5000.fr): " << net.canReach("capricorne-25.lyon.grid5000.fr") << endl;
  cout << "reach(dhcp-34-29.incubateur.ens-lyon.fr): " << net.canReach("dhcp-34-29.incubateur.ens-lyon.fr") << endl;
  cout << "reach(127.0.0.1): " << net.canReach("127.0.0.1") << endl;
  cout << "reach(140.77.34.29): " << net.canReach("140.77.34.29") << endl;

  }
*/
