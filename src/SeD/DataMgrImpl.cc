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
 * Add false boolean return data of DataMgrImpl::dataLookup function if DEVELOPPING_DATA_PERSISTENCY is set to 0.
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

#include "DataMgrImpl.hh"

#include "common_types.hh"
#include "dietTypes.hh"
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


char *
DataMgrImpl::setMyName(){
   char myName[261];

   strcpy (myName,this->localHostName);
   return CORBA::string_dup(myName);

 }

/************************************************************ 
 *Method which permit the copy of an arg present in the map *
 * to an element of corba_data_t type                       *  
 ************************************************************/

void 
DataMgrImpl::cpEltListToDataT(corba_data_t *cData)
{

#if DEVELOPPING_DATA_PERSISTENCY
  
  char *p1;
  double *value;
  corba_data_t &the_data=dataDescList[CORBA::string_dup(cData->desc.id.idNumber)] ;
  
  cData->desc=the_data.desc;
  
  long unsigned int size = (long unsigned int) data_sizeof(&(the_data.desc));
  value=(double *)malloc(size*sizeof(double));
  if ((diet_data_type_t)(the_data.desc.specific._d()) != DIET_FILE) {
    CORBA::Boolean orphan = 0;
    CORBA::Char *p;
    
    p = static_cast <CORBA::Char *>(the_data.value.get_buffer(orphan));
    
    cData->value.replace(size,size,p,0); /* the map keeps control of the data */
    cout << "\nGET IN DATA " << endl;
    p1=(char *) cData->value.get_buffer(0);
    
    value  = (double*)p1;
    for (unsigned int i=0;i<size/sizeof(double);i++)
      cout <<value[i] << " " ; 
    cout << "\nEND GET IN DATA" << endl;
  }
  
								     
#endif // DEVELOPPING_DATA_PERSISTENCY
}



void
DataMgrImpl::addDataIDToLockList(char *id)
{
#if DEVELOPPING_DATA_PERSISTENCY

#endif // DEVELOPPING_DATA_PERSISTENCY
}


void
DataMgrImpl::rmDataFromIDList(char *id)
{
#if DEVELOPPING_DATA_PERSISTENCY
   lockList.erase(strdup(id)) ;
#endif // DEVELOPPING_DATA_PERSISTENCY
}


/**
 * Add dataDesc to the map
 */
