/**
 * @file DagdaImpl.hh
 *
 * @brief Dagda component implementation
 *
 * @author Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _DAGDAIMPL_HH_
#define _DAGDAIMPL_HH_
#define MAXBUFFSIZE (getMaxMsgSize() == 0 ? 4294967295U : getMaxMsgSize())

#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <omnithread.h>

#include "OSIndependance.hh"
#include "Dagda.hh"
#include "common_types.hh"
#include "DIET_data.h"
#include "DataRelationMgr.hh"
#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif

#include "Forwarder.hh"
#include "DagdaFwdr.hh"
#include "debug.hh"
#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif
typedef enum {DGD_CLIENT_MNGR,
              DGD_AGENT_MNGR,
              DGD_SED_MNGR} dagda_manager_type_t;

class DagdaImpl : public POA_Dagda,
public PortableServer::RefCountServantBase {
public:
  static char NoID[];

  DagdaImpl()
    : ID(NoID) {
    char host[256];

    gethostname(host, 256);
    host[255] = '\0';
    hostname = CORBA::string_dup(host);
#ifdef USE_LOG_SERVICE
    logComponent = NULL;
#endif
  }

  ~DagdaImpl();

  /* CORBA part. To be remotely called. */
  virtual char *
  getHostname();

  /**
   * @brief Add new child data manager
   * @param name  the child data mgr ref
   */
  virtual void
  subscribe(const char *name) = 0;

  /**
   * @brief Remove child data manager
   * @param name the child data mgr ref
   */
  virtual void
  unsubscribe(const char *name) = 0;

  /**
   * @brief subscribe to a new parent
   * @param parentID ID of the new parent
   */
  virtual void
  subscribeParent(const char *parentID) = 0;

  /**
   * unsubscribe from current parent
   */
  virtual void
  unsubscribeParent() = 0;

  /**
   * Check if data is stored on the Data Mgr (local, subtree or platform)
   * @param dataID  data identifier
   */
  virtual CORBA::Boolean
  lclIsDataPresent(const char *dataID) = 0;

  virtual CORBA::Boolean
  lvlIsDataPresent(const char *dataID) = 0;

  virtual CORBA::Boolean
  pfmIsDataPresent(const char *dataID) = 0;

  /**
   * Store a data (description + value) on the local data manager
   * This will trigger the download of the data value from the src data mgr
   * @param src   the source data mgr
   * @param data  the data handle (ID already defined)
   */
  virtual void
  lclAddData(const char *src, const corba_data_t &data) = 0;

  /**
   * Store a data (description + value) at a given level of the platform
   * @param src   the source data mgr
   * @param data  the data handle (ID already defined)
   */
  virtual void
  lvlAddData(const char *src, const corba_data_t &data) = 0;

  /**
   * Store a data (description + value) on the platform
   * This lets the platform decide where it is best to store the data
   * @param src   the source data mgr
   * @param data  the data handle (ID already defined)
   */
  virtual void
  pfmAddData(const char *src, const corba_data_t &data) = 0;

  /**
   * Register a file on the local data manager (for file sharing)
   * @param data  the data handle
   */
  virtual void
  registerFile(const corba_data_t &data) = 0;

  /**
   * Add an element to a container (on the local Data Mgr)
   * The data element is not necessarily stored on the local data mgr
   * @param containerID the id of the container
   * @param dataID      the id of the data element (can be NULL)
   * @param index       the index of the element in the container
   * @param flag        flag (used for NULL elements)
   */
  virtual void
  lclAddContainerElt(const char *containerID,
                     const char *dataID,
                     CORBA::Long index,
                     CORBA::Long flag) = 0;

  /**
   * Get the number of elements of a container
   *
   * This method returns only the nb of elements directly contained in this
   * container, not elements indirectly contained in sub-containers.
   * Note: does not count the elements but uses an internal counter
   *
   * @param containerID the id of the container
   */
  virtual CORBA::Long
  lclGetContainerSize(const char *containerID) = 0;

  /**
   * Get the list of elements of a container
   *
   * @param containerID the id of the container
   * @param dataIDSeq   a sequence of strings (not pre-allocated)
   * @param flagSeq     a sequence of long integers (not pre-allocated)
   * @param ordered     if true, sort the elements using index field
   */
  virtual void
  lclGetContainerElts(const char *containerID,
                      SeqString &dataIDSeq,
                      SeqLong &flagSeq,
                      CORBA::Boolean ordered) = 0;

  virtual void
  lclRemData(const char *dataID) = 0;

  virtual void
  lvlRemData(const char *dataID) = 0;

  virtual void
  pfmRemData(const char *dataID) = 0;

  virtual void
  lclUpdateData(const char *src, const corba_data_t &data) = 0;

  virtual void
  lvlUpdateData(const char *src, const corba_data_t &data) = 0;

  virtual void
  pfmUpdateData(const char *src, const corba_data_t &data) = 0;

  virtual void
  lclReplicate(const char *dataID, CORBA::Long target,
               const char *pattern, CORBA::Boolean replace) = 0;

  virtual void
  lvlReplicate(const char *dataID, CORBA::Long,
               const char *pattern, CORBA::Boolean replace) = 0;

  virtual void
  pfmReplicate(const char *dataID, CORBA::Long,
               const char *pattern, CORBA::Boolean replace) = 0;

  virtual SeqCorbaDataDesc_t *
  lclGetDataDescList() = 0;

  virtual SeqCorbaDataDesc_t *
  lvlGetDataDescList() = 0;

  virtual SeqCorbaDataDesc_t *
  pfmGetDataDescList() = 0;

  virtual corba_data_desc_t *
  lclGetDataDesc(const char *dataID) = 0;

  virtual corba_data_desc_t *
  lvlGetDataDesc(const char *dataID) = 0;

  virtual corba_data_desc_t *
  pfmGetDataDesc(const char *dataID) = 0;


  virtual SeqString *
  lvlGetDataManagers(const char *dataID) = 0;

  virtual SeqString *
  pfmGetDataManagers(const char *dataID) = 0;

  virtual char *
  getBestSource(const char *dest, const char *dataID) = 0;

  virtual char *
  writeFile(const SeqChar &data, const char *basename,
            CORBA::Boolean replace);

  virtual char *
  sendFile(const corba_data_t &data, const char *dest);

  /**
   * Store binary data (block) in the local value buffer of the data
   * @param data      the buffer containing the binary data
   * @param dataDesc  the data description
   * @param replace   erase current content if yes
   * @param offset    start nb of bytes from the beginning of the buffer
   * @return THE ID OF THE DATA ON THE DESTINATION => WHY WOULD IT BE DIFFERENT?
   */
  virtual char *
  recordData(const SeqChar &data, const corba_data_desc_t &dataDesc,
             CORBA::Boolean replace, CORBA::Long offset);

  /**
   * Send a data (value) to a remote data manager
   * @param ID    the ID of the data
   * @param dest  the remote data mgr ref
   * @return THE ID OF THE DATA ON THE DESTINATION => WHY WOULD IT BE DIFFERENT?
   */
  virtual char *
  sendData(const char *ID, const char *dest);

  /**
   * Send a container to a remote data manager
   * @param containerID    the ID of the data
   * @param dest  the remote data mgr ref
   * @param sendElements  if true, also send each element of the container
   * @return THE ID OF THE DATA ON THE DESTINATION => WHY WOULD IT BE DIFFERENT?
   */
  virtual char *
  sendContainer(const char *containerID, const char *dest,
                CORBA::Boolean sendElements);

  /**
   * Download data from the src data manager
   * (will call the appropriate send method depending on data type)
   * @param src   the source data mgr ref
   * @param data  the data handle
   * @return THE ID OF THE DATA ON THE DESTINATION => WHY WOULD IT BE DIFFERENT?
   */
  virtual char *
  downloadData(Dagda_ptr src, const corba_data_t &data) = 0;

  virtual void
  lockData(const char *dataID);

  virtual void
  unlockData(const char *dataID);

  virtual Dagda::dataStatus
  getDataStatus(const char *dataID);

  virtual void
  setDataStatus(const char *dataID, Dagda::dataStatus status);

  /* Implementation dependent functions. */
  virtual bool
  isDataPresent(const char *dataID) = 0;

  virtual corba_data_t *
  getData(const char *dataID) = 0;

  /**
   * Register a data on the local data manager
   * This will only store the description of the data on the dataMgr (except
   * for scalar data which will be stored with its value)
   * @param data  the data handle (ID already defined)
   */
  virtual corba_data_t *
  addData(const corba_data_t &data) = 0;

  virtual void
  remData(const char *dataID) = 0;

  virtual SeqCorbaDataDesc_t *
  getDataDescList() = 0;

  virtual int
  init(const char *ID, const char *parentID,
       const char *dataPath, const unsigned long maxMsgSize,
       const unsigned long diskMaxSpace,
       const unsigned long memMaxSpace) = 0;

  // Accessors.
  void
  setDataPath(const char *path);

  const char *
  getDataPath();

  void
  setMaxMsgSize(const size_t maxMsgSize);

  size_t
  getMaxMsgSize();

  void
  setDiskMaxSpace(const size_t diskMaxSpace);

  size_t
  getDiskMaxSpace();

  void
  setMemMaxSpace(const size_t memMaxSpace);

  size_t
  getMemMaxSpace();

  size_t
  getUsedDiskSpace() {
    return usedDiskSpace;
  }

  size_t
  getUsedMemSpace() {
    return usedMemSpace;
  }

  void
  useDiskSpace(size_t size) {
    usedDiskSpaceMutex.lock();
    usedDiskSpace += size;
    usedDiskSpaceMutex.unlock();
  }

  void
  useMemSpace(size_t size) {
    usedMemSpaceMutex.lock();
    usedMemSpace += size;
    usedMemSpaceMutex.unlock();
  }

  void
  freeDiskSpace(size_t size) {
    usedDiskSpaceMutex.lock();
    if (usedDiskSpace < size) {
      WARNING("Used disk space will be negative. Setting it to 0.");
      usedDiskSpace = 0;
    } else {
      usedDiskSpace -= size;
    }
    usedDiskSpaceMutex.unlock();
  }

  void
  freeMemSpace(size_t size) {
    usedMemSpaceMutex.lock();
    if (usedMemSpace < size) {
      WARNING("Used memory space will be negative. Setting it to 0.");
      usedMemSpace = 0;
    } else {
      usedMemSpace -= size;
    }
    usedMemSpaceMutex.unlock();
  }

  std::map<std::string, Dagda_ptr> *
  getChildren() {
    return &children;
  }

  std::map<std::string, corba_data_t> *
  getData() {
    return &data;
  }

  std::map<std::string, Dagda::dataStatus> *
  getDataStatus() {
    return &dataStatus;
  }

  Dagda_ptr
  getParent() {
    return parent;
  }

  void
  setParent(Dagda_ptr parent) {
    this->parent = parent;
  }

  void
  setID(char *ID) {
    this->ID = ID;
  }

  char *
  getID() {
    return CORBA::string_dup(this->ID);
  }  // CORBA

  std::string
  getIDstr() const {
    return this->ID;
  }

  std::string
  getStateFile() {
    return stateFile;
  }

