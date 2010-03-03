/****************************************************************************/
/* DIET Data Location Manager implementation source code                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Bruno DEL FABBRO (Bruno.DelFabbro@lifc.univ-fcomte.fr)              */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.19  2010/03/03 10:19:03  bdepardo
 * Changed \n into endl
 *
 * Revision 1.18  2008/06/01 14:06:56  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.17  2007/07/13 10:00:25  ecaron
 * Remove deprecated code (ALTPREDICT part)
 *
 * Revision 1.16  2006/11/16 09:55:54  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 *
 * Revision 1.14  2005/09/05 16:04:10  hdail
 * Movement of subtree portion of whereData into separate method call so it can
 * also be used without searching higher in tree.
 *
 * Revision 1.13  2005/06/28 15:56:56  hdail
 * Changing the debug level of messages to make DIET less verbose (and in
 * agreement with the doc =).
 *
 * Revision 1.12  2005/04/13 08:49:11  hdail
 * Beginning of adoption of new persistency model: DTM is enabled by default and
 * JuxMem will be supported via configure flags.  DIET will always provide at
 * least one type of persistency.  As a first step, persistency across DTM and
 * JuxMem is not supported so all persistency handling should be surrounded by
 *     #if HAVE_JUXMEM
 *         // JuxMem code
 *     #else
 *         // DTM code
 *     #endif
 * This check-in prepares for the JuxMem check-in by cleaning up old
 * DEVELOPPING_DATA_PERSISTENCY flags and surrounding DTM code with
 * #if ! HAVE_JUXMEM / #endif flags to be replaced by above format by Mathieu's
 * check-in.  Currently the HAVE_JUXMEM flag is set in AgentImpl.hh - to be
 * replaced by Mathieu's check-in of a configure system for JuxMem.
 *
 * Revision 1.11  2004/10/15 13:03:16  bdelfabr
 * set_data_arg method modified to manage non correct id
 *
 * Revision 1.10  2004/09/13 14:12:19  hdail
 * Cleaned up memory management for class variables myName and localHostName.
 *
 * Revision 1.9  2004/03/03 09:04:57  bdelfabr
 * add thread safe management to list
 *
 * Revision 1.8  2004/02/27 10:25:39  bdelfabr
 * coding standard
 *
 * Revision 1.7  2003/11/10 14:04:59  bdelfabr
 * add methods invoked by DM for data transfer management
 *
 * Revision 1.6  2003/10/21 13:28:26  bdelfabr
 * set persistence flag to 0
 *
 * Revision 1.5  2003/10/14 20:27:16  bdelfabr
 * adding methods for demo RNTL (print List of persistent data)
 *
 * Revision 1.3  2003/09/30 15:10:15  bdelfabr
 * applying coding standard.
 * WhereData is modified to avoid warning while compiling
 *
 * Revision 1.2  2003/09/24 09:13:05  pcombes
 * DataMgr does not need a name: use its reference.
 *
 * Revision 1.1  2003/09/22 21:07:52  pcombes
 * Set all the modules and their interfaces for data persistency.
 ***************************************************************************/

#include <unistd.h>  // For gethostname()
#include "LocMgrImpl.hh"

#include "Counter.hh"
#include "DataMgr.hh"
#include "debug.hh"
#include "ms_function.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "ts_container/ts_map.hh"


#define MAX_HOSTNAME_LENGTH 256

/** The trace level. */
extern unsigned int TRACE_LEVEL;


LocMgrImpl::LocMgrImpl() 
{
  this->childID           = -1;
  this->parent            = LocMgr::_nil();
  this->localHostName     = NULL;
  this->myName            = NULL;
  this->childIDCounter    = 0;
  this->nbLocMgrChildren     = 0;
  this->nbDataMgrChildren = 0;
  this->dataLocList.clear();
}

LocMgrImpl::~LocMgrImpl() 
{
  this->dataLocList.clear();
}

/**
 * Launch this data location manager
 * (initialization + registration in the hierarchy).
 */
