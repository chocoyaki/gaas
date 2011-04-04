/***********************************************************/
/* Dagda component implementation.                         */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log$
 * Revision 1.51  2011/04/04 13:38:02  bdepardo
 * Close comment, and define FNM_CASEFOLD only if it is not already defined
 *
 * Revision 1.50  2011/04/04 13:18:37  ecaron
 * New defintion of FNM_CASEFOLD for AIX compatibility (AIX 5.1 or later)
 *
 * Revision 1.49  2011/03/18 17:37:45  bdepardo
 * Add dagda_reset method to reset internal variables.
 * This is used for allowing multiple consecutive diet_initialize/diet_finalize
 *
 * Revision 1.48  2011/03/18 08:43:32  hguemar
 * fix memory leak in DagdaImpl: getID() method returns a CORBA::String which is not always deallocated (patch from Gael Le Mahec)
 *
 * Revision 1.47  2011/03/16 22:02:34  bdepardo
 * Unbind dagda element in destructor
 *
 * Revision 1.46  2011/02/28 16:57:15  bdepardo
 * Do not unsubscribe from parent in destructor as it raises an exception
 *
 * Revision 1.45  2011/02/24 16:53:30  bdepardo
 * Use TRACE_TEXT instead of cout
 *
 * Revision 1.44  2011/02/09 17:15:53  bdepardo
 * Be less verbose
 *
 * Revision 1.43  2011/02/08 16:53:50  bdepardo
 * Fixed dynamics. They didn't work anymore
 *
 * Revision 1.42  2011/02/04 14:52:52  bdepardo
 * Removed unused variable
 *
 * Revision 1.41  2011/01/22 16:27:52  glemahec
 * Bug workflows/#156 correction (#155 too ?) - pointers management was bugged when reusing INOUT data.
 *
 * Revision 1.40  2011/01/21 18:12:02  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 *
 * Revision 1.39  2010/12/17 09:48:01  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.38  2010/10/15 07:25:08  glemahec
 * Correction. Needs more tests
 *
 * Revision 1.37  2010/07/12 16:14:12  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.36  2010/04/20 12:00:43  glemahec
 * Ajout de l option de compilation TRANSFER_PROGRESSION => Extension de l API DAGDA pour compatibilite services de gestion de fichiers.
 *
 * Revision 1.35  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.34  2009/11/06 14:42:44  bisnard
 * modified filename generator to avoid loosing the file extension
 * during transfers and to suppress double slash in pathes
 *
 * Revision 1.33  2009/10/26 09:11:26  bdepardo
 * Added method for dynamically managing the hierarchy:
 * - void subscribeParent(const char * parentID)
 * - void unsubscribeParent()
 *
 * Revision 1.32  2009/10/02 07:42:42  bisnard
 * reduced trace verbosity for containers
 *
 * Revision 1.31  2009/04/17 08:50:49  bisnard
 * added handling of container empty elements
 *
 * Revision 1.30  2009/03/27 09:04:10  bisnard
 * - replace container size attr by dynamic value
 * - added exception mgmt
 *
 * Revision 1.29  2009/01/16 13:36:13  bisnard
 * changed Container constructor signature
 * modified scope of getDataRelationMgr to protected
 * lock container during element addition
 * check lock before sending container
 *
 * Revision 1.28  2008/12/09 12:06:21  bisnard
 * changed container download method to transfer only the list of IDs
 * (each container element must be downloaded separately)
 *
 * Revision 1.27  2008/11/08 19:12:39  bdepardo
 * A few warnings removal
 *
 * Revision 1.26  2008/11/07 14:32:14  bdepardo
 * Headers correction
 *
 *
 ***********************************************************/


#include <unistd.h>
#include <cerrno>
#include <fnmatch.h>

#include <list>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>

#include "Dagda.hh"
#include "common_types.hh"

#include "ORBMgr.hh"
#include "DIET_data.h"
#include "DIET_data_internal.hh"
#include "debug.hh"
#include "marshalling.hh"

#include "DagdaImpl.hh"
#include "DagdaFactory.hh"
#include "Container.hh"
#if DAGDA_PROGRESSION
#include "Transfers.hh"
#endif

#if HAVE_ADVANCED_UUID
#include <uuid/uuid.h>
#endif

/**  FNM_CASEFOLD for AIX (available only in AIX 5.1 or later). */
#if __aix__
#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD FNM_IGNORECASE
#endif // !FNM_CASEFOLD
#endif // __aix__

char DagdaImpl::NoID[] = "NoID";

DagdaImpl::~DagdaImpl() {
}

/* CORBA */
char* DagdaImpl::getHostname() {
  return CORBA::string_dup(hostname);
}


/* Choose where to store the data. */
void DagdaImpl::setDataPath(const char* path) {
  dataPath = path;
}

/* Where are the data stored. */
const char* DagdaImpl::getDataPath() {
  return dataPath.c_str();
}

void DagdaImpl::setMaxMsgSize(const size_t maxMsgSize) {
  this->maxMsgSize = maxMsgSize;
}

size_t DagdaImpl::getMaxMsgSize() {
  return maxMsgSize;
}

void DagdaImpl::setDiskMaxSpace(const size_t diskMaxSpace) {
  this->diskMaxSpace = diskMaxSpace;
}

size_t DagdaImpl::getDiskMaxSpace() {
  return diskMaxSpace;
}

void DagdaImpl::setMemMaxSpace(const size_t memMaxSpace) {
  this->memMaxSpace = memMaxSpace;
}

size_t DagdaImpl::getMemMaxSpace() {
  return memMaxSpace;
}

/* Write the data to a file. Can be done in several time. */
/* CORBA */
char* DagdaImpl::writeFile(const SeqChar& data, const char* basename,
                           CORBA::Boolean replace) {
  std::ofstream file;
  std::string filename;

  filename = getDataPath();
  filename.append("/");
  filename.append(basename);
	
  if (replace) {
    file.open(filename.c_str());
  } else {
    file.open(filename.c_str(), std::ios_base::app);
  }
	
  if (!file.is_open()) throw Dagda::WriteError(errno);
  file.write((const char*) data.get_buffer(), data.length());
	
  if (file.bad()) throw Dagda::WriteError(errno);
  TRACE_TEXT(TRACE_ALL_STEPS, "Write " << data.length() << " bytes on the file "
             << filename << endl);
  return CORBA::string_dup(filename.c_str());
}

string gen_filename(string basename) {
  unsigned long int idx = basename.find_last_of('/');
  if (idx!=string::npos)
    basename=basename.substr(idx+1);
  unsigned long int dot = basename.find_last_of('.');
  string suffix;
  if (dot!=string::npos) {
    suffix=basename.substr(dot);
    basename=basename.substr(0, dot);
  }
  ostringstream name;
#if HAVE_ADVANCED_UUID
  uuid_t uuid;
  char ID[37];
	
  uuid_generate(uuid);
  uuid_unparse(uuid, ID);
	
  name << basename << "-" << ID << suffix;
#else
  name << basename << "." << getpid() << suffix;
#endif
  return name.str();
}

