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
 * Revision 1.1  2003/09/22 21:07:21  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 ***************************************************************************/

#include "DataMgrImpl.hh"

#include "common_types.hh"
#include "dietTypes.hh"
#include "LocMgr.hh"
#include "ms_function.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "ts_container/ts_map.hh"

#define DEVELOPPING_DATA_PERSISTENCY 0

DataMgrImpl::DataMgrImpl()
{
  this->childID  = (childID)-1;
  this->parent = LocMgr::_nil();
  this->dataDescList.emptyIt();
  this->lockList.emptyIt();
}

DataMgrImpl::~DataMgrImpl(){}


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

/** Copy the elt which identifier is id to an elt of corba_data_t type */
corba_data_t
DataMgrImpl::cpEltListToDataT(const char* id)
{
  corba_data_t dataDesc; 
#if DEVELOPPING_DATA_PERSISTENCY
  bool found = false;
  void* value(NULL);
  dietDataDescList::Iterator* iter = this->myDataDescList.getIterator();
 
  while ((iter->hasNext()) && (found == false)) {
    if (strcmp(((dietDataDescT)(iter->getCurrent())).dataID.idNumber,id)==0) {
      dataDesc.desc.id = CORBA::string_dup(((dietDataDescT)(iter->getCurrent())).dataID.idNumber) ;
    
      dataDesc.desc.mode = ((dietDataDescT)(iter->getCurrent())).mode;
      dataDesc.desc.base_type = ((dietDataDescT)(iter->getCurrent())).base_type;
      switch ((diet_data_type_t)(((dietDataDescT)(iter->getCurrent()).specific._d()))) {
      case DIET_SCALAR: {
	corba_scalar_specific_t scal;
	dataDesc.desc.specific.scal(scal);
	switch ((diet_base_type_t)((dietDataDescT)(iter->getCurrent())).base_type) {
	case DIET_CHAR:
	case DIET_BYTE: {
	  dataDesc.desc.specific.scal().value = ((dietDataDescT)(iter->getCurrent())).specific.scal().value;
	  break;
	}
	case DIET_INT: {
	  dataDesc.desc.specific.scal().value = ((dietDataDescT)(iter->getCurrent())).specific.scal().value;
	  break;
	}
	case DIET_LONGINT:{
	  dataDesc.desc.specific.scal().value = ((dietDataDescT)(iter->getCurrent())).specific.scal().value;
	  break;
	}
	case DIET_FLOAT:{
	  dataDesc.desc.specific.scal().value = ((dietDataDescT)(iter->getCurrent())).specific.scal().value;
	  break;
	}
	case DIET_DOUBLE: {
	  dataDesc.desc.specific.scal().value = ((dietDataDescT)(iter->getCurrent())).specific.scal().value;
	  break;
	}
#if HAVE_COMPLEX
	case DIET_SCOMPLEX:
	case DIET_DCOMPLEX:
#endif // HAVE_COMPLEX
	default:
	  cerr << "pb on Base type "
	       << dataDesc.desc.base_type << " not implemented.\n";
	}
	break;
      }
      case DIET_VECTOR:
	corba_vector_specific_t vect;
	dataDesc.desc.specific.vect(vect);
	dataDesc.desc.specific.vect().size = ((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.specific.vect().size;
	break;
      case DIET_MATRIX: 
	corba_matrix_specific_t mat;
	dataDesc.desc.specific.mat(mat);
	dataDesc.desc.specific.mat().nb_r = ((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.specific.mat().nb_r;
	dataDesc.desc.specific.mat().nb_c = ((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.specific.mat().nb_c;
	dataDesc.desc.specific.mat().order = ((dietDataDesT*)(iter->curr()))->dataDescPtr->dataDesc.specific.mat().order;
	break;
      case DIET_STRING: 
	corba_string_specific_t str;
	dataDesc.desc.specific.str(str);
	dataDesc.desc.specific.str().length = ((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.specific.str().length;
	break;
      case DIET_FILE:
	corba_file_specific_t file;
	dataDesc.desc.specific.file(file);
	dataDesc.desc.specific.file().size = ((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.specific.file().size;
	dataDesc.desc.specific.file().path = CORBA::string_dup(((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.specific.file().path);
      }
      long unsigned int size = (long unsigned int) data_sizeof(&(dataDesc.desc));
      if ( (diet_data_type_t)(dataDesc.desc.specific._d()) != DIET_FILE) {
	CORBA::Boolean orphan = 0;
	CORBA::Char *p;
	// p=malloc(size*sizeof(char));
	p= ((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.value.get_buffer(orphan);
	
	dataDesc.value.replace(size, size,p,0);   
      }
      found = true;
    }
  }
  delete(iter);
  TRACE_VAR(dataDesc.desc.id);
  TRACE_VAR(dataDesc.desc.specific.mat().nb_r);   
  TRACE_VAR(dataDesc.desc.specific.mat().nb_c);

#endif // DEVELOPPING_DATA_PERSISTENCY

 return(dataDesc); 
}
 


void
DataMgrImpl::addDataIDToLockList(char* id)
{
#if DEVELOPPING_DATA_PERSISTENCY
  dietataIdLockT* newLock = new dataIdLockT();
  char c;
  newLock->id = CORBA::string_dup(id) ; 
  dietDataIdLockList::Iterator* iter = this->myLockList.getIterator();
  if(!iter->hasCurrent()) {
    myLockList->addElement(newLock);
  } else {
    while (iter->next()) {}
    myLockList->addElement(newLock);
  }
  delete (iter);
#endif // DEVELOPPING_DATA_PERSISTENCY
}


void
DataMgrImpl::rmDataFromIDList(char * id)
{
#if DEVELOPPING_DATA_PERSISTENCY
  int deleted = 0;
  
  dietDataIdLockT* newLock = new dietDataIdLockT();
  
  
  //omni_mutex newLock->locker->lockMutex = new omni_mutex();
  
  dataIdlockListIterator* iter = this->myLockList.getIterator();
  
  
  
  while ((iter->next()) && (deleted==0)) {
    if (strcmp(((dietDataIdLockT*)(iter->curr()))->locker->id,id)==0)
      //  deleted = 1;
    iter->curr()->del(); 
    deleted = 1;
  }
  
  delete (iter);
#endif // DEVELOPPING_DATA_PERSISTENCY
}

void
DataMgrImpl::addDataDescToList(corba_data_t* dataDesc, int inout)
{
#if DEVELOPPING_DATA_PERSISTENCY
  char c;
  void* value(NULL);
  dietDataDescT *newDataDesc = new dietDataDescT();
  newDataDesc->dataDescPtr = new dietDataDescT();
  newDataDesc->dataDescPtr->dataDesc.dataID.idNumber  = CORBA::string_dup(dataDesc->desc.id);

  newDataDesc->dataDescPtr->dataDesc.dataID.myName  = CORBA::string_dup(myDataName);
  
  newDataDesc->dataDescPtr->dataDesc.dataID.dataCopy  = DIET_ORIGINAL;

  newDataDesc->dataDescPtr->dataDesc.dataID.state  = DIET_FREE;

  newDataDesc->dataDescPtr->dataDesc.base_type = dataDesc->desc.base_type;
 
   newDataDesc->dataDescPtr->dataDesc.mode = dataDesc->desc.mode;
   switch ((diet_data_type_t)(dataDesc->desc.specific._d())) {
   case DIET_SCALAR: {
     corba_scalar_specific_t scal;
     newDataDesc->dataDescPtr->dataDesc.specific.scal(scal);
     newDataDesc->dataDescPtr->dataDesc.base_type = (diet_base_type_t)(dataDesc->desc.base_type);
     switch ((diet_base_type_t)(dataDesc->desc.base_type)) {
     case DIET_CHAR:
     case DIET_BYTE: {
       newDataDesc->dataDescPtr->dataDesc.specific.scal().value = dataDesc->desc.specific.scal().value;
       break;
     }
     case DIET_INT:{
       newDataDesc->dataDescPtr->dataDesc.specific.scal().value = dataDesc->desc.specific.scal().value;
       break;
     }
     case DIET_LONGINT:{
       newDataDesc->dataDescPtr->dataDesc.specific.scal().value = dataDesc->desc.specific.scal().value;
       break;
     }
     case DIET_FLOAT:{
       dataDesc->desc.specific.scal().value;
       newDataDesc->dataDescPtr->dataDesc.specific.scal().value =  dataDesc->desc.specific.scal().value;
       break;
     }
     case DIET_DOUBLE:{
       newDataDesc->dataDescPtr->dataDesc.specific.scal().value =  dataDesc->desc.specific.scal().value;
       break;
     }
#if HAVE_COMPLEX
     case DIET_SCOMPLEX:
     case DIET_DCOMPLEX:
#endif  //HAVE_COMPLEX
     default:
       cerr << "mrsh_scalar_desc: Base type "
	    << dataDesc->desc.base_type << " not implemented.\n";
     } 
     break;
   }	     
   case DIET_VECTOR:  
     corba_vector_specific_t vect;
     newDataDesc->dataDescPtr->dataDesc.specific.vect(vect);
     newDataDesc->dataDescPtr->dataDesc.specific.vect().size = dataDesc->desc.specific.vect().size;
     break;
   case DIET_MATRIX:
     corba_matrix_specific_t mat;
     newDataDesc->dataDescPtr->dataDesc.specific.mat(mat);
     newDataDesc->dataDescPtr->dataDesc.specific.mat().nb_r = dataDesc->desc.specific.mat().nb_r;
     newDataDesc->dataDescPtr->dataDesc.specific.mat().nb_c = dataDesc->desc.specific.mat().nb_c;
     newDataDesc->dataDescPtr->dataDesc.specific.mat().order = dataDesc->desc.specific.mat().order;
    
     break;
   case DIET_STRING: 
     corba_string_specific_t str;
     newDataDesc->dataDescPtr->dataDesc.specific.str(str);
     newDataDesc->dataDescPtr->dataDesc.specific.str().length = dataDesc->desc.specific.str().length;
     break;
   case DIET_FILE:{ 
     corba_file_specific_t file;
     newDataDesc->dataDescPtr->dataDesc.specific.file(file);
     newDataDesc->dataDescPtr->dataDesc.specific.file().size = dataDesc->desc.specific.file().size;
     newDataDesc->dataDescPtr->dataDesc.specific.file().path = CORBA::string_dup(dataDesc->desc.specific.file().path);
   }  
       
   }

  
   
     
   long unsigned int size = (long unsigned int) data_sizeof(&(newDataDesc->dataDescPtr->dataDesc));
   TRACE_VAR(size);
   if ( (diet_data_type_t)(dataDesc->desc.specific._d()) != DIET_FILE) {
    
     if(inout == 0) {// IN ARGS
       CORBA::Boolean orphan = 1;
       CORBA::Char * p(NULL);
       //p = (char *)malloc(size*sizeof(char));
       p = static_cast <CORBA::Char *> (dataDesc->value.get_buffer(orphan));

       newDataDesc->dataDescPtr->dataDesc.value.replace(size,size,p,1);

     } else { // OUT ARGS
       CORBA::Boolean orphan = 0;
       CORBA::Char * p(NULL);
  
       p = static_cast <CORBA::Char *> (dataDesc->value.get_buffer(orphan));

       newDataDesc->dataDescPtr->dataDesc.value.replace(size,size,p,1);

       //dataDesc->value.replace(size,size,p,0);
     }
       
   }
 
     
   dietDataDescList::Iterator* iter = this->myDataDescList.getIterator();
     
   if(!iter->hasCurrent()) {
     myDataDescList->addElement(newDataDesc);
       
   }else {
     while (iter->next()) {}
     myDataDescList->addElement(newDataDesc);
   } 
     
   delete (iter);
   //free ((corba_DataMgr_desc_t *)dataDesc);
#endif // DEVELOPPING_DATA_PERSISTENCY
}

// call by Server
void
DataMgrImpl::rmDataDescFromList(char* id)
{
#if DEVELOPPING_DATA_PERSISTENCY
  int deleted = 0;

  dietDataDescList::Iterator* iter = this->myDataDescList.getIterator();
 
  while ((iter->next()) && (deleted == 0)) {   
    if (strcmp(((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.dataID.idNumber,id)==0) {
      dietDataDescT* tmp =
	(dietDataDescT)(iter->getCurrent());
      CORBA::Char *p1 (NULL);
       p1 = tmp->dataDescPtr->dataDesc.value.get_buffer(1);
       _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Char *)p1);
      tmp->del();
      delete(tmp);
      deleted = 1;
    
    }
  }
  delete(iter);
#endif // DEVELOPPING_DATA_PERSISTENCY
}

void
DataMgrImpl::dataIDLock(const corba_data_id_t& cDataID)
{
#if DEVELOPPING_DATA_PERSISTENCY
  int locked = 0;
  dataIdlockListIterator* iter = this->myLockList.getIterator();
 
   while ((iter->next()) && (locked == 0)) {
    if (strcmp(((dietDataIdLockT*)(iter->curr()))->locker->id,cdataId.idNumber)==0) {
      ((dietDataIdLockT*)(iter->curr()))->locker->lockMutex.lock();
       ((dietDataIdLockT*)(iter->curr()))->locker->lockMutex.lock();
       locked = 1;
   
    }

   }
   locked = 0;
#endif // DEVELOPPING_DATA_PERSISTENCY
}

void
DataMgrImpl::dataIDUnlock(const corba_data_id_t& cDataID)
{
#if DEVELOPPING_DATA_PERSISTENCY
  int unlocked = 0;
  dataIdlockListIterator* iter = this->myLockList.getIterator();
  
   while ((iter->next()) && (unlocked == 0)) {
    if (strcmp(((dietDataIdLockT*)(iter->curr()))->locker->id,cdataId.idNumber)==0) {
      ((dietDataIdLockT*)(iter->curr()))->locker->lockMutex.unlock();
       unlocked = 1;
  
      // suppression de l'elt dans la liste
    }

}
   unlocked = 0;
#endif // DEVELOPPING_DATA_PERSISTENCY
}


// invoque par SeD
#if 0
void
DataMgrImpl::updateDataRefOrder(corba_data_t& dataDesc)
{
  corba_dataMgr_desc_t *dataIdDesc = new(corba_dataMgr_desc_t);
  *dataIdDesc=moveToCorbaDataDesc(dataDesc);
  addDataDescToList(dataIdDesc); // ajout de la donnee  

  locFather->updateDataRef(dataIdDesc->dataID.idNumber,sonId,0); // 0 = UP
}

#endif // 0
 
 
bool
DataMgrImpl::isInLockList(char* id)
{
  bool found=false;
#if DEVELOPPING_DATA_PERSISTENCY
  
  dietDataDescList::Iterator* iter = this->myDataDescList.getIterator();
  
  while ((iter->next()) && (found == false)) {
    
    if (strcmp(((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.dataID.idNumber,id)==0) {
      found =true;

    }
  }
  delete(iter);
#endif // DEVELOPPING_DATA_PERSISTENCY
  return(found);
}

void
DataMgrImpl::sendData(corba_data_t& arg)
{
#if DEVELOPPING_DATA_PERSISTENCY
  if ( (diet_data_type_t)(data.desc.specific._d())== DIET_FILE) {
  
    if ((data.desc.specific.file().path != NULL)
	&& strcmp("", data.desc.specific.file().path)) {
      char* in_path   = CORBA::string_dup(data.desc.specific.file().path);
      char* file_name = strrchr(in_path, '/');
      char* out_path  = new char[256];
      pid_t pid = getpid();
      sprintf(out_path, "/tmp/DIET_%d_%s", pid,
	      (file_name) ? (char*)(1 + file_name) : in_path);
      
      ofstream outfile(out_path);
      for (int i = 0; i < data.desc.specific.file().size; i++) {
	outfile.put(data.value[i]);
      }
    }

  }
  addDataDescToList(&data,0);

#endif // DEVELOPPING_DATA_PERSISTENCY
}


corba_data_t
DataMgrImpl::getData(char* id)
{
  corba_data_t dataDesc;
#if DEVELOPPING_DATA_PERSISTENCY
  if(dataLookup(id)){ // si donnee présente

    dataDest=cpEltListToDataT(id);
 
  } else { // donnée ailleurs
    
    char *dataName=(char *)malloc(261*sizeof(char));
    corba_dataMgr_desc_t *dataDesc = new corba_dataMgr_desc_t();

    dataDesc->dataID.idNumber=CORBA::string_dup(id);

    dataName=locFather->whereData(id,myDataName);

   addDataIdToLockList(id);
   
    DataMgr_var dataSrc=resolveDataName(dataName);
   
  
    dataSrc->putData(id, myDataName);
      rmDataFromIdList(id);
      dataDest=cpEltListToDataT(id);
    locFather->updateDataRef(id,sonId,0);  
  }
#endif // DEVELOPPING_DATA_PERSISTENCY
  return(dataDesc);   
}
 


 

/**
 *
 */
void
DataMgrImpl::putData(const char* id, const char* me)
{
#if DEVELOPPING_DATA_PERSISTENCY
  corba_data_id_t dataId;
  corba_data_t *dataDesc = new corba_data_t();
  cout << "in putData " << endl;
  // lock pour l'envoi
  // dataIdLock(dataDesc->dataID);
  cout << " i am " << myDataName << "sending to " << me << "the data " << id << endl;
  *dataDesc=cpEltListToDataT(id);
  if ( (diet_data_type_t)(dataDesc->desc.specific._d()) == DIET_FILE) {
    CORBA::Char * dataValue(NULL);
    //    long unsigned int size = (long unsigned int) data_sizeof(&(dataDesc->desc));
    ifstream infile(dataDesc->desc.specific.file().path);
    if (dataDesc->desc.specific.file().path && strcmp("",dataDesc->desc.specific.file().path)){
      dataValue = SeqChar::allocbuf(dataDesc->desc.specific.file().size);
      for (int i = 0; i < dataDesc->desc.specific.file().size; i++) {
	dataValue[i] = infile.get();
      }
      infile.close();
    } else {
      dataValue = SeqChar::allocbuf(1);
      dataValue[0] = '\0';
    } 
    dataDesc->value.replace(dataDesc->desc.specific.file().size,dataDesc->desc.specific.file().size , dataValue, 1);
  }
  TRACE_VAR(dataDesc->desc.id);
  TRACE_VAR(dataDesc->desc.specific.mat().nb_r);   
  TRACE_VAR(dataDesc->desc.specific.mat().nb_c);
  DataMgr_var dest = resolveDataName((char *)me); 
  cout << "sending data " << endl;
  
  dest->sendData(*dataDesc);
  //  return dataDesc;
#endif // DEVELOPPING_DATA_PERSISTENCY
}


void
DataMgrImpl::updateDataList(corba_data_t& src)
{
#if DEVELOPPING_DATA_PERSISTENCY
  bool found = false;  
  dietDataDescList::Iterator* iter = this->myDataDescList.getIterator();
 
 while ((iter->hasNext()) && (found == false)) {
   iter->next();
   if (strcmp(((dietDataDescT)(iter->getCurrent())).dataID.idNumber,src.desc.id)==0) {
     found = true;
   }
 }
 long unsigned int size = (long unsigned int) data_sizeof(&(((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc));
 CORBA::Boolean orphan = 1;
 CORBA::Char *p, *p1;
 // p=malloc(size*sizeof(char));
 p= src.value.get_buffer(orphan);
 p1 = ((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.value.get_buffer(1);
 _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Char *)p1);
 
 ((dietDataDescT)(iter->getCurrent()))->dataDescPtr->dataDesc.value.replace(size, size,p,1);   
 if(src.desc.mode == DIET_PERSISTENT_RETURN){
   src.value.replace(size, size,p,0);
 }

 delete(iter);
#endif // DEVELOPPING_DATA_PERSISTENCY
}

// invoque par SeD
bool
DataMgrImpl::addData(corba_data_t& dataDesc, int inout)
{
#if DEVELOPPING_DATA_PERSISTENCY
  //corba_data_id_t data->dataId = new corbaDataId();
  //moveToCorbaDataDesc(dataDesc);
  //  if (inout==0)
    addDataDescToList(&dataDesc,inout);  
  //  if(dataDesc.desc.mode != DIET_STICKY)
  locFather->addDataRef(dataDesc.desc.id,sonId);
#endif // DEVELOPPING_DATA_PERSISTENCY
  return (true);
}


// invoqué par SeD lorsque l'état de la donnée a changé pour les donnees in voir
// apres solve pour passer en parametres les id uniquement A MODIFIER

void
DataMgrImpl::updateDataProperty(corba_data_t& dataDesc)
{
#if DEVELOPPING_DATA_PERSISTENCY
  int replaced = 0;
     dietDataDescT *newDataDesc = new dietDataDescT();
     dietDataDescList::Iterator* iter = this->myDataDescList.getIterator();
  // newDataDesc->dataDesc = new  dietDataDescT(); 
  while ((iter->hasNext()) && (replaced == 0)) {
    iter->next();
    if (strcmp(((dietDataDescT)(iter->getCurrent())).dataID.idNumber,dataDesc.desc.id)==0) {
      newDataDesc->dataDescPtr->dataDesc.dataID.idNumber = dataDesc.desc.id; 
      //myDataDescList->replace (newDataDesc); // A REVOIR
      replaced = 1;
    }
  }    
  if (replaced == 1){
    //corba_data_id_t *cDataId = new corba_data_id_t();
    //  cDataId->idNumber = dataDesc.desc.id; 
    locFather->updateDataProp(dataDesc.desc.id);
  }
  delete (iter); 
  // SUPPRIMER dataDesc
#endif // DEVELOPPING_DATA_PERSISTENCY
}



//call by parent Loc to destroy the reference to the data
#if 0
void
DataMgrImpl::rmAllData()
{
  dietDataDescList::Iterator* iter = this->myDataDescList.getIterator();
 
  while ((iter->next())) {   
  
      dietDataDescT tmp =
	(dietDataDescT)(iter->getCurrent());
      CORBA::Char *p1 (NULL);
       p1 = tmp.value.get_buffer(1);
       _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Char *)p1);
       //tmp->del();
       //delete(tmp);
    
      cout << "data deleted in data" << endl;
 
  }
  delete(iter);
}

#endif // 0

void
DataMgrImpl::rmDataRef(const char* argID)
{
  rmDataDescFromList(CORBA::string_dup(argID)); 
}
 


// call by parent Loc ??? A remplacer par PUTDATA ????????? 
char*
DataMgrImpl::whereData(const char* argID, const char* me)
{
#if DEVELOPPING_DATA_PERSISTENCY
  cout << "am i invoked ????????????????????????????????????????????????????" << endl;
  if(dataLookup(CORBA::string_dup(cDataId))){ 
    cout << "got the data - i am : " << myDataName << endl;
    // cin.get(c);
    return(CORBA::string_dup(myDataName));
 } else {
   cout << "not here" << endl;
  return((char *)NULL);
  }
#else // DEVELOPPING_DATA_PERSISTENCY
  return NULL;
#endif // DEVELOPPING_DATA_PERSISTENCY
}

DataMgr_var
DataMgrImpl::resolveDataName(char* myName)
{
  DataMgr_var remoteData = DataMgr::_nil();
#if DEVELOPPING_DATA_PERSISTENCY
  char remoteDataName[261];
  strcpy(remoteDataName,myName);
  //strcat(remoteDataName,"Data");
  
  remoteData = DataMgr::_duplicate(DataMgr::_narrow(ORBMgr::getDataReference(remoteDataName)));
  if (CORBA::is_nil(remoteData)) {
    cerr << "Cannot locate remote data " << remoteData << endl;
    // cin.get(c);
  }
#endif // DEVELOPPING_DATA_PERSISTENCY
  return(remoteData);
}



/*
 dans le cas où c'est le SeD qui demande l'envoi de la donnée, si non présente modifier la fonctonputDataRefOrder pour réaliser le whereData
- rajouter des methodes de modification du dataID 
*/


bool
DataMgrImpl::dataLookup(char* id)
{
  bool found= false;
#if DEVELOPPING_DATA_PERSISTENCY
  dietDataDescList::Iterator* iter = this->myDataDescList.getIterator();
  
 
  while ((iter->hasNext()) && (found == false)) {
    iter->next();
  if (strcmp(((dietDataDescT)(iter->getCurrent())).dataID.idNumber,id)==0) {
   found = true;
    }
  }
  delete(iter);
#endif // DEVELOPPING_DATA_PERSISTENCY

  return(found);
}
