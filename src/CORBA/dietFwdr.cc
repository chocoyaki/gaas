/****************************************************************************/
/* DIET forwarder implementation - Forwarder executable                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2011/03/28 10:07:19  bdepardo
 * Use 127.0.0.1 as default remote host instead of localhost.
 *
 * Revision 1.7  2011/03/25 18:03:11  bdepardo
 * Catch exception while transfering remote IOR file
 *
 * Revision 1.6  2011/03/25 17:29:23  bdepardo
 * More robust forwarder
 *
 * Revision 1.5  2010/10/28 10:21:48  bdepardo
 * Set nbRetry by default to 3.
 * Code formating.
 *
 * Revision 1.4  2010/07/27 16:16:48  glemahec
 * Forwarders robustness
 *
 * Revision 1.3  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 * Revision 1.2  2010/07/13 15:24:13  glemahec
 * Warnings corrections and some robustness improvements
 *
 * Revision 1.1  2010/07/12 16:11:04  glemahec
 * DIET 2.5 beta 1 - New ORB manager; dietForwarder application
 ****************************************************************************/

#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include "SSHTunnel.hh"
#include "Options.hh"

#include "dietFwdr.hh"

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <algorithm>

#include <omniORB4/CORBA.h>

#include <unistd.h>	// For sleep function

using namespace std;

