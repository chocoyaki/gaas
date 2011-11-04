/**
 * @file LogComponent.cc
 *
 * @brief  DIET forwarder implementation - Log component forwarder implementation
 *
 * @author  Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>

void
DIETForwarder::setTagFilter(const ::tag_list_t &tagList, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->setTagFilter(tagList, objString.c_str());
  }

  name = getName(objString);

  ComponentConfigurator_var cfg =
    ORBMgr::getMgr()->
    resolve<ComponentConfigurator, ComponentConfigurator_var>(LOGCOMPCTXT,
                                                              name,
                                                              this->name);
  return cfg->setTagFilter(tagList);
} // setTagFilter

void
DIETForwarder::addTagFilter(const ::tag_list_t &tagList, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->addTagFilter(tagList, objString.c_str());
  }

  name = getName(objString);

  ComponentConfigurator_var cfg =
    ORBMgr::getMgr()->
    resolve<ComponentConfigurator, ComponentConfigurator_var>(LOGCOMPCTXT,
                                                              name,
                                                              this->name);
  return cfg->addTagFilter(tagList);
} // addTagFilter

void
DIETForwarder::removeTagFilter(const ::tag_list_t &tagList,
                               const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->removeTagFilter(tagList, objString.c_str());
  }

  name = getName(objString);

  ComponentConfigurator_var cfg =
    ORBMgr::getMgr()->
    resolve<ComponentConfigurator, ComponentConfigurator_var>(LOGCOMPCTXT,
                                                              name,
                                                              this->name);
  return cfg->removeTagFilter(tagList);
} // removeTagFilter

void
DIETForwarder::test(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->test(objString.c_str());
  }

  name = getName(objString);

  ComponentConfigurator_var cfg =
    ORBMgr::getMgr()->
    resolve<ComponentConfigurator, ComponentConfigurator_var>(LOGCOMPCTXT,
                                                              name,
                                                              this->name);
  return cfg->test();
} // test