string conditional_filename(string basename) {
  unsigned long int idx = basename.find_last_of('/');
  if (idx!=string::npos)
    basename=basename.substr(idx+1);
	
  if (fnmatch("*-????????""-????""-????""-????????????*",
              basename.c_str(), FNM_CASEFOLD)==0) {
    return basename;
  }
  return gen_filename(basename);
}

/* Send a file to a node. */
/* CORBA */
char* DagdaImpl::sendFile(const corba_data_t &data, const char* destName) {
  Dagda_ptr dest = ORBMgr::getMgr()->resolve<Dagda, Dagda_ptr>(DAGDACTXT, destName);
	
  if (data.desc.specific.file().path==NULL) // path not initialized.
    throw Dagda::InvalidPathName(data.desc.id.idNumber, "null");
	
  std::ifstream file(data.desc.specific.file().path);
  if (!file.is_open()) {// Unable to open the file.
    throw Dagda::ReadError(errno);
  }
	
  TRACE_TEXT(TRACE_ALL_STEPS, "*** Sending file " << data.desc.specific.file().path
             << " (" << data.desc.id.idNumber << ")" << endl);
	
  unsigned long wrote = 0;
  unsigned long fileSize = data.desc.specific.file().size;
  string basename(data.desc.specific.file().path);
	
  /* TEST pour debuggage */
  //string name;
  //unsigned long int idx = basename.find_last_of('/');
  //if (idx!=string::npos)
  //  name=basename.substr(idx);
  //string name = gen_filename(basename); // <= Version originale.
  string name = conditional_filename(basename);
  /***********************/
	
  unsigned long nBlocks =  fileSize / getMaxMsgSize() + 1;
	
  bool replace = true;
  char* distPath = NULL;
#if DAGDA_PROGRESSION
  char* destHost = dest->getHostname();
  Transfers::getInstance()->newTransfer(string(data.desc.id.idNumber)+">>"+destHost,
                                        fileSize);
#endif
#ifdef USE_LOG_SERVICE
  if (getLogComponent())
    getLogComponent()->logDataBeginTransfer(data.desc.id.idNumber, dest->getID());
#endif
  for (unsigned long i=0; i<nBlocks; ++i) {
    SeqChar buffer;
    unsigned long toSend = (fileSize-wrote > getMaxMsgSize() ?
                            getMaxMsgSize():(fileSize-wrote));
    TRACE_TEXT(TRACE_ALL_STEPS, "\tSend " << toSend << " bytes..." << endl);
    buffer.length(toSend);
    file.read((char*) buffer.get_buffer(false), toSend);
    if (file.bad()) {
      throw Dagda::ReadError(errno);
    }
    // Send the data.
    if (distPath!=NULL) CORBA::string_free(distPath);
    distPath=dest->writeFile(buffer, name.c_str(), replace);
		
    wrote+=toSend;
    replace=false;
#if DAGDA_PROGRESSION
    Transfers::getInstance()->incProgress(string(data.desc.id.idNumber)+">>"+destHost);
#endif
  }
#ifdef USE_LOG_SERVICE
  if (getLogComponent())
    getLogComponent()->logDataEndTransfer(data.desc.id.idNumber, dest->getID());
#endif
  return distPath;
}

/* Warning : this function MUST be protected by a dataMutex.lock()/unlock() pair. */
/* New version: memory access optimization. */
char* DagdaImpl::recordData(const SeqChar& data,
                            const corba_data_desc_t& dataDesc,
                            CORBA::Boolean replace, CORBA::Long offset) {
  string dataId(dataDesc.id.idNumber);
	
  if (getData()->find(dataId)==getData()->end()) {
    TRACE_TEXT(TRACE_MAIN_STEPS, "Add data " << dataDesc.id.idNumber
               << endl);
    corba_data_t newData;
    newData.desc = dataDesc;
    lockData(dataId.c_str());
    (*getData())[dataId]=newData;
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "\tRecord " << data.length() << " bytes for the data "
             << dataDesc.id.idNumber << endl);
  if (replace) {
    if ((*getData())[dataId].value.length()!=data_sizeof(&dataDesc))
      (*getData())[dataId].value.length(data_sizeof(&dataDesc));
  }
  
  CORBA::Char* buffer;
  // If the pointer is managed by DAGDA, ask the control.
  if ((*getData())[dataId].value.release())
    buffer = (*getData())[dataId].value.get_buffer(true);
  else
    buffer = (*getData())[dataId].value.get_buffer(false);
  
  memcpy(buffer+offset, data.get_buffer(), data.length());
  (*getData())[dataId].value.replace(data_sizeof(&dataDesc), data_sizeof(&dataDesc), buffer, true);
	
  return CORBA::string_dup((*getData())[dataId].desc.id.idNumber);
}


/* CORBA */
char* DagdaImpl::sendData(const char* dataId, const char* destName) {
  Dagda_ptr dest = ORBMgr::getMgr()->resolve<Dagda,Dagda_ptr>(DAGDACTXT, destName);
	
  dataMutex.lock();
  if (getData()->find(dataId)==getData()->end()) {
    dataMutex.unlock();
    throw Dagda::DataNotFound(dataId);
  }
	
  if ((*getDataStatus())[dataId]==Dagda::downloading) {
    dataMutex.unlock();
    throw Dagda::UnavailableData(dataId);
  }
	
  TRACE_TEXT(TRACE_ALL_STEPS, "*** Sending data " << dataId << endl);
	
  corba_data_t* data = getData(dataId);
  unsigned long wrote = 0;
  unsigned long dataSize = data_sizeof(&data->desc);
  unsigned long nBlocks =  dataSize / getMaxMsgSize() + 1;
	
  bool replace = true;
  char* distID=NULL;
	
  for (unsigned long i=0; i<nBlocks; ++i) {
    unsigned long toSend = (dataSize-wrote > getMaxMsgSize() ?
                            getMaxMsgSize():(dataSize-wrote));
    TRACE_TEXT(TRACE_ALL_STEPS, "\tSend " <<  toSend << " bytes..." << endl);
    SeqChar buffer(toSend, toSend, data->value.get_buffer()+wrote);
		
    if (distID!=NULL) CORBA::string_free(distID);
		
    distID=dest->recordData(buffer, data->desc, replace, i*getMaxMsgSize());
		
    wrote+=toSend;
    replace=false;
  }
	
  dest->unlockData(distID);
  dataMutex.unlock();
  return distID;
}

/* CORBA */
char* DagdaImpl::sendContainer(const char* containerID, const char* destName,
                               CORBA::Boolean sendElements)
{
  Dagda_ptr dest = ORBMgr::getMgr()->resolve<Dagda, Dagda_ptr>(DAGDACTXT, destName);
	
  dataMutex.lock();
  if (getData()->find(containerID)==getData()->end()) {
    dataMutex.unlock();
    throw Dagda::DataNotFound(containerID);
  }
  if ((*getDataStatus())[containerID]==Dagda::downloading) {
    dataMutex.unlock();
    throw Dagda::UnavailableData(containerID);
  }
  dataMutex.unlock();
  TRACE_TEXT(TRACE_ALL_STEPS, "*** Sending container " << containerID << endl);
  dest->unlockData(containerID);
  // transfer the container elements
  Container *container = new Container(containerID,_this(),containerRelationMgr);
  char *distID = container->send(destName,sendElements);
	
  return distID;
}

