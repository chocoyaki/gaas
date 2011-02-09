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
 * $Log$
 * Revision 1.22  2011/02/09 11:04:00  bdepardo
 * Fixed a bug in getClientDataManager
 *
 * Revision 1.21  2011/02/08 09:44:18  bdepardo
 * getParentName returns 0 if the agent is an MA and the parentName property
 * is set nevertheless
 *
 * Revision 1.20  2011/02/04 15:20:49  hguemar
 * fixes to new configuration parser
 * some cleaning
 *
 * Revision 1.19  2011/02/02 13:32:28  hguemar
 * configuration parsers: environment variables, command line arguments, file configuration parser
 * moved Dagda and dietAgent (yay auto-generated help) to new configuration subsystem
 *
 * Revision 1.18  2010/09/02 17:29:49  bdepardo
 * Fixed a memory corruption when linking two DIET library:
 * defaultStorageDir cannot be a static string, we need to use a static method
 *
 * Revision 1.17  2010/07/27 10:24:34  glemahec
 * Improve robustness & general performance
 *
 * Revision 1.16  2010/07/12 16:14:12  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.15  2010/04/20 12:00:43  glemahec
 * Ajout de l option de compilation TRANSFER_PROGRESSION => Extension de l API DAGDA pour compatibilite services de gestion de fichiers.
 *
 * Revision 1.14  2010/03/08 13:59:36  bisnard
 * bug correction for agent name getters (removed static vars)
 *
 * Revision 1.13  2008/11/12 15:55:39  bdepardo
 * Added a test on DAGDA storage directory. Now reports an error if:
 * - the directory does not exist
 * - the directory does not have sufficient rights (rwx)
 *
 * Revision 1.12  2008/11/07 14:32:14  bdepardo
 * Headers correction
 *
 *
 ***********************************************************/

#include "DagdaFactory.hh"
#include "configuration.hh"
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
#include <algorithm>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

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
std::string DagdaFactory::storageDir = "";

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

  const std::string& algorithm = CONFIG(diet::CACHEALGORITHM);
  unsigned int shareFiles = simple_cast<unsigned int>(CONFIG(diet::SHAREFILES));
  bool share;

  NetworkStats* stats = new AvgNetworkStats();

  if (!shareFiles) {
      share = false;
  } else {
      // TODO: file sharing option is enabled only if shareFiles=1 ?
      share = (shareFiles == 1);
  }
  
  if (!algorithm.empty()) {
      if (algorithm == "LRU") {
	  result = new AdvancedDagdaComponent(type, LRU, stats, share);
      }
      if (algorithm == "LFU") {
	  result = new AdvancedDagdaComponent(type, LFU, stats, share);
      }
      if (algorithm == "FIFO") {
	  result = new AdvancedDagdaComponent(type, FIFO, stats, share);
      }
      if (!result) {
	  WARNING("Warning: " << algorithm 
		  << " is not a valid (implemented) cache"
		  << " management algorithm.");
	  result = new AdvancedDagdaComponent(type, stats, share);
      }
  } else  {
      result = new AdvancedDagdaComponent(type, stats, share);
  }

  return result;
}

std::string DagdaFactory::getDefaultStorageDir() {
    return "/tmp";
}

const char* DagdaFactory::getStorageDir() {
  DIR *dp;
  struct stat tmpStat;

  storageDir = CONFIG(diet::STORAGEDIR);
  // if storageDir is empty, get default instead
  if (storageDir.empty()) {
      storageDir = DagdaFactory::getDefaultStorageDir();
  }

  /* Test if the directory exists */
  if((dp  = opendir(storageDir.c_str())) == NULL) {
      // create the directory with rwxr-xr-x permissions
      if (mkdir(storageDir.c_str(), 493)) 
      ERROR_EXIT("The DAGDA storage directory '" 
		 << storageDir << "' cannot be opened");
  } else {
      closedir(dp);
  }

  /* Test if the directory has sufficient rights */
  if (!stat(storageDir.c_str(), &tmpStat)) {
      if (!(tmpStat.st_mode & S_IRUSR) || 
	  !(tmpStat.st_mode & S_IWUSR) || 
	  !(tmpStat.st_mode & S_IXUSR)) {
	  ERROR_EXIT("The DAGDA storage directory '" 
		     << storageDir 
		     << "' does not have sufficient rights (rwx)");
      }
  }

  return storageDir.c_str();
}

unsigned long DagdaFactory::getMaxMsgSize() {
    unsigned long maxMsgSize = 
	simple_cast<unsigned long>(CONFIG(diet::MAXMSGSIZE));

    if (!maxMsgSize) {
	maxMsgSize = defaultMaxMsgSize;
    }

    return maxMsgSize;
}

unsigned long DagdaFactory::getMaxDiskSpace() {
    unsigned long maxDiskSpace = 
	simple_cast<unsigned long>(CONFIG(diet::MAXDISKSPACE));

    if (!maxDiskSpace) {
	maxDiskSpace = defaultMaxDiskSpace;
    }

    return maxDiskSpace;
}

