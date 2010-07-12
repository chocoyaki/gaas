/****************************************************************************/
/* DIET forwarder implementation - Network configuration                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ */
/* $Log$
/* Revision 1.1  2010/07/12 16:11:03  glemahec
/* DIET 2.5 beta 1 - New ORB manager; dietForwarder application
/* */

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
	if (hp==NULL) return;

	for (it=hp->h_aliases; *it!=NULL; ++it) {
		string hostname = *it;
		size_t sp=0;
		while ((sp=hostname.find('.', sp))!=string::npos) {
			hostname.insert(sp,"\\");
			sp+=2;
		}
		l.push_back(hostname);
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
		if ((pos = line.find('#'))!=string::npos)
			line = line.substr(0, pos);
		/* Remove blank characters. */
		while ((pos = line.find(' '))!=string::npos) line.erase(pos);
		while ((pos = line.find('\t'))!=string::npos) line.erase(pos);
		/* Void line. */
		if (line=="") continue;
		/* Manage accepted networks. */
		if ((pos = line.find("accept:"))==0) {
			string network = line.substr(7);
			if (network=="localhost") {
				addLocalHost(accept);
			} else {
				addAcceptNetwork(network);
			}
		}
		if ((pos = line.find("reject:"))==0) {
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
	if (!match(hostname, accept)) return false;
	if (match(hostname, reject)) return false;
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