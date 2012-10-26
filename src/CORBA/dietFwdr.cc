/**
 * @file dietFwdr.cc
 *
 * @brief   DIET forwarder implementation - Forwarder executable
 *
 * @author  Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

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

#include "OSIndependance.hh"     // For sleep function
#include "DIET_uuid.hh"

#ifdef __WIN32__
#define sleep(value) (Sleep(value*1000))
#endif
int
main(int argc, char *argv[], char *envp[]) {
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

  /* Optionnal */
  opt.setOptCallback("--ssh-port", ssh_port);
  opt.setOptCallback("--ssh-login", ssh_login);
  opt.setOptCallback("--ssh-key", key_path);

  /* Optionnal parameters/flags. */
  opt.setOptCallback("--nb-retry", nb_retry);
  opt.setOptCallback("--peer-ior", peer_ior);

  opt.processOptions();
  if (cfg.getSshHost() != "") {
    cfg.createTo(true);
    cfg.createFrom(true);
  }

  if (cfg.getName() == "") {
    std::ostringstream name;
    char host[256];
    boost::uuids::uuid uuid = diet_generate_uuid();

    gethostname(host, 256);
    host[255] = '\0';

    std::transform(host, host + strlen(host), host, change);
    name << "Forwarder-" << host << "-" << uuid;
    WARNING(
      "Missing parameter: name (use --name to fix it)\n"
      << "Use default name: " << name.str() << "\"n");
    cfg.setName(name.str());
  }

  if (cfg.createFrom()) {
    if (cfg.getPeerName() == ""
        || cfg.getSshHost() == "") {
      ERROR_DEBUG("Missing parameter(s) to create tunnel."
            << " Mandatory parameters:\n"
            << '\t' << "- Peer name (--peer-name <name>)\n"
            << '\t' << "- SSH host (--ssh-host <host>)", EXIT_FAILURE);
    }
  }

  SSHTunnel tunnel;
  DIETForwarder *forwarder;
  try {
    forwarder = new DIETForwarder(cfg.getName());
  } catch (std::exception &e) {
    ERROR_DEBUG(e.what(), EXIT_FAILURE);
  }
  ORBMgr::init(argc, argv);
  ORBMgr *mgr = ORBMgr::getMgr();
  std::string ior;
  int count = 0;

  mgr->activate(forwarder);
  do {
    try {
      mgr->bind(FWRDCTXT, cfg.getName(), forwarder->_this(), true);
      break;
    } catch (CORBA::TRANSIENT &err) {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "Error when binding the forwarder "
                 << cfg.getName() << std::endl);
      if (count++ < cfg.getNbRetry()) {
        sleep(5);
        continue;
      }
      mgr->deactivate(forwarder);
      return EXIT_FAILURE;
    }
  } while (true);

  /* Write the IOR to a file on /tmp. */
  ior = mgr->getIOR(forwarder->_this());
  std::string iorFilename("/tmp/DIET-forwarder-ior-");
  iorFilename += cfg.getName() + ".tmp";
  std::ofstream of(iorFilename.c_str(), std::ios_base::trunc);

  if (!of.is_open()) {
    WARNING("cannot open file " << iorFilename
                                << " to store the IOR");
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS, "Write IOR to " << iorFilename << std::endl);
    if (cfg.createFrom()) {  // Creating tunnel(s)
      std::istringstream is(cfg.getRemotePortFrom());
      int port;

      is >> port;
      of << ORBMgr::convertIOR(ior, cfg.getRemoteHost(), port);
    } else {  // Waiting for connexion.
      of << ior;
    }
    of << std::endl;
    of << freeTCPport();  // also write a free port
    of.close();
  }
  TRACE_TEXT(TRACE_MAIN_STEPS, "Forwarder: " << ior << std::endl);

  tunnel.setSshHost(cfg.getSshHost());
  tunnel.setRemoteHost(cfg.getRemoteHost());

  tunnel.setSshPath(cfg.getSshPath());
  tunnel.setSshPort(cfg.getSshPort());
  tunnel.setSshLogin(cfg.getSshLogin());
  tunnel.setSshKeyPath(cfg.getSshKeyPath());

  tunnel.setWaitingTime(cfg.getWaitingTime());

  /* Manage the peer IOR. */
  if (cfg.getPeerIOR().empty() && cfg.createFrom()) {
    /* Try to retrieve the peer IOR. */
    SSHCopy copy(cfg.getSshHost(),
                 "/tmp/DIET-forwarder-ior-" + cfg.getPeerName() + ".tmp",
                 "/tmp/DIET-forwarder-ior-" + cfg.getPeerName() + ".tmp");
    copy.setSshPath("/usr/bin/scp");
    copy.setSshPort(cfg.getSshPort());
    copy.setSshLogin(cfg.getSshLogin());
    copy.setSshKeyPath(cfg.getSshKeyPath());
    try {
      if (copy.getFile()) {
        TRACE_TEXT(TRACE_MAIN_STEPS, "Got remote IOR file" << std::endl);
        cfg.setPeerIOR("/tmp/DIET-forwarder-ior-" + cfg.getPeerName() + ".tmp");
      } else {
        ERROR_DEBUG("Could not get remote IOR file.\n"
              << "Please check that you can scp files"
              << "between the ssh host and this host, _n"
              << "or specify the remote IOR with the following option:\n"
              << "\t- Remote IOR (--peer-ior <IOR>)", EXIT_FAILURE);
      }
    } catch (...) {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "Got an exception while retrieving IOR file" << std::endl);
    }
  }
  if (cfg.getPeerIOR() != "" && cfg.getPeerIOR().find("IOR:") != 0) {
    /* Extract the IOR from a file. */
    std::ifstream file(cfg.getPeerIOR().c_str());
    std::string peerIOR;
    std::string peerPort;
    if (!file.is_open()) {
      ERROR_DEBUG("Error: Invalid peer-ior parameter", EXIT_FAILURE);
    }
    file >> peerIOR;
    cfg.setPeerIOR(peerIOR);
    if (!file.eof() && (cfg.getRemotePortFrom() == "")) {
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
    if (cfg.getRemoteHost() != "") {
      tunnel.setRemoteHost(cfg.getRemoteHost());
    } else {
      tunnel.setRemoteHost("localhost");
    }
  }

  tunnel.setRemotePortFrom(cfg.getRemotePortFrom());
  // tunnel.setLocalPortFrom(cfg.getLocalPortFrom());
  if (cfg.createFrom()) {
    if (cfg.getRemotePortFrom() == "") {
      ERROR_DEBUG("Failed to automatically determine a remote free port.\n"
            << " You need to specify the remote port:\n"
            << '\t' << "- Remote port (--remote-port <port>)", EXIT_FAILURE);
    }
  }


  tunnel.setLocalPortTo(ORBMgr::getPort(ior));

  if (cfg.getSshHost() != "") {
    tunnel.createTunnelTo(cfg.createTo());
    tunnel.createTunnelFrom(cfg.createFrom());
  }
  tunnel.open();

  /* Try to find the peer. */
  bool canLaunch = true;
  if (cfg.getPeerIOR() != "") {
    try {
      if (connectPeer(ior, cfg.getPeerIOR(),
                      "localhost", tunnel.getRemoteHost(),
                      tunnel.getLocalPortFrom(), tunnel.getRemotePortFrom(),
                      forwarder, mgr)) {
        /* In this case it seems that there is a problem with
         * the alias 'localhost', thus try to use 127.0.0.1
         */
        if (tunnel.getRemoteHost() == "localhost") {
          tunnel.setRemoteHost("127.0.0.1");
        }
        if (connectPeer(ior, cfg.getPeerIOR(),
                        "127.0.0.1", tunnel.getRemoteHost(),
                        tunnel.getLocalPortFrom(), tunnel.getRemotePortFrom(),
                        forwarder, mgr)) {
          TRACE_TEXT(TRACE_MAIN_STEPS, "Unable to contact remote peer."
                     << "Waiting for connection...\n");
        }
      }
    } catch (...) {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "Error while connecting to remote peer\n");
      canLaunch = false;
    }
  }

  if (canLaunch) {
    try {
      ORBMgr::getMgr()->wait();
      mgr->unbind(FWRDCTXT, cfg.getName());
    } catch (...) {
      WARNING("Error while exiting the ORBMgr::wait() function");
    }
  }

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically
   */
  delete ORBMgr::getMgr();

  TRACE_TEXT(TRACE_MAIN_STEPS, "Forwarder is now terminated" << std::endl);

  return EXIT_SUCCESS;
} // main

