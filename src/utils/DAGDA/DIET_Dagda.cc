/***********************************************************/
/* Dagda specific functions.                               */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@in2p3.fr)                   */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
/* $Log
/*
************************************************************/
#include "DIET_Dagda.hh"
#include "DagdaFactory.hh"

#include "MasterAgent.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "marshalling.hh"
#include <omniORB4/CORBA.h>
#include "debug.hh"

#include <string>
#include <sstream>

#if HAVE_ADVANCED_UUID
#include <uuid/uuid.h>
#endif

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

using namespace std;

void dagda_mrsh_profile(corba_profile_t* corba_profile, diet_profile_t* profile,
  MasterAgent_var& MA) {
  DagdaImpl* dataManager = DagdaFactory::getDataManager();
  char* dataManagerIOR = ORBMgr::getIORString(dataManager->_this());

  corba_profile->parameters.length(profile->last_out+1);

#if defined HAVE_ALT_BATCH 
  corba_profile->parallel_flag = profile->parallel_flag ;
  corba_profile->nbprocs    = profile->nbprocs ;
  corba_profile->nbprocess  = profile->nbprocess ;
  corba_profile->walltime   = profile->walltime ;
#endif
  corba_profile->last_in    = profile->last_in;
  corba_profile->last_inout = profile->last_inout;
  corba_profile->last_out   = profile->last_out;
  corba_profile->dietReqID  = profile->dietReqID ;
  corba_profile->clientIOR = CORBA::string_dup(dataManagerIOR);

  for (int i=0; i<=profile->last_out; ++i) {
    bool haveID = false;
	corba_data_t data;
	//CORBA::Char* value = (CORBA::Char*) profile->parameters[i].value;
	//corba_profile->parameters[i].value.length(0);

	// Does the data need an ID ? Yes: Get one from the MA.
	// Else get the data description from the platform.
    if (profile->parameters[i].desc.id != NULL)
	  if (strlen(profile->parameters[i].desc.id)!=0) {
	    // The data is stored on the platform. Get its description.
	    data.desc = *MA->get_data_arg(profile->parameters[i].desc.id);
		haveID = true;
	  }

	// This is a new data. It needs an ID and its data manager is
	// this client data manager.
	if (i<=profile->last_inout && !haveID) {
	  corba_data_t* storedData;
	  size_t size;
	  char* dataID = MA->get_data_id();

	  // Set the ID and the data manager IOR.
	  profile->parameters[i].desc.id = dataID;
	  mrsh_data_desc(&data.desc, &profile->parameters[i].desc);
	  data.desc.dataManager = CORBA::string_dup(dataManagerIOR);
	  
	  // Add the data in the client data manager.
	  // And get the pointer on it.
	  dataManager->addData(data);
	  storedData = dataManager->getData(dataID);
	  
	  if (profile->parameters[i].desc.generic.type!=DIET_FILE)
	    size = data_sizeof(&profile->parameters[i].desc);
	  else size = 0;

	  // Data in the profile and in the data manager share the same
	  // pointer. The last parameter  of "replace" is the "release"
	  // parameter. It is set to 0 to avoid  double free.
	  if (profile->parameters[i].value!=NULL)
	    storedData->value.replace(size, size,
		  (CORBA::Char*) profile->parameters[i].value, 0);

	  // Only scalar values are sent into the profile.
	  // For other types, the peer has to download them.
	  if (profile->parameters[i].desc.generic.type==DIET_SCALAR)
	    corba_profile->parameters[i] = *storedData;
	  else
		corba_profile->parameters[i].desc = storedData->desc;

	  dataManager->unlockData(dataID);

	  continue;
	}
	// Out data needing an ID.
	if (i>profile->last_inout && !haveID) {
	  char* dataID = MA->get_data_id();

	  profile->parameters[i].desc.id = dataID;
	  mrsh_data_desc(&corba_profile->parameters[i].desc, &profile->parameters[i].desc);
	}
	// The data is on the platform. Set its description.
	if (haveID)
	  corba_profile->parameters[i].desc=data.desc;
  }
}

