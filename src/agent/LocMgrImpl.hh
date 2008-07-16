/****************************************************************************/
/* DIET Data Location Manager implementation header                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Bruno DEL FABBRO (Bruno.DelFabbro@lifc.univ-fcomte.fr)              */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.10  2008/07/16 00:44:11  ecaron
 * Remove HAVE_ALTPREDICT
 *
 *
 * Revision 1.8  2004/09/13 14:12:52  hdail
 * Improved memory management for class variables localHostName and myName.
 *
 * Revision 1.7  2004/02/27 10:25:39  bdelfabr
 * coding standard
 *
 * Revision 1.6  2003/12/01 14:49:30  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.5  2003/11/10 14:04:59  bdelfabr
 * add methods invoked by DM for data transfer management
 ***************************************************************************/

#ifndef _LOCMGRIMPL_HH_
#define _LOCMGRIMPL_HH_

#include "LocMgr.hh"

#include "ChildID.hh"
#include "Counter.hh"
#include "DataMgr.hh"
#include "DIET_data_internal.hh"
#include "NodeDescription.hh"
#include "ts_container/ts_map.hh"
#include "ts_container/ts_vector.hh"


/** Local Loc structure for data type management */

//  FIXME : I don't know in what file to store these types
struct store_desc_child
{
  ChildID childID_owner;
  corba_data_desc_t id_handle_type;
};

typedef store_desc_child store_desc_child_t;

struct cmpDataId
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};



class LocMgrImpl : public POA_LocMgr,
		   public PortableServer::RefCountServantBase
{
public:
  
  /**************************************************************************/
  /* Public methods                                                         */
  /**************************************************************************/

  LocMgrImpl();
  virtual
  ~LocMgrImpl();
  
  /**
   * Launch this data location manager
   * (initialization + registration in the hierarchy).
   */
  int
  run();
  
  /** Subscribe a LocMgr as a child. Remotely called by a LocMgr. */
  virtual CORBA::ULong
  locMgrSubscribe(LocMgr_ptr me, const char* hostName);
  /** Subscribe a DataMgr as a child. Remotely called by a DataMgr. */
  virtual CORBA::ULong
  dataMgrSubscribe(DataMgr_ptr me, const char* hostName);
 
  /** add data reference to the reference List  */
  virtual void
  addDataRef(const corba_data_desc_t& arg, CORBA::ULong cChildID); //  addDataRef(const char* argID, CORBA::ULong cChildID);

  /** remove reference from the reference list */
  virtual void
  rmDataRef(const char* argID, CORBA::ULong cChildID);
  
  /** remove reference from the reference list  */
  virtual CORBA::Long
  rm_pdata(const char* argID);
  
  /** update reference (add and remove) of the reference list */
  virtual void
  updateDataRef(const corba_data_desc_t& arg, CORBA::ULong cChildID, CORBA::Long upDown);
  
  /** look for a data reference, recover DataMgr object reference */
  virtual DataMgr_ptr
  whereData(const char* argID);

  /** look for a data reference, but only in the subtree, 
   * recover DataMgr object reference. */
  virtual DataMgr_ptr
  whereDataSubtree(const char* argID);

  void
  printList1();

  /** to be defined */
  virtual void
  updateDataProp(const char* argID);
  
  /** look for the data localization */
  virtual CORBA::ULong 
  dataLookUp(const char* argID);
 
  virtual char *
  setMyName();
  
  /** look for the data owner */
  char *
  whichSeDOwner(const char* argID);

  /** get data properties */
  corba_data_desc_t*
  set_data_arg(const char* argID);

private:
  
  /**************************************************************************/
  /* Private fields                                                         */
  /**************************************************************************/
  
  /** Local host name */
  char* localHostName;

  /** ID (-1 if root) of this LocMgr amongst the children of its parent */
  ChildID childID;

  /** Reference (nil if root) of the parent LocMgr */
  LocMgr_var parent;
  
  /** Identity in the CORBA Naming Service */
  char* myName;
  /** ID of next subscribing child */
  Counter childIDCounter;

  /** Number of children of this LocMgr that are Location managers */
  Counter nbLocMgrChildren;
  typedef NodeDescription<LocMgr, LocMgr_ptr> locMgrChild;
  /** List of the LocMgr children of this agent */
  ts_vector<locMgrChild> locMgrChildren;

  /** Number of children of this LocMgr that are Data managers */
  Counter nbDataMgrChildren;
  typedef NodeDescription<DataMgr, DataMgr_ptr> dataMgrChild;
  /** List of the SeD children of this agent */
  ts_vector<dataMgrChild> dataMgrChildren;
  
  /** List of couples data reference, dataManager owner */ 
  typedef ts_map<const char*, store_desc_child_t, cmpDataId> DataLocList_t ;
  DataLocList_t dataLocList;


  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/
 
  /** print DataLocList content */
  void printList();

};

#endif // _LOCMGRIMPL_HH_