void DagdaImpl::lockData(const char* dataID) {
  dataStatusMutex.lock();
  (*getDataStatus())[dataID]=Dagda::downloading;
  dataStatusMutex.unlock();
}

void DagdaImpl::unlockData(const char* dataID) {
  dataStatusMutex.lock();
  (*getDataStatus())[dataID]=Dagda::ready;
  dataStatusMutex.unlock();
}

Dagda::dataStatus DagdaImpl::getDataStatus(const char* dataID) {
  Dagda::dataStatus ret;
  dataStatusMutex.lock();
  ret=(*getDataStatus())[dataID];
  dataStatusMutex.unlock();
  return ret;
}

void DagdaImpl::setDataStatus(const char* dataID, Dagda::dataStatus status) {
  dataStatusMutex.lock();
  (*getDataStatus())[dataID] = status;
  dataStatusMutex.unlock();
}

#ifdef HAVE_DYNAMICS
void SimpleDagdaImpl::subscribeParent(const char * parentID) {
  if (parentID!=NULL) {
    string dagdaParentID = parentID;
    dagdaParentID += "_DAGDA";
    parentID = CORBA::string_dup(dagdaParentID.c_str());
    
    Dagda_ptr parent;
    try {
      parent = ORBMgr::getMgr()->resolve<Dagda, Dagda_ptr>(DAGDACTXT, parentID);
    } catch (...) {
      parent = Dagda::_nil();
    }
    
    /* We only change the parent if it exists */
    if (!CORBA::is_nil(parent)) {
      setParent(parent);
    }
  }
	
  if (getParent() != NULL) {
    try {
      getParent()->subscribe(getID());
    } catch (CORBA::Exception& e) {
      WARNING("Exception caught while subscribing to parent");
    }
    TRACE_TEXT(TRACE_ALL_STEPS, "*** Subscribed to parent: "<< parentID << endl);
  }
}

void SimpleDagdaImpl::unsubscribeParent() {
  if (!CORBA::is_nil(this->getParent())) {
    try {
      getParent()->unsubscribe(getID());
    } catch (CORBA::Exception& e) {
      WARNING("Exception caught while unsubscribing from parent: " << e._name());
    }
    TRACE_TEXT(TRACE_ALL_STEPS, "*** Unsubscribed from parent" << endl);
  }
}
#endif // HAVE_DYNAMICS


/* CORBA */
void SimpleDagdaImpl::subscribe(const char* myName) {
  Dagda_ptr me = ORBMgr::getMgr()->resolve<Dagda, Dagda_ptr>(DAGDACTXT, myName);

  string name(me->getID());
  childrenMutex.lock();
  map<string, Dagda_ptr>::iterator it = getChildren()->find(name);
	
  if (it!=getChildren()->end()) getChildren()->erase(name);

  (*getChildren())[name]=Dagda::_duplicate(me);

  childrenMutex.unlock();
}

/* CORBA */
void SimpleDagdaImpl::unsubscribe(const char* myName) {
  Dagda_ptr me = ORBMgr::getMgr()->resolve<Dagda, Dagda_ptr>(DAGDACTXT, myName);
	
  string name(me->getID());
  childrenMutex.lock();
  map<string, Dagda_ptr>::iterator it = getChildren()->find(name);
	
  if (it!=getChildren()->end()) getChildren()->erase(name);
  childrenMutex.unlock();
}

SimpleDagdaImpl::~SimpleDagdaImpl() {
  //CORBA::string_free(this->ID);
  // unsubscribeParent();
  ORBMgr::getMgr()->unbind(DAGDACTXT, getIDstr());
  ORBMgr::getMgr()->fwdsUnbind(DAGDACTXT, getIDstr());
  delete containerRelationMgr;
}

// Initialize this data manager.
int SimpleDagdaImpl::init(const char* ID, const char* parentID,
                          const char* dataPath, const unsigned long maxMsgSize,
                          const unsigned long diskMaxSpace,
                          const unsigned long memMaxSpace) {
  setID(CORBA::string_dup(ID));
  containerRelationMgr = new DataRelationMgr();
	
  /*if (ORBMgr::bindObjToName(_this(), ORBMgr::DATAMGR, getID())) {
    ERROR("Dagda: could not declare myself as " << getID(), 1);
    }*/
  ORBMgr::getMgr()->bind(DAGDACTXT, getIDstr(), _this(), true);
  ORBMgr::getMgr()->fwdsBind(DAGDACTXT, getIDstr(),
                             ORBMgr::getMgr()->getIOR(_this()));
	
  if (parentID==NULL) setParent(NULL);
  else {
    parentID = CORBA::string_dup(parentID);

    Dagda_ptr parent;
    try {
      parent = ORBMgr::getMgr()->resolve<Dagda,Dagda_ptr>(DAGDACTXT, parentID);
    } catch (...) {
      parent = Dagda::_nil();
    }
		
    if (CORBA::is_nil(parent)) {
      setParent(NULL);
    } else {
      setParent(parent);
    }
  }
	
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "## Launch Dagda data manager " << getIDstr() << endl);
  TRACE_TEXT(TRACE_ALL_STEPS,
             "IOR : " << ORBMgr::getMgr()->getIOR(_this()) << endl);
	

  if (getParent()!=NULL) getParent()->subscribe(getID());

  setMaxMsgSize(maxMsgSize);
  setDiskMaxSpace(diskMaxSpace);
  setMemMaxSpace(memMaxSpace);
  setDataPath(dataPath);
  return 0;
}

/* Return true if the data is on the node. */
/* CORBA */
CORBA::Boolean SimpleDagdaImpl::lclIsDataPresent(const char* dataID) {
  return isDataPresent(dataID);
}

/* Return true if the data is present locally. */
/* CORBA */
CORBA::Boolean SimpleDagdaImpl::lvlIsDataPresent(const char* dataID) {
  std::map<string,Dagda_ptr>::iterator itch;
	
  if (lclIsDataPresent(dataID)) return true;
  childrenMutex.lock();
	
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      if ((*itch).second->lvlIsDataPresent(dataID)) {
        childrenMutex.unlock();
        return true;
      }
      ++itch;
    } catch (CORBA::COMM_FAILURE& e1) {
      getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
  return false;
}

/* If the data is present locally return true else calls parent to find
   it. */
/* CORBA */
CORBA::Boolean SimpleDagdaImpl::pfmIsDataPresent(const char* dataID) {
  if (lclIsDataPresent(dataID)) return true;
  if (getParent()==NULL)
    return lvlIsDataPresent(dataID);
  return getParent()->pfmIsDataPresent(dataID);
}

// This method downloads the data from the node src.
char* SimpleDagdaImpl::downloadData(Dagda_ptr src, const corba_data_t& data) {
  if (data.desc.specific._d()==DIET_FILE)
    return src->sendFile(data, getID());
  else if (data.desc.specific._d()==DIET_CONTAINER)
    return src->sendContainer(data.desc.id.idNumber, getID(), false);
  else
    return src->sendData(data.desc.id.idNumber, getID());
}

