/****************************************************************************/
/* DIET forwarder implementation - Data manager (DTM) forwarder implem.     */
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

void DIETForwarder::putData(const char* argID,
														const char* me,
														const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->putData(argID, me, objString.c_str());
	}
	
	name = getName(objString);
	
	DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(LOCMGRCTXT, name,
																																		this->name);
	return mgr->putData(argID, me);
	
}

::CORBA::Long DIETForwarder::rmDataRefDataMgr(const char* argID,
																							const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->rmDataRefDataMgr(argID, objString.c_str());
	}
	
	name = getName(objString);
	
	DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(LOCMGRCTXT, name,
																																		this->name);
	return mgr->rmDataRefDataMgr(argID);
}

void DIETForwarder::sendData(::corba_data_t& arg,
														 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->sendData(arg, objString.c_str());
	}
	
	name = getName(objString);
	
	DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(LOCMGRCTXT, name,
																																		this->name);
	return mgr->sendData(arg);
	
}
