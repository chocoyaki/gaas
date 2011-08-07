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
 * Revision 1.13  2011/05/10 06:47:24  bdepardo
 * Properly handle exceptions when creating the DIETForwarder
 *
 * Revision 1.12  2011/04/20 09:00:09  bdepardo
 * Code formating
 *
 * Revision 1.11  2011/04/12 15:14:42  bdepardo
 * Command line option now has precedence on the automatically determined
 * remote port.
 *
 * Revision 1.10  2011/04/12 14:50:30  bdepardo
 * Try to automatically determine a free remote port
 *
 * Revision 1.9  2011/04/12 10:01:00  bdepardo
 * More robust forwarder: when communication to omniORB fails, correctly
 * exits and kill ssh tunnel
 *
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

#include "debug.hh"

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
  /* Optionnal, set to "localhost" by default. */
  opt.setOptCallback("--remote-host", remote_host);
  /* Optionnal, we try to determine it automatically. */
  opt.setOptCallback("--remote-port", remote_port_from);
  /* Optionnal, set waiting time for tunnel creation */
  opt.setOptCallback("--tunnel-wait", tunnel_wait);
  
  /* Optionnal.
   */
  opt.setOptCallback("--ssh-port", ssh_port);
  opt.setOptCallback("--ssh-login", ssh_login);
  opt.setOptCallback("--ssh-key", key_path);
	
  /* Optionnal parameters/flags. */
  opt.setOptCallback("--retry", nb_retry);
  opt.setOptCallback("--peer-ior", peer_ior);
  //  opt.setFlagCallback('C', create);  
  
  opt.processOptions();
  if (cfg.getSshHost()!="") {
    cfg.createTo(true);
    cfg.createFrom(true);
  }
	
  if (cfg.getName()=="") {
    std::ostringstream name;
    char host[256];
		
    gethostname(host, 256);
    host[255]='\0';
		
    std::transform(host, host+strlen(host), host, change);
    name << "Forwarder-" << host << "-" << getpid();
    WARNING("Missing parameter: name (use --name to fix it)" << endl
            << "Use default name: " << name.str() << endl);
    cfg.setName(name.str());
  }
  
  if (cfg.createFrom()) {
    if (cfg.getPeerName() == ""
        || cfg.getSshHost() == "") {
      ERROR("Missing parameter(s) to create tunnel."
            << " Mandatory parameters:" << endl
            << '\t' << "- Peer name (--peer-name <name>)" << endl
            << '\t' << "- SSH host (--ssh-host <host>)", EXIT_FAILURE);
    }
  }
	
  SSHTunnel tunnel;
  DIETForwarder* forwarder;
  try {
    forwarder = new DIETForwarder(cfg.getName());
  } catch (exception &e) {
    ERROR(e.what(), EXIT_FAILURE);
  }
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
      TRACE_TEXT(TRACE_MAIN_STEPS, "Error when binding the forwarder " << cfg.getName() << endl);
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
    WARNING("cannot open file " << iorFilename
            << " to store the IOR");
  } else {	
    TRACE_TEXT(TRACE_MAIN_STEPS, "Write IOR to " << iorFilename << endl);
    if (cfg.createFrom()) { // Creating tunnel(s)
      istringstream is(cfg.getRemotePortFrom());
      int port;
			
      is >> port;
      of << ORBMgr::convertIOR(ior, cfg.getRemoteHost(), port);
    }	else {// Waiting for connexion.
      of << ior;
    }
    of << std::endl;
    of << freeTCPport(); // also write a free port
    of.close();
  }
  TRACE_TEXT(TRACE_MAIN_STEPS, "Forwarder: " << ior << endl);
	
	
  tunnel.setSshHost(cfg.getSshHost());
  tunnel.setRemoteHost(cfg.getRemoteHost());
  
  tunnel.setSshPath(cfg.getSshPath());
  tunnel.setSshPort(cfg.getSshPort());
  tunnel.setSshLogin(cfg.getSshLogin());
  tunnel.setSshKeyPath(cfg.getSshKeyPath());
  
  tunnel.setWaitingTime(cfg.getWaitingTime());
	
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
        TRACE_TEXT(TRACE_MAIN_STEPS, "Got remote IOR file" << std::endl);
        cfg.setPeerIOR("/tmp/DIET-forwarder-ior-"+cfg.getPeerName()+".tmp");
      } else {
        ERROR("Could not get remote IOR file." << endl
              << "Please check that you can scp files between the ssh host and this host," << endl
              << "or specify the remote IOR with the following option:" << endl
              << "\t- Remote IOR (--peer-ior <IOR>)", EXIT_FAILURE);
      }
    } catch (...) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "Got an exception while retrieving IOR file" << std::endl);
    }
  }
  if (cfg.getPeerIOR()!="" && cfg.getPeerIOR().find("IOR:")!=0) {
    /* Extract the IOR from a file. */
    ifstream file(cfg.getPeerIOR().c_str());
    string peerIOR;
    string peerPort;
    if (!file.is_open()) {
      ERROR("Error: Invalid peer-ior parameter", EXIT_FAILURE);
    }
    file >> peerIOR;
    cfg.setPeerIOR(peerIOR);
    if (!file.eof() && cfg.getRemotePortFrom() == "") {
      file >> peerPort;
      cfg.setRemotePortFrom(peerPort);
    }
  }
	
  if (cfg.getPeerIOR() != "") {
    tunnel.setRemotePortTo(ORBMgr::getPort(cfg.getPeerIOR()));
  } else {
    tunnel.setRemotePortTo(cfg.getRemotePortTo());
  }
  if (cfg.getRemoteHost() == "auto") {
    if (cfg.getPeerIOR() != "") {
      tunnel.setRemoteHost(ORBMgr::getHost(cfg.getPeerIOR()));
    } else {
      tunnel.setRemoteHost("127.0.0.1");
    }
  } else {
    if (cfg.getRemoteHost() != "")
      tunnel.setRemoteHost(cfg.getRemoteHost());
    else
      tunnel.setRemoteHost("localhost");
  }
  
  tunnel.setRemotePortFrom(cfg.getRemotePortFrom());
  //	tunnel.setLocalPortFrom(cfg.getLocalPortFrom());
  if (cfg.createFrom()) {
    if (cfg.getRemotePortFrom() == "") {
      ERROR("Failed to automatically determine a remote free port." << endl
            << " You need to specify the remote port:" << endl
            << '\t' << "- Remote port (--remote-port <port>)", EXIT_FAILURE);
    }
  }
  
	
  tunnel.setLocalPortTo(ORBMgr::getPort(ior));
  
  if (cfg.getSshHost()!="") {
    tunnel.createTunnelTo(cfg.createTo());
    tunnel.createTunnelFrom(cfg.createFrom());
  }
  tunnel.open();
  
  /* Try to find the peer. */
  bool canLaunch = true;
  if (cfg.getPeerIOR()!="") {
    try {
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
          TRACE_TEXT(TRACE_MAIN_STEPS, "Unable to contact remote peer. Waiting for connection..." << endl);
        }
      }
    } catch (...) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "Error while connecting to remote peer" << endl);
      canLaunch = false;
    }
  }
  
  if (canLaunch) {
    mgr->wait();
  }
  
  TRACE_TEXT(TRACE_MAIN_STEPS, "Forwarder is now terminated" << std::endl);
	
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
    
    // Get the existing contexts except the Forwarders one
    list<string> contexts = mgr->contextList();
    contexts.remove(FWRDCTXT);
    // Get the other forwarder list
    list<string> fwds = forwarder->otherForwarders();
    string fwdName = forwarder->getName();
    string fwdTag = "@"+fwdName;
    
    // For each context
    for (list<string>::const_iterator it=contexts.begin();
         it!=contexts.end(); ++it)
    {
      // Get the local objects
      list<string> objects = mgr->localObjects(*it);
      // Get the object from other forwarders
      for (list<string>::const_iterator jt = fwds.begin();
           jt != fwds.end(); ++jt)
      {
        list<string> fwdObjects = mgr->forwarderObjects(*jt, *it);
        objects.insert(objects.end(), fwdObjects.begin(), fwdObjects.end());
      }
      
      // Bind them on the peer
      for (list<string>::const_iterator jt=objects.begin();
           jt!=objects.end(); ++jt)
      {
        string objName = *it+"/"+*jt;
        string ior = mgr->getIOR(*it, *jt);
        forwarder->bind(objName.c_str(), ior.c_str());
      }
      // Then, get the objects binded on the peer
      SeqString* remoteObjs = peer->getBindings(it->c_str());
      // And bind them locally and on others forwarders if they are not yet
      for (unsigned int i=0; i<remoteObjs->length(); i+=2) {
        string name((*remoteObjs)[i]);
        string ior((*remoteObjs)[i+1]);
        if (find(objects.begin(), objects.end(), name)!=objects.end()) {
          continue;
        }
        string newIOR = ORBMgr::convertIOR(ior, fwdTag, 0);
        mgr->bind(*it, name, newIOR, true);
        mgr->fwdsBind(*it, name, newIOR, fwdName);
      }
    }
  } catch (CORBA::TRANSIENT& err) {
    ERROR("Unable to contact remote peer using '" << newHost
          <<"' as a \"new remote host\"", 1);
  }
  
  TRACE_TEXT(TRACE_MAIN_STEPS, "Contacted remote peer using '" << newHost << "' as new remote host" << endl);
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

void tunnel_wait(const string& time, Configuration* cfg) {
  istringstream is(time);
  int n;
  is >> n;
  static_cast<FwrdConfig*>(cfg)->setWaitingTime(n);
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
  waitingTime = 0;
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
unsigned int  FwrdConfig::getWaitingTime() const {
  return waitingTime;
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
void FwrdConfig::setWaitingTime(const unsigned int time) {
  this->waitingTime = time;
}
void FwrdConfig::setCfgPath(const string& path) {
  this->cfgPath = path;
}

int change(int c) {
  if (c == '.') {
    return '-';
  }
  return c;
}