// Add a data locally.
/* CORBA */
void SimpleDagdaImpl::lclAddData(const char* srcName, const corba_data_t& data) {
  Dagda_ptr src = ORBMgr::getMgr()->resolve<Dagda,Dagda_ptr>(DAGDACTXT, srcName);
	
  TRACE_TEXT(TRACE_ALL_STEPS, "Add the data " << data.desc.id.idNumber
             << " locally." << endl);
  if (getIDstr() != src->getID()) {
    if (data.desc.specific._d()==DIET_FILE) {
      if (getDiskMaxSpace()!=0 && getUsedDiskSpace()+data_sizeof(&data.desc)>getDiskMaxSpace()) {
        throw Dagda::NotEnoughSpace(getDiskMaxSpace()-getUsedDiskSpace());
      }

      char* path = downloadData(src, data);
      if (path) {
        corba_data_t newData(data);
        newData.desc.specific.file().path=path;
        addData(newData);
        unlockData(data.desc.id.idNumber); //
        useDiskSpace(data.desc.specific.file().size);
      }
    } else if (data.desc.specific._d()==DIET_CONTAINER) {
      char* dataID;
      corba_data_t* inserted;
      inserted = addData(data);
      dataID = downloadData(src, data); // non-recursive (downloads only the id list)
      unlockData(dataID);
      useMemSpace(inserted->value.length());
      //         if (TRACE_LEVEL >= TRACE_ALL_STEPS)
      //           getContainerRelationMgr()->displayContent();
    } else {
      if (getMemMaxSpace()!=0 && getUsedMemSpace()+data_sizeof(&data.desc)>getMemMaxSpace()) {
        throw Dagda::NotEnoughSpace(getMemMaxSpace()-getUsedMemSpace());
      }

      char* dataID;
      corba_data_t* inserted;
      inserted = addData(data);
      dataID = downloadData(src, data);
      unlockData(dataID);
      useMemSpace(inserted->value.length());
    }
  }
}

// Add at this level.
// Plusieurs mani�res de faire �a :
// * on copie en local (m�thode choisie).
// * on essaye de copier en local. Si ce n'est pas possible, on essaye sur
//   les noeuds enfants.
// * Utilisation d'une heuristique pour choisir le noeud.
/* CORBA */
void SimpleDagdaImpl::lvlAddData(const char* src, const corba_data_t& data) {
  lclAddData(src, data);
}

// Simple implementation.
// Many way to do that... Ex :
// * Control if there is enough local storage capacity to store the data.
//   Otherwise ask to "parent" or "children" to store it. (Recursively).
/* CORBA */
void SimpleDagdaImpl::pfmAddData(const char* src, const corba_data_t& data) {
  // Control if the data is already on the platform ???
  lvlAddData(src, data);
}

// Simple implementation
/* CORBA */
void SimpleDagdaImpl::lclAddContainerElt(const char* containerID,
                                         const char* dataID,
                                         CORBA::Long index,
                                         CORBA::Long flag) {
  Container *container = new Container(containerID,_this(),containerRelationMgr);
  const char* eltID = dataID ? dataID : NoID;
  lockData(containerID);
  container->addData(eltID,index,flag);
  unlockData(containerID);
}

/* CORBA */
CORBA::Long SimpleDagdaImpl::lclGetContainerSize(const char* containerID) {
  Container *container = new Container(containerID,_this(),containerRelationMgr);
  return (CORBA::Long) container->size();
}

/* CORBA */
void SimpleDagdaImpl::lclGetContainerElts(const char* containerID,
                                          SeqString& dataIDSeq,
                                          SeqLong& flagSeq,
                                          CORBA::Boolean ordered) {
  Container *container = new Container(containerID,_this(),containerRelationMgr);
  try {
    container->getAllElements(dataIDSeq,flagSeq,ordered);
  } catch (CORBA::SystemException& e) {
    cerr << "lclGetContainerElts Caught a CORBA " << e._name() << " exception ("
         << e.NP_minorString() << ")" << endl ;
  } catch (...) {
    cerr << "UNMANAGED exception in lclGetContainerElts" << endl;
    throw;
  }
}

// Simple implementation.
/* CORBA */
void SimpleDagdaImpl::lclRemData(const char* dataID) {
  remData(dataID);
}

// SeD have no child...
/* CORBA */
void SimpleDagdaImpl::lvlRemData(const char* dataID) {
  std::map<string, Dagda_ptr>::iterator itch;
  lclRemData(dataID);
	
  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      if ((*itch).second->lvlIsDataPresent(dataID))
        (*itch).second->lvlRemData(dataID);
      ++itch;
    } catch (CORBA::COMM_FAILURE& e1) {
      getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
}

// Simple implementation :
// Should take into account the previous remove.
/* CORBA */
void SimpleDagdaImpl::pfmRemData(const char* dataID) {
  if (getParent()==NULL)
    lvlRemData(dataID);
  else
    getParent()->pfmRemData(dataID);
}

// Simple implementation : remove and add.
/* CORBA */
void SimpleDagdaImpl::lclUpdateData(const char* srcName, const corba_data_t& data) {
  Dagda_ptr src = ORBMgr::getMgr()->resolve<Dagda,Dagda_ptr>(DAGDACTXT, srcName);
  if (getIDstr() == src->getID()) return;
  lclRemData(data.desc.id.idNumber);
  lclAddData(srcName, data);
}

//
/* CORBA */
void SimpleDagdaImpl::lvlUpdateData(const char* srcName,
                                    const corba_data_t& data) {
  std::map<string,Dagda_ptr>::iterator itch;
  if (lclIsDataPresent(data.desc.id.idNumber))
    lclUpdateData(srcName, data);
	
  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      (*itch).second->lvlUpdateData(srcName, data);
      ++itch;
    } catch (CORBA::COMM_FAILURE& e1) {
      getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
}

// Simple implementation.
/* CORBA */
void SimpleDagdaImpl::pfmUpdateData(const char* srcName, const corba_data_t& data) {
  if (getParent()==NULL)
    lvlUpdateData(srcName, data);
  else
    getParent()->pfmUpdateData(srcName, data);
}

// To replicate a data on the nodes matching the pattern.
// Pattern matching function.
bool match(const char* str, const char* pattern) {
  int ret;
  ret = fnmatch(pattern, str, FNM_CASEFOLD);
	
  return (ret==0);
}


// Replication function
void replicate(void* paramPtr) {
  char* dataID = (char*) paramPtr;
  corba_data_t data;
  string srcName;
  corba_data_desc_t* desc;
  DagdaImpl* manager = DagdaFactory::getDataManager();
	
  try {
    srcName = manager->getBestSource(manager->getIDstr().c_str(), dataID);
  } catch (Dagda::DataNotFound& ex) {
    WARNING("Trying to replicate a data that does not exist on the platform.");
    return;
  }
	
  desc = manager->pfmGetDataDesc(dataID);
  data.desc = *desc;
	
  manager->lclAddData(srcName.c_str(), data);
}

