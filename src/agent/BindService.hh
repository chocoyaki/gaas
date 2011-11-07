/**
 * @file BindService.hh
 *
 * @brief  Service that sends the IOR of the agen
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#ifndef _BIND_SERVICE_HH_
#define _BIND_SERVICE_HH_ 1

#ifdef HAVE_MULTI_MA

#include <omniORB4/CORBA.h>
#include <omnithread.h>
#include "MasterAgent.hh"

class MasterAgentImpl;




class BindService : public omni_thread {
public:
  /**
   * Creates the service and set the port of the service and the MasterAgent
   *
   * @param ma the local Master Agent
   *
   * @param port the listen port of the service
   */
  BindService(MasterAgentImpl * ma, unsigned int port);
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
  MasterAgent_ptr
  lookup(const char *addr);

private:
  /**
   * listen to the port and send the ior of the MA to the socket
   * client until the listenSocket is closed.
   */
  void
  run(void *ptr);
  /**
   * The IOR of the MA  in the string format
   */
  char *ior;
  /**
   * the file descriptor of the socket
   */
  int listenSocket;
};  // ReferenceUpdateThread

#endif  // HAVE_MULTI_MA

#endif  // _BIND_SERVICE_HH_
