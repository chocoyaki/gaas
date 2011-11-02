/**
* @file SSHTunnel.cc
* 
* @brief  DIET forwarder implementation - SSH Tunnel implementation 
* 
* @author - Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)  
* 
* @section Licence
*   |LICENSE|                                                                
*/

#include "SSHTunnel.hh"

#include "debug.hh"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <string>
#include <fstream>
#include <iterator>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <unistd.h>    // For sleep & fork functions
#include <sys/wait.h>  // For waitpid function

/* To find a free tcp port. */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/**/

const unsigned int DEFAULT_WAITING_TIME = 10;

std::string SSHTunnel::cmdFormat  = "%p -l %u %s -p %P -N";
std::string SSHTunnel::cmdFormatDefault = "%p %s -N";
std::string SSHTunnel::localFormat = "-L%l:%h:%R";
std::string SSHTunnel::remoteFormat = "-R%r:%h:%L";
std::string SSHTunnel::keyFormat = "-i %k";

/* Return the current session login. */
std::string
SSHConnection::userLogin() {
  char* result = getlogin();

  if (result == NULL) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "Unable to determine the user login.\n");
    return "";
  }
  return result;
}

/* Get the default path to user SSH key. If the user does not use
 * a private key for connection, return empty std::string.
 */

std::string
SSHConnection::userKey() {
  char* home = getenv("HOME");
  std::string path;

  /* Try the RSA key default path. */
  path = ((home == NULL) ?
          std::string("") : std::string(home)) + "/.ssh/id_rsa";
  std::ifstream f(path.c_str());
  if (f.is_open()) {
    f.close();
    return path;
  }

  /* Try the DSA key default path. */
  path = ((home == NULL) ?
          std::string("") : std::string(home)) + "/.ssh/id_dsa";
  f.open(path.c_str());
  if (f.is_open()) {
    f.close();
    return path;
  }

  /* None of the two default keys were found */
  return "";
}

SSHConnection::SSHConnection() {
  setSshPath("/usr/bin/ssh");
}

SSHConnection::SSHConnection(const std::string& sshHost,
                             const std::string& sshPort,
                             const std::string& login,
                             const std::string& keyPath,
                             const std::string& sshPath) {
  setSshHost(sshHost);
  setSshPort(sshPort);
  setSshLogin(login);
  setSshKeyPath(keyPath);
  setSshPath(sshPath);
}

const std::string&
SSHConnection::getSshHost() const {
  return sshHost;
}

const std::string&
SSHConnection::getSshPath() const {
  return sshPath;
}

const std::string&
SSHConnection::getSshPort() const {
  return sshPort;
}

const std::string&
SSHConnection::getSshLogin() const {
  return login;
}

const std::string&
SSHConnection::getSshKeyPath() const {
  return keyPath;
}

const std::string&
SSHConnection::getSshOptions() const {
  return options;
}

void
SSHConnection::setSshHost(const std::string& host) {
  if (host != "") {
    this->sshHost = host;
  }
}

void
SSHConnection::setSshPath(const std::string& path) {
  if (path != "") {
    this->sshPath = path;
  }
}

void
SSHConnection::setSshPort(const std::string& port) {
  if (port != "") {
    this->sshPort = port;
  }
}

void
SSHConnection::setSshPort(const int port) {
  std::ostringstream os;
  os << port;
  this->sshPort = os.str();
}

void
SSHConnection::setSshLogin(const std::string& login) {
  if (login != "") {
    this->login = login;
  }
}

void
SSHConnection::setSshKeyPath(const std::string& path) {
  this->keyPath = path;
}

void SSHConnection::setSshOptions(const std::string& options) {
  this->options = options;
}

/* Replace "s" by "r" in "str". */
void
replace(const std::string& s, const std::string& r, std::string& str) {
  size_t pos;
  if ((pos = str.find(s)) != std::string::npos) {
    str.erase(pos, s.length());
    str.insert(pos, r);
  }
}

/* Try to find a free TCP port. "sfd" is
 * the socket file descriptor used to find
 * the port.
 */
std::string freeTCPport() {
  std::ostringstream os;
  struct sockaddr_in sck;

  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  sck.sin_family = AF_INET;
  sck.sin_addr.s_addr = INADDR_ANY;
  sck.sin_port = 0;
  bind(sfd, (struct sockaddr*) &sck, sizeof(sck));
  socklen_t len = sizeof(sck);
  getsockname(sfd, (struct sockaddr*) &sck, &len);
  os << sck.sin_port;
  close(sfd);

  return os.str();
}

