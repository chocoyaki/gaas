/****************************************************************************/
/* DIET Data Manager implementation source code                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Bruno DEL FABBRO (Bruno.DelFabbro@lifc.univ-fcomte)                 */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.13  2004/10/05 08:23:09  bdelfabr
 * fixing bug for persistent file : add a changePath method thta gives the good file access path
 *
 * Revision 1.12  2004/03/03 09:11:58  bdelfabr
 * bug correction
 *
 * Revision 1.11  2004/03/01 18:42:22  rbolze
 * add logservice
 *
 * Revision 1.10  2004/02/27 10:26:37  bdelfabr
 * let DIET_PERSISTENCE_MODE set to 1, coding standard
 *
 * Revision 1.9  2003/12/01 14:49:30  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.8  2003/11/10 14:03:11  bdelfabr
 * adding methods that allow tranfer time between servers to be computed
 *
 * Revision 1.7  2003/10/21 13:27:59  bdelfabr
 * set Persistence flag to 0
 *
 * Revision 1.6  2003/10/14 20:29:06  bdelfabr
 * adding print methods (PERSISTENT mode only)
 *
 * Revision 1.5  2003/10/02 17:08:00  bdelfabr
 * modifying memory management in case of in_out arg
 *
 * Revision 1.4  2003/10/01 07:40:51  cpera
 * Add false boolean return data of DataMgrImpl::dataLookup function
 * if DEVELOPPING_DATA_PERSISTENCY is set to 0.
 *
 * Revision 1.3  2003/09/30 15:08:09  bdelfabr
 * dlist are replaced by map.
 * Coding standards are applied
 *
 * Revision 1.2  2003/09/24 09:15:03  pcombes
 * DataMgr does not need a name: use its reference.
 *
 * Revision 1.1  2003/09/22 21:07:21  pcombes
 * Set all the modules and their interfaces for data persistency.
 ***************************************************************************/

#include <stdlib.h>

#include "DataMgrImpl.hh"

#include "common_types.hh"
#include "DIET_data_internal.hh"
#include "LocMgr.hh"
#include "ms_function.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "ts_container/ts_map.hh"

#define DEVELOPPING_DATA_PERSISTENCY 1


/** Data Manager Constructor */
DataMgrImpl::DataMgrImpl()
{
  this->childID  = (childID)-1;
  this->parent = LocMgr::_nil();
  this->dataDescList.clear();

#ifdef HAVE_LOGSERVICE
  this->dietLogComponent = NULL;
#endif
}

DataMgrImpl::~DataMgrImpl(){
  this->dataDescList.clear();
} 

/** Data Manager Launcher - subscribes to its Loc Manager parent */
int
DataMgrImpl::run()
{
  char* name(NULL);
  char parentName[260];

  /* Set host name */
  this->localHostName[257] = '\0';
  if (gethostname(this->localHostName, 256)) {
    ERROR("could not get hostname", 1);
  }
  
  name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (name == NULL)
    return 1;
  strcat(strcpy(parentName, name), "Loc");
  parent =
    LocMgr::_duplicate(LocMgr::_narrow(ORBMgr::getObjReference(ORBMgr::LOCMGR,
							       parentName)));
  if (CORBA::is_nil(this->parent)) {
    ERROR("cannot locate my parent " << parentName, 1);
  }

#if 0
  if (ORBMgr::bindDataToName(_this(), this->myName)) {
    cerr << "Data: cannot declare myself as " << this->myDataName << endl;
    return 1;
   }
#endif // 0

  this->childID = this->parent->dataMgrSubscribe(_this(), localHostName);

  return 0;
}

#if HAVE_LOGSERVICE
void
DataMgrImpl::setDietLogComponent(DietLogComponent* dietLogComponent) {
  this->dietLogComponent = dietLogComponent;
}
#endif

char *
DataMgrImpl::setMyName() {
   return CORBA::string_dup((const char*)(this->localHostName));
}

