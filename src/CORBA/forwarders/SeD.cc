/****************************************************************************/
/* DIET forwarder implementation - SeD forwarder implementation             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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

::CORBA::Long DIETForwarder::checkContract(::corba_estimation_t& estimation,
																					 const ::corba_pb_desc_t& pb,
																					 const char* objName)
{
	string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->checkContract(estimation, pb, objString.c_str());
	}
	
	name = getName(objString);
	
	SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name,
																												this->name);
	return sed->checkContract(estimation, pb);
}

void DIETForwarder::updateTimeSinceLastSolve(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->updateTimeSinceLastSolve(objString.c_str());
	}
	
	name = getName(objString);

	SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name,
																												this->name);
	return sed->updateTimeSinceLastSolve();
	
}

::CORBA::Long DIETForwarder::solve(const char* path,
																	 ::corba_profile_t& pb,
																	 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->solve(path, pb, objString.c_str());
	}
	
	name = getName(objString);

	SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name,
																												this->name);
	return sed->solve(path, pb);
}

void DIETForwarder::solveAsync(const char* path,
															 const ::corba_profile_t& pb,
															 const char* volatileclientPtr,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->solveAsync(path, pb, volatileclientPtr, objString.c_str());
	}
	
	name = getName(objString);

	SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name,
																												this->name);
	return sed->solveAsync(path, pb, volatileclientPtr);
}

#ifdef HAVE_DAGDA
char* DIETForwarder::getDataMgrID(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->getDataMgrID(objString.c_str());
	}
	
	name = getName(objString);

	SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name,
																												this->name);
	return sed->getDataMgrID();
}
#endif
