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

#ifndef _DAGDAIMPL_HH_
#define _DAGDAIMPL_HH_
#define MAXBUFFSIZE  (getMaxMsgSize()==0 ? 4294967295U:getMaxMsgSize())

#include "Dagda.hh"
#include "common_types.hh"

#include <unistd.h>

#include <list>
#include <map>
#include <iterator>
#include <sstream>

typedef enum {DGD_CLIENT_MNGR, DGD_AGENT_MNGR, DGD_SED_MNGR} dagda_manager_type_t;

class DagdaImpl : public POA_Dagda, public PortableServer::RefCountServantBase {
public:
  DagdaImpl() : parent(NULL), ID("NoID"), data() {}
  ~DagdaImpl();

  /* CORBA part. To be remotely called. */
  virtual char* getHostname();
  virtual void subscribe(Dagda_ptr me) = 0;
  virtual void unsubscribe(Dagda_ptr me) = 0;

  virtual CORBA::Boolean lclIsDataPresent(const char* dataID) = 0;
  virtual CORBA::Boolean lvlIsDataPresent(const char* dataID) = 0;
  virtual CORBA::Boolean pfmIsDataPresent(const char* dataID) = 0;

  virtual void lclAddData(Dagda_ptr src, const corba_data_t& data) = 0;
  virtual void lvlAddData(Dagda_ptr src, const corba_data_t& data) = 0;
  virtual void pfmAddData(Dagda_ptr src, const corba_data_t& data) = 0;
  virtual void registerFile(const corba_data_t& data) = 0;

  virtual void lclRemData(const char* dataID) = 0;
  virtual void lvlRemData(const char* dataID) = 0;
  virtual void pfmRemData(const char* dataID) = 0;

  virtual void lclUpdateData(Dagda_ptr src, const corba_data_t& data) = 0;
  virtual void lvlUpdateData(Dagda_ptr src, const corba_data_t& data) = 0;
  virtual void pfmUpdateData(Dagda_ptr src, const corba_data_t& data) = 0;

  virtual SeqCorbaDataDesc_t* lclGetDataDescList() = 0;
  virtual SeqCorbaDataDesc_t* lvlGetDataDescList() = 0;
  virtual SeqCorbaDataDesc_t* pfmGetDataDescList() = 0;

  virtual corba_data_desc_t* lclGetDataDesc(const char* dataID) = 0;
  virtual corba_data_desc_t* lvlGetDataDesc(const char* dataID) = 0;
  virtual corba_data_desc_t* pfmGetDataDesc(const char* dataID) = 0;

  virtual Dagda::SeqDagda_t* lvlGetDataManagers(const char* dataID) = 0;
  virtual Dagda::SeqDagda_t* pfmGetDataManagers(const char* dataID) = 0;
  
  virtual Dagda_ptr getBestSource(Dagda_ptr dest, const char* dataID) = 0;

  virtual char* writeFile(const SeqChar& data, const char* basename,
			  CORBA::Boolean replace);
  virtual char* sendFile(const corba_data_t &data, Dagda_ptr dest);
  virtual char* recordData(const SeqChar& data, const corba_data_desc_t& dataDesc,
			   CORBA::Boolean replace);
  virtual char* sendData(const char* ID, Dagda_ptr dest);

  virtual char* downloadData(Dagda_ptr src, const corba_data_t& data) = 0;
  
  virtual void lockData(const char* dataID);
  virtual void unlockData(const char* dataID);
  virtual Dagda::dataStatus getDataStatus(const char* dataID);
  
  /* Implementation dependent functions. */
  virtual bool isDataPresent(const char* dataID) = 0;
  virtual corba_data_t* getData(const char* dataID) = 0;
  virtual void addData(const corba_data_t& data) = 0;
  virtual void remData(const char* dataID) = 0;
  virtual SeqCorbaDataDesc_t* getDataDescList() = 0;
  virtual int init(const char* ID, const char* parentID,
		   const char* dataPath, const unsigned long maxMsgSize,
		   const unsigned long diskMaxSpace,
		   const unsigned long memMaxSpace) = 0;
  // Accessors.
  void setDataPath(const char* path);
  const char* getDataPath();
  void setMaxMsgSize(const size_t maxMsgSize);
  const size_t getMaxMsgSize();
  void setDiskMaxSpace(const size_t diskMaxSpace);
  const size_t getDiskMaxSpace();
  void setMemMaxSpace(const size_t memMaxSpace);
  const size_t getMemMaxSpace();
  const size_t getUsedDiskSpace() { return usedDiskSpace; }
  const size_t getUsedMemSpace() { return usedMemSpace; }
  void useDiskSpace(size_t size) {
    usedDiskSpaceMutex.lock();
	usedDiskSpace += size;
	usedDiskSpaceMutex.unlock();
  }
  void useMemSpace(size_t size) {
    usedMemSpaceMutex.lock();
    usedMemSpace += size;
	usedMemSpaceMutex.unlock();
  }
  void freeDiskSpace(size_t size) {
    usedDiskSpaceMutex.lock();
    usedDiskSpace -= size;
	usedDiskSpaceMutex.unlock();
  }
  void freeMemSpace(size_t size) {
    usedMemSpaceMutex.lock();
    usedMemSpace -= size;
	usedMemSpaceMutex.unlock();
  }
  std::map<std::string, Dagda_ptr>* getChildren() { return &children; }
  std::map<std::string, corba_data_t>* getData() { return &data; }
  std::map<std::string, Dagda::dataStatus>* getDataStatus() { return &dataStatus; }

