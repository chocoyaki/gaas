/***********************************************************/
/* Dagda component factory.                                */
/* Used to obtain this DIET component data manager easily  */
/* from everywhere in the DIET source code.                */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log $
 *
 ***********************************************************/

#include "DagdaFactory.hh"
#include "Parsers.hh"
#include "debug.hh"
#ifdef __OMNIORB4__
#include "omniORB4/omniORB.h"
#endif
#include "sys/statvfs.h"

#include "AdvancedDagdaComponent.hh"
#include "CacheAlgorithms.hh"
#include "NetworkStats.hh"

#include <sstream>
#include <string>

size_t availableDiskSpace(const char* path) {
  struct statvfs buffer;
  
  if (!statvfs(path, &buffer)) {
    size_t blksize, blocks, freeblks, disk_size, used, free;
    blksize = buffer.f_bsize;
    blocks = buffer.f_blocks;
    freeblks = buffer.f_bfree;
    disk_size = blocks * blksize;
    free = freeblks * blksize;
    used = disk_size - free;
	return free;
  } 
  return 0;
}

DagdaImpl* DagdaFactory::clientDataManager = NULL;
DagdaImpl* DagdaFactory::sedDataManager = NULL;
DagdaImpl* DagdaFactory::agentDataManager = NULL;
DagdaImpl* DagdaFactory::localDataManager = NULL;
std::string DagdaFactory::storageDir;

std::string DagdaFactory::defaultStorageDir("/tmp");
/* If somebody wants to use another ORB than omniORB... */
unsigned long DagdaFactory::defaultMaxMsgSize =
#ifdef __OMNIORB4__
  omniORB::giopMaxMsgSize() - 4096; // For data structure...
#else
  1073741824; // (1 GB)
#endif

unsigned long DagdaFactory::defaultMaxDiskSpace = 0;
unsigned long DagdaFactory::defaultMaxMemSpace = 0;

DagdaImpl* DagdaFactory::createDataManager(dagda_manager_type_t type) {
  AdvancedDagdaComponent* result=NULL;

  char* algorithm = (char*)
    Parsers::Results::getParamValue(Parsers::Results::CACHEALGORITHM);
  unsigned int* shareFiles = (unsigned int*)
    Parsers::Results::getParamValue(Parsers::Results::SHAREFILES);
  bool share;

  NetworkStats* stats = new AvgNetworkStats();

  if (shareFiles==NULL)
    share = false;
  else share = (*shareFiles==1);
  
  if (algorithm!=NULL) {
    if (strcmp(algorithm, "LRU")==0) {
      result = new AdvancedDagdaComponent(type, LRU, stats, share);
    }
    if (strcmp(algorithm, "LFU")==0) {
      result = new AdvancedDagdaComponent(type, LFU, stats, share);
    }
    if (strcmp(algorithm, "FIFO")==0) {
      result = new AdvancedDagdaComponent(type, FIFO, stats, share);
    }
    if (result==NULL) {
      WARNING("Warning: " << algorithm << " is not a valid (implemented) cache"
	    << " management algorithm.");
      result = new AdvancedDagdaComponent(type, stats, share);
    }
  } else  {
    result = new AdvancedDagdaComponent(type, stats, share);
  }

  return result;
}

const char* DagdaFactory::getStorageDir() {
  if (storageDir.empty()) {
    char* storage = (char*)
	  Parsers::Results::getParamValue(Parsers::Results::STORAGEDIR);
    if (storage==NULL) storageDir = defaultStorageDir;
	else storageDir = storage;
  }
  
  return storageDir.c_str();
}

unsigned long DagdaFactory::getMaxMsgSize() {
  unsigned long* maxMsgSize = (unsigned long*)
    Parsers::Results::getParamValue(Parsers::Results::MAXMSGSIZE);
  if (maxMsgSize==NULL) maxMsgSize = &defaultMaxMsgSize;
  return *maxMsgSize;
}

unsigned long DagdaFactory::getMaxDiskSpace() {
  unsigned long* maxDiskSpace = (unsigned long*)
    Parsers::Results::getParamValue(Parsers::Results::MAXDISKSPACE);
  //defaultMaxDiskSpace = availableDiskSpace(getStorageDir());
  if (maxDiskSpace==NULL) maxDiskSpace = &defaultMaxDiskSpace;
  return *maxDiskSpace;
}

