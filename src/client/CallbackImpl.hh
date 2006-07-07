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
 * Revision 1.4  2006/07/07 09:29:03  aamar
 * Modify the solveResults prototype: adding the parameter solve_res
 * that represents the execution status of the asynchronous request.
 *
 * Revision 1.3  2003/10/13 13:02:22  cpera
 * Replace long by CORBA::Long.
 *
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
  virtual CORBA::Long ping();
  virtual CORBA::Long notifyResults(const char * path,
				    const corba_profile_t& pb,
				    CORBA::Long reqID);
  virtual CORBA::Long solveResults(const char * path, 
				   const corba_profile_t& pb,
				   CORBA::Long reqID,
				   CORBA::Long solve_res);
};

#endif