void dagda_download_SeD_data(diet_profile_t* profile,
			corba_profile_t* pb) {
  DagdaImpl* dataManager = DagdaFactory::getDataManager();
  corba_data_t data;
  corba_data_t* inserted;
  size_t size;
  // Free the remote volatile data.
  for (int i=0; i<=pb->last_in; ++i) {
    Dagda_var remoteManager =
      Dagda::_narrow(ORBMgr::stringToObject(pb->parameters[i].desc.dataManager));
	if (pb->parameters[i].desc.mode==DIET_VOLATILE)
      remoteManager->lclRemData(pb->parameters[i].desc.id.idNumber);
  }
  // Downloads the data from the SeD to the client.
  for (int i=pb->last_in+1; i<=pb->last_inout; ++i) {
    Dagda_var remoteManager =
      Dagda::_narrow(ORBMgr::stringToObject(pb->parameters[i].desc.dataManager));
    if (pb->parameters[i].desc.mode != DIET_PERSISTENT &&
	  pb->parameters[i].desc.mode != DIET_STICKY) {

	  if (pb->parameters[i].desc.specific._d() != DIET_SCALAR ) {
	    inserted = dataManager->getData(pb->parameters[i].desc.id.idNumber);
	    size = inserted->value.length();
	    CORBA::Char* value = inserted->value.get_buffer();
	    char*  path = inserted->desc.specific.file().path;
	    size_t fileSize = inserted->desc.specific.file().size;
	  
	    dataManager->lclAddData(remoteManager, pb->parameters[i]);
	  	  
	    inserted = dataManager->getData(pb->parameters[i].desc.id.idNumber);
	  
	    if (pb->parameters[i].desc.specific._d() != DIET_FILE) {
	      for (int j=0; j<size; ++j)
		    value[j]=inserted->value[j]; 
          inserted->value.replace(size, size, value, 0);
	    }
	    else {
	      unlink(inserted->desc.specific.file().path);
	      rename(path, inserted->desc.specific.file().path);
		  inserted->desc.specific.file().path = path;
		  inserted->desc.specific.file().size = fileSize;
	    }
	  } else {
	    inserted = dataManager->getData(pb->parameters[i].desc.id.idNumber);
	    size = inserted->value.length();
	    for (int j=0; j<size; ++j)
	      inserted->value[j]=pb->parameters[i].value[j];
	  }
    }
	if (pb->parameters[i].desc.mode==DIET_VOLATILE)
      remoteManager->lclRemData(pb->parameters[i].desc.id.idNumber);
  }
  for (int i=pb->last_inout+1; i<=pb->last_out; ++i) {
    Dagda_var remoteManager =
      Dagda::_narrow(ORBMgr::stringToObject(pb->parameters[i].desc.dataManager));
    if (pb->parameters[i].desc.mode != DIET_PERSISTENT &&
	  pb->parameters[i].desc.mode != DIET_STICKY) {
      if (pb->parameters[i].desc.specific._d() != DIET_SCALAR ) {
  	    dataManager->lclAddData(remoteManager, pb->parameters[i]);
	    inserted = dataManager->getData(pb->parameters[i].desc.id.idNumber);
	    size = inserted->value.length();
	    unmrsh_data_desc(&profile->parameters[i].desc, &inserted->desc);
	    profile->parameters[i].value = inserted->value.get_buffer();
	  } else {
	    data.desc = pb->parameters[i].desc;
	    size = pb->parameters[i].value.length();
	    dataManager->addData(data);
	    inserted = dataManager->getData(data.desc.id.idNumber);
	    CORBA::Char* value = pb->parameters[i].value.get_buffer(1);
	    inserted->value.replace(size, size, value , 0);
	    unmrsh_data_desc(&profile->parameters[i].desc, &inserted->desc);
	    profile->parameters[i].value = inserted->value.get_buffer();
	    dataManager->unlockData(pb->parameters[i].desc.id.idNumber);
	  }
    }
	if (pb->parameters[i].desc.mode==DIET_VOLATILE)
      remoteManager->lclRemData(pb->parameters[i].desc.id.idNumber);
  }
  //dagda_free_volatile_data(pb);
}


diet_error_t dagda_get_data_desc(corba_pb_desc_t& corba_pb, MasterAgent_var& MA) {
    // Retrieves the information about a data stored on the platform.
    for (int i=0; i<=corba_pb.last_out; ++i) {
      if (strlen(corba_pb.param_desc[i].id.idNumber)!=0) {
	    if (!MA->dataLookUp(corba_pb.param_desc[i].id.idNumber)) {
	      ERROR(" data with ID " << corba_pb.param_desc[i].id.idNumber
		        << " not inside the platform.", 1);
	    } else {
	      const_cast<corba_data_desc_t&>(corba_pb.param_desc[i]) =
	        *MA->get_data_arg(corba_pb.param_desc[i].id.idNumber);
	    }
	  }
	}
	return 0;
}

