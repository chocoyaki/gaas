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
 * Revision 1.4  2006/11/16 09:55:54  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.3  2005/11/21 15:48:58  eboix
 *  - src/agent/BindService.hh: CORBA.h now included as omniORB4/CORBA.h.
 *  - Clean up of wired in options of Cmake/DIET_config.h.in
 *  - HAVE_MULTI_MA is now a cmake option.                  --- Injay2461
 *
 * Revision 1.2  2004/10/04 09:40:43  sdahan
 * warning fix :
 *  - debug.cc : change the printf format from %ul to %lu and from %l to %ld
 *  - ReferenceUpdateThread and BindService : The omniORB documentation said that
 *    it's better to create private destructor for the thread subclasses. But
 *    private destructors generate warning, so I set the destructors public.
 *  - CORBA.h and DIET_config.h define the same macros. So I include the CORBA.h
 *    before the DIET_config.h to avoid to define two times the same macros.
 *  - remove the deprecated warning when including iostream.h and set.h
 *
 * Revision 1.1  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 */


#ifndef _BIND_SERVICE_HH_
#define _BIND_SERVICE_HH_ 1

#ifdef HAVE_MULTI_MA

#include "omnithread.h"
#include <omniORB4/CORBA.h>
#include "MasterAgent.hh"

class MasterAgentImpl;

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
  char* ior;

  /**
   * the file descriptor of the socket
   */
  int listenSocket;

  /**
   * listen to the port and send the ior of the MA to the socket
   * client until the listenSocket is closed.
   */
  void run(void* ptr);

public :

  /**
   * Creates the service and set the port of the service and the MasterAgent
   *
   * @param ma the local Master Agent
   * 
   * @param port the listen port of the service
   */
  BindService(MasterAgentImpl* ma, unsigned int port);

  /**
   * destroys the thread.
   */
  ~BindService();

  /**
   * return the MasterAgent_ptr binded at the given address. The
   * format of the address is "host:port"
   *
   * @param addr address of the bind service of a Master Agent. The
   * format of the string is "host:port"
   *
   * @return a pointer on the Master Agent or nil if an error append
   */
  MasterAgent_ptr lookup(const char* addr);

}; // ReferenceUpdateThread

#endif // HAVE_MULTI_MA

#endif // _BIND_SERVICE_HH_
