/****************************************************************************/
/* DIET forwarder implementation - Log component forwarder implementation   */
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

void DIETForwarder::setTagFilter(const ::tag_list_t& tagList,
																 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->setTagFilter(tagList, objString.c_str());
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
		return peer->addTagFilter(tagList, objString.c_str());
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
		return peer->removeTagFilter(tagList, objString.c_str());
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
		return peer->test(objString.c_str());
	}
	
	name = getName(objString);
		
	ComponentConfigurator_var cfg =
	ORBMgr::getMgr()->resolve<ComponentConfigurator, ComponentConfigurator_var>(LOGCOMPCTXT,
																																							name,
																																							this->name);
	return cfg->test();
	
}