void replicateIfPossible(void* paramPtr) {
  char* dataID = (char*) paramPtr;
  corba_data_desc_t* desc;
  DagdaImpl* manager = DagdaFactory::getDataManager();
	
  try {
    desc = manager->pfmGetDataDesc(dataID);
  } catch (Dagda::DataNotFound& ex) {
    WARNING("Trying to replicate a data that does not exist on the platform.");
    return;
  }
	
  if (desc->specific._d()==DIET_FILE) {
    if (manager->getDiskMaxSpace()-manager->getUsedDiskSpace()>=data_sizeof(desc))
      replicate(paramPtr);
  } else
    if (manager->getMemMaxSpace()-manager->getUsedMemSpace()>=data_sizeof(desc))
      replicate(paramPtr);
}

void SimpleDagdaImpl::lclReplicate(const char* dataID, CORBA::Long target,
                                   const char* pattern, CORBA::Boolean replace) {
  bool replic;
  void* ID = CORBA::string_dup(const_cast<char*>(dataID));
	
  if (lclIsDataPresent(dataID)) return;
	
  if (target==0) // Target is the hostname.
    replic = match(getHostname(), pattern);
  else // Target is the ID
    replic = match(getIDstr().c_str(), pattern);
	
  if (replic) {
    if (replace)
      omni_thread::create(replicate, ID);
    else
      omni_thread::create(replicateIfPossible, ID);
  }
}

void SimpleDagdaImpl::lvlReplicate(const char* dataID, CORBA::Long target,
                                   const char* pattern, CORBA::Boolean replace) {
  std::map<string,Dagda_ptr>::iterator itch;
	
  lclReplicate(dataID, target, pattern, replace);
	
  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      (*itch).second->lvlReplicate(dataID, target, pattern, replace);
      ++itch;
    } catch (CORBA::COMM_FAILURE& e1) {
      getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
}

void SimpleDagdaImpl::pfmReplicate(const char* dataID, CORBA::Long target,
                                   const char* pattern, CORBA::Boolean replace) {
  if (getParent()==NULL)
    lvlReplicate(dataID, target, pattern, replace);
  else
    getParent()->lvlReplicate(dataID, target, pattern, replace);
}


// To get the descriptions of the data locally stored.
/* CORBA */
SeqCorbaDataDesc_t* SimpleDagdaImpl::lclGetDataDescList() {
  return getDataDescList();
}

//
/* CORBA */
SeqCorbaDataDesc_t* SimpleDagdaImpl::lvlGetDataDescList() {
  std::map<string,Dagda_ptr>::iterator itch;
  SeqCorbaDataDesc_t* local = lclGetDataDescList();
  std::map<char*,corba_data_desc_t> dataMap;
  SeqCorbaDataDesc_t* result = new SeqCorbaDataDesc_t();
	
  for (unsigned int i=0; i<local->length(); i++)
    dataMap[(*local)[i].id.idNumber]=(*local)[i];
  delete local;
	
  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      SeqCorbaDataDesc_t* childList = (*itch).second->lvlGetDataDescList();
      for (unsigned int j=0; j<childList->length(); ++j)
        dataMap[(*childList)[j].id.idNumber]=(*childList)[j];
      delete childList;
      ++itch;
    } catch (CORBA::COMM_FAILURE& e1) {
      getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
	
  childrenMutex.unlock();
  std::map<char*,corba_data_desc_t>::iterator itmap;
	
  result->length(dataMap.size());
  int i=0;
  for (itmap=dataMap.begin(); itmap!=dataMap.end(); ++itmap)
    (*result)[i++]=(*itmap).second;
	
  return result;
}

// Returns the descriptions of all the data stored on the
// platform. Recursive call to MA's data manager.
/* CORBA */
SeqCorbaDataDesc_t* SimpleDagdaImpl::pfmGetDataDescList() {
  if (getParent()==NULL) return lvlGetDataDescList();
  return getParent()->pfmGetDataDescList();
}

// Returns the description of a data. If not present, throws an exception.
/* CORBA */
corba_data_desc_t* SimpleDagdaImpl::lclGetDataDesc(const char* dataID) {
  if (lclIsDataPresent(dataID)) {
    corba_data_desc_t* result = new corba_data_desc_t((*getData())[dataID].desc);
    result->id.idNumber = CORBA::string_dup(dataID);
    return result;
  }
  throw Dagda::DataNotFound(dataID);
}

/* CORBA */
corba_data_desc_t* SimpleDagdaImpl::lvlGetDataDesc(const char* dataID) {
  std::map<string,Dagda_ptr>::iterator itch;
  if (lclIsDataPresent(dataID))
    return lclGetDataDesc(dataID);
	
  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      if ((*itch).second->lvlIsDataPresent(dataID)) {
        childrenMutex.unlock();
        return (*itch).second->lvlGetDataDesc(dataID);
      }
      ++itch;
    } catch (CORBA::COMM_FAILURE& e1) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "CORBA Comm failure!!" << endl);
      getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "CORBA Comm failure!!" << endl);
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
  throw Dagda::DataNotFound(dataID);
}

/* CORBA */
corba_data_desc_t* SimpleDagdaImpl::pfmGetDataDesc(const char* dataID) {
  if (lclIsDataPresent(dataID))
    return lclGetDataDesc(dataID);
  if (getParent()==NULL)
    return lvlGetDataDesc(dataID);
  return getParent()->pfmGetDataDesc(dataID);
}

/* CORBA */
SeqString* SimpleDagdaImpl::lvlGetDataManagers(const char* dataID) {
  std::map<string,Dagda_ptr>::iterator itch;
  SeqString* result = new SeqString();
  list<string> dtmList;
  if (lclIsDataPresent(dataID))
    dtmList.push_back(getIDstr());
	
  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      SeqString* managers = (*itch).second->lvlGetDataManagers(dataID);
      for (unsigned int j=0; j< managers->length(); ++j)
        dtmList.push_back(string((*managers)[j]));
      delete managers;
      ++itch;
    } catch (CORBA::COMM_FAILURE& e1) {
      getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
	
  std::list<string>::iterator itlist;
  int i=0;
  result->length(dtmList.size());
	
  for (itlist=dtmList.begin(); itlist!=dtmList.end(); ++itlist)
    (*result)[i++]=CORBA::string_dup(itlist->c_str());
	
  return result;
}

// Returns all the data managers of a data.
/* CORBA */
SeqString* SimpleDagdaImpl::pfmGetDataManagers(const char* dataID) {
  if (getParent()==NULL) return lvlGetDataManagers(dataID);
  return getParent()->pfmGetDataManagers(dataID);
}

/**/
bool SimpleDagdaImpl::isDataPresent(const char* dataID) {
  std::map<string, corba_data_t>::iterator it;
  bool ret;
  dataMutex.lock();
  it = getData()->find(dataID);
  ret = (it!=getData()->end());
  if (ret) ret = ((*getDataStatus())[dataID]==Dagda::ready);
  dataMutex.unlock();
  return ret;
}

// Simple implementation : return the first data manager of the list.
char* SimpleDagdaImpl::getBestSource(const char* dest, const char* dataID) {
  SeqString* managers = pfmGetDataManagers(dataID);
  TRACE_TEXT(TRACE_ALL_STEPS, "Data " << dataID << " has " <<
             managers->length() << " replica(s) on the platform." << endl);
  if (managers->length()==0)
    throw Dagda::DataNotFound(dataID);
  return CORBA::string_dup((*managers)[0]);
}

corba_data_t* SimpleDagdaImpl::getData(const char* dataID) {
  return &((*getData())[dataID]);
}

