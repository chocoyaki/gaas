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
 * Revision 1.22  2005/11/10 14:37:51  eboix
 *     Clean-up of Cmake/DIET_config.h.in and related changes. --- Injay2461
 *
 * Revision 1.21  2005/09/05 16:01:34  hdail
 * Addition of locationID information and getDataLoc method call.
 * (experimental and protected by HAVE_ALTPREDICT).
 *
 * Revision 1.20  2005/04/27 01:41:34  ycaniou
 * Added the stuff for a correct compilation, for a correct registration of
 * a batch profile, and for its execution.
 * Added the solve_batch() function
 *
 * Revision 1.19  2005/04/13 08:46:29  hdail
 * Beginning of adoption of new persistency model: DTM is enabled by default and
 * JuxMem will be supported via configure flags.  DIET will always provide at
 * least one type of persistency.  As a first step, persistency across DTM and
 * JuxMem is not supported so all persistency handling should be surrounded by
 *     #if HAVE_JUXMEM
 *       // JuxMem code
 *     #else
 *       // DTM code
 *     #endif
 * This check-in prepares for the JuxMem check-in by cleaning up old
 * DEVELOPPING_DATA_PERSISTENCY flags and surrounding DTM code with
 * #if ! HAVE_JUXMEM / #endif flags to be replaced by above format by Mathieu's
 * check-in.  Currently the HAVE_JUXMEM flag is set in SeDImpl.hh - to be replaced
 * by Mathieu's check-in of a configure system for JuxMem.
 *
 * Revision 1.18  2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.17  2004/12/06 07:32:50  bdelfabr
 * cleanup memory management for tranfers between servers.
 *
 * Revision 1.16  2004/12/02 11:25:14  bdelfabr
 * addi informarion for LogCentral on data profile
 *
 * Revision 1.15  2004/10/06 15:56:13  bdelfabr
 * bug persistent data fixed (hope so one more time)
 *
 * Revision 1.14  2004/10/06 11:59:04  bdelfabr
 * corrected inout bug (I hope so)
 *
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
 ***************************************************************************/

#include <stdlib.h>
#include <unistd.h>    // For gethostname()

#include "DataMgrImpl.hh"

#include "common_types.hh"
#include "DIET_data_internal.hh"
#include "LocMgr.hh"
#include "ms_function.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "ts_container/ts_map.hh"

/** Data Manager Constructor */
DataMgrImpl::DataMgrImpl()
{
  this->childID  = (childID)-1;
  this->parent = LocMgr::_nil();
  this->dataDescList.clear();
  this->dietLogComponent = NULL;
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

#if !HAVE_ALTPREDICT
  this->childID = this->parent->dataMgrSubscribe(_this(), localHostName);
#else
  /* Get locationID. */
  name = (char*)Parsers::Results::getParamValue(Parsers::Results::LOCATIONID); 
  if (name != NULL) {
    strcpy(this->locationID, name);
  } else {
    strcpy(this->locationID, "");
  }
  this->childID = this->parent->dataMgrSubscribe(_this(), locationID);
#endif

  return 0;
}

void
DataMgrImpl::setDietLogComponent(DietLogComponent* dietLogComponent) {
  this->dietLogComponent = dietLogComponent;
}

char *
DataMgrImpl::setMyName() {
#if !HAVE_ALTPREDICT
   return CORBA::string_dup((const char*)(this->localHostName));
#else
   return CORBA::string_dup((const char*)(this->locationID));
#endif
}

/************************************************************ 
 *Method that allow the copy of an arg present in the map *
 * to an element of corba_data_t type                       *  
 ************************************************************/

void 
DataMgrImpl::cpEltListToDataT(corba_data_t* cData)
{

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
} // cpEltListToDataT(corba_data_t* cData)


/** to be defined */
void
DataMgrImpl::addDataIDToLockList(char* id)
{
}

/** to be used later */
void
DataMgrImpl::rmDataFromIDList(char* id)
{
  lockList.erase(strdup(id)) ;
}


void
DataMgrImpl::printvalue(double *value,long unsigned int size){

  cout << "value of size" << size << endl;
  cout << "size of value " << sizeof(value) << endl;
  for(unsigned int i=0; i< size/8; i++)
    cout << "i = "  << i << "value = " << value[i] << endl;
}

/**
 * Add dataDesc to the map
 */