int
connectPeer(const std::string &ior, const std::string &peerIOR,
            const std::string &newHost, const std::string &remoteHost,
            int localPortFrom, int remotePortFrom,
            DIETForwarder *forwarder, ORBMgr *mgr) {
  std::string newPeerIOR = ORBMgr::convertIOR(peerIOR, newHost, localPortFrom);

  Forwarder_var peer;

  peer = mgr->resolve<Forwarder, Forwarder_var>(newPeerIOR);

  try {
    peer->connectPeer(ior.c_str(), remoteHost.c_str(), remotePortFrom);
    forwarder->setPeer(peer);

    // Get the existing contexts except the Forwarders one
    std::list<std::string> contexts = mgr->contextList();
    contexts.remove(FWRDCTXT);
    // Get the other forwarder list
    std::list<std::string> fwds = forwarder->otherForwarders();
    std::string fwdName = forwarder->getName();
    std::string fwdTag = "@" + fwdName;

    // For each context
    for (std::list<std::string>::const_iterator it = contexts.begin();
         it != contexts.end(); ++it) {
      // Get the local objects
      std::list<std::string> objects = mgr->localObjects(*it);
      // Get the object from other forwarders
      for (std::list<std::string>::const_iterator jt = fwds.begin();
           jt != fwds.end(); ++jt) {
        std::list<std::string> fwdObjects = mgr->forwarderObjects(*jt, *it);
        objects.insert(objects.end(), fwdObjects.begin(), fwdObjects.end());
      }

      // Bind them on the peer
      for (std::list<std::string>::const_iterator jt = objects.begin();
           jt != objects.end(); ++jt) {
        std::string objName = *it + "/" + *jt;
        std::string ior = mgr->getIOR(*it, *jt);
        forwarder->bind(objName.c_str(), ior.c_str());
      }
      // Then, get the objects binded on the peer
      SeqString *remoteObjs = peer->getBindings(it->c_str());
      // And bind them locally and on others forwarders if they are not yet
      for (unsigned int i = 0; i < remoteObjs->length(); i += 2) {
        std::string name((*remoteObjs)[i]);
        std::string ior((*remoteObjs)[i + 1]);
        if (find(objects.begin(), objects.end(), name) != objects.end()) {
          continue;
        }
        std::string newIOR = ORBMgr::convertIOR(ior, fwdTag, 0);
        mgr->bind(*it, name, newIOR, true);
        mgr->fwdsBind(*it, name, newIOR, fwdName);
      }
    }
  } catch (CORBA::TRANSIENT &err) {
    ERROR_DEBUG(
      "Unable to contact remote peer using '" << newHost
                                              <<
      "' as a \"new remote host\"", 1);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Contacted remote peer using '"
             << newHost << "' as new remote host\n");
  return 0;
} // connectPeer


