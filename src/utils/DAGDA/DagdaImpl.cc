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

void DagdaImpl::setMaxMsgSize(const unsigned long maxMsgSize) {
  this->maxMsgSize = maxMsgSize;
}

const unsigned long DagdaImpl::getMaxMsgSize() {
  return maxMsgSize;
}

void DagdaImpl::setDiskMaxSpace(const unsigned long diskMaxSpace) {
  this->diskMaxSpace = diskMaxSpace;
}

const unsigned long DagdaImpl::getDiskMaxSpace() {
  return diskMaxSpace;
}

void DagdaImpl::setMemMaxSpace(const unsigned long memMaxSpace) {
  this->memMaxSpace = memMaxSpace;
}

const unsigned long DagdaImpl::getMemMaxSpace() {
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
  std::string basename(data.desc.specific.file().path);
  int idx = basename.find_last_of('/');
  if (idx!=std::string::npos)
    basename=basename.substr(idx);
  std::ostringstream name;
  name << basename << "." << getpid();

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
    distPath=dest->writeFile(buffer, name.str().c_str(), replace);

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

  unsigned long wrote = 0;
  unsigned long dataSize = data_sizeof(&(*getData())[dataId].desc);
  unsigned long nBlocks =  dataSize / getMaxMsgSize() + 1;

  bool replace = true;
  char* distID=NULL;

  for (int i=0; i<nBlocks; ++i) {
    int toSend = (dataSize-wrote > getMaxMsgSize() ?
		  getMaxMsgSize():(dataSize-wrote));
    TRACE_TEXT(TRACE_ALL_STEPS, "\tSend " <<  toSend << " bytes..." << endl);
    SeqChar buffer(toSend, toSend, (*getData())[dataId].value.get_buffer()+wrote);

    if (distID!=NULL) CORBA::string_free(distID);
    distID=dest->recordData(buffer, (*getData())[dataId].desc, replace);

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
			  const char* dataPath, const unsigned long maxMsgSize,
			  const unsigned long diskMaxSpace,
			  const unsigned long memMaxSpace) {
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
corba_data_t* SimpleDagdaImpl::lclGetData(Dagda_ptr dest, const char* dataID) {
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
}

// Returns the data from this level.
/* CORBA */
corba_data_t* SimpleDagdaImpl::lvlGetData(Dagda_ptr dest, const char* dataID) {
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
}

// Look if the data is present on this level. Otherwise ask the parent
// to get it.
// This is the simplest implementation. There is many way to do that.
/* CORBA */
corba_data_t* SimpleDagdaImpl::pfmGetData(Dagda_ptr dest, const char* dataID) {
  if (lvlIsDataPresent(dataID)) return lvlGetData(dest, dataID);
  if (getParent()==NULL)
    return lvlGetData(dest, dataID);
  return getParent()->pfmGetData(dest, dataID);
}

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
    if (src!=_this()) {
	  if (data.desc.specific._d()==DIET_FILE) {
		char* path = downloadData(src, data);
        if (path) {
		  corba_data_t newData(data);
		  newData.desc.specific.file().path=path;
		  addData(newData);
		  unlockData(data.desc.id.idNumber); //
        }
	  } else {
	    char* dataID;
		addData(data);
		dataID = downloadData(src, data);
		unlockData(dataID);
	  }
	}
}

// Add at this level.
// Plusieurs maniËres de faire Áa :
// * on copie en local (mÈthode choisie).
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
  dataMutex.unlock();
}

void SimpleDagdaImpl::remData(const char* dataID) {
  // !!!!!!! A tester !!!!!!!!!!!!+ Ajouter suppression effective...
  std::map<string, corba_data_t>::iterator it;
  dataMutex.lock();
  it = getData()->find(dataID);
  if (it!=getData()->end()) getData()->erase(it);
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
  //std::InputIterator<corba_data_t> it;
  // Instable pour l'instant... Remis dans DAGDA v1.2 avec
  // ajout des mécanismes de déclaration auto pour des groupes de SeDs.
}
