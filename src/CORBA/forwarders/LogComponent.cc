/****************************************************************************/
/* DIET forwarder implementation - Log component forwarder implementation   */
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

void DIETForwarder::setTagFilter(const ::tag_list_t& tagList,
																 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->setTagFilter(tagList, objString.c_str());
	}
	
	name = getName(objString);
	
	ComponentConfigurator_var cfg =
		ORBMgr::getMgr()->resolve<ComponentConfigurator, ComponentConfigurator_var>(LOGCOMPCTXT,
																																								name,
																																								this->name);
	return cfg->setTagFilter(tagList);
}

void DIETForwarder::addTagFilter(const ::tag_list_t& tagList,
																 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->addTagFilter(tagList, objString.c_str());
	}
	
	name = getName(objString);
		
	ComponentConfigurator_var cfg =
	ORBMgr::getMgr()->resolve<ComponentConfigurator, ComponentConfigurator_var>(LOGCOMPCTXT,
																																							name,
																																							this->name);
	return cfg->addTagFilter(tagList);
	
}

void DIETForwarder::removeTagFilter(const ::tag_list_t& tagList,
																		const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->removeTagFilter(tagList, objString.c_str());
	}
	
	name = getName(objString);
	
	ComponentConfigurator_var cfg =
	ORBMgr::getMgr()->resolve<ComponentConfigurator, ComponentConfigurator_var>(LOGCOMPCTXT,
																																							name,
																																							this->name);
	return cfg->removeTagFilter(tagList);
	
}

void DIETForwarder::test(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->test(objString.c_str());
	}
	
	name = getName(objString);
		
	ComponentConfigurator_var cfg =
	ORBMgr::getMgr()->resolve<ComponentConfigurator, ComponentConfigurator_var>(LOGCOMPCTXT,
																																							name,
																																							this->name);
	return cfg->test();
	
}