void
name(const std::string &name, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setName(name);
}

void
peer_name(const std::string &name, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setPeerName(name);
}

void
peer_ior(const std::string &ior, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setPeerIOR(ior);
}

void
ssh_host(const std::string &host, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setSshHost(host);
}

void
remote_host(const std::string &host, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setRemoteHost(host);
}

void
remote_port_to(const std::string &port, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setRemotePortTo(port);
}

void
remote_port_from(const std::string &port, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setRemotePortFrom(port);
}

void
local_port_from(const std::string &port, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setLocalPortFrom(port);
}

void
ssh_path(const std::string &path, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setSshPath(path);
}

void
ssh_port(const std::string &port, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setSshPort(port);
}

void
ssh_login(const std::string &login, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setSshLogin(login);
}

void
key_path(const std::string &path, Configuration *cfg) {
  static_cast<FwrdConfig *>(cfg)->setSshKeyPath(path);
}

void
tunnel_wait(const std::string &time, Configuration *cfg) {
  std::istringstream is(time);
  int n;
  is >> n;
  static_cast<FwrdConfig *>(cfg)->setWaitingTime(n);
}

void
nb_retry(const std::string &nb, Configuration *cfg) {
  std::istringstream is(nb);
  int n;
  is >> n;
  static_cast<FwrdConfig *>(cfg)->setNbRetry(n);
}