corba_data_t* SimpleDagdaImpl::addData(const corba_data_t& data) {
  string dataManager = getIDstr();
  TRACE_TEXT(TRACE_ALL_STEPS, "Adding data " << data.desc.id.idNumber << " to this "
             << "data manager." << endl);
  dataMutex.lock();
  lockData(data.desc.id.idNumber);
  (*getData())[string(data.desc.id.idNumber)]=data;
  (*getData())[string(data.desc.id.idNumber)].desc.dataManager = CORBA::string_dup(dataManager.c_str());
  if (data.desc.specific._d()==DIET_SCALAR) useMemSpace(data.value.length());
  dataMutex.unlock();
  /* Log with DietLogComponent. */
#ifdef USE_LOG_SERVICE
  if (getLogComponent()!=NULL) {
    string dType;
    switch (data.desc.specific._d()) {
    case DIET_SCALAR:
      dType = "SCALAR";
      break;
    case DIET_VECTOR:
      dType = "VECTOR";
      break;
    case DIET_MATRIX:
      dType = "MATRIX";
      break;
    case DIET_STRING:
      dType = "STRING";
      break;
    case DIET_FILE:
      dType = "FILE";
      break;
    case DIET_CONTAINER:
      dType = "CONTAINER";
      break;
    default:
      dType = "UNKNOWN";
    }
    getLogComponent()->logDataStore(data.desc.id.idNumber,
				    data_sizeof(&data.desc),
				    data.desc.base_type,
				    dType.c_str());
  }
#endif
  return &(*getData())[string(data.desc.id.idNumber)];
}

void SimpleDagdaImpl::remData(const char* dataID) {
  std::map<string, corba_data_t>::iterator it;
  dataMutex.lock();
  it = getData()->find(dataID);
  if (it!=getData()->end()) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Removing data " << dataID << " from this "
               << "data manager." << endl);
    Dagda::dataStatus status = getDataStatus(dataID);
    if (it->second.desc.specific._d()==DIET_FILE && status!=Dagda::notOwner) {
      unlink(it->second.desc.specific.file().path);
      freeDiskSpace(it->second.desc.specific.file().size);
    } else {
      if (status!=Dagda::notOwner) {
        freeMemSpace(it->second.value.length());
      }
      if (it->second.desc.specific._d()==DIET_CONTAINER) {
        getContainerRelationMgr()->remAllRelation(dataID);
      }
      getData()->erase(it);
    }
  }
  dataMutex.unlock();
#ifdef USE_LOG_SERVICE
  if (getLogComponent()) {
    getLogComponent()->logDataRelease(dataID);
  }
#endif
}

SeqCorbaDataDesc_t* SimpleDagdaImpl::getDataDescList() {
  std::map<string, corba_data_t>::iterator it;
  SeqCorbaDataDesc_t* result = new SeqCorbaDataDesc_t();
  int i=0;
	
  dataMutex.lock();
  result->length(getData()->size());
  for (it=getData()->begin(); it!=getData()->end(); ++it)
    (*result)[i++]=it->second.desc;
  dataMutex.unlock();
  return result;
}

/* CORBA */
void SimpleDagdaImpl::registerFile(const corba_data_t& data) {
  /* To be reinitialized as pure virtual... This function has moved on
     AdvancedDagdaComponent... */
}


/* Bytes to write to the backup file. */
size_t dataSize(corba_data_t& data) {
  size_t baseSize = 0, nbElt = 0;
  switch (data.desc.base_type) {
  case DIET_CHAR:
    baseSize=1;
    break;
  case DIET_SHORT:
    baseSize=sizeof(short);
    break;
  case DIET_INT:
    baseSize=sizeof(int);
    break;
  case DIET_LONGINT:
    baseSize=sizeof(long);
    break;
  case DIET_FLOAT:
    baseSize=sizeof(float);
    break;
  case DIET_DOUBLE:
    baseSize=sizeof(double);
    break;
  }
  switch (data.desc.specific._d()) {
  case DIET_SCALAR:
    nbElt=1;
    break;
  case DIET_VECTOR:
    nbElt=data.desc.specific.vect().size;
    break;
  case DIET_MATRIX:
    nbElt=data.desc.specific.mat().nb_r *
      data.desc.specific.mat().nb_c;
    break;
  case DIET_STRING:
    nbElt=data.desc.specific.str().length+1;
    break;
  case DIET_PARAMSTRING:
    nbElt=data.desc.specific.pstr().length+1;
    break;
  case DIET_FILE:
    nbElt=strlen(data.desc.specific.file().path)+1;
    break;
  case DIET_CONTAINER:
    nbElt=0;
    break;
  }
  return baseSize*nbElt;
}

// Make a corba data from file informations
size_t DagdaImpl::make_corba_data(corba_data_t& data, diet_data_type_t type,
                                  diet_base_type_t base_type, diet_persistence_mode_t mode,
                                  size_t nb_r, size_t nb_c, diet_matrix_order_t order, void* value, char* path) {
  diet_data_t diet_data;
	
  //  string dataManagerIOR = ORBMgr::getMgr()->getIOR(_this());
  char* dataID = NULL;
	
  diet_data.desc.id = dataID;
  diet_data.desc.mode = mode;
  diet_data.desc.generic.type = type;
  diet_data.desc.generic.base_type = base_type;
	
  switch (type) {
  case DIET_SCALAR:
    diet_data.desc.specific.scal.value = value;
    break;
  case DIET_VECTOR:
    diet_data.desc.specific.vect.size = nb_c;
    break;
  case DIET_MATRIX:
    diet_data.desc.specific.mat.nb_r = nb_r;
    diet_data.desc.specific.mat.nb_c = nb_c;
    diet_data.desc.specific.mat.order = order;
    break;
  case DIET_STRING:
  case DIET_PARAMSTRING:
    diet_data.desc.specific.pstr.length = strlen((char*) value);
    break;
  case DIET_FILE:
    diet_data.desc.specific.file.path = path;
    break;
  case DIET_CONTAINER:
    diet_data.desc.specific.cont.size = 0;
    break;
  default:
    WARNING("This data type is not managed by DIET.");
  }
	
  mrsh_data_desc(&data.desc, &diet_data.desc);
  data.desc.dataManager = CORBA::string_dup(getIDstr().c_str());
  return data_sizeof(&diet_data.desc);
}

// Write the data description to the file.
int DagdaImpl::writeDataDesc(corba_data_t& data, ofstream& file) {
  size_t size = dataSize(data);
  long type = data.desc.specific._d();
	
  if (!file.is_open()) return -1;
	
  try {
    file.write((char*) data.desc.id.idNumber, strlen(data.desc.id.idNumber)+1);
    file.write((char*) &data.desc.mode, sizeof(long));
    file.write((char*) &data.desc.base_type, sizeof(long));
    file.write((char*) &type, sizeof(long));
    // Temporaire... Il y a un soucis avec l'IOR... Au red�marrage, il aura chang�...
    file.write((char*) data.desc.dataManager, strlen(data.desc.dataManager)+1);
    file.write((char*) &size, sizeof(size_t));
  } catch (ios_base::failure &ex) {
    return -1;
  }
  if (file.bad() || file.fail()) return -1;
  return 0;
}