/************************************************************ 
 *Method that allow the copy of an arg present in the map *
 * to an element of corba_data_t type                       *  
 ************************************************************/

void 
DataMgrImpl::cpEltListToDataT(corba_data_t* cData)
{

#if DEVELOPPING_DATA_PERSISTENCY
   
  corba_data_t &the_data = dataDescList[CORBA::string_dup(cData->desc.id.idNumber)] ;
  cData->desc = the_data.desc;
  long unsigned int size = (long unsigned int) data_sizeof(&(the_data.desc));
  if ((diet_data_type_t)(the_data.desc.specific._d()) != DIET_FILE) {
    CORBA::Boolean orphan = 0;
    CORBA::Char *p;
    p = static_cast <CORBA::Char *>(the_data.value.get_buffer(orphan));
    cData->value.replace(size,size,p,0); 
    /* the map keeps control of the data */
  } else {
    CORBA::Char* val(NULL);
    char* path = the_data.desc.specific.file().path;
    if (path && strcmp("", path)) {
      ifstream infile(path);
     val = SeqChar::allocbuf(size);
      if (!infile) {
	cerr << "cannot open file " << path << " for reading" << endl;
      }
      for (unsigned int i = 0; i < size; i++) {
	val[i] = infile.get();

      }
      infile.close();
    } else {
      val = SeqChar::allocbuf(1);
      val[0] = '\0';
    }
    
     cData->value.replace(size,size,val,1);
   
  }
  
								     
#endif // DEVELOPPING_DATA_PERSISTENCY
} // cpEltListToDataT(corba_data_t* cData)


/** to be defined */
void
DataMgrImpl::addDataIDToLockList(char* id)
{
#if DEVELOPPING_DATA_PERSISTENCY

#endif // DEVELOPPING_DATA_PERSISTENCY
}

/** to be used later */
void
DataMgrImpl::rmDataFromIDList(char* id)
{
#if DEVELOPPING_DATA_PERSISTENCY
  lockList.erase(strdup(id)) ;
#endif // DEVELOPPING_DATA_PERSISTENCY
}


/**
 * Add dataDesc to the map
 */
void
DataMgrImpl::addDataDescToList(corba_data_t* dataDesc, int inout) // FIXME : diet_grpc_arg_mode_t IN ...
{
#if DEVELOPPING_DATA_PERSISTENCY

  double *value;
  char *path;
  corba_data_t &the_data = dataDescList[ms_strdup(dataDesc->desc.id.idNumber)] ;
 
  the_data.desc = dataDesc->desc; 
  the_data.desc.specific.file().path = ms_strdup(dataDesc->desc.specific.file().path);

  the_data.desc.id.idNumber  = ms_strdup(dataDesc->desc.id.idNumber); 
  the_data.desc.id.dataCopy  = DIET_ORIGINAL;
  the_data.desc.id.state  = DIET_FREE;


  long unsigned int size = (long unsigned int) data_sizeof(&(dataDesc->desc));
  value = (double *)malloc(size*sizeof(double));
  if ( (diet_data_type_t)(dataDesc->desc.specific._d()) != DIET_FILE) {
    
    if(inout == 0) {/* IN ARGS */
    
      CORBA::Boolean orphan = 1; 
      CORBA::Char * p(NULL); 
      
      p = static_cast <CORBA::Char *>(dataDesc->value.get_buffer(orphan));          
      dataDesc->value.replace(size,size,p,0); /* used to not loose the value */      
      the_data.value.replace(size,size,p,1); /* map takes the control an value */

    } else { // OUT ARGS 
   
      CORBA::Boolean orphan = 0;
      CORBA::Char * p(NULL);            
      p = static_cast <CORBA::Char *> (dataDesc->value.get_buffer(orphan));     
      the_data.value.replace(size,size,p,1);     
      dataDesc->value.replace(size,size,p,0);

    } 
  } else {
     if(inout == 0) {
       path = CORBA::string_dup(the_data.desc.specific.file().path);
     }
    
  }


#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logDataStore(dataDesc->desc.id.idNumber);
  }
