/****************************************************************************/
/* A thread which updates the MultiMA links                                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 */

#include "BindService.hh"

#if HAVE_MULTI_MA

#include "ms_function.hh"
#include "MasterAgentImpl.hh"
#include "debug.hh"
#include "ORBMgr.hh"
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <errno.h>

extern unsigned int TRACE_LEVEL;

void BindService::run(void* ptr) {
  bool doNotStop = true ;
  char* ior = this->ior ;
  this->ior = NULL ;
  while(doNotStop) { 
    struct sockaddr_in clientAddr;
    socklen_t clientLen = static_cast<socklen_t>(sizeof(clientAddr));
    int newSockFd = accept(listenSocket, (struct sockaddr *) &clientAddr, 
			   &clientLen);
    if (newSockFd < 0)
      if (errno == EBADF) {
	doNotStop = false ;
	TRACE_TEXT(TRACE_ALL_STEPS, "bind service closed") ;
	continue ;
      } else {
	WARNING("error on accept (bind service): " << strerror(errno));
	continue ;
      }
    int n = write(newSockFd, ior, strlen(ior));
    if (n < 0) 
      WARNING("error when writing to socket (bind service): " << strerror(errno));
    n = close(newSockFd) ;
    if (n < 0) 
      WARNING("error when writing to socket (bind service): " << strerror(errno));
  }
  free(ior) ;
} // run(void*)

BindService::~BindService() {}

BindService::BindService(MasterAgentImpl* ma, unsigned int port) {
  ior = ORBMgr::getIORString(ma->_this()) ;

  struct sockaddr_in serverAddr;
  listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    ERROR("opening bind service socket: " << strerror(errno) << "\n", ;);
  }
  bzero((char *) &serverAddr, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);
  if (bind(listenSocket, (struct sockaddr *) &serverAddr,
	   sizeof(serverAddr)) < 0)  {
    ERROR("in binding the bind service socket: " << strerror(errno) << "\n", ;);
  }
  listen(listenSocket,5);

  TRACE_TEXT(TRACE_ALL_STEPS, "bind service open\n") ;
  start() ;
}


MasterAgent_ptr BindService::lookup(const char* addr) {
  assert(addr != NULL) ;
  char hostname[256] ;
  strncpy(hostname, addr, sizeof(hostname)) ;
  char* idx = strchr(hostname, ':') ;
  int portNo = 0;
  if (idx != NULL) {
    idx[0] = '\0' ;
    portNo = atoi(idx+1) ;
    
  }
  if(portNo == 0) {
    TRACE_TEXT(TRACE_ALL_STEPS, addr << " is not a valid address\n") ;
    return MasterAgent::_nil();
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    TRACE_TEXT(TRACE_ALL_STEPS, "opening socket: " << strerror(errno) << "\n");
    return MasterAgent::_nil();
  }

  struct hostent* server = gethostbyname(hostname);
  if (server == NULL) {
    TRACE_TEXT(TRACE_ALL_STEPS, "no such host: " << hostname << "\n");
    return MasterAgent::_nil();
  }

  struct sockaddr_in servAddr;
  bzero((char *) &servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	(char *)&servAddr.sin_addr.s_addr,
	server->h_length);
  servAddr.sin_port = htons(portNo);

  if (connect(sockfd,(const sockaddr*)&servAddr,sizeof(servAddr)) < 0)  {
    TRACE_TEXT(TRACE_ALL_STEPS, "not connecting: " << strerror(errno) << "\n");
    return MasterAgent::_nil();
  }
  char buffer[2048] ;
  bzero(buffer,sizeof(buffer));
  int n = read(sockfd,buffer,sizeof(buffer)-1);
  if (n < 0) {
    TRACE_TEXT(TRACE_ALL_STEPS, "reading from socket: " << strerror(errno) << "\n");
    return MasterAgent::_nil();
  }
  CORBA::Object_var obj = ORBMgr::stringToObject(buffer) ;
  if (CORBA::is_nil(obj)) {
    TRACE_TEXT(TRACE_ALL_STEPS, "is nil: " << buffer << "\n");
    return MasterAgent::_nil();
  }
  return MasterAgent::_narrow(obj);
}

#endif // HAVE_MULTI_MA
