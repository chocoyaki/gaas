/**
 * @file Dagda.cc
 *
 * @brief  DIET forwarder implementation - Dagda forwarder implementation
 *
 * @author  Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>
#include <iostream>


::CORBA::Boolean
DIETForwarder::lclIsDataPresent(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclIsDataPresent(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lclIsDataPresent(dataID);
} // lclIsDataPresent

::CORBA::Boolean
DIETForwarder::lvlIsDataPresent(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lvlIsDataPresent(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lvlIsDataPresent(dataID);
} // lvlIsDataPresent

::CORBA::Boolean
DIETForwarder::pfmIsDataPresent(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->pfmIsDataPresent(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->pfmIsDataPresent(dataID);
} // pfmIsDataPresent

void
DIETForwarder::lclAddData(const char *srcDagda,
                          const ::corba_data_t &data,
                          const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclAddData(srcDagda, data, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
                                                                this->name);
  return dagda->lclAddData(srcDagda, data);
} // lclAddData

void
DIETForwarder::lvlAddData(const char *srcDagda,
                          const ::corba_data_t &data,
                          const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lvlAddData(srcDagda, data, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lvlAddData(srcDagda, data);
} // lvlAddData

void
DIETForwarder::pfmAddData(const char *srcDagda,
                          const ::corba_data_t &data,
                          const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->pfmAddData(srcDagda, data, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->pfmAddData(srcDagda, data);
} // pfmAddData

void
DIETForwarder::registerFile(const ::corba_data_t &data, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->registerFile(data, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
                                                                this->name);
  return dagda->registerFile(data);
} // registerFile

void
DIETForwarder::lclAddContainerElt(const char *containerID,
                                  const char *dataID,
                                  ::CORBA::Long index,
                                  ::CORBA::Long flag,
                                  const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclAddContainerElt(containerID, dataID, index,
                                         flag, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lclAddContainerElt(containerID, dataID, index, flag);
} // lclAddContainerElt

::CORBA::Long
DIETForwarder::lclGetContainerSize(const char *containerID,
                                   const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclGetContainerSize(containerID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lclGetContainerSize(containerID);
} // lclGetContainerSize

void
DIETForwarder::lclGetContainerElts(const char *containerID,
                                   ::SeqString &dataIDSeq,
                                   ::SeqLong &flagSeq,
                                   ::CORBA::Boolean ordered,
                                   const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclGetContainerElts(containerID, dataIDSeq,
                                          flagSeq, ordered,
                                          objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lclGetContainerElts(containerID, dataIDSeq, flagSeq, ordered);
} // lclGetContainerElts

void
DIETForwarder::lclRemData(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclRemData(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lclRemData(dataID);
} // lclRemData

void
DIETForwarder::lvlRemData(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lvlRemData(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lvlRemData(dataID);
} // lvlRemData

void
DIETForwarder::pfmRemData(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->pfmRemData(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->pfmRemData(dataID);
} // pfmRemData

void
DIETForwarder::lclUpdateData(const char *srcDagda,
                             const ::corba_data_t &data,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclUpdateData(srcDagda, data, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lclUpdateData(srcDagda, data);
} // lclUpdateData

void
DIETForwarder::lvlUpdateData(const char *srcDagda,
                             const ::corba_data_t &data,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lvlUpdateData(srcDagda, data, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lvlUpdateData(srcDagda, data);
} // lvlUpdateData

void
DIETForwarder::pfmUpdateData(const char *srcDagda,
                             const ::corba_data_t &data,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->pfmUpdateData(srcDagda, data, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->pfmUpdateData(srcDagda, data);
} // pfmUpdateData

SeqCorbaDataDesc_t *
DIETForwarder::lclGetDataDescList(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclGetDataDescList(objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lclGetDataDescList();
} // lclGetDataDescList

SeqCorbaDataDesc_t *
DIETForwarder::lvlGetDataDescList(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lvlGetDataDescList(objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lvlGetDataDescList();
} // lvlGetDataDescList

SeqCorbaDataDesc_t *
DIETForwarder::pfmGetDataDescList(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->pfmGetDataDescList(objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->pfmGetDataDescList();
} // pfmGetDataDescList

corba_data_desc_t *
DIETForwarder::lclGetDataDesc(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclGetDataDesc(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lclGetDataDesc(dataID);
} // lclGetDataDesc

corba_data_desc_t *
DIETForwarder::lvlGetDataDesc(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lvlGetDataDesc(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name,
                                                                this->name);
  return dagda->lvlGetDataDesc(dataID);
} // lvlGetDataDesc

corba_data_desc_t *
DIETForwarder::pfmGetDataDesc(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->pfmGetDataDesc(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->pfmGetDataDesc(dataID);
} // pfmGetDataDesc

void
DIETForwarder::lclReplicate(const char *dataID,
                            ::CORBA::Long ruleTarget,
                            const char *pattern,
                            ::CORBA::Boolean replace,
                            const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lclReplicate(dataID, ruleTarget, pattern, replace,
                                   objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lclReplicate(dataID, ruleTarget, pattern, replace);
} // lclReplicate

void
DIETForwarder::lvlReplicate(const char *dataID,
                            ::CORBA::Long ruleTarget,
                            const char *pattern,
                            ::CORBA::Boolean replace,
                            const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lvlReplicate(dataID, ruleTarget, pattern, replace,
                                   objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lvlReplicate(dataID, ruleTarget, pattern, replace);
} // lvlReplicate

void
DIETForwarder::pfmReplicate(const char *dataID,
                            ::CORBA::Long ruleTarget,
                            const char *pattern,
                            ::CORBA::Boolean replace,
                            const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->pfmReplicate(dataID, ruleTarget, pattern, replace,
                                   objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->pfmReplicate(dataID, ruleTarget, pattern, replace);
} // pfmReplicate

char *
DIETForwarder::writeFile(const ::SeqChar &data,
                         const char *basename,
                         ::CORBA::Boolean replace,
                         const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->writeFile(data, basename, replace, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->writeFile(data, basename, replace);
} // writeFile

char *
DIETForwarder::sendFile(const ::corba_data_t &data,
                        const char *destDagda,
                        const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->sendFile(data, destDagda, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->sendFile(data, destDagda);
} // sendFile

char *
DIETForwarder::recordData(const ::SeqChar &data,
                          const ::corba_data_desc_t &dataDesc,
                          ::CORBA::Boolean replace,
                          ::CORBA::Long offset,
                          const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->recordData(data, dataDesc, replace, offset,
                                 objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->recordData(data, dataDesc, replace, offset);
} // recordData

char *
DIETForwarder::sendData(const char *ID,
                        const char *destDagda,
                        const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->sendData(ID, destDagda, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->sendData(ID, destDagda);
} // sendData

char *
DIETForwarder::sendContainer(const char *ID,
                             const char *destDagda,
                             ::CORBA::Boolean sendElements,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->sendContainer(ID, destDagda, sendElements,
                                    objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->sendContainer(ID, destDagda, sendElements);
} // sendContainer

SeqString *
DIETForwarder::lvlGetDataManagers(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lvlGetDataManagers(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lvlGetDataManagers(dataID);
} // lvlGetDataManagers

SeqString *
DIETForwarder::pfmGetDataManagers(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->pfmGetDataManagers(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->pfmGetDataManagers(dataID);
} // pfmGetDataManagers

void
DIETForwarder::subscribe(const char *dagdaName, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->subscribe(dagdaName, objString.c_str());
  }
  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  dagda->subscribe(dagdaName);
} // subscribe

void
DIETForwarder::unsubscribe(const char *dagdaName, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->unsubscribe(dagdaName, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->unsubscribe(dagdaName);
} // unsubscribe

char *
DIETForwarder::getID(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getID(objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->getID();
} // getID

void
DIETForwarder::lockData(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->lockData(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->lockData(dataID);
} // lockData

void
DIETForwarder::unlockData(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->unlockData(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->unlockData(dataID);
} // unlockData

Dagda::dataStatus
DIETForwarder::getDataStatus(const char *dataID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getDataStatus(dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->getDataStatus(dataID);
} // getDataStatus

char *
DIETForwarder::getBestSource(const char *destDagda,
                             const char *dataID,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getBestSource(destDagda, dataID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->getBestSource(destDagda, dataID);
} // getBestSource

void
DIETForwarder::checkpointState(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->checkpointState(objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->checkpointState();
} // checkpointState

void
DIETForwarder::subscribeParent(const char *parentID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->subscribeParent(parentID, objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->subscribeParent(parentID);
} // subscribeParent

void
DIETForwarder::unsubscribeParent(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->unsubscribeParent(objString.c_str());
  }

  name = getName(objString);

  Dagda_var dagda =
    ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, name, this->name);
  return dagda->unsubscribeParent();
} // unsubscribeParent