void
DataMgrImpl::addDataDescToList(corba_data_t *dataDesc, int inout) // FIXME : diet_grpc_arg_mode_t IN ...
{
#if DEVELOPPING_DATA_PERSISTENCY
  char *p1;
  double *value;

  corba_data_t &the_data= dataDescList[ms_strdup(dataDesc->desc.id.idNumber)] ;
 
  the_data.desc = dataDesc->desc; 
  the_data.desc.id.idNumber  = ms_strdup(dataDesc->desc.id.idNumber); 
  the_data.desc.id.dataCopy  = DIET_ORIGINAL;
  the_data.desc.id.state  = DIET_FREE;
    

  long unsigned int size = (long unsigned int) data_sizeof(&(dataDesc->desc));
  value=(double *)malloc(size*sizeof(double));
  if ( (diet_data_type_t)(dataDesc->desc.specific._d()) != DIET_FILE) {
    
    if(inout == 0) {/* IN ARGS */
    
      CORBA::Boolean orphan = 1; 
      CORBA::Char * p(NULL); 
      
      p = static_cast <CORBA::Char *>(dataDesc->value.get_buffer(orphan));     
     
      dataDesc->value.replace(size,size,p,0); /* used to not loose the value */
      
      the_data.value.replace(size,size,p,1); /* map takes the control an value */

      cout << "\nADD IN DATA " << endl;
      p1=(char *) the_data.value.get_buffer(0);
      
      value  = (double*)p1;
      for (unsigned int i=0;i<size/sizeof(double);i++)
	cout <<value[i] << " " ; 
      cout << "\nEND ADD IN DATA" << endl;
     
    } else { // OUT ARGS 
   
      CORBA::Boolean orphan = 0;
      CORBA::Char * p(NULL);
            
      p = static_cast <CORBA::Char *> (dataDesc->value.get_buffer(orphan));
      
      the_data.value.replace(size,size,p,1);
      
      dataDesc->value.replace(size,size,p,0);
     
      cout << "\nADD OUT DATA " << endl;
      p1=(char *) the_data.value.get_buffer(0);
      
      value  = (double*)p1;
      for (unsigned int i=0;i<size/sizeof(double);i++)
	cout <<value[i] << " " ; 
      cout << "\nEND ADD OUT DATA" << endl;
    }
    
  }
  
     
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/**
 * Remove data from map : call by Loc Manager after 
 * data was transferred from Data Manager to Data Manager. 
 */
void
DataMgrImpl::rmDataDescFromList(char *id)
{
#if DEVELOPPING_DATA_PERSISTENCY
  dataDescList.erase(ms_strdup(id));
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/**
 * data is "pushed" by the owner. This method takes arg and addit into the map
 */
void
DataMgrImpl::sendData(corba_data_t &arg)
{

#if DEVELOPPING_DATA_PERSISTENCY
  if ( (diet_data_type_t)(arg.desc.specific._d())== DIET_FILE) {
  
    if ((arg.desc.specific.file().path != NULL)
	&& strcmp("", arg.desc.specific.file().path)) {
      char* in_path   = CORBA::string_dup(arg.desc.specific.file().path);
      char* file_name = strrchr(in_path, '/');
      char* out_path  = new char[256];
      pid_t pid = getpid();
      sprintf(out_path, "/tmp/DIET_%d_%s", pid,
	      (file_name) ? (char*)(1 + file_name) : in_path);
      
      ofstream outfile(out_path);
      for (int i = 0; i < arg.desc.specific.file().size; i++) {
	outfile.put(arg.value[i]);
      }
    }

  }
  addDataDescToList(&arg,0);
  printList1();

#endif // DEVELOPPING_DATA_PERSISTENCY
}

/**
 * returns the SeD Name whose DM owns a data identified by argId
 */

char *
DataMgrImpl::whichSeDOwner(const char * argId){
  return this->localHostName;

}

char *
DataMgrImpl::whichDataMgr(const char * argId){

    char *dataSrc = parent->whichSeDOwner(strdup(argId));
    if (*dataSrc=='\0')
      return NULL;
    else 
      return dataSrc;
}

/**
 * Invoked by the server. It is looking for a persistent data. 
 * If local just update the value if not invoke its Loc Manager Parent to get it. 
 */

void
DataMgrImpl::getData(corba_data_t &cData)
{
  
#if DEVELOPPING_DATA_PERSISTENCY
  if(dataLookup(cData.desc.id.idNumber)){ // if data present
   dataDescList.unlock();
    cpEltListToDataT(&cData);
 
  } else { // data not lacally present
    cout << "DATA NOT HERE" << endl;
    DataMgr_ptr dataSrc;
 
    dataSrc=parent->whereData(strdup(cData.desc.id.idNumber));
    /* invoke remote Data Manager which will send Data */  
    dataSrc->putData(CORBA::string_dup(cData.desc.id.idNumber), this->_this());

    cpEltListToDataT(&cData);
 
    parent->updateDataRef(cData.desc.id.idNumber,childID,0);
    printList1();
  }
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/**
 * print the content of the map 
 */

void
DataMgrImpl::printList1(){

 
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
	 //    for(unsigned int i=0;i<size/sizeof(double);i++)
	 //	cout <<value[i] << " " ; 
	cout << "+-----------------+" << endl;
      cur++;
    }
    dataDescList.unlock();
  }
 
}

void
DataMgrImpl::printList(){
 
  printList1();
 
  this->parent->printList();
}
 

/**
 * Method invoked by Data Manager me to get data. Sends data to me 
 */
void
DataMgrImpl::putData(const char *id, const DataMgr_ptr me)
{
#if DEVELOPPING_DATA_PERSISTENCY
 
  corba_data_t dest;
  dest.desc.id.idNumber=CORBA::string_dup(id);
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
  
  me->sendData(dest);
 
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/**
 * Invoked by server; Case of in-out arg. Only update data value
 */
void
DataMgrImpl::updateDataList(corba_data_t &src)
{
#if DEVELOPPING_DATA_PERSISTENCY
  corba_data_t &the_data=dataDescList[strdup(src.desc.id.idNumber)];

  the_data.desc=src.desc;
  long unsigned int size = (long unsigned int) data_sizeof(&(src.desc));
  CORBA::Boolean orphan = 0;
  CORBA::Char * p(NULL);
  
  p = static_cast <CORBA::Char *> (src.value.get_buffer(orphan));
  
  the_data.value.replace(size,size,p,1);
  
  src.value.replace(size,size,p,0);
  
  
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/**
 * add data which are persistent
 */
void
DataMgrImpl::addData(corba_data_t &dataDesc, int inout)
{
#if DEVELOPPING_DATA_PERSISTENCY
 
  // FIXME :  if (inout == 0) ??
    addDataDescToList(&dataDesc,inout);  
 
    if(dataDesc.desc.mode != DIET_STICKY) // doesn't update Loc Manager references is sticky data
    parent->addDataRef(dataDesc.desc.id.idNumber,childID);
#endif // DEVELOPPING_DATA_PERSISTENCY
}

/**
 * call by Loc Manager Parent to destroy reference : 
 * the new owner (a Data Manager) get it
 */
void
DataMgrImpl::rmDataRef(const char *argID)
{
  rmDataDescFromList(CORBA::string_dup(argID));
  printList1();
}
 
/**
 * Invoked by Loc Manager Parent which is looking for a data. 
 * Returns _this() if data present nil elsewhere
 */
DataMgr_ptr
DataMgrImpl::whereData(const char *argID)
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
}


/**
 * returns true if data present, false elsewhere 
 */
bool
DataMgrImpl::dataLookup(char *argID)
{

  cout << "LOOKING FOR " << argID << endl;
  printList1();
#if DEVELOPPING_DATA_PERSISTENCY
  dataDescList.lock();
  // dataDescList.begin();
  return(dataDescList.find(ms_strdup(argID)) != dataDescList.end());
//{
    //dataDescList.unlock();
    // cout << "I HAVE FOUND DATA" << endl;
    // return true;
    // }else 
    // return false;
#endif // DEVELOPPING_DATA_PERSISTENCY
return false;
 
}


void
DataMgrImpl::dataIDLock(const corba_data_id_t &cDataID)
{
#if DEVELOPPING_DATA_PERSISTENCY
 
#endif // DEVELOPPING_DATA_PERSISTENCY
}

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
  *dataIdDesc=moveToCorbaDataDesc(dataDesc);
  addDataDescToList(dataIdDesc); // ajout de la donnee  

  parent->updateDataRef(dataIdDesc->dataID.idNumber,chilID,0); // 0 = UP
}
 
 
bool
DataMgrImpl::isInLockList(char* id)
{
  //#if DEVELOPPING_DATA_PERSISTENCY
  bool found=TRUE;
  //#endif // DEVELOPPING_DATA_PERSISTENCY
  return(found);
}

void
DataMgrImpl::updateDataProperty(corba_data_t& dataDesc)
{
  //#if DEVELOPPING_DATA_PERSISTENCY
 
 
  //#endif // DEVELOPPING_DATA_PERSISTENCY
}



//call by parent Loc to destroy all the reference of data (will be call by client)

void
DataMgrImpl::rmAllData() // How to decompose the id ?
{
 
}

#endif // 0

