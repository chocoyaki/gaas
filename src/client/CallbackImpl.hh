/****************************************************************************/
/* POAcallback implementation                                              */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Christophe PERA (christophe.pera@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2003/06/02 14:51:54  cpera
 * Applying coding standards on Callback.idl.
 *
 * Revision 1.1  2003/06/02 08:09:55  cpera
 * Beta version of asynchronize DIET API.
 *
 ****************************************************************************/


#ifndef _CALLBACKIMPL_H_
#define _CALLBACKIMPL_H_

#include "Callback.hh"
#include "CallAsyncMgr.hh"

class CallbackImpl : public POA_Callback,
	            public PortableServer::RefCountServantBase
{
public:
  CallbackImpl();
  virtual ~CallbackImpl();
  virtual long ping();
  virtual long notifyResults(const char * path,
                            const corba_profile_t& pb,
			    long reqID);
  virtual long solveResults(const char * path, 
                            const corba_profile_t& pb,
			    long reqID);
};

#endif
