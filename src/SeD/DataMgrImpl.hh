/****************************************************************************/
/* DIET Data Manager implementation header                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Bruno DEL FABBRO (Bruno.DelFabbro@lifc.univ-fcomte)                 */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2004/02/27 10:26:37  bdelfabr
 * let DIET_PERSISTENCE_MODE set to 1, coding standard
 *
 * Revision 1.7  2003/12/01 14:49:30  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.6  2003/11/10 14:03:10  bdelfabr
 * adding methods that allow tranfer time between servers to be computed
 *
 * Revision 1.5  2003/10/14 20:29:06  bdelfabr
 * adding print methods (PERSISTENT mode only)
 *
 * Revision 1.4  2003/10/01 07:06:42  bdelfabr
 * removing DataManagerID.hh which is not used
 *
 * Revision 1.3  2003/09/30 15:08:57  bdelfabr
 * dlist are replaced by stl map²
 *
 * Revision 1.2  2003/09/24 09:16:18  pcombes
 * Merge corba_DataMgr_desc_t and corba_data_desc_t.
 * DataMgr does not need a name: use its reference.
 *
 * Revision 1.1  2003/09/22 21:07:21  pcombes
 * Set all the modules and their interfaces for data persistency.
 ***************************************************************************/

#ifndef _DATAMGRIMPL_HH_
#define _DATAMGRIMPL_HH_

#include <map>


#include "DataMgr.hh"
#include "ChildID.hh"
#include "common_types.hh"
#include "DIET_data_internal.hh"
#include "LocMgr.hh"
#include "ServiceTable.hh"
#include "LinkedList.hh"
#include "ts_container/ts_map.hh"

/** defines string comparison for map management */

struct cmpID
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};



class DataMgrImpl : public POA_DataMgr,
		    public PortableServer::RefCountServantBase
{
public:                                              

  DataMgrImpl();
  ~DataMgrImpl();

  int
  run();
  /** look for data presence */
  bool
  dataLookup(char* argID);
  /** update add and remove data */
  void
  updateDataRefOrder(corba_data_t& arg);
  /** Copy data from list to SeD structure */
  void
  getData(corba_data_t& arg);
  /** addData to the dataDescLsit */
  void
  addData(corba_data_t& arg, int inout);
  /** invoke remote DataManger, it has to send the data */
  virtual void
  putData(const char* argID, const DataMgr_ptr me);
  /** remove data from the list */
  virtual CORBA::Long
  rmDataRef(const char* argID);
  /** looking for dataManager localization */
  virtual DataMgr_ptr
  whereData(const char* argID);
  /** send data to a DataManager */
  virtual void
  sendData(corba_data_t& arg);
  void 
  changePath(corba_data_t &arg, char *path);
 
  virtual char *
  setMyName();
  /* print list of data owned */
  void 
  printList();
  
  void
  printList1();

  void
  updateDataProperty(corba_data_t& arg);
  void
  updateDataList(corba_data_t& arg);
#if 0
  void
  rmAllData();
#endif
  /** looking for dataManager owner */ 
  virtual char *
  whichSeDOwner(const char * argId);
  /** looking for dataManager owner */ 
  char *
  whichDataMgr(const char * argId);
  
private:
 
  /**************************************************************************/
  /* Private fields                                                         */
  /**************************************************************************/
  
  /** Local host name */
  char localHostName[257];
  
  /** ID of this DataMgr amongst the children of its parent */
  ChildID childID;
  /* reference of the parent Data Location Manager */
  LocMgr_var parent;
  /** defines the type List of data */
  typedef ts_map<const char*,corba_data_t,cmpID> dietDataDescList_t;
  /** mutex list */
  typedef ts_map<const char *,omni_mutex,cmpID> dietDataIDLockList_t;
  
  /** List of the data held by the Data Manager*/
  dietDataDescList_t dataDescList;
  
  /** List of lock to manage data transmission */
  dietDataIDLockList_t lockList;


  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/
  /** copy corba_data_desc_t in corba_data_t */
  corba_data_t
  moveToCorbaDataT(corba_data_desc_t& dataDesc);
  /** copy a list element to a corba_data_t argument */
  void
  cpEltListToDataT(corba_data_t *arg);
  /** mutex management */
  bool
  isInLockList(char* argID);
  void
  dataIDUnlock(const corba_data_id_t& dataId);
  void
  dataIDLock(const corba_data_id_t& cdataId);
  /** remove data from list */
  void
  rmDataDescFromList(char* argID);
  /** add data in the list */
  void
  addDataDescToList(corba_data_t* arg, int inout);
  /** mutex management */
  void
  addDataIDToLockList(char* argID);
  void
  rmDataFromIDList(char* argID);
  /** copy a corba_data_t to a corba_data_desc_t argument */
  corba_data_desc_t
  moveToCorbaDataDesc(corba_data_t& arg);

};

#endif // _DATAMGRIMPL_HH_
