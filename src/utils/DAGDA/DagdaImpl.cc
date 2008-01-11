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

char* DagdaImpl::getID() {
  return CORBA::string_dup(ID);
}

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

char* DagdaImpl::recordData(const SeqChar& data,
			    const corba_data_desc_t& dataDesc,
			    CORBA::Boolean replace) {
  unsigned long offset;

  string dataId(dataDesc.id.idNumber);
  if (this->data.find(dataId)==this->data.end()) {
    TRACE_TEXT(TRACE_MAIN_STEPS, "Add data " << dataDesc.id.idNumber
								<< endl);
	corba_data_t newData;
    newData.desc = dataDesc;
    this->data[dataId]=newData;
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "\tRecord " << data.length() << " bytes for the data "
                        << dataDesc.id.idNumber << endl);
  if (replace) {
    this->data[dataId].value.length(data.length());
    offset=0;
  } else {
    offset=this->data[dataId].value.length();
    this->data[dataId].value.length(data.length()+offset);
  }
  for (int i=0; i<data.length(); ++i) {
    this->data[dataId].value[i+offset]=data[i];
  }

  return CORBA::string_dup(this->data[dataId].desc.id.idNumber);
}

char* DagdaImpl::sendData(const char* dataId, Dagda_ptr dest) {
  if (data.find(dataId)==data.end())
    throw Dagda::DataNotFound(dataId);

  TRACE_TEXT(TRACE_MAIN_STEPS, "*** Sending data " << dataId << endl);

  unsigned long wrote = 0;
  unsigned long dataSize = data_sizeof(&data[dataId].desc);
  unsigned long nBlocks =  dataSize / getMaxMsgSize() + 1;

  bool replace = true;
  char* distID=NULL;
  for (int i=0; i<nBlocks; ++i) {
    int toSend = (dataSize-wrote > getMaxMsgSize() ?
		  getMaxMsgSize():(dataSize-wrote));
    TRACE_TEXT(TRACE_ALL_STEPS, "\tSend " <<  toSend << " bytes..." << endl);
    SeqChar buffer(toSend, toSend, data[dataId].value.get_buffer()+wrote);

    if (distID!=NULL) CORBA::string_free(distID);
    distID=dest->recordData(buffer, data[dataId].desc, replace);

    wrote+=toSend;
    replace=false;
  }
  return distID;
}

void SimpleDagdaImpl::subscribe(Dagda_ptr me) {
  string name(me->getID());
  map<string, Dagda_ptr>::iterator it = getChildren().find(name);
  
  if (it!=children.end()) children.erase(name);
  children[name]=Dagda::_duplicate(me);
}

void SimpleDagdaImpl::unsubscribe(Dagda_ptr me) {
  string name(me->getID());
  map<string, Dagda_ptr>::iterator it = children.find(name);

  if (it!=children.end()) children.erase(name);
}

SimpleDagdaImpl::~SimpleDagdaImpl() {
  //CORBA::string_free(this->ID);
}

// Initialize this data manager.
int SimpleDagdaImpl::init(const char* ID, const char* parentID,
			  const char* dataPath, const unsigned long maxMsgSize,
			  const unsigned long diskMaxSpace,
			  const unsigned long memMaxSpace) {
  this->ID = CORBA::string_dup(ID);

  if (ORBMgr::bindObjToName(_this(), ORBMgr::DATAMGR, this->ID)) {
    ERROR("Dagda: could not declare myself as " << this->ID, 1);
  }
  if (parentID==NULL) parent=NULL;
  else {
	parentID = CORBA::string_dup(parentID);

    parent =
      Dagda::_duplicate(Dagda::_narrow(ORBMgr::getObjReference(ORBMgr::DATAMGR,
							   parentID)));
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, 
	     "## Launch Dagda data manager " << this->ID << endl);
  TRACE_TEXT(TRACE_ALL_STEPS,
	     "IOR : " << ORBMgr::getIORString(_this()) << endl);
		 

  if (parent!=NULL) parent->subscribe(_this());

  setMaxMsgSize(maxMsgSize);
  setDiskMaxSpace(diskMaxSpace);
  setMemMaxSpace(memMaxSpace);
  setDataPath(dataPath);
  return 0;
}

/* Return true if the data is on the node. */
CORBA::Boolean SimpleDagdaImpl::lclIsDataPresent(const char* dataID) {
  return isDataPresent(dataID);
}

/* Return true if the data is present locally. */
CORBA::Boolean SimpleDagdaImpl::lvlIsDataPresent(const char* dataID) {
  std::map<string,Dagda_ptr>::iterator itch;

  if (lclIsDataPresent(dataID)) return true;
  for (itch=children.begin();itch!=children.end();++itch) {
    if ((*itch).second->lvlIsDataPresent(dataID)) return true;
  }
  return false;
}

/* If the data is present locally return true else calls parent to find
   it. */
CORBA::Boolean SimpleDagdaImpl::pfmIsDataPresent(const char* dataID) {
  if (lclIsDataPresent(dataID)) return true;
  if (parent==NULL)
    return lvlIsDataPresent(dataID);
  return parent->pfmIsDataPresent(dataID);
}

