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
 * Revision 1.1  2003/09/22 21:07:52  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 ***************************************************************************/

#include "LocMgrImpl.hh"

#include "Counter.hh"
#include "DataMgr.hh"
#include "debug.hh"
#include "ms_function.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "ts_container/ts_map.hh"


#define DEVELOPPING_DATA_PERSISTENCY 0

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
LocMgrImpl::addDataRef(const char* argID, CORBA::ULong cChildID)
{
#if DEVELOPPING_DATA_PERSISTENCY 
  
  dataLocList[strdup(id)]=csonId;
 
  printList();

  if (*myAgentFatherName != '\0') {
  
    locMgrFather->addDataRef(id,sonId);
  }

#endif // DEVELOPPING_DATA_PERSISTENCY
}

// initially invoked by the MA : client wants to destroy all its data
#if 0
void
LocMgrImpl::rmAllRef()
{
  if ( dataLocList.size() > 0) {
    DataLocList_t::iterator cur = dataLocList.begin();
    ChildID sonId = dataLocList[strdup(cur->first)];
    if(sonId < static_cast<CORBA::ULong>(locSons.size())) {
      locDescription theLoc =  locSons[sonId];
      if (theLoc.defined()){
	LocMgr_var myLocChild = theLoc.getIor();
	myLocChild->rmAllRef(dataId);
      }
    }
    if(oldSonId < static_cast<CORBA::ULong>(dataSons.size())) {
      dataDescription theData = dataSons[oldSonId];
      if(theData.defined()){
	DataMgr_var myDataChild = theData.getIor();
	myDataChild->rmDataRef(dataId);
      }
    }
    while (cur != dataLocList.end()) {
      //if partie d'id = MA + session FIXME : decomposition de l'id  
      dataLocList.erase(strdup(cur->first));
      cur++;
    }
    ChildID oldSonId = dataLocList[strdup(dataId)];
    dataLocList.erase(strdup(dataId));
    if(oldSonId < static_cast<CORBA::ULong>(locSons.size())) {
      locDescription theLoc =  locSons[oldSonId];
      if (theLoc.defined()){
	LocMgr_var myLocChild = theLoc.getIor();
	myLocChild->rmDeprecatedVarRef(dataId);
      }
    }
    if(oldSonId < static_cast<CORBA::ULong>(dataSons.size())) {
      dataDescription theData = dataSons[oldSonId];
      if(theData.defined()){
	DataMgr_var myDataChild = theData.getIor();
	myDataChild->rmDataRef(dataId);
      }
    }
  }
}
#endif // 0

