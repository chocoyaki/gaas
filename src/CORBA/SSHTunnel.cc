/****************************************************************************/
/* DIET forwarder implementation - SSH Tunnel implementation                */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 * Revision 1.2  2010/07/13 15:24:13  glemahec
 * Warnings corrections and some robustness improvements
 *
 * Revision 1.1  2010/07/12 16:11:04  glemahec
 * DIET 2.5 beta 1 - New ORB manager; dietForwarder application
 ****************************************************************************/

#include "SSHTunnel.hh"

#include <string>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <sstream>	
#include <iterator>
#include <algorithm>
#include <cstdio>

#include <unistd.h> // For sleep & fork functions
#include <sys/wait.h> // For waitpid function

/* To find a free tcp port. */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/**/

using namespace std;


string SSHTunnel::cmdFormat  = "%p -l %u %s -p %P -N";
string SSHTunnel::localFormat = "-L%l:%h:%R";
string SSHTunnel::remoteFormat = "-R%r:%h:%L";
string SSHTunnel::keyFormat = "-i %k";

/* Return the current session login. */
string SSHConnection::userLogin() {
	char* result = getlogin();
	
	if (result==NULL)
		throw runtime_error("Unable to determine the user login.");
	return result;
}

/* Get the default path to user SSH key. If the user does not use
 * a private key for connection, return empty string.
 */

string SSHConnection::userKey() {
	char* home = getenv("HOME");
	string path;
	
	/* Try the RSA key default path. */
	path = (home==NULL ? string(""):string(home))+"/.ssh/id_rsa";
	ifstream f(path.c_str());
	if (f.is_open()) return path;
	/* Try the DSA key default path. */
	path = (home==NULL ? string(""):string(home))+"/.ssh/id_dsa";
	f.open(path.c_str());
	if (f.is_open()) return path;
	return "";
}

SSHConnection::SSHConnection() {
	setSshPath("/usr/bin/ssh");
	setSshPort("22");
	setSshLogin(userLogin());
	setSshKeyPath(userKey());
}

SSHConnection::SSHConnection(const std::string& sshHost, const std::string& sshPort,
														 const std::string& login,	const std::string& keyPath,
														 const std::string& sshPath)
{
	setSshHost(sshHost);
	setSshPort(sshPort);
	setSshLogin(login);
	setSshKeyPath(keyPath);
	setSshPath(sshPath);
}

const std::string& SSHConnection::getSshHost() const {
	return sshHost;
}

const std::string& SSHConnection::getSshPath() const {
	return sshPath;
}

const std::string& SSHConnection::getSshPort() const {
	return sshPort;
}

const std::string& SSHConnection::getSshLogin() const {
	return login;
}

const std::string& SSHConnection::getSshKeyPath() const {
	return keyPath;
}

const std::string& SSHConnection::getSshOptions() const {
	return options;
}

void SSHConnection::setSshHost(const std::string& host) {
	if (host!="") this->sshHost = host;
}

void SSHConnection::setSshPath(const std::string& path) {
	if (path!="") this->sshPath = path;
}

void SSHConnection::setSshPort(const std::string& port) {
	if (port!="") this->sshPort = port;
}

void SSHConnection::setSshPort(const int port) {
	ostringstream os;
	os << port;
	this->sshPort = os.str();
}

void SSHConnection::setSshLogin(const std::string& login) {
	if (login!="") this->login = login;
}

void SSHConnection::setSshKeyPath(const std::string& path) {
	if (keyPath!="") this->keyPath = path;
}

void SSHConnection::setSshOptions(const std::string& options) {
	this->options = options;
}

/* Replace "s" by "r" in "str". */
void replace(const string& s, const string& r, string& str) {
	size_t pos;
	if ((pos=str.find(s))!=string::npos) {
		str.erase(pos, s.length());
		str.insert(pos, r);
	}
}

/* Try to find a free TCP port. "sfd" is
 * the socket file descriptor used to find
 * the port.
 */
