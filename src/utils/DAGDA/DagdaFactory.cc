/**
 * @file DagdaFactory.cc
 *
 * @brief  Used to obtain this DIET component data manager easil from everywhere.
 *
 * @author  Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <sstream>
#include <string>
#include <algorithm>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

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

#include "DIET_Dagda.h"

size_t
availableDiskSpace(const char *path) {
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
} // availableDiskSpace

DagdaImpl *DagdaFactory::clientDataManager = NULL;
DagdaImpl *DagdaFactory::sedDataManager = NULL;
DagdaImpl *DagdaFactory::agentDataManager = NULL;
DagdaImpl *DagdaFactory::localDataManager = NULL;
std::string DagdaFactory::storageDir = "";

/* If somebody wants to use another ORB than omniORB... */
unsigned long DagdaFactory::defaultMaxMsgSize =
#ifdef __OMNIORB4__
  omniORB::giopMaxMsgSize() - 4096;  // For data structure...
#else
  1073741824; // (1 GB)
#endif

unsigned long DagdaFactory::defaultMaxDiskSpace = 0;
unsigned long DagdaFactory::defaultMaxMemSpace = 0;

DagdaImpl *
DagdaFactory::createDataManager(dagda_manager_type_t type) {
  AdvancedDagdaComponent *result = NULL;

  std::string algorithm;
  bool shareFiles = false;
  CONFIG_BOOL(diet::SHAREFILES, shareFiles);

  NetworkStats *stats = new AvgNetworkStats();

  if (CONFIG_STRING(diet::CACHEALGORITHM, algorithm)) {
    if (algorithm == "LRU") {
      result = new AdvancedDagdaComponent(type, LRU, stats, shareFiles);
    }
    if (algorithm == "LFU") {
      result = new AdvancedDagdaComponent(type, LFU, stats, shareFiles);
    }
    if (algorithm == "FIFO") {
      result = new AdvancedDagdaComponent(type, FIFO, stats, shareFiles);
    }
    if (!result) {
      WARNING("Warning: " << algorithm
                          << " is not a valid (implemented) cache"
                          << " management algorithm.");
      result = new AdvancedDagdaComponent(type, stats, shareFiles);
    }
  } else {
    result = new AdvancedDagdaComponent(type, stats, shareFiles);
  }

  return result;
} // createDataManager

std::string
DagdaFactory::getDefaultStorageDir() {
  return "/tmp";
}