int 
LocMgrImpl::run()
{
  char* name;
  char* parentName;

  /* Set host name */
  this->localHostName = new char[MAX_HOSTNAME_LENGTH];
  if (gethostname(this->localHostName, MAX_HOSTNAME_LENGTH)) {
    ERROR("could not get hostname", 1);
  }
  this->localHostName[MAX_HOSTNAME_LENGTH-1] = '\0';
   
  /* Bind this LocMgr to its name in the CORBA Naming Service */
  name = (char*)Parsers::Results::getParamValue(Parsers::Results::NAME);
  if (name == NULL)
    return 1;
  this->myName = new char[strlen(name) + 4]; // 3 for "Loc" + 1 for \0
  strcpy(this->myName, name);

  // FIXME : rewrite strcat to ms_strcat
  strcat(strcpy(this->myName, name), "Loc");

  if (ORBMgr::bindObjToName(_this(), ORBMgr::LOCMGR, this->myName)) {
    ERROR("LocMgr: could not declare myself as " << this->myName, 1);
  }

  /* Get the parent reference */
  name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (name != NULL) {
    parentName = new char[strlen(name) + 4]; // 3 for "Loc" + 1 for \0
    strcpy(parentName, name);
    strcat(parentName, "Loc");
    parent =
      LocMgr::_duplicate(LocMgr::_narrow(ORBMgr::getObjReference(ORBMgr::LOCMGR,
								 parentName)));
    if (CORBA::is_nil(parent)) {
      ERROR("LocMgr: cannot locate my parent " << parentName, 1);
    }
  }
  TRACE_TEXT(TRACE_ALL_STEPS,
	     endl << "Root LocMgr " << this->myName<< " started." << endl << endl);
  return 0;
}




/****************************************************************************/
/* Subscribing                                                              */
/****************************************************************************/

/** Subscribe a LocMgr as a child. Remotely called by an LocMgr. */
CORBA::ULong
LocMgrImpl::locMgrSubscribe(LocMgr_ptr me, const char* hostName)
{
  CORBA::ULong retID = (this->childIDCounter)++; // thread safe

  /* the size of the list is childIDCounter+1 (first index is 0) */
  this->locMgrChildren.resize(this->childIDCounter);
  this->locMgrChildren[retID] = locMgrChild(me, hostName);
  (this->nbLocMgrChildren)++; // thread safe
  
  if (! CORBA::is_nil(this->parent)) { // LocMgr associated to an MA.
    if (this->childID == (ChildID)-1)
      this->childID = this->parent->locMgrSubscribe(_this(), this->localHostName);
  }
  
  return retID;
} // locMgrSubscribe(...)


/** Subscribe a DataMgr as a child. Remotely called by a DataMgr. */
CORBA::ULong
LocMgrImpl::dataMgrSubscribe(DataMgr_ptr me, const char* hostName)
{
   CORBA::ULong retID = (this->childIDCounter)++; // thread safe

  /* the size of the list is childIDCounter+1 (first index is 0) */
  this->dataMgrChildren.resize(this->childIDCounter);
  this->dataMgrChildren[retID] = dataMgrChild(me, hostName);
  (this->nbDataMgrChildren)++; // thread safe

  if (! CORBA::is_nil(this->parent)) { // LocMgr associated to an MA.
    if (this->childID == (ChildID)-1)
      this->childID = this->parent->locMgrSubscribe(_this(), this->localHostName);
  }
  
  return(retID);
} // dataMgrSubscribe(...)



/****************************************************************************/
/* Manipulating data                                                        */
/****************************************************************************/


/** Add Data Reference to the Reference List */
void
LocMgrImpl::addDataRef(const corba_data_desc_t& arg, CORBA::ULong cChildID)
{
  store_desc_child_t &data_to_store = dataLocList[strdup(arg.id.idNumber)];
  data_to_store.childID_owner = cChildID;
  data_to_store.id_handle_type = arg;
  if (!CORBA::is_nil(this->parent)) {
    parent->addDataRef(arg,this->childID);
  }
}

/** initially invoked by the MA : client wants to destroy data */
CORBA::Long
LocMgrImpl::rm_pdata(const char* argID)
{
  if ( dataLocList.size() > 0) {
    store_desc_child_t &data_to_store = dataLocList[ms_strdup(argID)];
    ChildID cChildID = data_to_store.childID_owner;
    dataLocList.erase(ms_strdup(argID));
    printList1();
    if(cChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
      locMgrChild theLoc = locMgrChildren[cChildID];
      if (theLoc.defined()){
	LocMgr_ptr locChild = theLoc.getIor();
	return locChild->rm_pdata(argID);
      } else return 0;
    } else {
      if(cChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
	dataMgrChild theData = dataMgrChildren[cChildID];
	if(theData.defined()){
	  DataMgr_ptr dataChild = theData.getIor();
	  return dataChild->rmDataRef(ms_strdup(argID));
	} else return 0;
      } else return 0;
    }
  } else {
    return 0; 
  }
}