#endif // HAVE_LOGSERVICE

#endif // DEVELOPPING_DATA_PERSISTENCY
}// addDataDescToList(corba_data_t* dataDesc, int inout)


void 
DataMgrImpl::changePath(corba_data_t& dataDesc, char * newPath) 
{
  corba_data_t &the_data = dataDescList[ms_strdup(dataDesc.desc.id.idNumber)] ;
  the_data.desc.specific.file().path = ms_strdup(newPath);
}


/**
 * Remove data from map : call by Loc Manager after 
 * data was transferred from Data Manager to Data Manager. 
 */

void
DataMgrImpl::rmDataDescFromList(char* argID)
{
#if DEVELOPPING_DATA_PERSISTENCY
#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logDataRelease(argID);
  }
#endif // HAVE_LOGSERVICE

  corba_data_t &the_data= dataDescList[ms_strdup(argID)] ;
  //CORBA::Char *p1 (NULL);
  //  p1 = pbc.parameters[i].value.get_buffer(1);
  if(the_data.desc.specific._d() != DIET_FILE) {
    CORBA::Char *p1 (NULL);
    p1 = the_data.value.get_buffer(1);
    _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Char*)p1);//(_CORBA_SeqChar)
  } else {
    char *path = (char *)malloc(sizeof(the_data.desc.specific.file().path)+7);   
    sprintf(path,"rm -f %s",ms_strdup(the_data.desc.specific.file().path));
    system(path);
  }
  dataDescList.erase(ms_strdup(argID));
#endif // DEVELOPPING_DATA_PERSISTENCY
}// rmDataDescFromList(char* argID)

/**
 * data is "pushed" by the owner. This method takes arg and add it into the map
 */
void
DataMgrImpl::sendData(corba_data_t& arg)
{

#if DEVELOPPING_DATA_PERSISTENCY
  if ( (diet_data_type_t)(arg.desc.specific._d())== DIET_FILE) {
  
    if ((arg.desc.specific.file().path != NULL)
	&& strcmp("", arg.desc.specific.file().path)) {
      char* in_path   = CORBA::string_dup(arg.desc.specific.file().path);
      //   char* file_name = strrchr(in_path, '/');
      char* out_path  = new char[256];
      // pid_t pid = getpid();
      // sprintf(out_path, "/tmp/DIET_%d_%s", pid,
      //	      (file_name) ? (char*)(1 + file_name) : in_path);
      sprintf(out_path, "%s",in_path);
      ofstream outfile(out_path);
      for (int i = 0; i < arg.desc.specific.file().size; i++) {
	outfile.put(arg.value[i]);
      }
    }

  }
  addDataDescToList(&arg,0);
  printList1();

#endif // DEVELOPPING_DATA_PERSISTENCY
} // sendData(corba_data_t& arg)

/**
 * returns the SeD Name whose DM owns a data identified by argId
 */
char *
DataMgrImpl::whichSeDOwner(const char* argId)
{
  return this->localHostName;

}

// whichSeDOwner(const char* argId)

/** got the owner of the data identified by argID */


char *
DataMgrImpl::whichDataMgr(const char* argId)
{

    char *dataSrc = parent->whichSeDOwner(strdup(argId));
    if (*dataSrc == '\0')
      return NULL;
    else 
      return dataSrc;
}// whichDataMgr(const char* argId)


/**
 * Invoked by the server. It is looking for a persistent data. 
 * If local just update the value if not invoke its Loc Manager Parent to get it. 
 */
