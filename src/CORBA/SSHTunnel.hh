/**
* @file SSHTunnel.hh
* 
* @brief  DIET forwarder implementation - SSH Tunnel implementation 
* 
* @author - Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)  
* 
* @section Licence
*   |LICENSE|                                                                
*/

#ifndef SSHTUNNEL_HH
#define SSHTUNNEL_HH

#include <csignal>
#include <string>
#include <unistd.h>

class SSHConnection {
public:
  SSHConnection();

  SSHConnection(const std::string& sshHost, const std::string& sshPort,
                const std::string& login, const std::string& keyPath,
                const std::string& sshPath);

  const std::string&
  getSshHost() const;

  const std::string&
  getSshPath() const;

  const std::string&
  getSshPort() const;

  const std::string&
  getSshLogin() const;

  const std::string&
  getSshKeyPath() const;

  const std::string&
  getSshOptions() const;

  void
  setSshHost(const std::string& host);

  void
  setSshPath(const std::string& path);

  void
  setSshPort(const std::string& port);

  void
  setSshPort(const int port);

  void
  setSshLogin(const std::string& login);

  void
  setSshKeyPath(const std::string& path);

  void
  setSshOptions(const std::string& options);

protected:
  /* Get the default user login and private key. */
  static std::string
  userLogin();

  static std::string
  userKey();

private:
  /* SSH executable path. */
  std::string sshPath;
  /* SSH connection params. */
  std::string login;
  std::string keyPath;
  std::string sshHost;
  std::string sshPort;
  std::string options;
};

class SSHTunnel : public SSHConnection {
public:
  SSHTunnel();
  /* Constructor for bi-directionnal SSH tunnel. */
  SSHTunnel(const std::string& sshHost,
            const std::string& remoteHost,
            const std::string& localPortFrom,
            const std::string& remotePortTo,
            const std::string& remotePortFrom,
            const std::string& localPortTo,
            const bool createTo = true,
            const bool createFrom = true,
            const std::string& sshPath = "/usr/bin/ssh",
            const std::string& sshPort = "22",
            const std::string& login = userLogin(),
            const std::string& keyPath = userKey());

  /* Constructor for unidirectionnal SSH tunnel. */
  SSHTunnel(const std::string& sshHost,
            const std::string& remoteHost,
            const std::string& localPortFrom,
            const std::string& remotePortTo,
            const bool createTo = true,
            const std::string& sshPath = "/usr/bin/ssh",
            const std::string& serverPort = "22",
            const std::string& login = userLogin(),
            const std::string& keyPath = userKey());

  ~SSHTunnel();

  void
  open();
  void
  close();

  const std::string&
  getRemoteHost() const;

  int
  getLocalPortFrom() const;

  int
  getLocalPortTo() const;

  int
  getRemotePortFrom() const;

  int
  getRemotePortTo() const;

  void
  setRemoteHost(const std::string& host);

  void
  setLocalPortFrom(const std::string& port);

  void
  setLocalPortFrom(const int port);

  void
  setRemotePortTo(const std::string& port);

  void
  setRemotePortTo(const int port);

  void
  setRemotePortFrom(const std::string& port);

  void
  setRemotePortFrom(const int port);

  void
  setLocalPortTo(const std::string& port);

  void
  setLocalPortTo(const int port);

  void
  setWaitingTime(const unsigned int time);

  void
  createTunnelTo(const bool create);

  void
  createTunnelFrom(const bool create);
private:
  /* Format strings for ssh commands. */
  static std::string cmdFormat;
  static std::string cmdFormatDefault;
  static std::string localFormat;
  static std::string remoteFormat;
  static std::string keyFormat;
  /* Tunnel configuration. */
  bool createFrom;
  bool createTo;
  unsigned int waitingTime;
  std::string localPortTo;
  std::string localPortFrom;
  std::string remoteHost;
  std::string remotePortTo;
  std::string remotePortFrom;

  /* Process pid. */
  pid_t pid;

  std::string
  makeCmd();
};

/* Copy a file using scp. */
class SSHCopy : public SSHConnection {
public:
  SSHCopy(const std::string& sshHost,
          const std::string& remoteFilename,
          const std::string& localFilename);

  bool
  getFile() const;

  bool
  putFile() const;

private:
  std::string remoteFilename;
  std::string localFilename;

  /* Process pid. */
  mutable pid_t pid;
};


std::string
freeTCPport();

#endif
