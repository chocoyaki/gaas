/**
 * @file DagdaFactory.hh
 *
 * @brief  Used to obtain this DIET component data manager easil from everywhere.
 *
 * @author  Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _DAGDAFACTORY_HH_
#define _DAGDAFACTORY_HH_

#include <string>
#include "DagdaImpl.hh"

/* A class factory to simplify the extensions of the Dagda object. */
extern "C" class __declspec (dllexport) DagdaFactory {
public:
  static DagdaImpl *
  getClientDataManager();

  static DagdaImpl *
  getSeDDataManager();

  static DagdaImpl *
  getAgentDataManager();

  static DagdaImpl *
  getDataManager();

  static const char *
  getStorageDir();

  static unsigned long
  getMaxMsgSize();

  static unsigned long
  getMaxDiskSpace();

  static unsigned long
  getMaxMemSpace();

  static const char *
  getParentName();

  static const char *
  getClientName();

  static const char *
  getSeDName();

  static const char *
  getAgentName();

  static const char *
  getDefaultName();

 static void
  reset();

private:
  static DagdaImpl *clientDataManager;
  static DagdaImpl *sedDataManager;
  static DagdaImpl *agentDataManager;
  static DagdaImpl *localDataManager;
  static std::string storageDir;

  static DagdaImpl *
  createDataManager(dagda_manager_type_t type);

  static unsigned long defaultMaxMsgSize;
  static unsigned long defaultMaxDiskSpace;
  static unsigned long defaultMaxMemSpace;

  static std::string
  getDefaultStorageDir();
};

#endif /* ifndef _DAGDAFACTORY_HH_ */