std::string
SSHTunnel::makeCmd() {
  std::string result;

  if (getSshLogin() == "" && getSshPort() == "" && getSshKeyPath() == "") {
    result = cmdFormatDefault;
  } else {
    result = cmdFormat;
  }
  replace("%p", getSshPath(), result);
  replace("%u", getSshLogin(), result);
  if (getSshLogin() != "" && getSshPort() == "") {
    setSshPort(22);
  }
  replace("%P", getSshPort(), result);
  replace("%s", getSshHost(), result);

  if (createTo) {
    result += " ";
    result += localFormat;
    if (localPortFrom == "") {
      localPortFrom = freeTCPport();
      replace("%l", localPortFrom, result);
    }
    replace("%h", remoteHost, result);
    replace("%R", remotePortTo, result);
  }
  if (getSshKeyPath() != "") {
    result += " ";
    result += keyFormat;
    replace("%k", getSshKeyPath(), result);
  }

  if (createFrom) {
    result += " ";
    result += remoteFormat;
    replace("%L", localPortTo, result);
    replace("%h", remoteHost, result);
    replace("%r", remotePortFrom, result);
  }

  if (getSshOptions() != "") {
    result += " "+getSshOptions();
  }

  return result;
}

SSHTunnel::SSHTunnel() : SSHConnection() {
  this->createTo = false;
  this->createFrom = false;
  this->waitingTime = DEFAULT_WAITING_TIME;
}

/* Constructor for bi-directionnal SSH tunnel. */
SSHTunnel::SSHTunnel(const std::string& sshHost,
                     const std::string& remoteHost,
                     const std::string& localPortFrom,
                     const std::string& remotePortTo,
                     const std::string& remotePortFrom,
                     const std::string& localPortTo,
                     const bool createTo,
                     const bool createFrom,
                     const std::string& sshPath,
                     const std::string& sshPort,
                     const std::string& login,
                     const std::string& keyPath)
  : SSHConnection(sshHost, sshPort, login, keyPath, sshPath) {
  this->remoteHost = remoteHost;
  this->localPortFrom = localPortFrom;
  this->remotePortTo = remotePortTo;
  this->remotePortFrom = remotePortFrom;
  this->localPortTo = localPortTo;
  this->createTo = createTo;
  this->createFrom = createFrom;
  this->waitingTime = DEFAULT_WAITING_TIME;
}

/* Constructor for unidirectionnal SSH tunnel. */
SSHTunnel::SSHTunnel(const std::string& sshHost,
                     const std::string& remoteHost,
                     const std::string& localPortFrom,
                     const std::string& remotePortTo,
                     const bool createTo,
                     const std::string& sshPath,
                     const std::string& sshPort,
                     const std::string& login,
                     const std::string& keyPath)
  : SSHConnection(sshHost, sshPort, login, keyPath, sshPath) {
  this->remoteHost = remoteHost;
  this->localPortFrom = localPortFrom;
  this->remotePortTo = remotePortTo;
  this->createTo = createTo;
  this->createFrom = false;
  this->waitingTime = DEFAULT_WAITING_TIME;
}

SSHTunnel::~SSHTunnel() {
  close();
}

void
SSHTunnel::open() {
  if (!createTo && !createFrom) {
    return;
  }

  std::vector<std::string> tokens;
  std::string command = makeCmd();
  std::istringstream is(command);

  std::copy(std::istream_iterator<std::string>(is),
            std::istream_iterator<std::string>(),
            std::back_inserter<std::vector<std::string> >(tokens));

  char* argv[tokens.size() + 1];
  argv[tokens.size()] = NULL;

  for (unsigned int i = 0; i < tokens.size(); ++i) {
    argv[i] = strdup(tokens[i].c_str());
  }

  pid = fork();
  if (pid == -1) {
    throw std::runtime_error("Error forking process.");
  }
  if (pid == 0) {
    if (execvp(argv[0], argv)) {
      ERROR_EXIT("Error executing command " << command);
    }
  }
  for (unsigned int i = 0; i < tokens.size(); ++i) {
    free(argv[i]);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Sleep " << this->waitingTime
             << " s. waiting for tunnel\n");
  sleep(this->waitingTime);
  TRACE_TEXT(TRACE_MAIN_STEPS, "Wake up!\n");
}

void
SSHTunnel::close() {
  if (!createTo && !createFrom) {
    return;
  }
  if (pid && kill(pid, SIGTERM)) {
    if (kill(pid, SIGKILL)) {
      throw std::runtime_error("Unable to stop the ssh process");
    }
  }
  pid = 0;
}

const std::string&
SSHTunnel::getRemoteHost() const {
  return remoteHost;
}

int
SSHTunnel::getLocalPortFrom() const {
  int res;
  std::istringstream is(localPortFrom);

  is >> res;

  return res;
}

int
SSHTunnel::getLocalPortTo() const {
  int res;
  std::istringstream is(localPortTo);

  is >> res;

  return res;
}

int
SSHTunnel::getRemotePortFrom() const {
  int res;
  std::istringstream is(remotePortFrom);

  is >> res;

  return res;
}

int
SSHTunnel::getRemotePortTo() const {
  int res;
  std::istringstream is(remotePortTo);

  is >> res;

  return res;
}

