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
  return 0;
}

CORBA::Long CallbackImpl::solveResults(const char * path, 
    const corba_profile_t& pb,
    CORBA::Long reqID)
{
  // notify solve result availability
  return CallAsyncMgr::Instance()->notifyRst(int(reqID), &(const_cast<corba_profile_t&>(pb)));
}