void
DataMgrImpl::getData(corba_data_t& cData)
{
  
#if DEVELOPPING_DATA_PERSISTENCY
  if(dataLookup(cData.desc.id.idNumber)){ // if data present
   dataDescList.unlock();
  
    cpEltListToDataT(&cData);
 
  } else { // data not locally present
    DataMgr_ptr dataSrc;
    dataSrc = parent->whereData(strdup(cData.desc.id.idNumber));
    /* invoke remote Data Manager that will send Data */  
    dataSrc->putData(CORBA::string_dup(cData.desc.id.idNumber), this->_this());
    // copy value to cData that is used by solver
    cpEltListToDataT(&cData);
 
    parent->updateDataRef(cData.desc,childID,0);
    printList1();
  }
#endif // DEVELOPPING_DATA_PERSISTENCY
}// getData(corba_data_t& cData)


/**
 * print the content of the map. Doesn't print value 
 */
void
DataMgrImpl::printList1()
{ 
  if( dataDescList.size() > 0){
    cout << "+-----------------+" << endl;
    cout << "|  Data ID        |" << endl;
    cout << "+-----------------+" << endl;
    dataDescList.lock();
    dietDataDescList_t::iterator cur = dataDescList.begin();

    while (cur != dataDescList.end()) {
     
      char *p1;
      double *value;
      long unsigned int size = (long unsigned int) data_sizeof(&(cur->second.desc));
      value=(double *)malloc(size*sizeof(double));
    
      p1=(char *) cur->second.value.get_buffer(0);
      value  = (double*)p1;
      cout << "|        " << cur->first << "        |" << endl; // cur->first[2]
         cout << "VALUE  " << endl; 
	cout << "+-----------------+" << endl;
      cur++;
    }
    dataDescList.unlock();
  }
 
}// printList1(){

void
DataMgrImpl::printList()
{
 
  printList1();
 
  this->parent->printList();
} // printList()
 

/*******************************************************************************
 * Method invoked by Data Manager me to get data. Sends data to me DataManager *
 *******************************************************************************/

void
DataMgrImpl::putData(const char* argID, const DataMgr_ptr me)
{
#if DEVELOPPING_DATA_PERSISTENCY
 
  corba_data_t dest;
  dest.desc.id.idNumber=CORBA::string_dup(argID);
  cpEltListToDataT(&dest);
 
  if ( (diet_data_type_t)(dest.desc.specific._d()) == DIET_FILE) {
    CORBA::Char *dataValue(NULL);

    ifstream infile(dest.desc.specific.file().path);
    if (dest.desc.specific.file().path && strcmp("",dest.desc.specific.file().path)){
      dataValue = SeqChar::allocbuf(dest.desc.specific.file().size);
      for (int i = 0; i < dest.desc.specific.file().size; i++) {
	dataValue[i] = infile.get();
      }
      infile.close();
    } else {
      dataValue = SeqChar::allocbuf(1);
      dataValue[0] = '\0';
    } 
    dest.value.replace(dest.desc.specific.file().size,dest.desc.specific.file().size , dataValue, 1);
  }
  
#if HAVE_LOGSERVICE
  // FIXME: we cannot get the name of the receiving agent yet
  if (dietLogComponent != NULL) {
    dietLogComponent->logDataBeginTransfer(argID, "");
  }
#endif

  me->sendData(dest);

#if HAVE_LOGSERVICE
  // FIXME: we cannot get the name of the receiving agent yet
  if (dietLogComponent != NULL) {
    dietLogComponent->logDataEndTransfer(argID, "");
  }
#endif
 
#endif // DEVELOPPING_DATA_PERSISTENCY
} // putData(const char* argID, const DataMgr_ptr me)


/*****************************************************************
 * Invoked by server; Case of in-out arg. Only update data value *
 *****************************************************************/
void
DataMgrImpl::updateDataList(corba_data_t& src)
{
#if DEVELOPPING_DATA_PERSISTENCY
  corba_data_t& the_data = dataDescList[strdup(src.desc.id.idNumber)];

  the_data.desc = src.desc;
  long unsigned int size = (long unsigned int) data_sizeof(&(src.desc));
  if ( (diet_data_type_t)(src.desc.specific._d()) != DIET_FILE) {
    CORBA::Boolean orphan = 0;
    CORBA::Char * p(NULL);
    
    p = static_cast <CORBA::Char *> (src.value.get_buffer(orphan));
    
    the_data.value.replace(size,size,p,1);
    
    src.value.replace(size,size,p,0);
  }
  
#endif // DEVELOPPING_DATA_PERSISTENCY
} // updateDataList(corba_data_t& src)


