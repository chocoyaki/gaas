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
#include "dietTypes.hh"
#include "LocMgr.hh"
#include "ServiceTable.hh"
#include "LinkedList.hh"

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
  bool
  dataLookup(char* id);
  void
  updateDataRefOrder(corba_data_t& dataDesc);
  void
  getData(corba_data_t& arg);
  void
  addData(corba_data_t& dataDesc, int inout);

  virtual void
  putData(const char* argID, const DataMgr_ptr me);
  virtual void
  rmDataRef(const char* argID);

  virtual DataMgr_ptr
  whereData(const char* argID);

  virtual void
  sendData(corba_data_t& arg);


  void
  updateDataProperty(corba_data_t& dataDesc);
  void
  updateDataList(corba_data_t& src);
#if 0
  void
  rmAllData();
#endif

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

  typedef map<const char*,corba_data_t,cmpID> dietDataDescList_t;
  typedef map<const char *,omni_mutex,cmpID> dietDataIDLockList_t;
  
  /*List of the data held by the Data Manager*/
  dietDataDescList_t dataDescList;
  
  /* list of lock to manage data transmission */
  dietDataIDLockList_t lockList;


  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/
  corba_data_t
  moveToCorbaDataT(corba_data_desc_t& dataDesc);
  void
  cpEltListToDataT(corba_data_t *cData);
  bool
  isInLockList(char* id);
  void
  dataIDUnlock(const corba_data_id_t& dataId);
  void
  dataIDLock(const corba_data_id_t& cdataId);
  void
  rmDataDescFromList(char* id);
  void
  addDataDescToList(corba_data_t* dataDesc, int inout);
  void
  addDataIDToLockList(char* id);
  void
  rmDataFromIDList(char* id);
  corba_data_desc_t
  moveToCorbaDataDesc(corba_data_t& dataDesc);
  void 
  printList();

};

#endif // _DATAMGRIMPL_HH_