/* Fwdr configuration implementation. */
FwrdConfig::FwrdConfig(const std::string &pgName): Configuration(pgName) {
  createTunnelTo = false;
  createTunnelFrom = false;
  nbRetry = 3;
  waitingTime = 0;
}

const std::string &
FwrdConfig::getName() const {
  return name;
}
const std::string &
FwrdConfig::getPeerName() const {
  return peerName;
}

const std::string &
FwrdConfig::getPeerIOR() const {
  return peerIOR;
}

const std::string &
FwrdConfig::getSshHost() const {
  return sshHost;
}

const std::string &
FwrdConfig::getRemoteHost() const {
  return remoteHost;
}

const std::string &
FwrdConfig::getRemotePortTo() const {
  return remotePortTo;
}

const std::string &
FwrdConfig::getRemotePortFrom() const {
  return remotePortFrom;
}

const std::string &
FwrdConfig::getLocalPortFrom() const {
  return localPortFrom;
}

bool
FwrdConfig::createTo() const {
  return createTunnelTo;
}

bool
FwrdConfig::createFrom() const {
  return createTunnelFrom;
}

const std::string &
FwrdConfig::getSshPath() const {
  return sshPath;
}

const std::string &
FwrdConfig::getSshPort() const {
  return sshPort;
}
const std::string &
FwrdConfig::getSshLogin() const {
  return sshLogin;
}

const std::string &
FwrdConfig::getSshKeyPath() const {
  return sshKeyPath;
}

int
FwrdConfig::getNbRetry() const {
  return nbRetry;
}

unsigned int
FwrdConfig::getWaitingTime() const {
  return waitingTime;
}

const std::string &
FwrdConfig::getCfgPath() const {
  return cfgPath;
}

void
FwrdConfig::setName(const std::string &name) {
  this->name = name;
}

void
FwrdConfig::setPeerName(const std::string &name) {
  this->peerName = name;
}

void
FwrdConfig::setPeerIOR(const std::string &ior) {
  this->peerIOR = ior;
}

void
FwrdConfig::setSshHost(const std::string &host) {
  this->sshHost = host;
}

void
FwrdConfig::setRemoteHost(const std::string &host) {
  this->remoteHost = host;
}

void
FwrdConfig::setRemotePortTo(const std::string &port) {
  this->remotePortTo = port;
}

void
FwrdConfig::setRemotePortFrom(const std::string &port) {
  this->remotePortFrom = port;
}

void
FwrdConfig::setLocalPortFrom(const std::string &port) {
  this->localPortFrom = port;
}

void
FwrdConfig::createTo(bool create) {
  this->createTunnelTo = create;
}

void
FwrdConfig::createFrom(bool create) {
  this->createTunnelFrom = create;
}

void
FwrdConfig::setSshPath(const std::string &path) {
  this->sshPath = path;
}

void
FwrdConfig::setSshPort(const std::string &port) {
  this->sshPort = port;
}

void
FwrdConfig::setSshLogin(const std::string &login) {
  this->sshLogin = login;
}

void
FwrdConfig::setSshKeyPath(const std::string &path) {
  this->sshKeyPath = path;
}

void
FwrdConfig::setNbRetry(const int nb) {
  this->nbRetry = nb;
}

void
FwrdConfig::setWaitingTime(const unsigned int time) {
  this->waitingTime = time;
}

void
FwrdConfig::setCfgPath(const std::string &path) {
  this->cfgPath = path;
}

int
change(int c) {
  if (c == '.') {
    return '-';
  }
  return c;
}