void
DataMgrImpl::addDataDescToList(corba_data_t* dataDesc, int inout) // FIXME : diet_grpc_arg_mode_t IN ...
{
  char *path;
  corba_data_t &the_data = dataDescList[ms_strdup(dataDesc->desc.id.idNumber)] ;
 
  // cout << "value of id to add" << dataDesc->desc.id.idNumber << endl;
  the_data.desc = dataDesc->desc; 

 if ( (diet_data_type_t)(dataDesc->desc.specific._d()) == DIET_FILE) 
  the_data.desc.specific.file().path = ms_strdup(dataDesc->desc.specific.file().path);

  the_data.desc.id.idNumber  = ms_strdup(dataDesc->desc.id.idNumber); 
  the_data.desc.id.dataCopy  = DIET_ORIGINAL;
  the_data.desc.id.state  = DIET_FREE;
 
  long unsigned int size = (long unsigned int) data_sizeof(&(dataDesc->desc));
  // value = (double *)malloc(size*sizeof(double));
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
    if(inout == 0) {
      path = CORBA::string_dup(the_data.desc.specific.file().path);
    }
    
  }
 
  char * type_data = (char *)malloc(10*sizeof(char));
  if (dietLogComponent != NULL) {
    switch ((diet_data_type_t)(dataDesc->desc.specific._d())) {
      case DIET_SCALAR: {
        strcpy(type_data,"SCALAR");
      }
      case DIET_VECTOR: {
        strcpy(type_data,"VECTOR");
        break;
      }
      case DIET_MATRIX: {
        strcpy(type_data,"MATRIX");
        break;
      }
      case DIET_STRING: {
        strcpy(type_data,"STRING");
        break;
      }
      case DIET_FILE: {
        strcpy(type_data,"FILE");
        break;
      }
      default: {
        strcpy(type_data,"UNKNOWN");
        break;
      }
    }
    dietLogComponent->logDataStore(dataDesc->desc.id.idNumber, data_sizeof(&(dataDesc->desc)),(long)(dataDesc->desc.base_type), type_data);
  }
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
  if (dietLogComponent != NULL) {
    dietLogComponent->logDataRelease(argID);
  }

  corba_data_t &the_data= dataDescList[ms_strdup(argID)] ;
  //CORBA::Char *p1 (NULL);
  //  p1 = pbc.parameters[i].value.get_buffer(1);
  if(the_data.desc.specific._d() != DIET_FILE) {
    CORBA::Char *p1 (NULL);
    p1 = the_data.value.get_buffer(1);
    _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Char*)p1);//(_CORBA_SeqChar) cout << "REMOVED --  DATA REFERENCE" << endl;
  } else {
    char *path = (char *)malloc(sizeof(the_data.desc.specific.file().path)+7);   
    sprintf(path,"rm -f %s",ms_strdup(the_data.desc.specific.file().path));
    system(path);
  }

  dataDescList.erase(ms_strdup(argID));
 
}// rmDataDescFromList(char* argID)

/**
 * data is "pushed" by the owner. This method takes arg and add it into the map
 */
void 
DataMgrImpl::persistent_data_release(corba_data_t* arg){

  switch((diet_data_type_t)(arg->desc.specific._d())) {
    case DIET_VECTOR: {
      corba_vector_specific_t vect;
      arg->desc.specific.vect(vect);
      arg->desc.specific.vect().size = 0;
      break;
    }
    case DIET_MATRIX: {
      corba_matrix_specific_t mat;
      arg->desc.specific.mat(mat);
      arg->desc.specific.mat().nb_r  = 0;
      arg->desc.specific.mat().nb_c  = 0;
      break;
    }
    case DIET_STRING: {
      corba_string_specific_t str;
      arg->desc.specific.str(str);
      arg->desc.specific.str().length = 0;
      break;
    }
    case DIET_FILE: {
      corba_file_specific_t file;
      arg->desc.specific.file(file);
      arg->desc.specific.file().path = CORBA::string_dup("");
      arg->desc.specific.file().size = 0;
      break;
    }
    default: {
      break;
    }
  }
}


