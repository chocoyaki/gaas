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
 * Revision 1.1  2003/09/22 21:07:52  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 ***************************************************************************/

#ifndef _LOCMGRIMPL_HH_
#define _LOCMGRIMPL_HH_

#include "LocMgr.hh"

#include "ChildID.hh"
#include "Counter.hh"
#include "DataMgr.hh"
#include "dietTypes.hh"
#include "NodeDescription.hh"
#include "ts_container/ts_map.hh"
#include "ts_container/ts_vector.hh"


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
 
  /**  */
  virtual void
  addDataRef(const char* argID, CORBA::ULong cChildID);
  /**  */
  virtual void
  rmDataRef(const char* argID, CORBA::ULong cChildID);
  /**  */
  virtual void
  updateDataRef(const char* argID, CORBA::ULong cChildID, CORBA::Long upDown);
  /**  */
  virtual char*
  whereData(const char* argID, const char* hostName);
 
  /**  */
  virtual void
  updateDataProp(const char* argID);
  /**  */
  CORBA::ULong
  dataLookUp(const char* argID);


private:
  
  /**************************************************************************/
  /* Private fields                                                         */
  /**************************************************************************/
  
  /** Local host name */
  char localHostName[257];

  /** ID (-1 if root) of this LocMgr amongst the children of its parent */
  ChildID childID;
  /** Reference (nil if root) of the parent LocMgr */
  LocMgr_var parent;
  
  /** Identity in the CORBA Naming Service */
  char myName[260]; // 257 + 3 for "Loc"
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
  
  // FIXME: what is cmpCorbaDataID ????
  typedef map<const char*, ChildID, cmpCorbaDataID> DataLocList_t ;
  DataLocList_t dataLocList;


  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/
 
  // FIXME: aff for "afficher" is not english, as far as I know.
  void printList();

};

#endif // _LOCMGRIMPL_HH_