// Returns the data from this data manager.
corba_data_t* SimpleDagdaImpl::lclGetData(Dagda_ptr dest, const char* dataID) {
  corba_data_t* found = getData(dataID);
  
  if (dest==_this()) return found;
  
  corba_data_t* result = new corba_data_t();

  if (found==NULL) throw Dagda::DataNotFound(dataID);
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
corba_data_t* SimpleDagdaImpl::lvlGetData(Dagda_ptr dest, const char* dataID) {
  std::map<string, Dagda_ptr>::iterator itch;

  if (lclIsDataPresent(dataID))
    return lclGetData(dest, dataID);

  for (itch=children.begin();itch!=children.end();++itch)
    if ((*itch).second->lvlIsDataPresent(dataID))
      return (*itch).second->lvlGetData(dest, dataID);

  throw Dagda::DataNotFound(dataID);
}

// Look if the data is present on this level. Otherwise ask the parent
// to get it.
// This is the simplest implementation. There is many way to do that.
corba_data_t* SimpleDagdaImpl::pfmGetData(Dagda_ptr dest, const char* dataID) {
  if (lvlIsDataPresent(dataID)) return lvlGetData(dest, dataID);
  if (parent==NULL)
    return lvlGetData(dest, dataID);
  return parent->pfmGetData(dest, dataID);
}

// This method downloads the data from the node src.
char* SimpleDagdaImpl::downloadData(Dagda_ptr src, const corba_data_t& data) {
  if (data.desc.specific._d()==DIET_FILE)
    return src->sendFile(data, _this());
  else
    return src->sendData(data.desc.id.idNumber, _this());
}

// Add a data locally.
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
        }
	  } else {
	    char* dataID;
		addData(data);
		dataID = downloadData(src, data);
	  }
	}
}

// Add at this level.
// Plusieurs maniËres de faire Áa :
// * on copie en local (mÈthode choisie).
// * on essaye de copier en local. Si ce n'est pas possible, on essaye sur
//   les noeuds enfants.
// * Utilisation d'une heuristique pour choisir le noeud.
void SimpleDagdaImpl::lvlAddData(Dagda_ptr src, const corba_data_t& data) {
  lclAddData(src, data);
}

// Simple implementation.
// Many way to do that... Ex :
// * Control if there is enough local storage capacity to store the data.
//   Otherwise ask to "parent" or "children" to store it. (Recursively).
void SimpleDagdaImpl::pfmAddData(Dagda_ptr src, const corba_data_t& data) {
  // Control if the data is already on the platform ???
  lvlAddData(src, data);
}

// Simple implementation.
void SimpleDagdaImpl::lclRemData(const char* dataID) {
  remData(dataID);
}

// SeD have no child...
void SimpleDagdaImpl::lvlRemData(const char* dataID) {
  std::map<string, Dagda_ptr>::iterator itch;
  lclRemData(dataID);
  for (itch=children.begin();itch!=children.end();++itch)
    (*itch).second->lvlRemData(dataID);
}

// Simple implementation :
// Should take into account the previous remove.
void SimpleDagdaImpl::pfmRemData(const char* dataID) {
  if (parent==NULL)
    lvlRemData(dataID);
  else
    parent->pfmRemData(dataID);
}

// Simple implementation : remove and add.
void SimpleDagdaImpl::lclUpdateData(Dagda_ptr src, const corba_data_t& data) {
  lclRemData(data.desc.id.idNumber);
  lclAddData(src, data);
}

// 
void SimpleDagdaImpl::lvlUpdateData(Dagda_ptr src, const corba_data_t& data) {
  std::map<string,Dagda_ptr>::iterator itch;
  if (lclIsDataPresent(data.desc.id.idNumber))
    lclUpdateData(src, data);
  for (itch=children.begin();itch!=children.end();++itch)
    (*itch).second->lvlUpdateData(src, data);
}

// Simple implementation.
void SimpleDagdaImpl::pfmUpdateData(Dagda_ptr src, const corba_data_t& data) {
  if (parent==NULL)
    lvlUpdateData(src, data);
  else
    parent->pfmUpdateData(src, data);
}

// To get the descriptions of the data locally stored.
SeqCorbaDataDesc_t* SimpleDagdaImpl::lclGetDataDescList() {
  return getDataDescList();
}

// 
SeqCorbaDataDesc_t* SimpleDagdaImpl::lvlGetDataDescList() {
  std::map<string,Dagda_ptr>::iterator itch;
  SeqCorbaDataDesc_t* local = lclGetDataDescList();
  std::map<char*,corba_data_desc_t> dataMap;
  SeqCorbaDataDesc_t* result = new SeqCorbaDataDesc_t();

  for (int i=0; i<local->length(); i++)
    dataMap[(*local)[i].id.idNumber]=(*local)[i];
  delete local;

  for (itch=children.begin();itch!=children.end();++itch) {
    SeqCorbaDataDesc_t* childList = (*itch).second->lvlGetDataDescList();
    for (int j=0; j<childList->length(); ++j)
      dataMap[(*childList)[j].id.idNumber]=(*childList)[j];
    delete childList;
  }

  std::map<char*,corba_data_desc_t>::iterator itmap;

  result->length(dataMap.size());
  int i=0;
  for (itmap=dataMap.begin(); itmap!=dataMap.end(); ++itmap)
    (*result)[i++]=(*itmap).second;

  return result;  
}

