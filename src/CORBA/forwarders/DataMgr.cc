/****************************************************************************/
/* DIET forwarder implementation - Data manager (DTM) forwarder implem.     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ */
/* $Log$
/* Revision 1.1  2010/07/12 16:08:56  glemahec
/* DIET 2.5 beta 1 - Forwarder implementations
/* */

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
		return peer->putData(argID, me, objString.c_str());
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
		return peer->rmDataRefDataMgr(argID, objString.c_str());
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
		return peer->sendData(arg, objString.c_str());
	}
	
	name = getName(objString);
	
	DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(LOCMGRCTXT, name,
																																		this->name);
	return mgr->sendData(arg);
	
}