/** Remove Data Reference from the Reference List */
void
LocMgrImpl::rmDataRef(const char *argID, CORBA::ULong cChildID)
{
  dataLocList.erase(strdup(argID));
  printList1();
  if (this->parent != LocMgr::_nil()) {
    parent->rmDataRef(argID,cChildID);
  }
}

/**
 * initially invoked by a Data Manager.
 * the owner a the data has changed. The list must be updated.
 */ 
void
LocMgrImpl::updateDataRef(const corba_data_desc_t& arg,
			  CORBA::ULong cChildID, CORBA::Long upDown)
{
  if(upDown == 0){ // UP = replace old owner by new one
    if ( CORBA::is_nil(this->parent)) { // Root Loc Manager
   
      store_desc_child_t &stored_desc= dataLocList[ms_strdup(arg.id.idNumber)];
      
      ChildID oldChildID = stored_desc.childID_owner;
      stored_desc.childID_owner = cChildID;
      
      dataLocList[ms_strdup(arg.id.idNumber)]=stored_desc;
      printList1();
      
      upDown = 1;
      if(oldChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc = locMgrChildren[oldChildID];
	if (theLoc.defined()){
	  LocMgr_ptr locChild = theLoc.getIor();
	  locChild->updateDataRef(arg,(unsigned)-1,1);
	}
      }
      if(oldChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
	dataMgrChild theData = dataMgrChildren[oldChildID];
	if(theData.defined()){
	  DataMgr_ptr dataChild = theData.getIor();
	  dataChild->rmDataRef(ms_strdup(arg.id.idNumber));
	}
      }
    } else {
     // Not Root Loc Manager
      if(dataLookUp(ms_strdup(arg.id.idNumber)) == 1){ // data not found - adding it
	store_desc_child_t stored_desc ;
	stored_desc.childID_owner = cChildID;
	stored_desc.id_handle_type = arg;
	dataLocList[ms_strdup(arg.id.idNumber)] = stored_desc;
	printList1();
	parent->updateDataRef(arg, this->childID,0); 
      } else {
	store_desc_child_t &stored_desc= dataLocList[ms_strdup(arg.id.idNumber)];
	ChildID oldChildID = stored_desc.childID_owner;
	stored_desc.childID_owner = cChildID;
	dataLocList[ms_strdup(arg.id.idNumber)] = stored_desc;
	
	upDown = 1;
	if(oldChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	  locMgrChild theLoc = locMgrChildren[oldChildID]; 
	  if (theLoc.defined()){
	    LocMgr_ptr locChild = theLoc.getIor();
	    locChild->updateDataRef(arg,(unsigned)-1,1);
	  }
	}

	if(oldChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
	  dataMgrChild theData = dataMgrChildren[oldChildID];	  
	  if(theData.defined()){
	    DataMgr_ptr dataChild = theData.getIor();
	    dataChild->rmDataRef(ms_strdup(arg.id.idNumber));
	  }
	}
      }
    }
  } else { // DOWN = removing old owner
    if (!CORBA::is_nil(this->parent)){
      store_desc_child_t &stored_desc = dataLocList[ms_strdup(arg.id.idNumber)];
      ChildID oldChildID = stored_desc.childID_owner;
      stored_desc.childID_owner = cChildID;
      dataLocList.erase(strdup(arg.id.idNumber));
      printList1();
      if(oldChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc = locMgrChildren[oldChildID];
	if (theLoc.defined()){
	  LocMgr_ptr locChild = theLoc.getIor();
	  locChild->updateDataRef(arg,(unsigned)-1,1);
	}
      }
      if(oldChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {    
	dataMgrChild theData = dataMgrChildren[oldChildID];
	if (theData.defined()){
	  DataMgr_ptr dataChild = theData.getIor();
	  dataChild->rmDataRef(arg.id.idNumber);
	}
      }
    }
  } 
} //updateDataRef(const corba_data_desc_t& arg, CORBA::ULong cChildID, CORBA::Long upDown)


/**
 * Returns the Id of :
 *   - the Data Manager holding the Data referenced by its id OR
 *   - the Loc Manager holding the reference of the Data
 */
CORBA::ULong
LocMgrImpl::dataLookUp(const char *argID)
{
  dataLocList.lock();
  dataLocList.begin();
  if(dataLocList.find(strdup(argID)) != dataLocList.end()){
    dataLocList.unlock();
    return 0;
  }
  else {
    dataLocList.unlock();
    return 1;
  }
} //dataLookUp(const char *argID)


/** get data properties (type and data type) */
corba_data_desc_t*
LocMgrImpl::set_data_arg(const char* argID)
{
  corba_data_desc_t *arg_data_desc = new corba_data_desc_t;

  //printList1();
  if( dataLookUp(argID) == 1 ) {
    arg_data_desc->id.idNumber = CORBA::string_dup("-1");
    arg_data_desc->id.dataCopy =  DIET_ORIGINAL;
    arg_data_desc->id.state = DIET_FREE;
  } else {
    store_desc_child_t stored_desc = dataLocList[ms_strdup(argID)];
    
    *arg_data_desc = stored_desc.id_handle_type;
  }
  return arg_data_desc;
  
} // set_data_arg(const char* argID)


/** Display the List of known data Reference */
char *
LocMgrImpl::setMyName(){
  return CORBA::string_dup(this->myName);
}

void
LocMgrImpl::printList1()
{
 
  char *SonName = NULL;
  if( dataLocList.size() > 0){
    TRACE_TEXT(TRACE_ALL_STEPS,
    "+------------+----------------------------+" << endl <<
    "| Data Name  | Data Manager Owner         |" << endl <<
    "+------------+----------------------------+" << endl);
    dataLocList.lock();
    DataLocList_t::iterator cur = dataLocList.begin();
    while (cur != dataLocList.end()) {
      if(cur->second.childID_owner < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc = locMgrChildren[cur->second.childID_owner];
	if (theLoc.defined()) {
	  LocMgr_ptr locChild = theLoc.getIor();
	  SonName = locChild->setMyName();
	}
      } else {
     	dataMgrChild theData = dataMgrChildren[cur->second.childID_owner];
	if (theData.defined()) {
	  DataMgr_ptr dataChild = theData.getIor();
	  SonName = dataChild->setMyName();
	  
	}
      }
      TRACE_TEXT(TRACE_ALL_STEPS,"| " << cur->first << " |   " << SonName  << endl);
      TRACE_TEXT(TRACE_ALL_STEPS,"+------------+" << endl); 
      cur++;
    }
    dataLocList.unlock();
    TRACE_TEXT(TRACE_ALL_STEPS,"+------------+----------------------------+" << endl); 
  } /*else {
    cout << "+-----No Data-----+" << endl;
  }*/
} // printList1()


void
LocMgrImpl::printList()
{
  printList1();
  if (this->parent != LocMgr::_nil()) {
   this->parent->printList();
  }
} // printList()

/**
 * returns the name of the DataManager that owns a data identified by argID,
 * NULL if data not present, prints message if data not known  
 */
char *
LocMgrImpl::whichSeDOwner(const char* argID)
{
  if (this->parent == LocMgr::_nil()) {
    dataLocList.lock();
    dataLocList.begin();
    if (dataLocList.find(strdup(argID)) != dataLocList.end()) { 
      dataLocList.unlock();
     store_desc_child_t &stored_desc = dataLocList[ms_strdup(argID)];
      ChildID cChildID = stored_desc.childID_owner;
    
      if(cChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc = locMgrChildren[cChildID];
	if (theLoc.defined()) {
	  LocMgr_ptr locChild = theLoc.getIor();
	  return locChild->whichSeDOwner(ms_strdup(argID)) ;
	} else return NULL;
      } else {
	if(cChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
	  dataMgrChild theData = dataMgrChildren[cChildID];
	  if (theData.defined()) {
	  DataMgr_ptr dataChild = theData.getIor();
	  return dataChild->whichSeDOwner(ms_strdup(argID)) ;
	  } else return NULL;
	} else {
	  cerr << "OBJECT UNKNOWN " << endl;
	  return NULL;
	}
      }
    } else {
      dataLocList.unlock();
      cerr << "DATA NOT IN HIERARCHY " << endl;
      return NULL;
    }

  } else { /** I am not root Loc Manager */ 
    dataLocList.lock();
    dataLocList.begin();
    if (dataLocList.find(strdup(argID)) != dataLocList.end()) { /** Data Reference found */
      dataLocList.unlock();
      store_desc_child_t &stored_desc = dataLocList[ms_strdup(argID)];
      ChildID cChildID = stored_desc.childID_owner;
      if(cChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc = locMgrChildren[cChildID];
	if (theLoc.defined()){
	  LocMgr_ptr locChild = theLoc.getIor();
	  return locChild->whichSeDOwner(ms_strdup(argID)) ;
	  
	} else return NULL;
      } else {

	if(cChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
	  dataMgrChild theData = dataMgrChildren[cChildID];
	  if (theData.defined()){
	    DataMgr_ptr dataChild = theData.getIor();
	    return dataChild->whichSeDOwner(ms_strdup(argID));
	  } else return NULL;
	} else {
	  return NULL; 
	}
      }
    } else {
       dataLocList.unlock(); /** data Reference not found - invoke my parent */
      return this->parent->whichSeDOwner(ms_strdup(argID));
    }
  }

} // whichSeDOwner(const char* argID)

 
/**
 * Returns the IOR of the Data Manager holding the searched data
 */
DataMgr_ptr
LocMgrImpl::whereData(const char* argID)
{
  /** Check first if data exists locally or in subtree */
  DataMgr_ptr dataMgrRef = this->whereDataSubtree(argID);

  if (dataMgrRef == DataMgr::_nil()) {
    dataLocList.lock();
    dataLocList.begin();

    /** Not found.  Try looking to parent if not the root node */
    if (this->parent == LocMgr::_nil()) {
      /** I am the root Loc Manager */
      dataLocList.unlock();
      WARNING("Data item " << argID << " not found in hierarchy" << endl);
      dataMgrRef = DataMgr::_nil();
    } else {
      /** I am not the root.  Try asking my parent. */
      dataLocList.unlock();
      dataMgrRef = this->parent->whereData(ms_strdup(argID));
    }
  }
  return dataMgrRef;
} // whereData(const char* argID)

/**
 * Returns the reference to the Data Manager holding the needed data.
 * Only search for the data locally and in the subtree under this LocMgr.
 */
DataMgr_ptr
LocMgrImpl::whereDataSubtree(const char* argID)
{
  dataLocList.lock();
  dataLocList.begin();

  if (dataLocList.find(strdup(argID)) != dataLocList.end()) {
    /** Data location found */
    dataLocList.unlock();
    store_desc_child_t &stored_desc = dataLocList[ms_strdup(argID)];
    ChildID cChildID = stored_desc.childID_owner;

    if(cChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
      /** Data available with a LocMgr */
      locMgrChild theLoc =  locMgrChildren[cChildID];
      if (theLoc.defined()) {
        LocMgr_ptr locChild = theLoc.getIor();
        return locChild->whereDataSubtree(ms_strdup(argID)) ;
      } else {
        return DataMgr::_nil();
      }

    } else {
      /** Data available with a dataMgr */
      if(cChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
        dataMgrChild theData = dataMgrChildren[cChildID];
        if (theData.defined()) {
          DataMgr_ptr dataChild = theData.getIor();
          return dataChild->whereData(ms_strdup(argID)) ;
        } else {
          return DataMgr::_nil();
        }
      } else {
        WARNING("Unknown object cChildID" << endl);
        return DataMgr::_nil();
      }
    }
  } else {
    dataLocList.unlock();
    TRACE_TEXT(TRACE_STRUCTURES, "Data " << argID << " not in subhierarchy" << endl);
    return DataMgr::_nil();
  }
} // whereDataSubtree(const char* argID)


void
LocMgrImpl::updateDataProp(const char *argID)
{
  if (this->parent == LocMgr::_nil()) {
    this->parent->updateDataProp(ms_strdup(argID));
  }
}