/* !!! Important note to programers: !!!
     - The CORBA::sequence::get_buffer(orphan == true) makes the next
	   call to get_buffer returns NULL and gives the pointer control
	   to the user;
	 - The CORBA::sequence::replace(size, maxSize, ptr, release == false)
	   gives the pointer control to the user;
   These two parameters mean the opposite. Be careful to have it in mind.
*/
void dagda_download_data(diet_profile_t& profile, corba_profile_t& pb) {
  DagdaImpl* dataManager = DagdaFactory::getDataManager();
  corba_data_t data;
  corba_data_t* inserted;
  size_t size;
  profile.parameters = new diet_data_t[pb.last_out + 1];
  profile.last_in = pb.last_in;
  profile.last_inout = pb.last_inout;
  profile.last_out = pb.last_out;
  profile.dietReqID = pb.dietReqID;
#if defined HAVE_ALT_BATCH
  profile.parallel_flag = pb.parallel_flag ;
  profile.nbprocs    = pb.nbprocs ;
  profile.nbprocess  = pb.nbprocess ;
  profile.walltime   = pb.walltime ;
#endif
  for (int i=0; i<= pb.last_inout; ++i) {
    if (!dataManager->pfmIsDataPresent(pb.parameters[i].desc.id.idNumber)) {
      Dagda_var remoteManager =
        Dagda::_narrow(ORBMgr::stringToObject(pb.parameters[i].desc.dataManager));
	  // The data is not yet registered here.
	  if (pb.parameters[i].desc.specific._d() != DIET_SCALAR) {
	    // Data is transmitted from the remote manager.
	    dataManager->lclAddData(remoteManager, pb.parameters[i]);
	    inserted =  dataManager->getData(pb.parameters[i].desc.id.idNumber);
	    size = inserted->value.length();
	    unmrsh_data_desc(&profile.parameters[i].desc, &inserted->desc);
		// (1):
		// IN value: The data manager still have control on the value pointer.
		// INOUT value: The data manager gives the pointer control to the SeD.
		// get_buffer(boolean orphan) :
		// orphan == true => inserted->value=NULL && param.value=ptr
		// The SeD can modify *ptr.
		// orphan == false => inserted->value=param.value=ptr
		// The SeD must NOT to modify *ptr !
	    profile.parameters[i].value = inserted->value.get_buffer(i>pb.last_in ? true:false);
	  }
	  else {
	    // Scalar values are transmitted in the profile.
	    data.desc = pb.parameters[i].desc;
	    size = pb.parameters[i].value.length();
	    dataManager->addData(data);
	    inserted = dataManager->getData(data.desc.id.idNumber);

	    // If the data is INOUT, we leave its management to the SeD developper.
	    // Otherwise, the data should not be modified.
	    // To modify it is a major bug.
		// The corba profile leaves the pointer control to the data manager.
	    inserted->value.replace(size, size, pb.parameters[i].value.get_buffer(true), true);
	    unmrsh_data_desc(&profile.parameters[i].desc, &inserted->desc);
		// See (1) below.
	    profile.parameters[i].value = inserted->value.get_buffer(i>pb.last_in ? true:false);
	 	dataManager->unlockData(pb.parameters[i].desc.id.idNumber);
		dataManager->useMemSpace(size);
	  }
	} else {
	  // Data is present. We use it.
	  Dagda_var bestSource = dataManager->getBestSource(dataManager->_this(),
	    pb.parameters[i].desc.id.idNumber);
	  
	  dataManager->lclAddData(bestSource, pb.parameters[i]);
	  inserted = dataManager->getData(pb.parameters[i].desc.id.idNumber);
	  size = inserted->value.length();
	  unmrsh_data_desc(&profile.parameters[i].desc, &inserted->desc);
	  // See (1) below.
	  profile.parameters[i].value = inserted->value.get_buffer(i>pb.last_in ? true:false);
	}
  }
  for (int i=pb.last_inout+1;i<=pb.last_out; ++i)
    unmrsh_data_desc(&profile.parameters[i].desc, &pb.parameters[i].desc);
}

