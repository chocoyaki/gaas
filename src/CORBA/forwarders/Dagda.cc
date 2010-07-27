/****************************************************************************/
/* DIET forwarder implementation - Dagda forwarder implementation           */
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
#include <iostream>

using namespace std;

::CORBA::Boolean DIETForwarder::lclIsDataPresent(const char* dataID,
																								 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->lclIsDataPresent(dataID, objString.c_str());
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
		return getPeer()->lvlIsDataPresent(dataID, objString.c_str());
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
		return getPeer()->pfmIsDataPresent(dataID, objString.c_str());
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
		return getPeer()->lclAddData(srcDagda, data, objString.c_str());
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
		return getPeer()->lvlAddData(srcDagda, data, objString.c_str());
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
		return getPeer()->pfmAddData(srcDagda, data, objString.c_str());
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
		return getPeer()->registerFile(data, objString.c_str());
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
		return getPeer()->lclAddContainerElt(containerID, dataID, index,
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
		return getPeer()->lclGetContainerSize(containerID, objString.c_str());
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
		return getPeer()->lclGetContainerElts(containerID, dataIDSeq,
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
		return getPeer()->lclRemData(dataID, objString.c_str());
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
		return getPeer()->lvlRemData(dataID, objString.c_str());
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
		return getPeer()->pfmRemData(dataID, objString.c_str());
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
		return getPeer()->lclUpdateData(srcDagda, data, objString.c_str());
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
		return getPeer()->lvlUpdateData(srcDagda, data, objString.c_str());
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
		return getPeer()->pfmUpdateData(srcDagda, data, objString.c_str());
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
		return getPeer()->lclGetDataDescList(objString.c_str());
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
		return getPeer()->lvlGetDataDescList(objString.c_str());
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
		return getPeer()->pfmGetDataDescList(objString.c_str());
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
		return getPeer()->lclGetDataDesc(dataID, objString.c_str());
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
		return getPeer()->lvlGetDataDesc(dataID, objString.c_str());
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
		return getPeer()->pfmGetDataDesc(dataID, objString.c_str());
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
		return getPeer()->lclReplicate(dataID, ruleTarget, pattern, replace,
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
		return getPeer()->lvlReplicate(dataID, ruleTarget, pattern, replace,
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
		return getPeer()->pfmReplicate(dataID, ruleTarget, pattern, replace,
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
		return getPeer()->writeFile(data, basename, replace,	objString.c_str());
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
		return getPeer()->sendFile(data, destDagda,	objString.c_str());
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
		return getPeer()->recordData(data, dataDesc, replace, offset,
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
		return getPeer()->sendData(ID, destDagda, objString.c_str());
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
		return getPeer()->sendContainer(ID, destDagda, sendElements,
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
		return getPeer()->lvlGetDataManagers(dataID, objString.c_str());
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
		return getPeer()->pfmGetDataManagers(dataID, objString.c_str());
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
		return getPeer()->subscribe(dagdaName, objString.c_str());
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
		return getPeer()->unsubscribe(dagdaName, objString.c_str());
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
		return getPeer()->getID(objString.c_str());
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
		return getPeer()->lockData(dataID, objString.c_str());
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
		return getPeer()->unlockData(dataID, objString.c_str());
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
		return getPeer()->getDataStatus(dataID, objString.c_str());
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
		return getPeer()->getBestSource(destDagda, dataID, objString.c_str());
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
		return getPeer()->checkpointState(objString.c_str());
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
		return getPeer()->subscribeParent(parentID, objString.c_str());
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
		return getPeer()->unsubscribeParent(objString.c_str());
	}
	
	name = getName(objString);
	
	Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
																																this->name);
	return dagda->unsubscribeParent();	
}

#endif