  Dagda_ptr getParent() { return parent; }
  void setParent(Dagda_ptr parent) { this->parent = parent; }
  void setID(char* ID) { this->ID = ID; }
  char* getID() { return CORBA::string_dup(this->ID); } // CORBA
private:
  char* ID;
  char* hostname;
  size_t maxMsgSize;
  size_t diskMaxSpace;
  size_t memMaxSpace;
  size_t usedDiskSpace;
  size_t usedMemSpace;
  Dagda_ptr parent;
  std::string dataPath;
  std::map<std::string, Dagda_ptr> children;
  std::map<std::string, corba_data_t> data;
  std::map<std::string, Dagda::dataStatus> dataStatus;
protected:
  omni_mutex dataMutex;
  omni_mutex dataStatusMutex;
  omni_mutex childrenMutex;
  omni_mutex usedDiskSpaceMutex;
  omni_mutex usedMemSpaceMutex;
};

class SimpleDagdaImpl : public DagdaImpl {
private:
  dagda_manager_type_t type;
public:
  SimpleDagdaImpl(dagda_manager_type_t t) : DagdaImpl(), type(t) { }
  ~SimpleDagdaImpl();

  virtual void subscribe(Dagda_ptr me);
  virtual void unsubscribe(Dagda_ptr me);

  virtual CORBA::Boolean lclIsDataPresent(const char* dataID);
  virtual CORBA::Boolean lvlIsDataPresent(const char* dataID);
  virtual CORBA::Boolean pfmIsDataPresent(const char* dataID);

/*  virtual corba_data_t* lclGetData(Dagda_ptr dest, const char* dataID);
  virtual corba_data_t* lvlGetData(Dagda_ptr dest, const char* dataID);
  virtual corba_data_t* pfmGetData(Dagda_ptr dest, const char* dataID);*/

  virtual void lclAddData(Dagda_ptr src, const corba_data_t& data);
  virtual void lvlAddData(Dagda_ptr src, const corba_data_t& data);
  virtual void pfmAddData(Dagda_ptr src, const corba_data_t& data);
  virtual void registerFile(const corba_data_t& data);

  virtual void lclRemData(const char* dataID);
  virtual void lvlRemData(const char* dataID);
  virtual void pfmRemData(const char* dataID);

  virtual void lclUpdateData(Dagda_ptr src, const corba_data_t& data);
  virtual void lvlUpdateData(Dagda_ptr src, const corba_data_t& data);
  virtual void pfmUpdateData(Dagda_ptr src, const corba_data_t& data);

  virtual SeqCorbaDataDesc_t* lclGetDataDescList();
  virtual SeqCorbaDataDesc_t* lvlGetDataDescList();
  virtual SeqCorbaDataDesc_t* pfmGetDataDescList();

  virtual corba_data_desc_t* lclGetDataDesc(const char* dataID);
  virtual corba_data_desc_t* lvlGetDataDesc(const char* dataID);
  virtual corba_data_desc_t* pfmGetDataDesc(const char* dataID);

  virtual Dagda::SeqDagda_t* lvlGetDataManagers(const char* dataID);
  virtual Dagda::SeqDagda_t* pfmGetDataManagers(const char* dataID);
  
  virtual Dagda_ptr getBestSource(Dagda_ptr dest, const char* dataID);

  /* Local part. */
  /* Implementation dependent functions. */
  virtual bool isDataPresent(const char* data);
  virtual corba_data_t* getData(const char* data);
  virtual void addData(const corba_data_t& data);
  virtual void remData(const char* dataID);
  virtual SeqCorbaDataDesc_t* getDataDescList();
  virtual char* downloadData(Dagda_ptr src, const corba_data_t& data);

  /* Get an iterator over children. */
  std::map<std::string, Dagda_ptr>::iterator childIterator() {
    return getChildren()->begin();
  }

  /* Get an iterator over data. */
  std::map<std::string, corba_data_t>::iterator dataIterator() {
    return DagdaImpl::getData()->begin();
  }
  
  /* To avoid a lot of "DagdaImpl::getData()... */
  std::map<std::string, corba_data_t>* getData() { return DagdaImpl::getData(); }
  
  /* Return the type of this data manager. */
  dagda_manager_type_t getType() {
	return type;
  }

  /* Initialisation. */
  virtual int init(const char* ID, const char* parentID,
		   const char* dataPath, const unsigned long maxMsgSize,
		   const unsigned long diskMaxSpace,
		   const unsigned long memMaxSpace);

};


#endif