void dagda_upload_data(diet_profile_t& profile, corba_profile_t& pb) {
  DagdaImpl* manager = DagdaFactory::getDataManager();
  for (int i=0;i<=pb.last_in;++i)
    pb.parameters[i].desc.dataManager=ORBMgr::getIORString(manager->_this());
  for (int i=profile.last_in+1; i<= profile.last_out; ++i) {
	// marshalling of the data
	corba_data_t data;
	corba_data_t* inserted;
	size_t size = data_sizeof(&profile.parameters[i].desc);
	string origIOR(pb.parameters[i].desc.dataManager);
	
	mrsh_data_desc(&data.desc, &profile.parameters[i].desc);

    if (i<=pb.last_inout && data.desc.specific._d()==DIET_FILE) {
	  // We cannot delete an INOUT file but we have to update the used
	  // disk space.
      size_t previousSize;
	  previousSize =
	    (manager->getData(data.desc.id.idNumber))->desc.specific.file().size;
	  manager->freeDiskSpace(previousSize);
	}
	manager->addData(data);
	inserted = manager->getData(data.desc.id.idNumber);

    if (data.desc.specific._d()!=DIET_FILE) {
	  // The data manager obtains the pointer control. (release=true).
      inserted->value.replace(size, size, (CORBA::Char*)profile.parameters[i].value, true);
      if (data.desc.specific._d() == DIET_SCALAR &&
	      pb.parameters[i].desc.mode == DIET_VOLATILE ||
		  pb.parameters[i].desc.mode == DIET_PERSISTENT_RETURN ||
		  pb.parameters[i].desc.mode == DIET_STICKY_RETURN) {
        pb.parameters[i] = *inserted;
	  }
    }
    pb.parameters[i].desc = inserted->desc;
	
	if (i>pb.last_inout || data.desc.specific._d()==DIET_FILE) {
	  if (data.desc.specific._d()==DIET_FILE) {
	    manager->useDiskSpace(inserted->desc.specific.file().size);
	  }
	  else
	    manager->useMemSpace(inserted->value.length());
	}
	
	manager->unlockData(data.desc.id.idNumber);

    if (manager->pfmIsDataPresent(data.desc.id.idNumber))
	  manager->pfmUpdateData(manager->_this(), data);
  }
}

Dagda_var entryPoint = NULL;

char * get_data_id()
{
#if ! HAVE_ADVANCED_UUID
  static int num_data = 0;
  ostringstream id;
  char* name =
    (char*) Parsers::Results::getParamValue(Parsers::Results::NAME);
	
  if (name!=NULL)
    id << "DAGDA://id." << name << "." << getpid() << "." << num_data++;
  else
    id << "DAGDA://id." << "client." << getpid() << "." << num_data++;
  
  return CORBA::string_dup(id.str().c_str());
#else
  uuid_t uuid;
  char ID[37];
  ostringstream id;
  char* name =
    (char*) Parsers::Results::getParamValue(Parsers::Results::NAME);

  uuid_generate(uuid);
  uuid_unparse(uuid, ID);

  if (name!=NULL)
    id << "DAGDA://id-" << ID << "-" << name;
  else
    id << "DAGDA://id-" << ID << "-client-" << getpid();

  return CORBA::string_dup(id.str().c_str());
#endif
}

Dagda_var getEntryPoint() {
  if (entryPoint!=NULL) return entryPoint;
  
  SimpleDagdaImpl* localManager = (SimpleDagdaImpl*) DagdaFactory::getDataManager();
  Dagda_var manager;

  if (localManager->getType()==DGD_CLIENT_MNGR) {
    char* MA_name =
      (char*) Parsers::Results::getParamValue(Parsers::Results::MANAME);
    MasterAgent_var MA = MasterAgent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT, MA_name));
    if (CORBA::is_nil(MA)) {
      //ERROR("cannot locate Master Agent " << MA_name, 1);
    }
    manager = MA->getDataManager();
    entryPoint = manager;
    return entryPoint;
  } else return NULL;
}