/****************************************************************
 * adds persistent data to the list                             *
 * propagates this operation to its parent                      *
 ***************************************************************/
void
DataMgrImpl::addData(corba_data_t& dataDesc, int inout)
{
#if DEVELOPPING_DATA_PERSISTENCY
 
  // FIXME :  if (inout == 0) ??
    addDataDescToList(&dataDesc,inout);  
 
    if(dataDesc.desc.mode != DIET_STICKY) // doesn't update Loc Manager references is sticky data
      parent->addDataRef(dataDesc.desc,childID);
#endif // DEVELOPPING_DATA_PERSISTENCY
} // addData(corba_data_t& dataDesc, int inout)

/*******************************************************
 * call by Loc Manager Parent to destroy reference :   *
 * the new owner (a Data Manager) got it               *
 ******************************************************/
CORBA::Long
DataMgrImpl::rmDataRef(const char* argID)
{
  rmDataDescFromList(CORBA::string_dup(argID));
  printList1();
  return 0;
} // rmDataRef(const char* argID)

 
/***************************************************************
 * Invoked by Loc Manager Parent which is looking for a data.  *
 * Returns _this() if data present nil elsewhere               *
 **************************************************************/
DataMgr_ptr
DataMgrImpl::whereData(const char* argID)
{

#if DEVELOPPING_DATA_PERSISTENCY

  if(dataLookup(CORBA::string_dup(argID))){ 
    dataDescList.unlock();
    return(this->_this());
  } else {
    return(DataMgr::_nil());
  }
#else // DEVELOPPING_DATA_PERSISTENCY
  return NULL;
#endif // DEVELOPPING_DATA_PERSISTENCY
} // whereData(const char* argID)


/***************************************************************
 * returns true if data present, false elsewhere               *
 **************************************************************/
bool
DataMgrImpl::dataLookup(char* argID)
{
 
 printList1();
#if DEVELOPPING_DATA_PERSISTENCY
  if(dataDescList.size() > 0){
    dataDescList.lock();
    return(dataDescList.find(ms_strdup(argID)) != dataDescList.end());
  } else return false;
#endif // DEVELOPPING_DATA_PERSISTENCY
return false;
 
} //dataLookup(char* argID)

/** to be defined */
void
DataMgrImpl::dataIDLock(const corba_data_id_t& cDataID)
{
#if DEVELOPPING_DATA_PERSISTENCY
 
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/** to be defined */
void
DataMgrImpl::dataIDUnlock(const corba_data_id_t &cDataID)
{
#if DEVELOPPING_DATA_PERSISTENCY
  
#endif // DEVELOPPING_DATA_PERSISTENCY
}


#if 0
void
DataMgrImpl::updateDataRefOrder(corba_data_t& dataDesc)
{
  corba_dataMgr_desc_t *dataIdDesc = new(corba_dataMgr_desc_t);
  *dataIdDesc = moveToCorbaDataDesc(dataDesc);
  addDataDescToList(dataIdDesc); // ajout de la donnee  

  parent->updateDataRef(dataIdDesc->dataID.idNumber,chilID,0); // 0 = UP
}
 
 
bool
DataMgrImpl::isInLockList(char* argID)
{
  //#if DEVELOPPING_DATA_PERSISTENCY
  bool found = TRUE;
  //#endif // DEVELOPPING_DATA_PERSISTENCY
  return(found);
}

void
DataMgrImpl::updateDataProperty(corba_data_t& dataDesc)
{
  //#if DEVELOPPING_DATA_PERSISTENCY
 
 
  //#endif // DEVELOPPING_DATA_PERSISTENCY
}


#endif // 0

