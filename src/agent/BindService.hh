/****************************************************************************/
/* Service that sends the IOR of the agent                                  */
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


#ifndef _BIND_SERVICE_HH_
#define _BIND_SERVICE_HH_ 1

#include "DIET_config.h"

#ifdef HAVE_MULTI_MA

#include "omnithread.h"
#include <CORBA.h>
#include "MasterAgent.hh"

class MasterAgentImpl ;

/**
 * When this service is loaded, it listen to a given port and send to
 * every socket which connect to this port, the IOR of the MA in the
 * string format
 *
 * @author Sylvain DAHAN - LIFC Besancon (France)
 */


class BindService : public omni_thread {

private :

  /**
   * The IOR of the MA  in the string format
   */
  char* ior ;

  /**
   * the file descriptor of the socket
   */
  int listenSocket ;

  /**
   * destroys the thread.
   */
  ~BindService() ;

  /**
   * listen to the port and send the ior of the MA to the socket
   * client until the listenSocket is closed.
   */
  void run(void* ptr) ;

public :

  /**
   * Creates the service and set the port of the service and the MasterAgent
   *
   * @param ma the local Master Agent
   * 
   * @param port the listen port of the service
   */
  BindService(MasterAgentImpl* ma, unsigned int port) ;

  /**
   * return the MasterAgent_ptr binded at the given address. The
   * format of the address is "host:port"
   *
   * @param addr address of the bind service of a Master Agent. The
   * format of the string is "host:port"
   *
   * @return a pointer on the Master Agent or nil if an error append
   */
  MasterAgent_ptr lookup(const char* addr) ;

} ; // ReferenceUpdateThread

#endif // HAVE_MULTI_MA

#endif // _BIND_SERVICE_HH_