std::string freeTCPport() {
	ostringstream os;
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

string SSHTunnel::makeCmd() {
	string result;
	
	result = cmdFormat;
	replace("%p", getSshPath(), result);
	replace("%u", getSshLogin(), result);
	replace("%P", getSshPort(), result);
	replace("%s", getSshHost(), result);
	
	if (createTo) {
		result+=" ";
		result+=localFormat;
		if (localPortFrom=="") {
			localPortFrom = freeTCPport();
			replace("%l", localPortFrom, result);
		}
		replace("%h", remoteHost, result);
		replace("%R", remotePortTo, result);
	}
	if (getSshKeyPath()!="") {
		result+=" ";
		replace("%k", getSshKeyPath(), result);
	}
	
	if (createFrom) {
		result+=" ";
		result+=remoteFormat;
		replace("%L", localPortTo, result);
		replace("%h", remoteHost, result);
		replace("%r", remotePortFrom, result);
	}
	
	if (getSshOptions()!="") {
		result+=" "+getSshOptions();
	}
	return result;	
}

SSHTunnel::SSHTunnel() : SSHConnection() {
	createTo = false;
	createFrom = false;
}

/* Constructor for bi-directionnal SSH tunnel. */
SSHTunnel::SSHTunnel(const string& sshHost,
										 const string& remoteHost,
										 const string& localPortFrom,
										 const string& remotePortTo,
										 const string& remotePortFrom,
										 const string& localPortTo,
										 const bool createTo,
										 const bool createFrom,
										 const string& sshPath,
										 const string& sshPort,
										 const string& login,
										 const string& keyPath) :
  SSHConnection(sshHost, sshPort, login, keyPath, sshPath)
{
	this->remoteHost = remoteHost;
	this->localPortFrom = localPortFrom;
	this->remotePortTo = remotePortTo;
	this->remotePortFrom = remotePortFrom;
	this->localPortTo = localPortTo;
	this->createTo = createTo;
	this->createFrom = createFrom;
}

/* Constructor for unidirectionnal SSH tunnel. */
SSHTunnel::SSHTunnel(const string& sshHost,
										 const string& remoteHost,
										 const string& localPortFrom,
										 const string& remotePortTo,
										 const bool createTo,
										 const string& sshPath,
										 const string& sshPort,
										 const string& login,
										 const string& keyPath) :
  SSHConnection(sshHost, sshPort, login, keyPath, sshPath)
{
	this->remoteHost = remoteHost;
	this->localPortFrom = localPortFrom;
	this->remotePortTo = remotePortTo;
	this->createTo = createTo;
	this->createFrom = false;
}

SSHTunnel::~SSHTunnel() {
	close();
}

void SSHTunnel::open() {
	if (!createTo && !createFrom) return;
	
	vector<string> tokens;
	string command = makeCmd();
	
	istringstream is(command);
  
  copy(istream_iterator<string>(is),
       istream_iterator<string>(),
       back_inserter<vector<string> >(tokens));
  
  char* argv[tokens.size()+1];
  argv[tokens.size()]=NULL;
  
  for (unsigned int i=0; i<tokens.size(); ++i)
    argv[i]=strdup(tokens[i].c_str());
	
	pid = fork();
	if (pid==-1)
		throw runtime_error("Error forking process.");
	if (pid==0) {
		if (execvp(argv[0], argv)) {
			cerr << "Error executing command " << command << endl;
			exit(1);
		}
	}
	for (unsigned int i=0; i<tokens.size(); ++i)
		free(argv[i]);
	cout << "Sleep 10 s. waiting for tunnel" << endl;
	sleep(10);
	cout << "Wake up!" << endl;
}

void SSHTunnel::close() {
	if (!createTo && !createFrom) return;
	if (kill(pid, SIGTERM))
		if (kill(pid, SIGKILL))
			throw runtime_error("Unable to stop the ssh process");
}

const string& SSHTunnel::getRemoteHost() const {
	return remoteHost;
}

int SSHTunnel::getLocalPortFrom() const {
	int res;
	istringstream is(localPortFrom);
	
	is >> res;
	
	return res;
}

int SSHTunnel::getLocalPortTo() const {
	int res;
	istringstream is(localPortTo);
	
	is >> res;
	
	return res;
}

int SSHTunnel::getRemotePortFrom() const {
	int res;
	istringstream is(remotePortFrom);
	
	is >> res;
	
	return res;
}

int SSHTunnel::getRemotePortTo() const {
	int res;
	istringstream is(remotePortTo);
	
	is >> res;
	
	return res;
}

void SSHTunnel::setRemoteHost(const std::string& host) {
	this->remoteHost = host;
}

void SSHTunnel::setLocalPortFrom(const std::string& port) {
	this->localPortFrom = port;
}

void SSHTunnel::setLocalPortFrom(const int port) {
	ostringstream os;
	os << port;
	this->localPortFrom = os.str();
}

void SSHTunnel::setRemotePortTo(const std::string& port) {
	this->remotePortTo = port;
}

void SSHTunnel::setRemotePortTo(const int port) {
	ostringstream os;
	os << port;
	this->remotePortTo = os.str();
}

void SSHTunnel::setRemotePortFrom(const std::string& port) {
	this->remotePortFrom = port;
}

void SSHTunnel::setRemotePortFrom(const int port) {
	ostringstream os;
	os << port;
	this->remotePortFrom = os.str();
}

void SSHTunnel::setLocalPortTo(const std::string& port) {
	this->localPortTo = port;
}

void SSHTunnel::setLocalPortTo(const int port) {
	ostringstream os;
	os << port;
	this->localPortTo = os.str();
}

void SSHTunnel::createTunnelTo(const bool create) {
	this->createTo = create;
}

void SSHTunnel::createTunnelFrom(const bool create) {
	this->createFrom = create;
}


SSHCopy::SSHCopy(const string& sshHost,
								 const string& remoteFilename,
								 const string& localFilename)
{
	setSshHost(sshHost);
	this->remoteFilename = remoteFilename;
	this->localFilename = localFilename;
}

bool SSHCopy::getFile() const {
	vector<string> tokens;
	int status;
	string command = getSshPath()+" -P "+getSshPort();
	command += " "+getSshLogin()+"@"+getSshHost()+":"+remoteFilename;
	command += " "+localFilename;
	
	if (getSshKeyPath()!="")
		command += " -i "+getSshKeyPath();
	istringstream is(command);
  
  copy(istream_iterator<string>(is),
       istream_iterator<string>(),
       back_inserter<vector<string> >(tokens));
  
  char* argv[tokens.size()+1];
  argv[tokens.size()]=NULL;
  
  for (unsigned int i=0; i<tokens.size(); ++i)
    argv[i]=strdup(tokens[i].c_str());
	
	pid = fork();
	if (pid==-1)
		throw runtime_error("Error forking process.");
	if (pid==0) {
		fclose(stdout);
		if (execvp(argv[0], argv)) {
			cerr << "Error executing command " << command << endl;
			exit(1);
		}
	}
	
	for (unsigned int i=0; i<tokens.size(); ++i)
		free(argv[i]);
	
	if (waitpid(pid, &status, 0)==-1)
		throw runtime_error("Error executing scp command");
	return (WIFEXITED(status)!=0 ? (WEXITSTATUS(status)==0):false);
}

bool SSHCopy::putFile() const {
	vector<string> tokens;
	int status;
	string command = getSshPath()+" -P "+getSshPort()+" -i "+getSshKeyPath();
	command += " "+localFilename;
	command += getSshLogin()+"@"+getSshHost()+":"+remoteFilename;
	
	istringstream is(command);
  
  copy(istream_iterator<string>(is),
       istream_iterator<string>(),
       back_inserter<vector<string> >(tokens));
  
  char* argv[tokens.size()+1];
  argv[tokens.size()]=NULL;
  
  for (unsigned int i=0; i<tokens.size(); ++i)
    argv[i]=strdup(tokens[i].c_str());
	
	pid = fork();
	if (pid==-1)
		throw runtime_error("Error forking process.");
	if (pid==0) {
		fclose(stdout);
		if (execvp(argv[0], argv)) {
			cerr << "Error executing command " << command << endl;
			exit(1);
		}
	}
	
	for (unsigned int i=0; i<tokens.size(); ++i)
		free(argv[i]);
	
	if (waitpid(pid, &status, 0)==-1)
		throw runtime_error("Error executing scp command");
	return (WIFEXITED(status)!=0 ? (WEXITSTATUS(status)==0):false);
}