size_t corba_data_init(corba_data_t& data, diet_data_type_t type,
	diet_base_type_t base_type, diet_persistence_mode_t mode,
	size_t nb_r, size_t nb_c, diet_matrix_order_t order, void* value, char* path) {
  diet_data_t diet_data;
  DagdaImpl*  manager = DagdaFactory::getDataManager();
  
  if (mode==DIET_VOLATILE) {
    WARNING("Trying to add a volatile data to DAGDA... The data is " <<
	  "will be persistent.");
	mode=DIET_PERSISTENT;
  }
  char* dataManagerIOR = ORBMgr::getIORString(manager->_this());
  char* dataID = get_data_id();

  diet_data.desc.id = dataID;
  diet_data.desc.mode = mode;
  diet_data.desc.generic.type = type;
  diet_data.desc.generic.base_type = base_type;
  
  switch (type) {
    case DIET_SCALAR:
      diet_data.desc.specific.scal.value = value;
	  break;
    case DIET_VECTOR:
	  diet_data.desc.specific.vect.size = nb_c;
      break;
    case DIET_MATRIX:
      diet_data.desc.specific.mat.nb_r = nb_r;
	  diet_data.desc.specific.mat.nb_c = nb_c;
	  diet_data.desc.specific.mat.order = order;
	  break;
    case DIET_STRING:
    case DIET_PARAMSTRING:
      diet_data.desc.specific.pstr.length = strlen((char*) value);
   	  break;
    case DIET_FILE:
      diet_data.desc.specific.file.path = path;
	  break;
  }
  
  mrsh_data_desc(&data.desc, &diet_data.desc);
  data.desc.dataManager = CORBA::string_dup(dataManagerIOR);
  return data_sizeof(&diet_data.desc);
}

BEGIN_API
int dagda_add_data(void* value, diet_data_type_t type,
	diet_base_type_t base_type, diet_persistence_mode_t mode,
	size_t nb_r, size_t nb_c, diet_matrix_order_t order, char* path, char** ID) {
  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl*  manager = DagdaFactory::getDataManager();
  corba_data_t* inserted;

  corba_data_t data;
  char* dataID;
  size_t size = corba_data_init(data, type, base_type,
	mode, nb_r, nb_c, order, value, path);
  dataID = data.desc.id.idNumber;
  manager->addData(data);
  inserted = manager->getData(dataID);
  if (value!=NULL)
    inserted->value.replace(size, size, (CORBA::Char*) value, false);

  manager->unlockData(dataID);
  
  if (entryPoint!=NULL) {
    entryPoint->pfmAddData(manager->_this(), data);
	// Client side. Don't need to keep the data reference.
	if (type==DIET_FILE) manager->setDataStatus(dataID, Dagda::notOwner);
	manager->remData(dataID);
  }	else {
    manager->pfmAddData(manager->_this(), data);
  }
  if (ID!=NULL)
    *ID=CORBA::string_dup(dataID);
  return 0;
}

int dagda_get_data(char* dataID, void** value, diet_data_type_t type,
	diet_base_type_t* base_type, size_t* nb_r, size_t* nb_c,
	diet_matrix_order_t* order, char** path) {
  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl*  manager = DagdaFactory::getDataManager();
  Dagda_ptr src;
  corba_data_t data;
  corba_data_t* inserted;

  data.desc.id.idNumber = CORBA::string_dup(dataID);
  
  if (entryPoint!=NULL)
    try {
	  data.desc = *entryPoint->pfmGetDataDesc(dataID);
      src = entryPoint->getBestSource(manager->_this(), dataID);
      manager->lclAddData(src, data);
	  inserted = manager->getData(dataID);
    } catch (Dagda::DataNotFound& ex) {
      return 1;
    }
  else
    try {
	  data.desc = *manager->pfmGetDataDesc(dataID);
      src = manager->getBestSource(manager->_this(), dataID);
      manager->lclAddData(src, data);
	  inserted = manager->getData(dataID);
	} catch (Dagda::DataNotFound& ex) {
      return 1;
    }
  if (inserted->desc.specific._d()!=type) {
    return 1;
  }
  if (value!=NULL) *value = inserted->value.get_buffer(false);
  if (base_type!=NULL) *base_type = (diet_base_type_t) inserted->desc.base_type;
  switch (type) {
  case DIET_SCALAR:
	break;
  case DIET_VECTOR:
	if (nb_c!=NULL) *nb_c = inserted->desc.specific.vect().size;
	break;
  case DIET_MATRIX:
	if (nb_r!=NULL) *nb_r = inserted->desc.specific.mat().nb_r;
	if (nb_c!=NULL) *nb_c = inserted->desc.specific.mat().nb_c;
	if (order!=NULL) *order = (diet_matrix_order_t) inserted->desc.specific.mat().order;
	break;
  case DIET_STRING:
  case DIET_PARAMSTRING:
    break;
  case DIET_FILE:
    if (path!=NULL) *path = inserted->desc.specific.file().path;
  }
  return 0;
}

END_API
