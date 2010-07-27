/****************************************************************************/
/* DIET forwarder implementation - Client forwarder implementation          */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2010/07/27 16:16:49  glemahec
 * Forwarders robustness
 *
 * Revision 1.3  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 * Revision 1.2  2010/07/13 15:24:13  glemahec
 * Warnings corrections and some robustness improvements
 *
 * Revision 1.1  2010/07/12 16:08:56  glemahec
 * DIET 2.5 beta 1 - Forwarder implementations
 ****************************************************************************/

#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>

using namespace std;


::CORBA::Long DIETForwarder::notifyResults(const char* path,
																					 const ::corba_profile_t& pb,
																					 ::CORBA::Long reqID,
																					 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->notifyResults(path, pb, reqID, objString.c_str());
	}
	
	name = getName(objString);
	
	Callback_var cb = ORBMgr::getMgr()->resolve<Callback, Callback_var>(CLIENTCTXT, name,
																																			this->name);
	return cb->notifyResults(path, pb, reqID);
}

::CORBA::Long DIETForwarder::solveResults(const char* path,
																					const ::corba_profile_t& pb,
																					::CORBA::Long reqID,
																					::CORBA::Long result,
																					const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->solveResults(path, pb, reqID, result, objString.c_str());
	}
	
	name = getName(objString);
	
	Callback_var cb = ORBMgr::getMgr()->resolve<Callback, Callback_var>(CLIENTCTXT, name,
																																			this->name);
	return cb->solveResults(path, pb, reqID, result);
}