#ifdef USE_LOG_SERVICE
  DietLogComponent *
  getLogComponent() {
    return logComponent;
  }

  void
  setLogComponent(DietLogComponent *comp) {
    logComponent = comp;
  }
#endif  // USE_LOG_SERVICE

  void
  setStateFile(std::string path) {
    stateFile = path;
  }

  void
  saveState();

  void
  restoreState();

  void
  checkpointState();

protected:
  DataRelationMgr *
  getContainerRelationMgr() {
    return containerRelationMgr;
  }

  DataRelationMgr *containerRelationMgr;  // container-elements relationships
  omni_mutex dataMutex;
  omni_mutex dataStatusMutex;
  omni_mutex childrenMutex;
  omni_mutex usedDiskSpaceMutex;
  omni_mutex usedMemSpaceMutex;

private:
  size_t
  make_corba_data(corba_data_t &data, diet_data_type_t type,
                  diet_base_type_t base_type, diet_persistence_mode_t mode,
                  size_t nb_r, size_t nb_c, diet_matrix_order_t order,
                  void *value, char *path);

  int
  writeDataDesc(corba_data_t &data, std::ofstream &file);

  int
  writeData(corba_data_t &data, std::ofstream &file);

  int
  readData(corba_data_t &data, std::ifstream &file);

  char *ID;
  char *hostname;
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
  std::string stateFile;