const char *
DagdaFactory::getStorageDir() {
  DIR *dp;
  struct stat tmpStat;

  // if storageDir is not set, get default instead
  if (!CONFIG_STRING(diet::STORAGEDIR, storageDir)) {
    storageDir = DagdaFactory::getDefaultStorageDir();
  }

  /* Test if the directory exists */
  if ((dp = opendir(storageDir.c_str())) == NULL) {
    // create the directory with rwxr-xr-x permissions
    if (mkdir(storageDir.c_str(), 493)) {
      ERROR_EXIT("The DAGDA storage directory '"
                 << storageDir << "' cannot be opened");
    }
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
} // getStorageDir

unsigned long
DagdaFactory::getMaxMsgSize() {
  unsigned long maxMsgSize;

  if (!CONFIG_ULONG(diet::MAXMSGSIZE, maxMsgSize)) {
    maxMsgSize = defaultMaxMsgSize;
  }

  return maxMsgSize;
}

unsigned long
DagdaFactory::getMaxDiskSpace() {
  unsigned long maxDiskSpace;

  if (!CONFIG_ULONG(diet::MAXDISKSPACE, maxDiskSpace)) {
    maxDiskSpace = defaultMaxDiskSpace;
  }

  return maxDiskSpace;
}

unsigned long
DagdaFactory::getMaxMemSpace() {
  unsigned long maxMemSpace;

  if (!CONFIG_ULONG(diet::MAXMEMSPACE, maxMemSpace)) {
    maxMemSpace = defaultMaxMemSpace;
  }

  return maxMemSpace;
}

const char *
DagdaFactory::getParentName() {
  std::string parentName;
  std::string agentType;
  bool resultName = CONFIG_STRING(diet::PARENTNAME, parentName);
  bool resultType = CONFIG_AGENT(diet::AGENTTYPE, agentType);

  if (resultName && resultType
      && (agentType == "MA" || agentType == "DIET_MASTER_AGENT")) {
    WARNING("Agent data manager found a parent name "
            "for the agent, but this agent is a Master Agent");
    return NULL;
  }

  if (!resultName) {
    return NULL;
  }

  parentName.append("_DAGDA");
  return CORBA::string_dup(parentName.c_str());
} // getParentName

const char *
DagdaFactory::getAgentName() {
  std::string agentName;

  if (!CONFIG_STRING(diet::NAME, agentName)) {
    return NULL;
  }

  agentName.append("_DAGDA");
  return CORBA::string_dup(agentName.c_str());
}

/* Transformation function for the host name. */
int
chg(int c) {
  if (c == '.') {
    return '-';
  }
  return c;
}

const char *
DagdaFactory::getClientName() {
  std::ostringstream name;
  char host[256];

  gethostname(host, 256);
  host[255] = '\0';

  std::transform(host, host + strlen(host), host, chg);

  name << "DAGDA-client-" << host << "-" << getpid();
  return CORBA::string_dup(name.str().c_str());
} // getClientName

const char *
DagdaFactory::getSeDName() {
  std::ostringstream name;
  char host[256];

  gethostname(host, 256);
  host[255] = '\0';

  std::transform(host, host + strlen(host), host, chg);

  name << "DAGDA-SeD-" << host << "-" << getpid();
  return CORBA::string_dup(name.str().c_str());
} // getSeDName

const char *
DagdaFactory::getDefaultName() {
  std::ostringstream name;
  char host[256];

  gethostname(host, 256);
  host[255] = '\0';

  std::transform(host, host + strlen(host), host, chg);

  name << "DAGDA-Agent-" << host << "-" << getpid();
  return CORBA::string_dup(name.str().c_str());
} // getDefaultName

DagdaImpl *
DagdaFactory::getClientDataManager() {
  if (!clientDataManager) {
    clientDataManager = createDataManager(DGD_CLIENT_MNGR);

    clientDataManager->init(getClientName(), NULL, getStorageDir(),
                            getMaxMsgSize(), getMaxDiskSpace(),
                            getMaxMemSpace());
  }
  localDataManager = clientDataManager;
  return clientDataManager;
} // getClientDataManager

DagdaImpl *
DagdaFactory::getSeDDataManager() {
  if (!sedDataManager) {
    const char *parentName = getParentName();
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

  std::string backupFile;
  if (CONFIG_STRING(diet::DATABACKUPFILE, backupFile)) {
    localDataManager->setStateFile(backupFile);
    bool restore = false;
    CONFIG_BOOL(diet::RESTOREONSTART, restore);
    if (restore) {
      localDataManager->restoreState();
    }
  }

  return sedDataManager;
} // getSeDDataManager

DagdaImpl *
DagdaFactory::getAgentDataManager() {
  if (!agentDataManager) {
    const char *parentName = getParentName();
    const char *name = getAgentName();

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

  std::string backupFile;
  if (CONFIG_STRING(diet::DATABACKUPFILE, backupFile)) {
    localDataManager->setStateFile(backupFile);
    bool restore = false;
    CONFIG_BOOL(diet::RESTOREONSTART, restore);
    if (restore) {
      localDataManager->restoreState();
    }
  }

  return agentDataManager;
} // getAgentDataManager

DagdaImpl *
DagdaFactory::getDataManager() {
  if (!localDataManager) {
    throw "No local data manager was instanciated.";
  }

  return localDataManager;
}

void
DagdaFactory::reset() {
  // Reset everything to default value
  // TODO: Do not delete managers, otherwise we get a segfault...
  dagda_reset();
  clientDataManager = NULL;
  sedDataManager = NULL;
  agentDataManager = NULL;
  localDataManager = NULL;
  storageDir = "";
  defaultMaxDiskSpace = 0;
  defaultMaxMemSpace = 0;
} // reset
