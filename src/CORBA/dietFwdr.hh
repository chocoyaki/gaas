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
 * Revision 1.4  2011/01/17 18:35:17  bdepardo
 * Add missing #ifndef... #define
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

#ifndef _DIETFWDR_HH_
#define _DIETFWDR_HH_


#include <string>
#include "Options.hh"

class FwrdConfig : public Configuration {
private:
	std::string name;
	std::string peerName;
	std::string peerHost;
	std::string peerPort;
	std::string peerIOR;
	
	std::string sshHost;
	std::string remoteHost;
	std::string localPortFrom;
	std::string remotePortTo;
	std::string remotePortFrom;
	bool createTunnelTo;
	bool createTunnelFrom;
	std::string sshPath;
	std::string sshPort;
	std::string sshLogin;
	std::string sshKeyPath;
	int nbRetry;
	std::string cfgPath;
public:
	FwrdConfig(const std::string& pgName);
	const std::string& getName() const;
	const std::string& getPeerName() const;
	const std::string& getPeerIOR() const;
	const std::string& getSshHost() const;
	const std::string& getRemoteHost() const;
	const std::string& getRemotePortTo() const;
	const std::string& getRemotePortFrom() const;
	const std::string& getLocalPortFrom() const;
	bool createTo() const;
	bool createFrom() const;
	const std::string& getSshPath() const;
	const std::string& getSshPort() const;
	const std::string& getSshLogin() const;
	const std::string& getSshKeyPath() const;
	int getNbRetry() const;
	const std::string& getCfgPath() const;
	
	void setName(const std::string& name);
	void setPeerName(const std::string& name);
	void setPeerIOR(const std::string& ior);
	void setSshHost(const std::string& host);
	void setRemoteHost(const std::string& host);
	void setRemotePortTo(const std::string& port);
	void setRemotePortFrom(const std::string& port);
	void setLocalPortFrom(const std::string& port);
	void createTo(bool create);
	void createFrom(bool create);
	void setSshPath(const std::string& path);
	void setSshPort(const std::string& port);
	void setSshLogin(const std::string& login);
	void setSshKeyPath(const std::string& path);
	void setNbRetry(const int nb);
	void setCfgPath(const std::string& path);
};

void name(const std::string& name, Configuration* cfg);
void peer_name(const std::string& name, Configuration* cfg);
void peer_ior(const std::string& ior, Configuration* cfg);
void net_config(const std::string& path, Configuration* cfg);

void ssh_host(const std::string& host, Configuration* cfg);
void remote_host(const std::string& host, Configuration* cfg);
void remote_port_to(const std::string& port, Configuration* cfg);
void remote_port_from(const std::string& port, Configuration* cfg);
void local_port_from(const std::string& port, Configuration* cfg);
void create(const std::string& create, Configuration* cfg);

void ssh_path(const std::string& path, Configuration* cfg);
void ssh_port(const std::string& port, Configuration* cfg);
void ssh_login(const std::string& login, Configuration* cfg);
void key_path(const std::string& path, Configuration* cfg);
void nb_retry(const std::string& nb, Configuration* cfg);

/* Transformation function for the host name. */
int change(int c);

#endif // _DIETFWDR_HH_