#ifdef USE_LOG_SERVICE
  DietLogComponent *logComponent;
#endif
};

class SimpleDagdaImpl : public DagdaImpl {
public:
  explicit
  SimpleDagdaImpl(dagda_manager_type_t t)
    : DagdaImpl(), type(t) {
  }

  ~SimpleDagdaImpl();

  virtual void
  subscribe(const char *name);

  virtual void
  unsubscribe(const char *name);

  virtual void
  subscribeParent(const char *parentID);

  virtual void
  unsubscribeParent();

  virtual CORBA::Boolean
  lclIsDataPresent(const char *dataID);

  virtual CORBA::Boolean
  lvlIsDataPresent(const char *dataID);

  virtual CORBA::Boolean
  pfmIsDataPresent(const char *dataID);

  virtual void
  lclAddData(const char *src, const corba_data_t &data);

  virtual void
  lvlAddData(const char *src, const corba_data_t &data);

  virtual void
  pfmAddData(const char *src, const corba_data_t &data);

  virtual void
  registerFile(const corba_data_t &data);

  virtual void
  lclAddContainerElt(const char *containerID,
                     const char *dataID,
                     CORBA::Long index,
                     CORBA::Long flag);

  virtual CORBA::Long
  lclGetContainerSize(const char *containerID);