/** Remove Data Reference from the Reference List */
void
LocMgrImpl::rmDataRef(const char* argID, CORBA::ULong cChildID)
{
#if DEVELOPPING_DATA_PERSISTENCY
  if (*myAgentFatherName != '\0')
    dataLocList.erase(strdup(dataId));
  
    locMgrFather->rmDataRef(dataId,sonId);
#endif // DEVELOPPING_DATA_PERSISTENCY
}

 
void
LocMgrImpl::updateDataRef(const char* argID,
			  CORBA::ULong cCchildID, CORBA::Long upDown)
{
#if DEVELOPPING_DATA_PERSISTENCY
  if(upDown == 0){ // UP
    if (*myAgentFatherName == '\0') {
   
      ChildID oldSonId = dataLocList[strdup(dataId)];
 
  
      dataLocList[strdup(dataId)]=csonId;
     
    
      UpDown = 1;
      if(oldSonId < static_cast<CORBA::Long>(locMgrSons.size())) {
	locMgrDescription theLoc =  locSons[oldSonId];
	
	
	if (theLoc.defined()){
	  
	  LocMgr_var myLocChild = theLoc.getIor();
	  myLocChild->updateDataRef(dataId,(unsigned)-1,1);
	}
      }
      if(oldSonId < static_cast<CORBA::Long>(dataSons.size())) {
	dataDescription theData = dataSons[oldSonId];
	
	if(theData.defined()){
	  DataMgr_var myDataChild = theData.getIor();
	  
	  myDataChild->rmDataRef(dataId);
	}
      }
    } else {
      dataLocList[strdup(dataId)] = csonId;
      
      //affList();
      locFather->updateDataRef(dataId,sonId,0); 
    }
  }else{ 
    if(*myAgentFatherName != '\0'){// DOWN
      ChildID oldSonId = dataLocList[strdup(dataId)];
      
      dataLocList.erase(strdup(dataId));
     
      // affList();
      if(oldSonId < static_cast<CORBA::Long>(locSons.size())) {
	locDescription theLoc =  locSons[oldSonId];
	if (theLoc.defined()){
	  LocMgr_var myLocChild = theLoc.getIor();
	  myLocChild->updateDataRef(dataId,(unsigned)-1,1);
	}
      }
      if(oldSonId < static_cast<CORBA::Long>(dataSons.size())) {    
	dataDescription theData = dataSons[oldSonId];
	if (theData.defined()){
	  DataMgr_var myDataChild = theData.getIor();
	  myDataChild->rmDataRef(dataId);
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
LocMgrImpl::dataLookUp(const char* argID)
{
#if DEVELOPPING_DATA_PERSISTENCY 
  return (dataLocList.find(strdup(id)) != dataLocList.end());
#else
  return 0;
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/** Display the List of known data Reference */
void
LocMgrImpl::printList()
{
#if DEVELOPPING_DATA_PERSISTENCY
  cout << "______________________" << endl;
  cout << "|  size of list is " << dataLocList.size() << "  |"<< endl;
  if( dataLocList.size() > 0){
    DataLocList_t::iterator cur = dataLocList.begin();
    cout << "| - - - - - - - - - - |" << endl;
    while (cur != dataLocList.end()) {
      cout << "| key: " << cur->first << "* value: " << cur->second << " |" << endl;
      cout << "|                     |" << endl;
      cur++;
    }
  }
  else {
    cout << "nil list" << endl;
  }
  cout << "_______________________" << endl;

#endif // DEVELOPPING_DATA_PERSISTENCY
}

 
/**
 * Return the Name of the Data Manager holding the searching Data
 */
char*
LocMgrImpl::whereData(const char* argID, const char* hostName)
{
#if DEVELOPPING_DATA_PERSISTENCY
  char *a=(char *)(malloc(261*sizeof(char)));
  if (*myAgentFatherName == '\0') {
    if (dataLocList.find(strdup(dataId)) != dataLocList.end()) {
   
     
      ChildID theSonId=dataLocList[strdup(dataId)];
      if(theSonId < static_cast<CORBA::Long>(locSons.size())) {
	locDescription theLoc =  locSons[theSonId];
	if (theLoc.defined()) {
	  LocMgr_var myChild = theLoc.getIor();
	  return(CORBA::string_dup(myChild->whereData(dataId,hostname)));
	}
      } else {
	if(theSonId < static_cast<CORBA::Long>(dataSons.size())) {
	  dataDescription theData = dataSons[theSonId];
	  DataMgr_var myChild = theData.getIor();
	  return(CORBA::string_dup(myChild->whereData(dataId, hostname)));
	  
	} else {
	  cerr << "OBJECT UNKNOWN " << endl;
	}
      }
    } else {
      cerr << "DATA NOT IN HIERARCHY " << endl;
      *a = '\0';
      return a;
    }
    return a;

  } else { 
    if (dataLocList.find(strdup(dataId)) != dataLocList.end()) { 
      ChildID theSonId=dataLocList[strdup(dataId)];
      if(theSonId < static_cast<CORBA::Long>(locSons.size())) {
	locDescription theLoc =  locSons[theSonId];
	if (theLoc.defined()){
	  LocMgr_var myChild = theLoc.getIor();
	  return(CORBA::string_dup(myChild->whereData(dataId,hostname)));
	  
	}
      } else {
	if(theSonId < static_cast<CORBA::Long>(dataSons.size())) {
	  dataDescription theData = dataSons[theSonId];
	  DataMgr_var myChild = theData.getIor();
	  return(CORBA::string_dup(myChild->whereData(dataId, hostname)));
	} else {
	;
	  cerr << "BIG PROBLEM" << endl;
	  *a='\0';
	}
      }
    } else {
	return(CORBA::string_dup(locFather->whereData(dataId, hostname)));

      return a;
    }
    return a;
  }
#else
  return NULL;
#endif // DEVELOPPING_DATA_PERSISTENCY
}


void
LocMgrImpl::updateDataProp(const char* argID)
{
#if DEVELOPPING_DATA_PERSISTENCY  
  if (*myAgentFatherName != '\0') 
    locFather->updateDataProp(cDataId);
#endif
}