unsigned long DagdaFactory::getMaxMemSpace() {
  unsigned long* maxMemSpace = (unsigned long*)
    Parsers::Results::getParamValue(Parsers::Results::MAXMEMSPACE);
  string storageDir(getStorageDir());
  //defaultMaxMemSpace = availableDiskSpace(storageDir.c_str());
  if (maxMemSpace==NULL) maxMemSpace = &defaultMaxMemSpace;
  return *maxMemSpace;
}

const char* DagdaFactory::getParentName() {
  static std::string result;
  char* prtName = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (prtName==NULL)
    return NULL;
  result=prtName;
  result+="_DAGDA";
  return result.c_str();
}

const char* DagdaFactory::getAgentName() {
  static std::string result;
  char* agtName = (char*)
	  Parsers::Results::getParamValue(Parsers::Results::NAME);
  if (agtName==NULL)
    return NULL;
  result=agtName;
  result+="_DAGDA";
  return result.c_str();
}

const char* DagdaFactory::getClientName() {
  static std::ostringstream name;
  char host[256];
   
  gethostname(host, 256);
  host[255]='\0';
	
  name << "DAGDA-client." << host << "." << getpid();
  return name.str().c_str();
}

const char* DagdaFactory::getSeDName() {
  static std::ostringstream name;
  char host[256];
   
  gethostname(host, 256);
  host[255]='\0';
	
  name << "DAGDA-SeD." << host << "." << getpid();
  return name.str().c_str();
}

const char* DagdaFactory::getDefaultName() {
  static std::ostringstream name;
  char host[256];
   
  gethostname(host, 256);
  host[255]='\0';
	
  name << "DAGDA-Agent." << host << "." << getpid();
  return name.str().c_str();
}

DagdaImpl* DagdaFactory::getClientDataManager() {
  if (clientDataManager==NULL) {
    clientDataManager = createDataManager(DGD_CLIENT_MNGR);
	
    clientDataManager->init(getClientName(), NULL, getStorageDir(),
			    getMaxMsgSize(), getMaxDiskSpace(),
			    getMaxMemSpace());
  }
  localDataManager = clientDataManager;
  return clientDataManager;
}

DagdaImpl* DagdaFactory::getSeDDataManager() {
  char* backupFile = (char*)
    Parsers::Results::getParamValue(Parsers::Results::DATABACKUPFILE);
  unsigned int* restoreOnStart = (unsigned int*)
    Parsers::Results::getParamValue(Parsers::Results::RESTOREONSTART);
  bool restore = false;
  
  if (restoreOnStart!=NULL)
    restore = (*restoreOnStart==1);
	
  if (sedDataManager == NULL) {
    const char* parentName = getParentName();
	if (parentName == NULL) {
	  WARNING("SeD data manager didn't find the name of the agent in the configuration file.");
	}
	
	sedDataManager = createDataManager(DGD_SED_MNGR);
	
	sedDataManager->init(CORBA::string_dup(getSeDName()), CORBA::string_dup(parentName),
                       CORBA::string_dup(getStorageDir()),
	                     getMaxMsgSize(), getMaxDiskSpace(),
						           getMaxMemSpace());
  }
  localDataManager = sedDataManager;

  if (backupFile!=NULL) {
    localDataManager->setStateFile(backupFile);
	if (restore)
	  localDataManager->restoreState();
  }

  return sedDataManager;
}

DagdaImpl* DagdaFactory::getAgentDataManager() {
  char* backupFile = (char*)
    Parsers::Results::getParamValue(Parsers::Results::DATABACKUPFILE);
  unsigned int* restoreOnStart = (unsigned int*)
    Parsers::Results::getParamValue(Parsers::Results::RESTOREONSTART);
  bool restore = false;
  
  if (restoreOnStart!=NULL)
    restore = (*restoreOnStart==1);

  if (agentDataManager == NULL) {
    const char* parentName = getParentName();
	  const char* name = getAgentName();
	  if (name==NULL) {
	    WARNING("Agent data manager didn't find a valid name for the agent in the configuration file.");
	    name = getDefaultName();
	  }
	  //cout << "name = " << name << endl;
	  agentDataManager = createDataManager(DGD_AGENT_MNGR);
	
	  agentDataManager->init(name, parentName, getStorageDir(),
	                         getMaxMsgSize(), getMaxDiskSpace(),
                           getMaxMemSpace());
  }
  localDataManager = agentDataManager;
  
  if (backupFile!=NULL) {
    localDataManager->setStateFile(backupFile);
	if (restore)
	  localDataManager->restoreState();
  }

  return agentDataManager;
}

DagdaImpl* DagdaFactory::getDataManager() {
  if (localDataManager==NULL)
    throw "No local data manager was instanciated.";
  return localDataManager;
}