void
SSHTunnel::setRemoteHost(const std::string& host) {
  this->remoteHost = host;
}

void
SSHTunnel::setLocalPortFrom(const std::string& port) {
  this->localPortFrom = port;
}

void
SSHTunnel::setLocalPortFrom(const int port) {
  std::ostringstream os;
  os << port;
  this->localPortFrom = os.str();
}

void
SSHTunnel::setRemotePortTo(const std::string& port) {
  this->remotePortTo = port;
}

void
SSHTunnel::setRemotePortTo(const int port) {
  std::ostringstream os;
  os << port;
  this->remotePortTo = os.str();
}

void
SSHTunnel::setRemotePortFrom(const std::string& port) {
  this->remotePortFrom = port;
}

void
SSHTunnel::setRemotePortFrom(const int port) {
  std::ostringstream os;
  os << port;
  this->remotePortFrom = os.str();
}

void
SSHTunnel::setLocalPortTo(const std::string& port) {
  this->localPortTo = port;
}

void
SSHTunnel::setLocalPortTo(const int port) {
  std::ostringstream os;
  os << port;
  this->localPortTo = os.str();
}

void
SSHTunnel::setWaitingTime(const unsigned int time) {
  if (time != 0) {
    this->waitingTime = time;
  }
}

void
SSHTunnel::createTunnelTo(const bool create) {
  this->createTo = create;
}

void
SSHTunnel::createTunnelFrom(const bool create) {
  this->createFrom = create;
}


SSHCopy::SSHCopy(const std::string& sshHost,
                 const std::string& remoteFilename,
                 const std::string& localFilename) {
  setSshHost(sshHost);
  this->remoteFilename = remoteFilename;
  this->localFilename = localFilename;
}

bool
SSHCopy::getFile() const {
  std::vector<std::string> tokens;
  int status;
  std::string command;

  command = getSshPath();
  if (getSshPort() != "") {
    command += " -P " + getSshPort();
  }
  if (getSshKeyPath() != "") {
    command += " -i " + getSshKeyPath();
  }
  if (getSshLogin() != "") {
    command += " " + getSshLogin() + "@" + getSshHost() + ":" + remoteFilename;
  } else {
    command += " " + getSshHost() + ":" + remoteFilename;
  }

  command += " " + localFilename;

  std::istringstream is(command);

  std::copy(std::istream_iterator<std::string>(is),
            std::istream_iterator<std::string>(),
            std::back_inserter<std::vector<std::string> >(tokens));

  char* argv[tokens.size() + 1];
  argv[tokens.size()] = NULL;

  for (unsigned int i = 0; i < tokens.size(); ++i) {
    argv[i] = strdup(tokens[i].c_str());
  }
  pid = fork();
  if (pid == -1) {
    throw std::runtime_error("Error forking process.");
  }
  if (pid == 0) {
    fclose(stdout);
    if (execvp(argv[0], argv)) {
      ERROR_EXIT("Error executing command " << command);
    }
  }

  for (unsigned int i = 0; i < tokens.size(); ++i) {
    free(argv[i]);
  }

  if (waitpid(pid, &status, 0) == -1) {
    throw std::runtime_error("Error executing scp command");
  }
  // FIXME: WTF ?
  return ((WIFEXITED(status) !=0) ? (WEXITSTATUS(status) == 0) : false);
}

bool
SSHCopy::putFile() const {
  std::vector<std::string> tokens;
  int status;
  std::string command;

  command = getSshPath();
  if (getSshPort() != "") {
    command += " -P " + getSshPort();
  }
  if (getSshKeyPath() != "") {
    command += " -i " + getSshKeyPath();
  }

  command += " " + localFilename;
  if (getSshLogin() != "")  {
    command += " "+getSshLogin()+"@"+getSshHost()+":"+remoteFilename;
  } else {
    command += " "+getSshHost()+":"+remoteFilename;
  }

  std::istringstream is(command);

  std::copy(std::istream_iterator<std::string>(is),
            std::istream_iterator<std::string>(),
            std::back_inserter<std::vector<std::string> >(tokens));

  char* argv[tokens.size() + 1];
  argv[tokens.size()] = NULL;

  for (unsigned int i = 0; i < tokens.size(); ++i) {
    argv[i] = strdup(tokens[i].c_str());
  }

  pid = fork();
  if (pid == -1) {
    throw std::runtime_error("Error forking process.");
  }
  if (pid == 0) {
    fclose(stdout);
    if (execvp(argv[0], argv)) {
      ERROR_EXIT("Error executing command " << command);
    }
  }

  for (unsigned int i = 0; i < tokens.size(); ++i) {
    free(argv[i]);
  }

  if (waitpid(pid, &status, 0) == -1) {
    throw std::runtime_error("Error executing scp command");
  }

  return ((WIFEXITED(status) != 0) ? (WEXITSTATUS(status) == 0) : false);
}
