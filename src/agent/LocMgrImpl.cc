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

#include "LocMgrImpl.hh"

#include "Counter.hh"
#include "DataMgr.hh"
#include "debug.hh"
#include "ms_function.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "ts_container/ts_map.hh"


#define DEVELOPPING_DATA_PERSISTENCY 1

/** The trace level. */
extern unsigned int TRACE_LEVEL;


LocMgrImpl::LocMgrImpl() 
{
  this->localHostName[0]  = '\0';
  this->childID           = -1;
  this->parent            = LocMgr::_nil();
  this->myName[0]         = '\0';
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
int LocMgrImpl::run()
{
  char* name;
  char parentName[260];

  /* Set host name */
  this->localHostName[257] = '\0';
  if (gethostname(this->localHostName, 256)) {
    ERROR("could not get hostname", 1);
  }
   
  /* Bind this LocMgr to its name in the CORBA Naming Service */
  name = (char*)Parsers::Results::getParamValue(Parsers::Results::NAME);
  if (name == NULL)
    return 1;
  strncpy(this->myName, name, MIN(256, strlen(name)));
  this->myName[256] = '\0';
  // FIXME : rewrite strcat to ms_strcat
  strcat(strcpy(this->myName, name), "Loc");
  if (ORBMgr::bindObjToName(_this(), ORBMgr::LOCMGR, this->myName)) {
    ERROR("LocMgr: could not declare myself as " << this->myName, 1);
  }

  /* Get the parent reference */
  name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (name != NULL) {
    strcat(strcpy(parentName, name), "Loc");
    parent =
      LocMgr::_duplicate(LocMgr::_narrow(ORBMgr::getObjReference(ORBMgr::LOCMGR,
								 parentName)));

    if (CORBA::is_nil(parent)) {
      ERROR("LocMgr: cannot locate my parent " << parentName, 1);
    }
    TRACE_TEXT(TRACE_MAIN_STEPS,
	       "\nLocal LocMgr " << this->myName << " started.\n\n");
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS,
	       "\nRoot LocMgr " << this->myName<< " started.\n\n");
  }
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
LocMgrImpl::addDataRef(const char *argID, CORBA::ULong cChildID)
{
#if DEVELOPPING_DATA_PERSISTENCY 
  
  dataLocList[strdup(argID)]=cChildID;
 
  //printList();

  if (!CORBA::is_nil(this->parent)) {
  
    parent->addDataRef(argID,this->childID);
  }

#endif // DEVELOPPING_DATA_PERSISTENCY
}

/** initially invoked by the MA : client wants to destroy all its data */
#if 0
void
LocMgrImpl::rmAllRef()
{
  if ( dataLocList.size() > 0) {
    DataLocList_t::iterator cur = dataLocList.begin();
    ChildID cChildID = dataLocList[strdup(cur->first)];
    if(cChildID < static_cast<CORBA::ULong>(locSons.size())) {
      locDescription theLoc =  locSons[cChildID];
      if (theLoc.defined()){
	LocMgr_ptr locChild = theLoc.getIor();
	locChild->rmAllRef();
      }
    }
    if(oldSonId < static_cast<CORBA::ULong>(dataSons.size())) {
      dataDescription theData = dataSons[oldSonId];
      if(theData.defined()){
	DataMgr_ptr dataChild = theData.getIor();
	dataChild->rmDataRef();
      }
    }
    while (cur != dataLocList.end()) {
      // FIXME id decomposition 
      dataLocList.erase(strdup(cur->first));
      cur++;
    }
    ChildID oldChildID = dataLocList[strdup(dataId)];
    dataLocList.erase(strdup(dataId));
    if(oldChildID < static_cast<CORBA::ULong>(locSons.size())) {
      locDescription theLoc =  locSons[oldSonId];
      if (theLoc.defined()){
	LocMgr_ptr locChild = theLoc.getIor();
	locChild->rmDeprecatedVarRef();
      }
    }
    if(oldSonId < static_cast<CORBA::ULong>(dataSons.size())) {
      dataDescription theData = dataSons[oldSonId];
      if(theData.defined()){
	DataMgr_ptr dataChild = theData.getIor();
	dataChild->rmDataRef();
      }
    }
  }
}
#endif // 0

/** Remove Data Reference from the Reference List */
void
LocMgrImpl::rmDataRef(const char *argID, CORBA::ULong cChildID)
{
#if DEVELOPPING_DATA_PERSISTENCY
  dataLocList.erase(strdup(argID));
  printList1();
  if (this->parent != LocMgr::_nil())
    parent->rmDataRef(argID,cChildID);
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/**
 * initially invoked by a Data Manager.
 * the owner a the data has changed. The list must be updated.
 */ 
void
LocMgrImpl::updateDataRef(const char *argID,
			  CORBA::ULong cChildID, CORBA::Long upDown)
{
#if DEVELOPPING_DATA_PERSISTENCY
  if(upDown == 0){ // UP = replace old owner by new one
    if ( CORBA::is_nil(this->parent)) { // Root Loc Manager
   
      ChildID oldChildID = dataLocList[ms_strdup(argID)];
   
      dataLocList[ms_strdup(argID)]=cChildID;
      printList1();
    
      upDown = 1;
      if(oldChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc =   locMgrChildren[oldChildID];
	
	
	if (theLoc.defined()){
	  
	  LocMgr_ptr locChild = theLoc.getIor();
	  locChild->updateDataRef(argID,(unsigned)-1,1);
	}
      }
      if(oldChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
	dataMgrChild theData = dataMgrChildren[oldChildID];
	
	if(theData.defined()){
	  DataMgr_ptr dataChild = theData.getIor();
	  
	  dataChild->rmDataRef(ms_strdup(argID));
	}
      }
    } else { // Not Root Loc Manager
      if(dataLookUp(argID)==1){ // data not found - adding it
	dataLocList[ms_strdup(argID)] = cChildID;
	printList1();
	parent->updateDataRef(argID, this->childID,0); 
      } else {
	ChildID oldChildID = dataLocList[ms_strdup(argID)];
	dataLocList[ms_strdup(argID)] = cChildID;
	upDown=1;
	if(oldChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	  locMgrChild theLoc =   locMgrChildren[oldChildID]; 
	  if (theLoc.defined()){
	    LocMgr_ptr locChild = theLoc.getIor();
	    locChild->updateDataRef(argID,(unsigned)-1,1);
	  }
	}

	if(oldChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
	  dataMgrChild theData = dataMgrChildren[oldChildID];	  
	  if(theData.defined()){
	    DataMgr_ptr dataChild = theData.getIor();
	    dataChild->rmDataRef(ms_strdup(argID));
	  }
	}
      }
    }
  } else { // DOWN = removing old owner
    if (!CORBA::is_nil(this->parent)){
      ChildID oldChildID = dataLocList[strdup(argID)];
      
      dataLocList.erase(strdup(argID));
      printList1();
      if(oldChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc =  locMgrChildren[oldChildID];
	if (theLoc.defined()){
	  LocMgr_ptr locChild = theLoc.getIor();
	  locChild->updateDataRef(argID,(unsigned)-1,1);
	}
      }
      if(oldChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {    
	dataMgrChild theData = dataMgrChildren[oldChildID];
	if (theData.defined()){
	  DataMgr_ptr dataChild = theData.getIor();
	  dataChild->rmDataRef(argID);
	}
      }
    }
  } 
#endif // DEVELOPPING_DATA_PERSISTENCY
}


/**
 * Returns the Id of :
 *   - the Data Manager holding the Data referenced by its id OR
 *   - the Loc Manager holding the reference of the Data
 */
CORBA::ULong
LocMgrImpl::dataLookUp(const char *argID)
{
#if DEVELOPPING_DATA_PERSISTENCY 
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
#else
  return 0;
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/** Display the List of known data Reference */

char *
LocMgrImpl::setMyName(){
  return CORBA::string_dup(this->myName);
}

void
LocMgrImpl::printList1()
{
 
  char *SonName=NULL;
  if( dataLocList.size() > 0){
    cout << "+------------+----------------------------+" << endl;
    cout << "| Data Name  | Data Manager Owner         |" << endl;
    cout << "+------------+----------------------------+" << endl;
    dataLocList.lock();
    DataLocList_t::iterator cur = dataLocList.begin();
    while (cur != dataLocList.end()) {
      if(cur->second < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc =  locMgrChildren[cur->second];
	if (theLoc.defined()) {
	  LocMgr_ptr locChild = theLoc.getIor();
	  SonName = locChild->setMyName();
	}
      } else {
     	dataMgrChild theData =  dataMgrChildren[cur->second];
	if (theData.defined()) {
	  DataMgr_ptr dataChild = theData.getIor();
	  SonName = dataChild->setMyName();
	  
	}
      }
      cout << "|    " << cur->first << "     |   " << SonName  << endl;
      cout << "+------------+" << endl; 
      cur++;
    }
    dataLocList.unlock();
    cout << "+------------+----------------------------+" << endl; 
  }
  
}

void
LocMgrImpl::printList()
{
 
#if DEVELOPPING_DATA_PERSISTENCY

  printList1();
 if (this->parent != LocMgr::_nil())
   this->parent->printList();
#endif // DEVELOPPING_DATA_PERSISTENCY
}

char *
LocMgrImpl::whichSeDOwner(const char* argID){
#if DEVELOPPING_DATA_PERSISTENCY
  if (this->parent == LocMgr::_nil()) {
    dataLocList.lock();
    dataLocList.begin();
    if (dataLocList.find(strdup(argID)) != dataLocList.end()) {     
      ChildID cChildID=dataLocList[strdup(argID)];
      dataLocList.unlock();
      if(cChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc =  locMgrChildren[cChildID];
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
    
      ChildID cChildID = dataLocList[strdup(argID)];
       dataLocList.unlock();
      if(cChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc =  locMgrChildren[cChildID];
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
#else
  return NULL;
#endif // DEVELOPPING_DATA_PERSISTENCY

 }

 
/**
 * Return the Name of the Data Manager holding the searched data
 */
DataMgr_ptr
LocMgrImpl::whereData(const char* argID)
{
#if DEVELOPPING_DATA_PERSISTENCY
  if (this->parent == LocMgr::_nil()) {
    dataLocList.lock();
    dataLocList.begin();
    if (dataLocList.find(strdup(argID)) != dataLocList.end()) {     
      ChildID cChildID=dataLocList[strdup(argID)];
      dataLocList.unlock();
      if(cChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc =  locMgrChildren[cChildID];
	if (theLoc.defined()) {
	  LocMgr_ptr locChild = theLoc.getIor();
	  return locChild->whereData(ms_strdup(argID)) ;
	} else return DataMgr::_nil();
      } else {
	if(cChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
	  dataMgrChild theData = dataMgrChildren[cChildID];
	  if (theData.defined()) {
	  DataMgr_ptr dataChild = theData.getIor();
	  return dataChild->whereData(ms_strdup(argID)) ;
	  } else return DataMgr::_nil();
	} else {
	  cerr << "OBJECT UNKNOWN " << endl;
	  return DataMgr::_nil();
	}
      }
    } else {
      dataLocList.unlock();
      cerr << "DATA NOT IN HIERARCHY " << endl;
      return DataMgr::_nil();
    }

  } else { /** I am not root Loc Manager */ 
    dataLocList.lock();
    dataLocList.begin();
    if (dataLocList.find(strdup(argID)) != dataLocList.end()) { /** Data Reference found */
    
      ChildID cChildID = dataLocList[strdup(argID)];
       dataLocList.unlock();
      if(cChildID < static_cast<CORBA::Long>(locMgrChildren.size())) {
	locMgrChild theLoc =  locMgrChildren[cChildID];
	if (theLoc.defined()){
	  LocMgr_ptr locChild = theLoc.getIor();
	  return locChild->whereData(ms_strdup(argID)) ;
	  
	} else return DataMgr::_nil();
      } else {

	if(cChildID < static_cast<CORBA::Long>(dataMgrChildren.size())) {
	  dataMgrChild theData = dataMgrChildren[cChildID];
	  if (theData.defined()){
	    DataMgr_ptr dataChild = theData.getIor();
	    return dataChild->whereData(ms_strdup(argID));
	  } else return DataMgr::_nil();
	} else {
	  return DataMgr::_nil(); 
	}
      }
    } else {
       dataLocList.unlock(); /** data Reference not found - invoke my parent */
      return this->parent->whereData(ms_strdup(argID));
    }
  }
#else
  return DataMgr::_nil();
#endif // DEVELOPPING_DATA_PERSISTENCY
}


void
LocMgrImpl::updateDataProp(const char *argID)
{
#if DEVELOPPING_DATA_PERSISTENCY  
  if (this->parent == LocMgr::_nil())
    this->parent->updateDataProp(ms_strdup(argID));
#endif
}