void
DataMgrImpl::sendData(corba_data_t& arg)
{

  if ( (diet_data_type_t)(arg.desc.specific._d())== DIET_FILE) {
  
    if ((arg.desc.specific.file().path != NULL)
	&& strcmp("", arg.desc.specific.file().path)) {
      char* in_path   = CORBA::string_dup(arg.desc.specific.file().path);
 
      char* out_path  = new char[256];

      sprintf(out_path, "%s",in_path);
      ofstream outfile(out_path);
      for (int i = 0; i < arg.desc.specific.file().size; i++) {
	outfile.put(arg.value[i]);
      }
    }
  }
 
  // dataDescList.unlock();

  addDataDescToList(&arg,0);
  if (arg.desc.specific._d() != DIET_FILE) {
    CORBA::Char *p1 (NULL);
    arg.value.replace(0,0,p1,1);
  }
  persistent_data_release(&arg); 
 
  printList1();
} // sendData(corba_data_t& arg)

/**
 * returns the SeD Name whose DM owns a data identified by argId
 */
char *
DataMgrImpl::whichSeDOwner(const char* argId)
{
#if ! HAVE_ALTPREDICT
  return this->localHostName;
#else
  return this->locationID;
#endif
}

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
  struct timeval t1, t2;
  if(dataLookup(cData.desc.id.idNumber)){ // if data present
    // dataDescList.unlock();
 
    cpEltListToDataT(&cData);
 
  } else { // data not locally present
 
    DataMgr_ptr dataSrc;
  
    dataSrc = parent->whereData(strdup(cData.desc.id.idNumber));
    /* invoke remote Data Manager that will send Data */  
    gettimeofday(&t1, NULL);

    dataSrc->putData(CORBA::string_dup(cData.desc.id.idNumber), this->_this());
    // copy value to cData that is used by solver
   gettimeofday(&t2, NULL);
   cout << "MEASURED TRANSFER TIME = " << ((t2.tv_sec - t1.tv_sec) + ((float)(t2.tv_usec - t1.tv_usec))/1000000) << " seconds" << endl;
    cpEltListToDataT(&cData);
  
    parent->updateDataRef(cData.desc,childID,0);

//  printList1();
  }
} // getData(corba_data_t& cData)


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
    
      p1 = (char *) cur->second.value.get_buffer(0);
      value  = (double*) p1;
      cout << "|    " << cur->first << "    |" << endl; // cur->first[2]
      /*    cout << "--- VALUE ---" << endl;
	    for(unsigned int i=0; i<size/8;i++) cout << " -  " << value[i] ;*/
      cout << "+-----------------+" << endl;
      cur++;
    }
    dataDescList.unlock();
  } /*else {
    cout << "+-----No Data-----+" << endl;
  }*/
} // printList1(){

void
DataMgrImpl::printList()
{
 
  printList1();
 
  this->parent->printList();
} // printList()
 

/*******************************************************************************
 * Method invoked by Data Manager me to get data. Sends data to me DataManager *
 ******************************************************************************/
void
DataMgrImpl::putData(const char* argID, const DataMgr_ptr me)
{
  corba_data_t *dest= new corba_data_t;
  dest->desc.id.idNumber=CORBA::string_dup(argID);
  cpEltListToDataT(dest);
  if ( (diet_data_type_t)(dest->desc.specific._d()) == DIET_FILE) {
    CORBA::Char *dataValue(NULL);

    ifstream infile(dest->desc.specific.file().path);
    if (dest->desc.specific.file().path && strcmp("",dest->desc.specific.file().path)){
      dataValue = SeqChar::allocbuf(dest->desc.specific.file().size);
      for (int i = 0; i < dest->desc.specific.file().size; i++) {
	dataValue[i] = infile.get();
      }
      infile.close();
    } else {
      dataValue = SeqChar::allocbuf(1);
      dataValue[0] = '\0';
    } 
    dest->value.replace(dest->desc.specific.file().size,dest->desc.specific.file().size , dataValue, 1);
  }
  
  // FIXME: we cannot get the name of the receiving agent yet
  if (dietLogComponent != NULL) {
    dietLogComponent->logDataBeginTransfer(argID, "");
  }

  struct timeval t1, t2;
  gettimeofday(&t1, NULL);
  me->sendData(*dest);
  gettimeofday(&t2, NULL);
  cout << "TIME TO SENDATA = " << ((t2.tv_sec - t1.tv_sec) + ((float)(t2.tv_usec - t1.tv_usec))/1000000)  << endl;

  // FIXME: we cannot get the name of the receiving agent yet
  if (dietLogComponent != NULL) {
    dietLogComponent->logDataEndTransfer(argID, "");
  }

  delete dest;
} // putData(const char* argID, const DataMgr_ptr me)