int main(int argc, char* argv[], char* envp[]) {
  /* Forwarder configuration. */
  FwrdConfig cfg(argv[0]);
	
  Options opt(&cfg, argc, argv, envp);
  /* Mandatory parameter. */
  opt.setOptCallback("--name", name);
	
  /* Mandatory when creating tunnels. */
  opt.setOptCallback("--peer-name", peer_name);
  opt.setOptCallback("--ssh-host", ssh_host);
  opt.setOptCallback("--remote-port", remote_port_from);
  /* Optionnal, set to "localhost" by default. */
  opt.setOptCallback("--remote-host", remote_host);

  /* Optionnal - default values are set to port 22,
   * current user login and $HOME/.ssh/id_[rsa|dsa].
   */
  opt.setOptCallback("--ssh-port", ssh_port);
  opt.setOptCallback("--ssh-login", ssh_login);
  opt.setOptCallback("--ssh-key", key_path);
	
  /* Optionnal parameters/flags. */
  opt.setOptCallback("--retry", nb_retry);
  opt.setOptCallback("--peer-ior", peer_ior);
  opt.setFlagCallback('C', create);
  //opt.setFlagCallback('f', create_from);
	
  opt.setOptCallback("--net-config", net_config);

  opt.processOptions();
	
  if (cfg.getName()=="") {
    std::ostringstream name;
    char host[256];
		
    gethostname(host, 256);
    host[255]='\0';
		
    std::transform(host, host+strlen(host), host, change);
    name << "Forwarder-" << host << "-" << getpid();
    cerr << "Missing parameter: name (use --name to fix it)" << endl;
    cerr << "Use default name: " << name.str() << endl;
    cfg.setName(name.str());
  }
  if (cfg.getCfgPath()=="") {
    cerr << "Missing parameter: net-config (use --net-config <file> to fix it)" << endl;
    return EXIT_FAILURE;
  }
	
  if (cfg.createFrom()) {
    if (cfg.getPeerName()==""       ||
	cfg.getSshHost()==""        ||
	cfg.getRemotePortFrom()=="")
      {
	cerr << "Missing parameter(s) to create tunnel.";
	cerr << " Mandatory parameters:" << endl;
	cerr << '\t' << "- Peer name (--peer-name <name>)" << endl;
	cerr << '\t' << "- SSH host (--ssh-host <host>)" << endl;
	cerr << '\t' << "- Remote port (--remote-port <port>)" << endl;
	return EXIT_FAILURE;
      }
  }
	
	
  SSHTunnel tunnel;
  DIETForwarder* forwarder = new DIETForwarder(cfg.getName(), cfg.getCfgPath());
  ORBMgr::init(argc, argv);
  ORBMgr* mgr = ORBMgr::getMgr();
  string ior;
  int count = 0;
	
  mgr->activate(forwarder);
  do {
    try {
      mgr->bind(FWRDCTXT, cfg.getName(), forwarder->_this(), true);
      break;
    } catch (CORBA::TRANSIENT& err) {
      cerr << "Error when binding the forwarder " << cfg.getName() << endl;
      if (count++<cfg.getNbRetry()) {
	sleep(5);
	continue;
      }
      mgr->deactivate(forwarder);
      return EXIT_FAILURE;
    }
  } while (true);
	
  /* Write the IOR to a file on /tmp. */
  ior = mgr->getIOR(forwarder->_this());
  string iorFilename("/tmp/DIET-forwarder-ior-");
  iorFilename+=cfg.getName()+".tmp";
  ofstream of(iorFilename.c_str(), ios_base::trunc);
	
  if (!of.is_open()) {
    cerr << "Warning: cannot open file " << iorFilename
	 << " to store the IOR" << endl;
  } else {	
    cout << "Write IOR to " << iorFilename << endl;
    if (cfg.createFrom()) { // Creating tunnel(s)
      istringstream is(cfg.getRemotePortFrom());
      int port;
			
      is >> port;
      of << ORBMgr::convertIOR(ior, cfg.getRemoteHost(), port);
    }	else // Waiting for connexion.
      of << ior;
    of.close();
  }
  cout << "Forwarder: " << ior << endl;
	
	
  tunnel.setSshHost(cfg.getSshHost());
  tunnel.setRemoteHost(cfg.getRemoteHost());
	
  tunnel.setRemotePortFrom(cfg.getRemotePortFrom());
  //	tunnel.setLocalPortFrom(cfg.getLocalPortFrom());
	
  tunnel.setSshPath(cfg.getSshPath());
  tunnel.setSshPort(cfg.getSshPort());
  tunnel.setSshLogin(cfg.getSshLogin());
  tunnel.setSshKeyPath(cfg.getSshKeyPath());
	
  /* Manage the peer IOR. */
  if (cfg.getPeerIOR()=="" && cfg.createFrom()) {
    /* Try to retrieve the peer IOR. */
    SSHCopy copy(cfg.getSshHost(),
		 "/tmp/DIET-forwarder-ior-"+cfg.getPeerName()+".tmp",
		 "/tmp/DIET-forwarder-ior-"+cfg.getPeerName()+".tmp");
    copy.setSshPath("/usr/bin/scp");
    copy.setSshPort(cfg.getSshPort());
    copy.setSshLogin(cfg.getSshLogin());
    copy.setSshKeyPath(cfg.getSshKeyPath());
    try {
      if (copy.getFile()) {
        std::cout << "Got remote IOR file" << std::endl;
        cfg.setPeerIOR("/tmp/DIET-forwarder-ior-"+cfg.getPeerName()+".tmp");
      }
    } catch (...) {
      std::cout << "Got an exception while retrieving IOR file" << std::endl;
    }
  }
  if (cfg.getPeerIOR()!="" && cfg.getPeerIOR().find("IOR:")!=0) {
    /* Extract the IOR from a file. */
    ifstream file(cfg.getPeerIOR().c_str());
    string peerIOR;
    if (!file.is_open()) {
      cerr << "Error: Invalid peer-ior parameter" << endl;
      return EXIT_FAILURE;
    }
    file >> peerIOR;
    cfg.setPeerIOR(peerIOR);
  }
	
  if (cfg.getPeerIOR()!="") {
    tunnel.setRemotePortTo(ORBMgr::getPort(cfg.getPeerIOR()));
  } else {
    tunnel.setRemotePortTo(cfg.getRemotePortTo());
  }
  if (cfg.getRemoteHost()=="") {
    if (cfg.getPeerIOR()!="")
      tunnel.setRemoteHost(ORBMgr::getHost(cfg.getPeerIOR()));
    else
      tunnel.setRemoteHost("127.0.0.1");
  } else {
    tunnel.setRemoteHost(cfg.getRemoteHost());
  }
	
  tunnel.setLocalPortTo(ORBMgr::getPort(ior));

  tunnel.createTunnelTo(cfg.createTo());
  tunnel.createTunnelFrom(cfg.createFrom());
	
  tunnel.open();

  /* Try to find the peer. */
  if (cfg.getPeerIOR()!="") {
    if (connectPeer(ior, cfg.getPeerIOR(), "localhost", tunnel.getRemoteHost(),
                    tunnel.getLocalPortFrom(), tunnel.getRemotePortFrom(), forwarder, mgr)) {
      /* In this case it seems that there is a problem with the alias 'localhost', thus we
       * try to use 127.0.0.1
       */
      if (tunnel.getRemoteHost() == "localhost") {
        tunnel.setRemoteHost("127.0.0.1");
      }
      if (connectPeer(ior, cfg.getPeerIOR(), "127.0.0.1", tunnel.getRemoteHost(),
                      tunnel.getLocalPortFrom(), tunnel.getRemotePortFrom(), forwarder, mgr)) {
        cout << "Unable to contact remote peer. Waiting for connection..." << endl;
      }
    }
  }

  mgr->wait();

  std::cout << "Forwarder is now terminated" << std::endl;
	
  return EXIT_SUCCESS;
}

int
connectPeer(const std::string &ior, const std::string &peerIOR,
            const std::string &newHost, const std::string &remoteHost,
            int localPortFrom, int remotePortFrom, DIETForwarder *forwarder, ORBMgr* mgr) {
  
  std::string newPeerIOR = ORBMgr::convertIOR(peerIOR, newHost, localPortFrom);

  Forwarder_var peer;
		
  peer = mgr->resolve<Forwarder, Forwarder_var>(newPeerIOR);
		
  try {
    peer->connectPeer(ior.c_str(), remoteHost.c_str(),
                      remotePortFrom);
    forwarder->setPeer(peer);
    SeqString* bindings = peer->getBindings(AGENTCTXT);
    for (unsigned int i=0; i<bindings->length(); ++i) {
      cout << "************ Dist bindings ****************" << endl;
      cout << (*bindings)[i] << endl;
    }
  } catch (CORBA::TRANSIENT& err) {
    cout << "Unable to contact remote peer using '" << newHost <<"' as a \"new remote host\"" << endl;
    return 1;
  }

  cout << "Contacted remote peer using '" << newHost << "' as new remote host" << endl;
  return 0;
}