// Write the data to the file.
int DagdaImpl::writeData(corba_data_t& data, ofstream& file) {
  size_t size = dataSize(data);
  long type = data.desc.specific._d();
	
  if (!file.is_open() || file.bad() || file.fail())
    return -1;
	
  try {
    switch (type) {
    case DIET_SCALAR:
      file.write((char*) data.value.get_buffer(), size);
      break;
    case DIET_VECTOR:
      file.write((char*) &data.desc.specific.vect().size, sizeof(size_t));
      file.write((char*) data.value.get_buffer(), size);
      break;
    case DIET_MATRIX:
      file.write((char*) &data.desc.specific.mat().nb_r, sizeof(size_t));
      file.write((char*) &data.desc.specific.mat().nb_c, sizeof(size_t));
      file.write((char*) &data.desc.specific.mat().order, sizeof(size_t));
      file.write((char*) data.value.get_buffer(), size);
      break;
    case DIET_STRING:
      file.write((char*) data.value.get_buffer(), strlen((char*) data.value.get_buffer())+1);
      break;
    case DIET_PARAMSTRING:
      file.write((char*) data.value.get_buffer(), strlen((char*) data.value.get_buffer())+1);
      break;
    case DIET_CONTAINER:
      file.write((char*) &data.desc.specific.cont().size, sizeof(size_t));
      //TODO write the relationships container-element
      break;
    case DIET_FILE:
      bool ownerShip = (getDataStatus(data.desc.id.idNumber)!=Dagda::notOwner);
      file.write((char*) &ownerShip, sizeof(bool));
      file.write((char*) data.desc.specific.file().path,
                 strlen((char*) data.desc.specific.file().path)+1);
      break;
    }
  } catch (ios_base::failure &ex) {
    return -1;
  }
  return 0;
}

// Read a data from a file.
int DagdaImpl::readData(corba_data_t& data, ifstream& file) {
  if (!file.is_open() || file.bad() || file.fail())
    return -1;
  string inputString;
  long inputLong;
  size_t inputSize;
  char c;
	
  diet_data_type_t type;
  diet_base_type_t base_type;
  diet_persistence_mode_t mode;
  size_t nb_r, nb_c;
  diet_matrix_order_t order;
  char* path = NULL;
  char* id;
	
  try {
    do {
      file.get(c);
      inputString+=c;
    } while (c!='\0');
    id = CORBA::string_dup(inputString.c_str());
    file.read((char*) &inputLong, sizeof(long));
    mode = (diet_persistence_mode_t) inputLong;
    file.read((char*) &inputLong, sizeof(long));
    base_type = (diet_base_type_t) inputLong;
    file.read((char*) &inputLong, sizeof(long));
    type = (diet_data_type_t) inputLong;
    inputString = "";
    do {
      file.get(c);
      inputString+=c;
    } while (c!='\0');
		
    file.read((char*) &inputSize, sizeof(size_t));
    CORBA::Char* buffer = NULL;
		
    switch (type) {
    case DIET_SCALAR:
      buffer = new CORBA::Char[inputSize];
      file.read((char*) buffer, inputSize);
      nb_r=0; nb_c=0;
      order = (diet_matrix_order_t) 0;
      path=NULL;
      break;
    case DIET_VECTOR:
      file.read((char*) &nb_c, sizeof(size_t));
      buffer = new CORBA::Char[inputSize];
      file.read((char*) buffer, inputSize);
      nb_r=0;	order = (diet_matrix_order_t) 0;
      path=NULL;
      break;
    case DIET_MATRIX:
      file.read((char*) &nb_r, sizeof(size_t));
      file.read((char*) &nb_c, sizeof(size_t));
      file.read((char*) &inputLong, sizeof(long));
      buffer = new CORBA::Char[inputSize];
      file.read((char*) buffer, inputSize);
      order = (diet_matrix_order_t) inputLong;
      path=NULL;
      break;
    case DIET_STRING:
    case DIET_PARAMSTRING:
      buffer = new CORBA::Char[inputSize];
      file.read((char*) buffer, inputSize);
      nb_r=0; nb_c=0;
      order = (diet_matrix_order_t) 0;
      path = NULL;
      break;
    case DIET_FILE:
      bool ownerShip;
      file.read((char*) &ownerShip, sizeof(bool));
      if (!ownerShip) setDataStatus(id, Dagda::notOwner);
      else setDataStatus(id, Dagda::ready);
      buffer = new CORBA::Char[inputSize];
      file.read((char*) buffer, inputSize);
      nb_r=0; nb_c=0;
      order = (diet_matrix_order_t) 0;
      path = (char*) buffer;
      break;
    case DIET_CONTAINER:
      file.read((char*) &nb_c, sizeof(size_t));
      //TODO read the relationships container-element
      nb_r  = 0;
      order = (diet_matrix_order_t) 0;
      path  = NULL;
      break;
    default:
      WARNING("This data type is not managed by DIET.");
      order = (diet_matrix_order_t) 0;
    }
    make_corba_data(data, type, base_type, mode, nb_r, nb_c,
                    order, buffer, path);
    data.desc.id.idNumber = id;
    data.value.replace(inputSize, inputSize, buffer, true);
  } catch (ios_base::failure &ex) {
    return -1;
  }
	
  return 0;
}


void DagdaImpl::saveState() {
  if (getStateFile()=="") return;
  std::map<std::string, corba_data_t>::iterator it;
  std::ofstream file(getStateFile().c_str(), ios_base::trunc);
	
  if (!file.is_open()) {
    WARNING("Error opening file " << getStateFile() << " to save current data state.");
    return;
  }
  dataMutex.lock();
  for (it=getData()->begin(); it!=getData()->end(); ++it) {
    if (writeDataDesc(it->second, file)!=0) {
      WARNING("DAGDA state backup failed.");
      break;
    }
    if (writeData(it->second, file)!=0) {
      WARNING("Error while writing the data to " << getStateFile());
      break;
    }
  }
  dataMutex.unlock();
  file.close();
}

void DagdaImpl::restoreState() {
  std::ifstream file(getStateFile().c_str());
  if (!file.is_open()) return;
  file.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
  corba_data_t dataFromFile;
  corba_data_t data;
  corba_data_t* inserted;
	
  while (readData(dataFromFile, file)==0) {
    size_t size = dataFromFile.value.length();
    data.desc = dataFromFile.desc;
    inserted = addData(data);
    unlockData(data.desc.id.idNumber);
		
    inserted->value.replace(size, size, dataFromFile.value.get_buffer(true), true);
    if (inserted->desc.specific._d()!=DIET_FILE)
      useMemSpace(inserted->value.length());
    else if (getDataStatus(data.desc.id.idNumber)!=Dagda::notOwner)
      useDiskSpace(inserted->desc.specific.file().size);
  }
}

void thrdCheckpointChild(void* child) {
  Dagda_ptr ch = (Dagda_ptr) child;
  ch->checkpointState();
}

void DagdaImpl::checkpointState() {
  std::map<string,Dagda_ptr>::iterator itch;
  saveState();
	
  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      omni_thread::create(thrdCheckpointChild, (*itch).second);
      ++itch;
    } catch (CORBA::COMM_FAILURE& e1) {
      getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
}