  virtual void
  lclGetContainerElts(const char *containerID,
                      SeqString &dataIDSeq,
                      SeqLong &flagSeq,
                      CORBA::Boolean ordered);

  virtual void
  lclRemData(const char *dataID);

  virtual void
  lvlRemData(const char *dataID);

  virtual void
  pfmRemData(const char *dataID);

  virtual void
  lclUpdateData(const char *src, const corba_data_t &data);

  virtual void
  lvlUpdateData(const char *src, const corba_data_t &data);

  virtual void
  pfmUpdateData(const char *src, const corba_data_t &data);

  virtual void
  lclReplicate(const char *dataID, CORBA::Long target,
               const char *pattern, bool replace);

  virtual void
  lvlReplicate(const char *dataID, CORBA::Long target,
               const char *pattern, bool replace);

  virtual void
  pfmReplicate(const char *dataID, CORBA::Long target,
               const char *pattern, bool replace);

  virtual SeqCorbaDataDesc_t *
  lclGetDataDescList();

  virtual SeqCorbaDataDesc_t *
  lvlGetDataDescList();

  virtual SeqCorbaDataDesc_t *
  pfmGetDataDescList();

  virtual corba_data_desc_t *
  lclGetDataDesc(const char *dataID);

  virtual corba_data_desc_t *
  lvlGetDataDesc(const char *dataID);

  virtual corba_data_desc_t *
  pfmGetDataDesc(const char *dataID);

  virtual SeqString *
  lvlGetDataManagers(const char *dataID);

  virtual SeqString *
  pfmGetDataManagers(const char *dataID);

  virtual char *
  getBestSource(const char *dest, const char *dataID);

  /* Local part. */
  /* Implementation dependent functions. */
  virtual bool
  isDataPresent(const char *data);

  virtual corba_data_t *
  getData(const char *data);

  virtual corba_data_t *
  addData(const corba_data_t &data);

  virtual void
  remData(const char *dataID);

  virtual SeqCorbaDataDesc_t *
  getDataDescList();

  virtual char *
  downloadData(Dagda_ptr src, const corba_data_t &data);

  /* Get an iterator over children. */
  std::map<std::string, Dagda_ptr>::iterator
  childIterator() {
    return getChildren()->begin();
  }

  /* Get an iterator over data. */
  std::map<std::string, corba_data_t>::iterator
  dataIterator() {
    return DagdaImpl::getData()->begin();
  }

  /* To avoid a lot of "DagdaImpl::getData()... */
  std::map<std::string, corba_data_t> *
  getData() {
    return DagdaImpl::getData();
  }

