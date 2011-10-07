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
 * Revision 1.5  2011/05/09 13:10:10  bdepardo
 * Added method diet_get_SeD_services to retreive the services of a SeD given
 * its name
 *
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

::CORBA::Long
 DIETForwarder::checkContract(::corba_estimation_t& estimation,
                              const ::corba_pb_desc_t& pb,
                              const char* objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->checkContract(estimation, pb, objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->checkContract(estimation, pb);
}

void
DIETForwarder::updateTimeSinceLastSolve(const char* objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->updateTimeSinceLastSolve(objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->updateTimeSinceLastSolve();

}

::CORBA::Long
DIETForwarder::solve(const char* path,
                     ::corba_profile_t& pb,
                     const char* objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->solve(path, pb, objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->solve(path, pb);
}

void
DIETForwarder::solveAsync(const char* path,
                          const ::corba_profile_t& pb,
                          const char* volatileclientPtr,
                          const char* objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->solveAsync(path, pb, volatileclientPtr, objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->solveAsync(path, pb, volatileclientPtr);
}

char*
DIETForwarder::getDataMgrID(const char* objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getDataMgrID(objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->getDataMgrID();
}

SeqCorbaProfileDesc_t*
DIETForwarder::getSeDProfiles(::CORBA::Long& length,
                              const char* objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getSeDProfiles(length, objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->getSeDProfiles(length);
}