/*****************************************************************
 * Invoked by server; Case of in-out arg. Only update data value *
 *****************************************************************/
void
DataMgrImpl::updateDataList(corba_data_t& src)
{
  corba_data_t& the_data = dataDescList[strdup(src.desc.id.idNumber)];

  the_data.desc = src.desc;
  // long unsigned int size = (long unsigned int) data_sizeof(&(src.desc));
  //if ( (diet_data_type_t)(src.desc.specific._d()) != DIET_FILE) {

    /*  CORBA::Char *p1 (NULL);
    p1 = the_data.value.get_buffer(1);
    
    _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Char*)p1);
    
    char * p1; 
    double *value;
    value=(double *)malloc(size*sizeof(double));
    p1=(char *) the_data.value.get_buffer(0);
    value  = (double*)p1;
     cout << " BEFORE UPDATEDATA IN LIST  VALUE  = " << endl;
    for(unsigned int i=0; i<size/8;i++) cout << " - " << value[i] ;
    cout << endl;
    CORBA::Boolean orphan = 1;
    CORBA::Char * p(NULL);
    
    p = static_cast <CORBA::Char *> (src.value.get_buffer(orphan));
    
    the_data.value.replace(size,size,p,1);
    
    src.value.replace(size,size,p,0);
    
    char *p1, *p2;
    double *value, *value1;
    
    value=(double *)malloc(size*sizeof(double));
    value1 = (double *)malloc(size*sizeof(double));
    p1=(char *) the_data.value.get_buffer(0);
    p2 = (char *) src.value.get_buffer(0);
    value  = (double*)p1;
    value1  = (double*)p2;
    cout << " UPDATEDATA IN LIST  VALUE  = " << endl;
    for(unsigned int i=0; i<size/8;i++) cout << " - " << value[i] ;
    cout << endl;
    cout << " UPDATEDATA IN ELT  VALUE  = " << endl;
    for(unsigned int i=0; i<size/8;i++) cout << " -  " << value1[i] ;
    cout << endl;
    */
} // updateDataList(corba_data_t& src)


/****************************************************************
 * adds persistent data to the list                             *
 * propagates this operation to its parent                      *
 ***************************************************************/
void
DataMgrImpl::addData(corba_data_t& dataDesc, int inout)
{
  // FIXME :  if (inout == 0) ??
  addDataDescToList(&dataDesc,inout);  
 
  if(dataDesc.desc.mode != DIET_STICKY) {
    // doesn't update Loc Manager references is sticky data
      parent->addDataRef(dataDesc.desc,childID);
  }
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
  if(dataLookup(CORBA::string_dup(argID))){ 
    //  dataDescList.unlock();
    return(this->_this());
  } else {
    return(DataMgr::_nil());
  }
} // whereData(const char* argID)

#if HAVE_ALTPREDICT
/***************************************************************
 * Look for data reference in the DataManager, but only recover*
 * some location information about the data (for scheduling)   *
 **************************************************************/
corba_data_loc_t*
DataMgrImpl::getDataLoc(const char* argID) {
  corba_data_loc_t* dataLoc;
  dataLoc = new corba_data_loc_t;

  if(dataLookup(CORBA::string_dup(argID))){
    // Data exists locally
    dataLoc->idNumber = CORBA::string_dup(argID);
    dataLoc->hostName = CORBA::string_dup(this->localHostName);
    dataLoc->locationID = CORBA::string_dup(this->locationID);
    return (dataLoc);
  } else {
    WARNING("getDataLoc called on DataMgr for " << argID
          << " - data doesn't exist!");
    return NULL;
  }
}
#endif // HAVE_ALTPREDICT

/***************************************************************
 * returns true if data present, false elsewhere               *
 **************************************************************/
bool
DataMgrImpl::dataLookup(char* argID)
{
  printList1();
 // if(dataDescList.size() > 0){
    dataDescList.lock();
    bool found = dataDescList.find(ms_strdup(argID)) != dataDescList.end();
    dataDescList.unlock();
    return found;
} //dataLookup(char* argID)

/** to be defined */
void
DataMgrImpl::dataIDLock(const corba_data_id_t& cDataID)
{
}

/** to be defined */
void
DataMgrImpl::dataIDUnlock(const corba_data_id_t &cDataID)
{
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
  bool found = TRUE;
  return(found);
}

void
DataMgrImpl::updateDataProperty(corba_data_t& dataDesc)
{

}

#endif // 0
