/***********************************************************/
/* Dagda component factory header file.                    */
/* Used to obtain this DIET component data manager easily  */
/* from everywhere in the DIET source code.                */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2008/11/07 14:32:14  bdepardo
 * Headers correction
 *
 *
 ***********************************************************/

#ifndef _DAGDAFACTORY_HH_
#define _DAGDAFACTORY_HH_

#include "DagdaImpl.hh"

/* A class factory to simplify the extensions of the Dagda object. */
class DagdaFactory {
private:
  static DagdaImpl* clientDataManager;
  static DagdaImpl* sedDataManager;
  static DagdaImpl* agentDataManager;
  static DagdaImpl* localDataManager;
  static std::string storageDir;
  static DagdaImpl* createDataManager(dagda_manager_type_t type);

  static unsigned long defaultMaxMsgSize;
  static unsigned long defaultMaxDiskSpace;
  static unsigned long defaultMaxMemSpace;
  static std::string defaultStorageDir;
public:
  static DagdaImpl* getClientDataManager();
  static DagdaImpl* getSeDDataManager();
  static DagdaImpl* getAgentDataManager();
  static DagdaImpl* getDataManager();
  static const char* getStorageDir();
  static unsigned long getMaxMsgSize();
  static unsigned long getMaxDiskSpace();
  static unsigned long getMaxMemSpace();
  static const char* getParentName();
  static const char* getClientName();
  static const char* getSeDName();
  static const char* getAgentName();
  static const char* getDefaultName();
};

#endif
