/****************************************************************************/
/* DIET forwarder implementation - Dagda forwarder implementation           */
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
#include <iostream>

using namespace std;

::CORBA::Boolean DIETForwarder::lclIsDataPresent(const char* dataID,
																								 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclIsDataPresent(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclIsDataPresent(dataID);
}

::CORBA::Boolean DIETForwarder::lvlIsDataPresent(const char* dataID,
																								 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lvlIsDataPresent(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lvlIsDataPresent(dataID);
}

::CORBA::Boolean DIETForwarder::pfmIsDataPresent(const char* dataID,
																								 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->pfmIsDataPresent(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->pfmIsDataPresent(dataID);
}

void DIETForwarder::lclAddData(const char* srcDagda,
															 const ::corba_data_t& data,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclAddData(srcDagda, data, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclAddData(srcDagda, data);
}

void DIETForwarder::lvlAddData(const char* srcDagda,
															 const ::corba_data_t& data,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lvlAddData(srcDagda, data, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lvlAddData(srcDagda, data);
}

void DIETForwarder::pfmAddData(const char* srcDagda, 
															 const ::corba_data_t& data,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->pfmAddData(srcDagda, data, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->pfmAddData(srcDagda, data);
}

void DIETForwarder::registerFile(const ::corba_data_t& data,
																 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->registerFile(data, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->registerFile(data);
}

void DIETForwarder::lclAddContainerElt(const char* containerID,
																			 const char* dataID,
																			 ::CORBA::Long index,
																			 ::CORBA::Long flag,
																			 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclAddContainerElt(containerID, dataID, index,
																		flag, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclAddContainerElt(containerID, dataID, index, flag);
}

::CORBA::Long DIETForwarder::lclGetContainerSize(const char* containerID,
																								 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclGetContainerSize(containerID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclGetContainerSize(containerID);
}

void DIETForwarder::lclGetContainerElts(const char* containerID,
																				::SeqString& dataIDSeq,
																				::SeqLong& flagSeq,
																				::CORBA::Boolean ordered,
																				const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclGetContainerElts(containerID, dataIDSeq,
																		 flagSeq, ordered,
																		 objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclGetContainerElts(containerID, dataIDSeq, flagSeq, ordered);
}

void DIETForwarder::lclRemData(const char* dataID,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclRemData(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclRemData(dataID);
}

void DIETForwarder::lvlRemData(const char* dataID,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lvlRemData(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lvlRemData(dataID);
}

void DIETForwarder::pfmRemData(const char* dataID,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->pfmRemData(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->pfmRemData(dataID);
}

void DIETForwarder::lclUpdateData(const char* srcDagda,
																	const ::corba_data_t& data,
																	const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclUpdateData(srcDagda, data, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclUpdateData(srcDagda, data);
}

void DIETForwarder::lvlUpdateData(const char* srcDagda,
																	const ::corba_data_t& data,
																	const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lvlUpdateData(srcDagda, data, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lvlUpdateData(srcDagda, data);
}

void DIETForwarder::pfmUpdateData(const char* srcDagda,
																	const ::corba_data_t& data,
																	const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->pfmUpdateData(srcDagda, data, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->pfmUpdateData(srcDagda, data);
}

SeqCorbaDataDesc_t* DIETForwarder::lclGetDataDescList(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclGetDataDescList(objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclGetDataDescList();
}

SeqCorbaDataDesc_t* DIETForwarder::lvlGetDataDescList(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lvlGetDataDescList(objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lvlGetDataDescList();
}

SeqCorbaDataDesc_t* DIETForwarder::pfmGetDataDescList(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->pfmGetDataDescList(objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->pfmGetDataDescList();
}

corba_data_desc_t* DIETForwarder::lclGetDataDesc(const char* dataID,
																								 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclGetDataDesc(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclGetDataDesc(dataID);
}

corba_data_desc_t* DIETForwarder::lvlGetDataDesc(const char* dataID,
																								 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lvlGetDataDesc(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lvlGetDataDesc(dataID);	
}

corba_data_desc_t* DIETForwarder::pfmGetDataDesc(const char* dataID,
																								 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->pfmGetDataDesc(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->pfmGetDataDesc(dataID);
}

void DIETForwarder::lclReplicate(const char* dataID,
																 ::CORBA::Long ruleTarget,
																 const char* pattern,
																 ::CORBA::Boolean replace,
																 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lclReplicate(dataID, ruleTarget, pattern, replace,
															objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lclReplicate(dataID, ruleTarget, pattern, replace);
}

void DIETForwarder::lvlReplicate(const char* dataID,
																 ::CORBA::Long ruleTarget,
																 const char* pattern,
																 ::CORBA::Boolean replace,
																 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lvlReplicate(dataID, ruleTarget, pattern, replace,
															objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lvlReplicate(dataID, ruleTarget, pattern, replace);
}

void DIETForwarder::pfmReplicate(const char* dataID,
																 ::CORBA::Long ruleTarget,
																 const char* pattern,
																 ::CORBA::Boolean replace,
																 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->pfmReplicate(dataID, ruleTarget, pattern, replace,
															objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->pfmReplicate(dataID, ruleTarget, pattern, replace);
}

char* DIETForwarder::writeFile(const ::SeqChar& data,
															 const char* basename,
															 ::CORBA::Boolean replace,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->writeFile(data, basename, replace,	objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->writeFile(data, basename, replace);
}

char* DIETForwarder::sendFile(const ::corba_data_t& data,
															const char* destDagda,
															const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->sendFile(data, destDagda,	objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->sendFile(data, destDagda);
}

char* DIETForwarder::recordData(const ::SeqChar& data,
																const ::corba_data_desc_t& dataDesc,
																::CORBA::Boolean replace,
																::CORBA::Long offset,
																const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->recordData(data, dataDesc, replace, offset,
														objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->recordData(data, dataDesc, replace, offset);
}

char* DIETForwarder::sendData(const char* ID,
															const char* destDagda,
															const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->sendData(ID, destDagda, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->sendData(ID, destDagda);
}

char* DIETForwarder::sendContainer(const char* ID,
																	 const char* destDagda,
																	 ::CORBA::Boolean sendElements,
																	 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->sendContainer(ID, destDagda, sendElements,
															 objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->sendContainer(ID, destDagda, sendElements);
}

SeqString* DIETForwarder::lvlGetDataManagers(const char* dataID,
																						 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lvlGetDataManagers(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lvlGetDataManagers(dataID);
}

SeqString* DIETForwarder::pfmGetDataManagers(const char* dataID,
																						 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->pfmGetDataManagers(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->pfmGetDataManagers(dataID);
}

void DIETForwarder::subscribe(const char* dagdaName,
															const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->subscribe(dagdaName, objString.c_str());
	}
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	dagda->subscribe(dagdaName);
}

void DIETForwarder::unsubscribe(const char* dagdaName,
																const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->unsubscribe(dagdaName, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->unsubscribe(dagdaName);
}

char* DIETForwarder::getID(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->getID(objString.c_str());
	}
	
	name = getName(objString);

	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->getID();
}

void DIETForwarder::lockData(const char* dataID,
														 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->lockData(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->lockData(dataID);
}

void DIETForwarder::unlockData(const char* dataID,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->unlockData(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->unlockData(dataID);
}

Dagda::dataStatus DIETForwarder::getDataStatus(const char* dataID,
																							 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->getDataStatus(dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->getDataStatus(dataID);
}

char* DIETForwarder::getBestSource(const char* destDagda,
																	 const char* dataID,
																	 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->getBestSource(destDagda, dataID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->getBestSource(destDagda, dataID);
}

void DIETForwarder::checkpointState(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->checkpointState(objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->checkpointState();
}

#ifdef HAVE_DYNAMICS
void DIETForwarder::subscribeParent(const char* parentID,
																		const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->subscribeParent(parentID, objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->subscribeParent(parentID);
}

void DIETForwarder::unsubscribeParent(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->unsubscribeParent(objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->unsubscribeParent();	
}

#endif