unsigned long DagdaFactory::getMaxMemSpace() {
    unsigned long maxMemSpace = 
	simple_cast<unsigned long>(CONFIG(diet::MAXMEMSPACE));

    if (!maxMemSpace) {
	maxMemSpace = defaultMaxMemSpace;
    }

    return maxMemSpace;
}

const char* DagdaFactory::getParentName() {
    std::string result = CONFIG(diet::PARENTNAME);
    std::string agentType = CONFIG(diet::AGENTTYPE);

    if (!result.empty() && !agentType.empty() && 
        (agentType == "MA" || agentType == "DIET_MASTER_AGENT")) {
      WARNING("Agent data manager found a parent name "
              "for the agent, but this agent is a Master Agent");
      return 0;
    }

    if (result.empty()) {
	return 0;
    }

    result.append("_DAGDA");
    return CORBA::string_dup(result.c_str());
}

const char* DagdaFactory::getAgentName() {
    std::string result = CONFIG(diet::NAME);

    if (result.empty()) {
	return 0;
    }

    result.append("_DAGDA");
    return CORBA::string_dup(result.c_str());
}

/* Transformation function for the host name. */
int chg(int c) {
    if (c=='.') return '-';
    return c;
}

const char* DagdaFactory::getClientName() {
    std::ostringstream name;
    char host[256];
    
    gethostname(host, 256);
    host[255]='\0';
	
    std::transform(host, host+strlen(host), host, chg);
    
    name << "DAGDA-client-" << host << "-" << getpid();
    return CORBA::string_dup(name.str().c_str());
}

const char* DagdaFactory::getSeDName() {
    std::ostringstream name;
    char host[256];
    
    gethostname(host, 256);
    host[255]='\0';
    
    std::transform(host, host+strlen(host), host, chg);
    
    name << "DAGDA-SeD-" << host << "-" << getpid();
    return CORBA::string_dup(name.str().c_str());
}

const char* DagdaFactory::getDefaultName() {
    std::ostringstream name;
    char host[256];

    gethostname(host, 256);
    host[255]='\0';
    
    std::transform(host, host+strlen(host), host, chg);
    
    name << "DAGDA-Agent-" << host << "-" << getpid();
    return CORBA::string_dup(name.str().c_str());
}

DagdaImpl* DagdaFactory::getClientDataManager() {
    if (!clientDataManager) {
      clientDataManager = createDataManager(DGD_CLIENT_MNGR);
      
      clientDataManager->init(getClientName(), NULL, getStorageDir(),
                              getMaxMsgSize(), getMaxDiskSpace(),
                              getMaxMemSpace());
    }
    localDataManager = clientDataManager;
    return clientDataManager;
}

DagdaImpl* DagdaFactory::getSeDDataManager() {
    const std::string& backupFile = CONFIG(diet::DATABACKUPFILE);
    unsigned int restoreOnStart = 
	simple_cast<unsigned int>(CONFIG(diet::RESTOREONSTART));
    
    bool restore(false);
   
    if (!restoreOnStart) {
	restore = (restoreOnStart == 1);
    }
  
    if (!sedDataManager) {
	const char* parentName = getParentName();
	if (!parentName) {
	    WARNING("SeD data manager didn't find the name of the agent "
		    "in the configuration file.");
	}

	sedDataManager = createDataManager(DGD_SED_MNGR);

	sedDataManager->init(CORBA::string_dup(getSeDName()),
			     CORBA::string_dup(parentName),
			     CORBA::string_dup(getStorageDir()),
	                     getMaxMsgSize(), getMaxDiskSpace(),
			     getMaxMemSpace());
    }
    localDataManager = sedDataManager;
    
    if (!backupFile.empty()) {
	localDataManager->setStateFile(backupFile);
	if (restore)
	    localDataManager->restoreState();
    }
    
    return sedDataManager;
}

DagdaImpl* DagdaFactory::getAgentDataManager() {
    const std::string& backupFile = CONFIG(diet::DATABACKUPFILE);
    unsigned int restoreOnStart = 
	simple_cast<unsigned int>(CONFIG(diet::RESTOREONSTART));
    
    bool restore(false);
   
    if (!restoreOnStart) {
	restore = (restoreOnStart == 1);
    }
    
    if (!agentDataManager) {
	const char* parentName = getParentName();
	const char* name = getAgentName();

	if (!name) {
          WARNING("Agent data manager didn't find a valid name "
                  "for the agent in the configuration file.");
          name = getDefaultName();
	}
	agentDataManager = createDataManager(DGD_AGENT_MNGR);
	
	agentDataManager->init(name, parentName, getStorageDir(),
			       getMaxMsgSize(), getMaxDiskSpace(),
			       getMaxMemSpace());
    }
    localDataManager = agentDataManager;
    
    if (backupFile.empty()) {
	localDataManager->setStateFile(backupFile);
	if (restore) {
	    localDataManager->restoreState();
	}
	
    }

    return agentDataManager;
}

DagdaImpl* DagdaFactory::getDataManager() {
    if (!localDataManager) {
	throw "No local data manager was instanciated.";
    }
    
    return localDataManager;
}