/* Forwarder part*/

DagdaFwdrImpl::DagdaFwdrImpl(Forwarder_ptr fwdr, const char* objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

void DagdaFwdrImpl::subscribe(const char* name) {
  forwarder->subscribe(name, objName);
}

void DagdaFwdrImpl::unsubscribe(const char* name) {
  forwarder->unsubscribe(name, objName);
}

#ifdef HAVE_DYNAMICS
void DagdaFwdrImpl::subscribeParent(const char * parentID) {
  forwarder->subscribeParent(parentID, objName);
}

void DagdaFwdrImpl::unsubscribeParent() {
  forwarder->unsubscribeParent(objName);
}
#endif // HAVE_DYNAMICS


char* DagdaFwdrImpl::writeFile(const SeqChar& data, const char* basename,
                               CORBA::Boolean replace)
{
  return forwarder->writeFile(data, basename, replace, objName);
}

char* DagdaFwdrImpl::sendFile(const corba_data_t &data, const char* dest) {
  return forwarder->sendFile(data, dest, objName);
}

char* DagdaFwdrImpl::recordData(const SeqChar& data, const corba_data_desc_t& dataDesc,
                                CORBA::Boolean replace, CORBA::Long offset)
{
  return forwarder->recordData(data, dataDesc, replace, offset, objName);
}

char* DagdaFwdrImpl::sendData(const char* ID, const char* dest) {
  return forwarder->sendData(ID, dest, objName);	
}

char* DagdaFwdrImpl::sendContainer(const char* containerID, const char* dest,
                                   CORBA::Boolean sendElements)
{
  return forwarder->sendContainer(containerID, dest, sendElements, objName);
}

void DagdaFwdrImpl::lockData(const char* dataID) {
  forwarder->lockData(dataID, objName);
}

void DagdaFwdrImpl::unlockData(const char* dataID) {
  forwarder->unlockData(dataID, objName);
}

Dagda::dataStatus DagdaFwdrImpl::getDataStatus(const char* dataID) {
  return forwarder->getDataStatus(dataID, objName);
}

CORBA::Boolean DagdaFwdrImpl::lclIsDataPresent(const char* dataID) {
  return forwarder->lclIsDataPresent(dataID, objName);
}

CORBA::Boolean DagdaFwdrImpl::lvlIsDataPresent(const char* dataID) {
  return forwarder->lvlIsDataPresent(dataID, objName);
}

CORBA::Boolean DagdaFwdrImpl::pfmIsDataPresent(const char* dataID) {
  return forwarder->pfmIsDataPresent(dataID, objName);
}

void DagdaFwdrImpl::lclAddData(const char* src, const corba_data_t& data) {
  forwarder->lclAddData(src, data, objName);
}

void DagdaFwdrImpl::lvlAddData(const char* src, const corba_data_t& data) {
  forwarder->lvlAddData(src, data, objName);
}

void DagdaFwdrImpl::pfmAddData(const char* src, const corba_data_t& data) {
  forwarder->pfmAddData(src, data, objName);
}

void DagdaFwdrImpl::registerFile(const corba_data_t& data) {
  forwarder->registerFile(data, objName);
}

void DagdaFwdrImpl::lclAddContainerElt(const char* containerID,
                                       const char* dataID,
                                       CORBA::Long index,
                                       CORBA::Long flag)
{
  forwarder->lclAddContainerElt(containerID, dataID, index, flag, objName);
}

CORBA::Long DagdaFwdrImpl::lclGetContainerSize(const char* containerID) {
  return forwarder->lclGetContainerSize(containerID, objName);
}

void DagdaFwdrImpl::lclGetContainerElts(const char* containerID,
                                        SeqString& dataIDSeq,
                                        SeqLong& flagSeq,
                                        CORBA::Boolean ordered)
{
  forwarder->lclGetContainerElts(containerID, dataIDSeq, flagSeq, ordered, objName);
}

void DagdaFwdrImpl::lclRemData(const char* dataID) {
  forwarder->lclRemData(dataID, objName);
}

void DagdaFwdrImpl::lvlRemData(const char* dataID) {
  forwarder->lvlRemData(dataID, objName);
}

void DagdaFwdrImpl::pfmRemData(const char* dataID) {
  forwarder->pfmRemData(dataID, objName);
}

void DagdaFwdrImpl::lclUpdateData(const char* src, const corba_data_t& data) {
  forwarder->lclUpdateData(src, data, objName);
}

void DagdaFwdrImpl::lvlUpdateData(const char* src, const corba_data_t& data) {
  forwarder->lvlUpdateData(src, data, objName);
}

void DagdaFwdrImpl::pfmUpdateData(const char* src, const corba_data_t& data) {
  forwarder->pfmUpdateData(src, data, objName);
}

void DagdaFwdrImpl::lclReplicate(const char* dataID, CORBA::Long target,
                                 const char* pattern, bool replace)
{
  forwarder->lclReplicate(dataID, target, pattern, replace, objName);
}

void DagdaFwdrImpl::lvlReplicate(const char* dataID, CORBA::Long target,
                                 const char* pattern, bool replace)
{
  forwarder->lvlReplicate(dataID, target, pattern, replace, objName);
}

void DagdaFwdrImpl::pfmReplicate(const char* dataID, CORBA::Long target,
                                 const char* pattern, bool replace)
{
  forwarder->pfmReplicate(dataID, target, pattern, replace, objName);
}

SeqCorbaDataDesc_t* DagdaFwdrImpl::lclGetDataDescList() {
  return forwarder->lclGetDataDescList(objName);
}

SeqCorbaDataDesc_t* DagdaFwdrImpl::lvlGetDataDescList() {
  return forwarder->lvlGetDataDescList(objName);
}

SeqCorbaDataDesc_t* DagdaFwdrImpl::pfmGetDataDescList() {
  return forwarder->pfmGetDataDescList(objName);
}

corba_data_desc_t* DagdaFwdrImpl::lclGetDataDesc(const char* dataID) {
  return forwarder->lclGetDataDesc(dataID, objName);
}

corba_data_desc_t* DagdaFwdrImpl::lvlGetDataDesc(const char* dataID) {
  return forwarder->lvlGetDataDesc(dataID, objName);
}

corba_data_desc_t* DagdaFwdrImpl::pfmGetDataDesc(const char* dataID) {
  return forwarder->pfmGetDataDesc(dataID, objName);
}

SeqString* DagdaFwdrImpl::lvlGetDataManagers(const char* dataID) {
  return forwarder->lvlGetDataManagers(dataID, objName);
}

SeqString* DagdaFwdrImpl::pfmGetDataManagers(const char* dataID) {
  return forwarder->pfmGetDataManagers(dataID, objName);
}

char* DagdaFwdrImpl::getBestSource(const char* dest, const char* dataID) {
  return forwarder->getBestSource(dest, dataID, objName);
}

char* DagdaFwdrImpl::getID() {
  return forwarder->getID(objName);
}

void DagdaFwdrImpl::checkpointState() {
  forwarder->checkpointState(objName);
}

char* DagdaFwdrImpl::getHostname() {
  return forwarder->getHostname(objName);
}
