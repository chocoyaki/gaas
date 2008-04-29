/***********************************************************/
/* Dagda component implementation.                         */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$													   */
/* $Log													   */
/*														   */
/***********************************************************/

#include "Dagda.hh"
#include "common_types.hh"

#include "ORBMgr.hh"
#include "DIET_data.h"
#include "DIET_data_internal.hh"
#include "debug.hh"
#include "marshalling.hh"

#include <unistd.h>
#include <errno.h>

#include <list>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>

#include "DagdaImpl.hh"
#include "DagdaFactory.hh"

#if HAVE_ADVANCED_UUID
#include <uuid/uuid.h>
#endif

DagdaImpl::~DagdaImpl() {
  if (parent!=NULL) parent->unsubscribe(_this());
}

/* CORBA */
char* DagdaImpl::getHostname() {
  return hostname;
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

const size_t DagdaImpl::getMaxMsgSize() {
  return maxMsgSize;
}

void DagdaImpl::setDiskMaxSpace(const size_t diskMaxSpace) {
  this->diskMaxSpace = diskMaxSpace;
}

const size_t DagdaImpl::getDiskMaxSpace() {
  return diskMaxSpace;
}

void DagdaImpl::setMemMaxSpace(const size_t memMaxSpace) {
  this->memMaxSpace = memMaxSpace;
}

const size_t DagdaImpl::getMemMaxSpace() {
  return memMaxSpace;
}

/* Write the data to a file. Can be done in several time. */
/* CORBA */
char* DagdaImpl::writeFile(const SeqChar& data, const char* basename,
			CORBA::Boolean replace) {
  std::ofstream file;
  std::string filename(getDataPath());
  filename.append("/");
  filename.append(basename);

  if (replace)
    file.open(filename.c_str());
  else
    file.open(filename.c_str(), std::ios_base::app);

  if (!file.is_open()) throw Dagda::WriteError(errno);
  file.write((const char*) data.get_buffer(), data.length());

  if (file.bad()) throw Dagda::WriteError(errno);
  TRACE_TEXT(TRACE_ALL_STEPS, "Write " << data.length() << " bytes on the file "
                             << filename << endl);
  return CORBA::string_dup(filename.c_str());
}

string gen_filename(string basename) {
  int idx = basename.find_last_of('/');
  if (idx!=string::npos)
    basename=basename.substr(idx);
  ostringstream name;
#if HAVE_ADVANCED_UUID
  uuid_t uuid;
  char ID[37];
  
  uuid_generate(uuid);
  uuid_unparse(uuid, ID);
  
  name << basename << "-" << ID;
#else
  name << basename << "." << getpid();
#endif
  return name.str();
}

/* Send a file to a node. */
/* CORBA */
char* DagdaImpl::sendFile(const corba_data_t &data, Dagda_ptr dest) {
  if (data.desc.specific.file().path==NULL) // path not initialized.
    throw Dagda::InvalidPathName(data.desc.id.idNumber, "null");

  std::ifstream file(data.desc.specific.file().path);
  if (!file.is_open()) // Unable to open the file.
    throw Dagda::ReadError(errno);
	
  TRACE_TEXT(TRACE_MAIN_STEPS, "*** Sending file " << data.desc.specific.file().path
                              << " (" << data.desc.id.idNumber << ")" << endl);
  
  unsigned long wrote = 0;
  unsigned long fileSize = data.desc.specific.file().size;
  string basename(data.desc.specific.file().path);

  string name = gen_filename(basename);

  unsigned long nBlocks =  fileSize / getMaxMsgSize() + 1;

  bool replace = true;
  char* distPath = NULL;
  for (int i=0; i<nBlocks; ++i) {
    SeqChar buffer;
    int toSend = (fileSize-wrote > getMaxMsgSize() ?
		  getMaxMsgSize():(fileSize-wrote));
	TRACE_TEXT(TRACE_ALL_STEPS, "\tSend " << toSend << " bytes..." << endl);
    buffer.length(toSend);
    file.read((char*) buffer.get_buffer(false), toSend);
    if (file.bad()) throw Dagda::ReadError(errno);
    // Send the data.
    if (distPath!=NULL) CORBA::string_free(distPath);
    distPath=dest->writeFile(buffer, name.c_str(), replace);

    wrote+=toSend;
    replace=false;
  }
  return distPath;
}

/* CORBA */
/* Warning : this function MUST be protected by a dataMutex.lock()/unlock() pair. */
char* DagdaImpl::recordData(const SeqChar& data,
			    const corba_data_desc_t& dataDesc,
			    CORBA::Boolean replace) {
  unsigned long offset;

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
  /*if (data.length()==0)
    return CORBA::string_dup((*getData())[dataId].desc.id.idNumber);*/
  if (replace) {
    (*getData())[dataId].value.length(data.length());
    offset=0;
  } else {
    offset=(*getData())[dataId].value.length();
    (*getData())[dataId].value.length(data.length()+offset);
  }

  for (int i=0; i<data.length(); ++i) {
    (*getData())[dataId].value[i+offset]=data[i];
  }

  return CORBA::string_dup((*getData())[dataId].desc.id.idNumber);
}

/* CORBA */
char* DagdaImpl::sendData(const char* dataId, Dagda_ptr dest) {
  dataMutex.lock();
  if (getData()->find(dataId)==getData()->end()) {
    dataMutex.unlock();
    throw Dagda::DataNotFound(dataId);
  }

  if ((*getDataStatus())[dataId]==Dagda::downloading) {
	dataMutex.unlock();
	throw Dagda::UnavailableData(dataId);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, "*** Sending data " << dataId << endl);

  corba_data_t* data = getData(dataId);
  size_t wrote = 0;
  size_t dataSize = data_sizeof(&data->desc);
  size_t nBlocks =  dataSize / getMaxMsgSize() + 1;

  bool replace = true;
  char* distID=NULL;

  for (size_t i=0; i<nBlocks; ++i) {
    int toSend = (dataSize-wrote > getMaxMsgSize() ?
		  getMaxMsgSize():(dataSize-wrote));
    TRACE_TEXT(TRACE_ALL_STEPS, "\tSend " <<  toSend << " bytes..." << endl);
    SeqChar buffer(toSend, toSend, data->value.get_buffer()+wrote);

    if (distID!=NULL) CORBA::string_free(distID);
    distID=dest->recordData(buffer, data->desc, replace);

    wrote+=toSend;
    replace=false;
  }

  dest->unlockData(distID);
  dataMutex.unlock();
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

/* CORBA */
void SimpleDagdaImpl::subscribe(Dagda_ptr me) {
  string name(me->getID());
  childrenMutex.lock();
  map<string, Dagda_ptr>::iterator it = getChildren()->find(name);
  
  if (it!=getChildren()->end()) getChildren()->erase(name);
  (*getChildren())[name]=Dagda::_duplicate(me);
  childrenMutex.unlock();
}

/* CORBA */
void SimpleDagdaImpl::unsubscribe(Dagda_ptr me) {
  string name(me->getID());
  childrenMutex.lock();
  map<string, Dagda_ptr>::iterator it = getChildren()->find(name);

  if (it!=getChildren()->end()) getChildren()->erase(name);
  childrenMutex.unlock();
}

SimpleDagdaImpl::~SimpleDagdaImpl() {
  //CORBA::string_free(this->ID);
}

// Initialize this data manager.
int SimpleDagdaImpl::init(const char* ID, const char* parentID,
			  const char* dataPath, const size_t maxMsgSize,
			  const size_t diskMaxSpace,
			  const size_t memMaxSpace) {
  setID(CORBA::string_dup(ID));

  if (ORBMgr::bindObjToName(_this(), ORBMgr::DATAMGR, getID())) {
    ERROR("Dagda: could not declare myself as " << getID(), 1);
  }
  if (parentID==NULL) setParent(NULL);
  else {
	parentID = CORBA::string_dup(parentID);

    setParent(Dagda::_duplicate(Dagda::_narrow(
		ORBMgr::getObjReference(ORBMgr::DATAMGR, parentID))));
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, 
	     "## Launch Dagda data manager " << getID() << endl);
  TRACE_TEXT(TRACE_ALL_STEPS,
	     "IOR : " << ORBMgr::getIORString(_this()) << endl);
		 

  if (getParent()!=NULL) getParent()->subscribe(_this());

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
	  itch++;
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

// Returns the data from this data manager.
/* CORBA */
/*corba_data_t* SimpleDagdaImpl::lclGetData(Dagda_ptr dest, const char* dataID) {
  dataMutex.lock();
  corba_data_t* found = getData(dataID);
  dataMutex.unlock();
  
  if (dest==_this()) return found;
  if (found==NULL) throw Dagda::DataNotFound(dataID);
  if ((*getDataStatus())[dataID]==Dagda::downloading) {
	throw Dagda::UnavailableData(dataID);
  }

  corba_data_t* result = new corba_data_t();

  result->desc=found->desc;

  if (found->desc.specific._d()==DIET_FILE) {
    char* remotePath = sendFile(*found, dest);
    result->desc.specific.file().path=remotePath;
  }

  SeqChar* data = new SeqChar(found->value);
  result->value = *data;

  delete data;
  return result;
}*/

// Returns the data from this level.
/* CORBA */
/*corba_data_t* SimpleDagdaImpl::lvlGetData(Dagda_ptr dest, const char* dataID) {
  std::map<string, Dagda_ptr>::iterator itch;

  if (lclIsDataPresent(dataID))
    return lclGetData(dest, dataID);

  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      if ((*itch).second->lvlIsDataPresent(dataID)) {
	    childrenMutex.unlock();
        return (*itch).second->lvlGetData(dest, dataID);
	  }
	  itch++;
	} catch (CORBA::COMM_FAILURE& e1) {
	  getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    } catch (Dagda::UnavailableData &ex) {
	  itch++;
	}
  childrenMutex.unlock();
  throw Dagda::DataNotFound(dataID);
}*/

// Look if the data is present on this level. Otherwise ask the parent
// to get it.
// This is the simplest implementation. There is many way to do that.
/* CORBA */
/*corba_data_t* SimpleDagdaImpl::pfmGetData(Dagda_ptr dest, const char* dataID) {
  if (lvlIsDataPresent(dataID)) return lvlGetData(dest, dataID);
  if (getParent()==NULL)
    return lvlGetData(dest, dataID);
  return getParent()->pfmGetData(dest, dataID);
}*/

// This method downloads the data from the node src.
/* CORBA */
char* SimpleDagdaImpl::downloadData(Dagda_ptr src, const corba_data_t& data) {
  if (data.desc.specific._d()==DIET_FILE)
    return src->sendFile(data, _this());
  else
    return src->sendData(data.desc.id.idNumber, _this());
}

// Add a data locally.
/* CORBA */
void SimpleDagdaImpl::lclAddData(Dagda_ptr src, const corba_data_t& data) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Add the data " << data.desc.id.idNumber
                             << " locally." << endl);
    if (strcmp(src->getID(), getID()) != 0) {
	  if (data.desc.specific._d()==DIET_FILE) {
	    if (getDiskMaxSpace()!=0 && getUsedDiskSpace()+data_sizeof(&data.desc)>getDiskMaxSpace())
		  throw Dagda::NotEnoughSpace(getDiskMaxSpace()-getUsedDiskSpace());
		char* path = downloadData(src, data);
        if (path) {
		  corba_data_t newData(data);
		  newData.desc.specific.file().path=path;
		  addData(newData);
		  unlockData(data.desc.id.idNumber); //
		  useDiskSpace(data.desc.specific.file().size);
        }
	  } else {
	    if (getMemMaxSpace()!=0 && getUsedMemSpace()+data_sizeof(&data.desc)>getMemMaxSpace())
		  throw Dagda::NotEnoughSpace(getMemMaxSpace()-getUsedMemSpace());
	    char* dataID;
		corba_data_t* inserted;
		addData(data);
		dataID = downloadData(src, data);
		unlockData(dataID);
		inserted = getData(data.desc.id.idNumber);
		useMemSpace(inserted->value.length());
	  }
	}
}

// Add at this level.
// Plusieurs manières de faire ça :
// * on copie en local (méthode choisie).
// * on essaye de copier en local. Si ce n'est pas possible, on essaye sur
//   les noeuds enfants.
// * Utilisation d'une heuristique pour choisir le noeud.
/* CORBA */
void SimpleDagdaImpl::lvlAddData(Dagda_ptr src, const corba_data_t& data) {
  lclAddData(src, data);
}

// Simple implementation.
// Many way to do that... Ex :
// * Control if there is enough local storage capacity to store the data.
//   Otherwise ask to "parent" or "children" to store it. (Recursively).
/* CORBA */
void SimpleDagdaImpl::pfmAddData(Dagda_ptr src, const corba_data_t& data) {
  // Control if the data is already on the platform ???
  lvlAddData(src, data);
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
	  itch++;
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
void SimpleDagdaImpl::lclUpdateData(Dagda_ptr src, const corba_data_t& data) {
  if (strcmp(src->getID(), this->getID())==0) return;

  lclRemData(data.desc.id.idNumber);
  lclAddData(src, data);
}

// 
/* CORBA */
void SimpleDagdaImpl::lvlUpdateData(Dagda_ptr src, const corba_data_t& data) {
  std::map<string,Dagda_ptr>::iterator itch;
  if (lclIsDataPresent(data.desc.id.idNumber))
    lclUpdateData(src, data);

  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      (*itch).second->lvlUpdateData(src, data);
	  itch++;
	} catch (CORBA::COMM_FAILURE& e1) {
	  getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
}

// Simple implementation.
/* CORBA */
void SimpleDagdaImpl::pfmUpdateData(Dagda_ptr src, const corba_data_t& data) {
  if (getParent()==NULL)
    lvlUpdateData(src, data);
  else
    getParent()->pfmUpdateData(src, data);
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

  for (int i=0; i<local->length(); i++)
    dataMap[(*local)[i].id.idNumber]=(*local)[i];
  delete local;

  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      SeqCorbaDataDesc_t* childList = (*itch).second->lvlGetDataDescList();
      for (int j=0; j<childList->length(); ++j)
        dataMap[(*childList)[j].id.idNumber]=(*childList)[j];
      delete childList;
	  itch++;
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
	  itch++;
    } catch (CORBA::COMM_FAILURE& e1) {
	  getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
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
SeqDagda_t* SimpleDagdaImpl::lvlGetDataManagers(const char* dataID) {
  std::map<string,Dagda_ptr>::iterator itch;
  SeqDagda_t* result = new SeqDagda_t();
  std::list<Dagda_ptr> dtmList;
  if (lclIsDataPresent(dataID))
    dtmList.push_back(Dagda::_duplicate(_this()));
	
  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      SeqDagda_t* managers = (*itch).second->lvlGetDataManagers(dataID);
      for (int j=0; j< managers->length(); ++j)
        dtmList.push_back(Dagda::_duplicate((*managers)[j]));
      delete managers;
	  itch++;
    } catch (CORBA::COMM_FAILURE& e1) {
	  getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
    
  std::list<Dagda_ptr>::iterator itlist;
  int i=0;
  result->length(dtmList.size());

  for (itlist=dtmList.begin(); itlist!=dtmList.end(); ++itlist)
    (*result)[i++]=*itlist;
    
  return result;
}

// Returns all the data managers of a data.
/* CORBA */
SeqDagda_t* SimpleDagdaImpl::pfmGetDataManagers(const char* dataID) {
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
Dagda_ptr SimpleDagdaImpl::getBestSource(Dagda_ptr dest, const char* dataID) {
  SeqDagda_t* managers = pfmGetDataManagers(dataID);
  TRACE_TEXT(TRACE_ALL_STEPS, "Data " << dataID << " has " <<
             managers->length() << " replica(s) on the platform." << endl);
  if (managers->length()==0)
    throw Dagda::DataNotFound(dataID);
  return (*managers)[0];
}

corba_data_t* SimpleDagdaImpl::getData(const char* dataID) {
  return &((*getData())[dataID]);
}

void SimpleDagdaImpl::addData(const corba_data_t& data) {
  char* dataManager = ORBMgr::getIORString(DagdaFactory::getDataManager()->_this());
  TRACE_TEXT(TRACE_ALL_STEPS, "Adding data " << data.desc.id.idNumber << " to this "
			 << "data manager." << endl);
  dataMutex.lock();

  lockData(data.desc.id.idNumber);
  (*getData())[string(data.desc.id.idNumber)]=data;
  (*getData())[string(data.desc.id.idNumber)].desc.dataManager = CORBA::string_dup(dataManager);
  if (data.desc.specific._d()==DIET_SCALAR) useMemSpace(data.value.length());
  dataMutex.unlock();
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
	} else
	  if (status!=Dagda::notOwner)
	    freeMemSpace(it->second.value.length());
    getData()->erase(it);
  }
  dataMutex.unlock();
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
  size_t baseSize, nbElt;
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
  }
  return baseSize*nbElt;
}

// Make a corba data from file informations
size_t DagdaImpl::make_corba_data(corba_data_t& data, diet_data_type_t type,
	diet_base_type_t base_type, diet_persistence_mode_t mode,
	size_t nb_r, size_t nb_c, diet_matrix_order_t order, void* value, char* path) {
  diet_data_t diet_data;
  
  char* dataManagerIOR = ORBMgr::getIORString(_this());
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
  }
  
  mrsh_data_desc(&data.desc, &diet_data.desc);
  data.desc.dataManager = CORBA::string_dup(dataManagerIOR);
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
	// Temporaire... Il y a un soucis avec l'IOR... Au redŽmarrage, il aura changŽ...
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
  void* value = NULL;
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
	CORBA::Char* buffer;

	switch (type) {
	  case DIET_SCALAR:
	    buffer = new CORBA::Char[inputSize];
    	file.read((char*) buffer, inputSize);
	    nb_r=0; nb_c=0;
		order = (diet_matrix_order_t) 0;
		value=buffer;
		path=NULL;
		break;
	  case DIET_VECTOR:
	    file.read((char*) &nb_c, sizeof(size_t));
		buffer = new CORBA::Char[inputSize];
    	file.read((char*) buffer, inputSize);
		nb_r=0;	order = (diet_matrix_order_t) 0;
		value=NULL;	path=NULL;
		break;
	  case DIET_MATRIX:
	    file.read((char*) &nb_r, sizeof(size_t));
        file.read((char*) &nb_c, sizeof(size_t));
		file.read((char*) &inputLong, sizeof(long));
		buffer = new CORBA::Char[inputSize];
    	file.read((char*) buffer, inputSize);
		order = (diet_matrix_order_t) inputLong;
		value=NULL; path=NULL;
		break;
	  case DIET_STRING:
	  case DIET_PARAMSTRING:
	    buffer = new CORBA::Char[inputSize];
    	file.read((char*) buffer, inputSize);
	    nb_r=0; nb_c=0;
		order = (diet_matrix_order_t) 0;
		value = buffer; path = NULL;
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
		value = NULL;
		path = (char*) buffer;
		break;
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
	addData(data);
	unlockData(data.desc.id.idNumber);
	inserted = getData(data.desc.id.idNumber);
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
      //(*itch).second->saveState();
	  itch++;
    } catch (CORBA::COMM_FAILURE& e1) {
	  getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    }
  childrenMutex.unlock();
}
