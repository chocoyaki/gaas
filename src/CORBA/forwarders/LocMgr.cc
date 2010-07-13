/****************************************************************************/
/* DIET forwarder implementation - Location mngr (DTM) forwarder implem.    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ */
/* $Log$
/* Revision 1.2  2010/07/13 15:24:13  glemahec
/* Warnings corrections and some robustness improvements
/* */
/* Revision 1.1  2010/07/12 16:08:56  glemahec */
/* DIET 2.5 beta 1 - Forwarder implementations */
/* */

#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>

using namespace std;

::CORBA::ULong DIETForwarder::locMgrSubscribe(const char* lmName,
																							const char* hostname,
																							const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->locMgrSubscribe(lmName, hostname, objString.c_str());
	}
	
	name = getName(objString);
	
	LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																 this->name);
	return mgr->locMgrSubscribe(lmName, hostname);
																																 
}

::CORBA::ULong DIETForwarder::dataMgrSubscribe(const char* dtmName,
																							 const char* hostname,
																							 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->dataMgrSubscribe(dtmName, hostname, objString.c_str());
	}
	
	name = getName(objString);
	
	LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																 this->name);
	return mgr->dataMgrSubscribe(dtmName, hostname);
}

void DIETForwarder::addDataRef(const ::corba_data_desc_t& arg,
															 ::CORBA::ULong cChildID,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->addDataRef(arg, cChildID, objString.c_str());
	}
	
	name = getName(objString);
	
	LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																 this->name);
	return mgr->addDataRef(arg, cChildID);
}

void DIETForwarder::rmDataRefLocMgr(const char* argID,
																		::CORBA::ULong cChildID,
																		const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->rmDataRefLocMgr(argID, cChildID, objString.c_str());
	}
	
	name = getName(objString);
	
	LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																 this->name);
	return mgr->rmDataRefLocMgr(argID, cChildID);
}

void DIETForwarder::updateDataRef(const ::corba_data_desc_t& arg,
																	::CORBA::ULong cChildID,
																	::CORBA::Long upDown,
																	const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->updateDataRef(arg, cChildID, upDown, objString.c_str());
	}
	
	name = getName(objString);
	
	LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																 this->name);
	return mgr->updateDataRef(arg, cChildID, upDown);
}

char* DIETForwarder::whereDataSubtree(const char* argID,
																			const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->whereDataSubtree(argID, objString.c_str());
	}
	
	name = getName(objString);
	
	LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																 this->name);
	return mgr->whereDataSubtree(argID);
}

void DIETForwarder::updateDataProp(const char* argID,
																	 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->updateDataProp(argID, objString.c_str());
	}
	
	name = getName(objString);
	
	LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																 this->name);
	return mgr->updateDataProp(argID);
}

::CORBA::Long DIETForwarder::rm_pdata(const char* argID,
																			const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->rm_pdata(argID, objString.c_str());
	}
	
	name = getName(objString);
	
	LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																 this->name);
	return mgr->rm_pdata(argID);
}