// Returns the descriptions of all the data stored on the
// platform. Recursive call to MA's data manager.
SeqCorbaDataDesc_t* SimpleDagdaImpl::pfmGetDataDescList() {
  if (parent==NULL) return lvlGetDataDescList();
  return parent->pfmGetDataDescList();
}

// Returns the description of a data. If not present, throws an exception.
corba_data_desc_t* SimpleDagdaImpl::lclGetDataDesc(const char* dataID) {
  if (lclIsDataPresent(dataID)) {
    corba_data_desc_t* result = new corba_data_desc_t(data[dataID].desc);
	result->id.idNumber = CORBA::string_dup(dataID);
	return result;
  }
  throw Dagda::DataNotFound(dataID);
}

corba_data_desc_t* SimpleDagdaImpl::lvlGetDataDesc(const char* dataID) {
  std::map<string,Dagda_ptr>::iterator itch;

  if (lclIsDataPresent(dataID))
    return lclGetDataDesc(dataID);

  for (itch=children.begin();itch!=children.end();++itch) {
    if ((*itch).second->lvlIsDataPresent(dataID))
      return (*itch).second->lvlGetDataDesc(dataID);
  }
  throw Dagda::DataNotFound(dataID);
}

corba_data_desc_t* SimpleDagdaImpl::pfmGetDataDesc(const char* dataID) {
  if (lclIsDataPresent(dataID))
    return lclGetDataDesc(dataID);
  if (parent==NULL)
    return lvlGetDataDesc(dataID);
  return parent->pfmGetDataDesc(dataID);
}

SeqDagda_t* SimpleDagdaImpl::lvlGetDataManagers(const char* dataID) {
  std::map<string,Dagda_ptr>::iterator itch;
  SeqDagda_t* result = new SeqDagda_t();
  std::list<Dagda_ptr> dtmList;
  if (lclIsDataPresent(dataID))
    dtmList.push_back(Dagda::_duplicate(_this()));
  for (itch=children.begin();itch!=children.end();++itch) {
    SeqDagda_t* managers = (*itch).second->lvlGetDataManagers(dataID);
    for (int j=0; j< managers->length(); ++j)
      dtmList.push_back(Dagda::_duplicate((*managers)[j]));
    delete managers;
  }
    
  std::list<Dagda_ptr>::iterator itlist;
  int i=0;
  result->length(dtmList.size());

  for (itlist=dtmList.begin(); itlist!=dtmList.end(); ++itlist)
    (*result)[i++]=*itlist;
    
  return result;
}

// Returns all the data managers of a data.
SeqDagda_t* SimpleDagdaImpl::pfmGetDataManagers(const char* dataID) {
  if (parent==NULL) return lvlGetDataManagers(dataID);
  return parent->pfmGetDataManagers(dataID);
}

/**/

bool SimpleDagdaImpl::isDataPresent(const char* dataID) {
  std::map<string, corba_data_t>::iterator it;
  it = data.find(dataID);
  return (it!=data.end());
}

corba_data_t* SimpleDagdaImpl::getData(const char* dataID) {
  return &data[dataID];
}

void SimpleDagdaImpl::addData(const corba_data_t& data) {
  char* dataManager = ORBMgr::getIORString(DagdaFactory::getDataManager()->_this());
  TRACE_TEXT(TRACE_ALL_STEPS, "Adding data " << data.desc.id.idNumber << " to this "
			 << "data manager." << endl);
  this->data[string(data.desc.id.idNumber)]=data;
  this->data[string(data.desc.id.idNumber)].desc.dataManager = CORBA::string_dup(dataManager);
}

void SimpleDagdaImpl::remData(const char* dataID) {
  // !!!!!!! A tester !!!!!!!!!!!!+ Ajouter suppression effective...
  std::map<string, corba_data_t>::iterator it;
  it = data.find(dataID);
  if (it!=data.end()) data.erase(it);
}

SeqCorbaDataDesc_t* SimpleDagdaImpl::getDataDescList() {
  std::map<string, corba_data_t>::iterator it;
  SeqCorbaDataDesc_t* result = new SeqCorbaDataDesc_t();
  int i=0;

  result->length(data.size());
  for (it=data.begin(); it!=data.end(); ++it)
    (*result)[i++]=it->second.desc;

  return result;
}

void SimpleDagdaImpl::registerFile(const corba_data_t& data) {
  //std::InputIterator<corba_data_t> it;
  // Instable pour l'instant... Remis dans DAGDA v1.2 avec
  // ajout des mécanismes de déclaration auto pour des groupes de SeDs.
}