  /* Return the type of this data manager. */
  dagda_manager_type_t
  getType() {
    return type;
  }

  /* Initialisation. */
  virtual int
  init(const char *ID, const char *parentID,
       const char *dataPath, const unsigned long maxMsgSize,
       const unsigned long diskMaxSpace,
       const unsigned long memMaxSpace);

private:
  dagda_manager_type_t type;
};

class DIET_API_LIB DagdaFwdrImpl : public POA_DagdaFwdr,
public PortableServer::RefCountServantBase {
public:
  DagdaFwdrImpl(Forwarder_ptr fwdr, const char *objName);

  virtual void
  subscribe(const char *name);

  virtual void
  unsubscribe(const char *name);

  virtual void
  subscribeParent(const char *parentID);

  virtual void
  unsubscribeParent();

  /* ------------ */
  virtual char *
  writeFile(const SeqChar &data, const char *basename,
            CORBA::Boolean replace);

  virtual char *
  sendFile(const corba_data_t &data, const char *dest);

  virtual char *
  recordData(const SeqChar &data, const corba_data_desc_t &dataDesc,
             CORBA::Boolean replace, CORBA::Long offset);

  virtual char *
  sendData(const char *ID, const char *dest);

  virtual char *
  sendContainer(const char *containerID, const char *dest,
                CORBA::Boolean sendElements);
  virtual void
  lockData(const char *dataID);

  virtual void
  unlockData(const char *dataID);

  virtual Dagda::dataStatus
  getDataStatus(const char *dataID);

  /* ------------ */


  virtual CORBA::Boolean
  lclIsDataPresent(const char *dataID);

  virtual CORBA::Boolean
  lvlIsDataPresent(const char *dataID);

  virtual CORBA::Boolean
  pfmIsDataPresent(const char *dataID);

  virtual void
  lclAddData(const char *src, const corba_data_t &data);

  virtual void
  lvlAddData(const char *src, const corba_data_t &data);

  virtual void
  pfmAddData(const char *src, const corba_data_t &data);

  virtual void
  registerFile(const corba_data_t &data);

  virtual void
  lclAddContainerElt(const char *containerID,
                     const char *dataID,
                     CORBA::Long index,
                     CORBA::Long flag);

  virtual CORBA::Long
  lclGetContainerSize(const char *containerID);

  virtual void
  lclGetContainerElts(const char *containerID,
                      SeqString &dataIDSeq,
                      SeqLong &flagSeq,
                      CORBA::Boolean ordered);

  virtual void
  lclRemData(const char *dataID);

  virtual void
  lvlRemData(const char *dataID);

  virtual void
  pfmRemData(const char *dataID);

  virtual void
  lclUpdateData(const char *src, const corba_data_t &data);

  virtual void
  lvlUpdateData(const char *src, const corba_data_t &data);

  virtual void
  pfmUpdateData(const char *src, const corba_data_t &data);

  virtual void
  lclReplicate(const char *dataID, CORBA::Long target,
               const char *pattern, bool replace);

  virtual void
  lvlReplicate(const char *dataID, CORBA::Long target,
               const char *pattern, bool replace);

  virtual void
  pfmReplicate(const char *dataID, CORBA::Long target,
               const char *pattern, bool replace);

  virtual SeqCorbaDataDesc_t *
  lclGetDataDescList();

  virtual SeqCorbaDataDesc_t *
  lvlGetDataDescList();

  virtual SeqCorbaDataDesc_t *
  pfmGetDataDescList();

  virtual corba_data_desc_t *
  lclGetDataDesc(const char *dataID);

  virtual corba_data_desc_t *
  lvlGetDataDesc(const char *dataID);

  virtual corba_data_desc_t *
  pfmGetDataDesc(const char *dataID);

  virtual SeqString *
  lvlGetDataManagers(const char *dataID);

  virtual SeqString *
  pfmGetDataManagers(const char *dataID);

  virtual char *
  getBestSource(const char *dest, const char *dataID);

  virtual char *
  getID();

  virtual void
  checkpointState();

  virtual char *
  getHostname();

private:
  Forwarder_ptr forwarder;
  char *objName;
};
#endif /* ifndef _DAGDAIMPL_HH_ */