void name(const string& name, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setName(name);
}

void peer_name(const string& name, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setPeerName(name);
}

void peer_ior(const string& ior, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setPeerIOR(ior);
}

void net_config(const string& path, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setCfgPath(path);
}

void ssh_host(const string& host, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setSshHost(host);
}

void remote_host(const string& host, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setRemoteHost(host);
}

void remote_port_to(const string& port, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setRemotePortTo(port);
}

void remote_port_from(const string& port, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setRemotePortFrom(port);
}

void local_port_from(const string& port, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setLocalPortFrom(port);
}

void ssh_path(const string& path, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setSshPath(path);
}

void ssh_port(const string& port, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setSshPort(port);
}

void ssh_login(const string& login, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setSshLogin(login);
}

void key_path(const string& path, Configuration* cfg) {
  static_cast<FwrdConfig*>(cfg)->setSshKeyPath(path);
}

void create(const string& create, Configuration* cfg) {
  (void) create;
  static_cast<FwrdConfig*>(cfg)->createTo(true);
  static_cast<FwrdConfig*>(cfg)->createFrom(true);
}

void nb_retry(const string& nb, Configuration* cfg) {
  istringstream is(nb);
  int n;
  is >> n;
  static_cast<FwrdConfig*>(cfg)->setNbRetry(n);
}
/* Fwdr configuration implementation. */
FwrdConfig::FwrdConfig(const string& pgName) : Configuration(pgName)
{
  createTunnelTo = false;
  createTunnelFrom = false;
  nbRetry = 3;
}

const string& FwrdConfig::getName() const {
  return name;
}
const string& FwrdConfig::getPeerName() const {
  return peerName;
}
const string& FwrdConfig::getPeerIOR() const {
  return peerIOR;
}

const string& FwrdConfig::getSshHost() const {
  return sshHost;
}
const string& FwrdConfig::getRemoteHost() const {
  return remoteHost;
}
const string& FwrdConfig::getRemotePortTo() const {
  return remotePortTo;
}
const string& FwrdConfig::getRemotePortFrom() const {
  return remotePortFrom;
}
const string& FwrdConfig::getLocalPortFrom() const {
  return localPortFrom;
}
bool  FwrdConfig::createTo() const {
  return createTunnelTo;
}
bool  FwrdConfig::createFrom() const {
  return createTunnelFrom;
}
const string& FwrdConfig::getSshPath() const {
  return sshPath;
}
const string& FwrdConfig::getSshPort() const {
  return sshPort;
}
const string& FwrdConfig::getSshLogin() const {
  return sshLogin;
}
const string& FwrdConfig::getSshKeyPath() const {
  return sshKeyPath;
}
int  FwrdConfig::getNbRetry() const {
  return nbRetry;
}
const string& FwrdConfig::getCfgPath() const {
  return cfgPath;
}

void FwrdConfig::setName(const string& name) {
  this->name = name;
}
void FwrdConfig::setPeerName(const string& name) {
  this->peerName = name;
}
void FwrdConfig::setPeerIOR(const string& ior) {
  this->peerIOR = ior;
}

void FwrdConfig::setSshHost(const string& host) {
  this->sshHost = host;
}
void FwrdConfig::setRemoteHost(const string& host) {
  this->remoteHost = host;
}
void FwrdConfig::setRemotePortTo(const string& port) {
  this->remotePortTo = port;
}
void FwrdConfig::setRemotePortFrom(const string& port) {
  this->remotePortFrom = port;
}
void FwrdConfig::setLocalPortFrom(const string& port) {
  this->localPortFrom = port;
}
void FwrdConfig::createTo(bool create) {
  this->createTunnelTo = create;
}
void FwrdConfig::createFrom(bool create) {
  this->createTunnelFrom = create;
}
void FwrdConfig::setSshPath(const string& path) {
  this->sshPath = path;
}
void FwrdConfig::setSshPort(const string& port) {
  this->sshPort = port;
}
void FwrdConfig::setSshLogin(const string& login) {
  this->sshLogin = login;
}
void FwrdConfig::setSshKeyPath(const string& path) {
  this->sshKeyPath = path;
}
void FwrdConfig::setNbRetry(const int nb) {
  this->nbRetry = nb;
}
void FwrdConfig::setCfgPath(const string& path) {
  this->cfgPath = path;
}

int change(int c) {
  if (c=='.') return '-';
  return c;
}

