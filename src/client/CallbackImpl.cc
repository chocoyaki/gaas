/****************************************************************************/
/* POA callback implementation                                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Christophe PERA (christophe.pera@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2006/08/09 21:39:35  aamar
 * Notify result before setting the request error code
 *
 * Revision 1.7  2006/07/10 13:46:00  aamar
 * Correcting a warning
 *
 * Revision 1.6  2006/07/07 09:29:03  aamar
 * Modify the solveResults prototype: adding the parameter solve_res
 * that represents the execution status of the asynchronous request.
 *
 * Revision 1.5  2003/10/13 13:02:22  cpera
 * Replace long by CORBA::Long.
 *
 * Revision 1.4  2003/09/25 10:06:24  cpera
 * Delete messages and useless include.
 *
 * Revision 1.3  2003/09/16 10:26:41  cpera
 * Fix a warning.
 *
 * Revision 1.2  2003/06/02 09:00:19  cpera
 * Delete debug infos.
 *
 * Revision 1.1  2003/06/02 08:09:55  cpera
 * Beta version of asynchronize DIET API.
 *
 ****************************************************************************/

using namespace std;
#include <iostream>
#include <CallbackImpl.hh>
#define TRACE_LEVEL 50
#include "debug.hh"

CallbackImpl::CallbackImpl()
{
  // nothing
}

CallbackImpl::~CallbackImpl()
{
  // nothing
}

CORBA::Long CallbackImpl::ping()
{
  return 0;
}

CORBA::Long CallbackImpl::notifyResults(const char * path,
					const corba_profile_t& pb,
					CORBA::Long reqID)
{
  /*
  if (solve_res != GRPC_NO_ERROR)
    CallAsyncMgr::Instance()->setReqErrorCode(reqID, solve_res);
  */
  return 0;
}

CORBA::Long
CallbackImpl::solveResults(const char * path, 
			   const corba_profile_t& pb,
			   CORBA::Long reqID,
			   CORBA::Long solve_res)
{
  // notify solve result availability
  int err = CallAsyncMgr::Instance()->notifyRst(int(reqID), &(const_cast<corba_profile_t&>(pb)));
  if (solve_res != GRPC_NO_ERROR)
    CallAsyncMgr::Instance()->setReqErrorCode(reqID, solve_res);
  return err;
}
