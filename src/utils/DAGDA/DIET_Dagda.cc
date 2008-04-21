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
#include <string>

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
	//corba_profile->parameters[i].desc.dataManager=CORBA::string_dup(dataManagerIOR);
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
	
	manager->unlockData(data.desc.id.idNumber);

    if (manager->pfmIsDataPresent(data.desc.id.idNumber))
	  manager->pfmUpdateData(manager->_this(), data);

/*	cout << "l." << __LINE__ << " file: " << __FILE__ << endl;
	cout << "IOR origine = " << endl << origIOR << endl;
	cout << "profile.clientIOR = " << endl << pb.clientIOR << endl;
	cout << "profile.parameters[" << i << "].desc.dataManager = " << endl <<
	  pb.parameters[i].desc.dataManager << endl;
	cout << " clientIOR =? param.dM : " << (strcmp(pb.clientIOR, pb.parameters[i].desc.dataManager) ? "non":"oui") << endl;
	cout << " origIOR =? param.dM : " <<  (strcmp(origIOR.c_str(), pb.parameters[i].desc.dataManager) ? "non":"oui") << endl;
	cout << " clientIOR =? origIOR : " << (strcmp(origIOR.c_str(), pb.clientIOR) ? "non":"oui") << endl;*/
  }
}

void dagda_free_volatile_data(corba_profile_t* pb) {
  for (int i=0; i<=pb->last_out; ++i) {
    Dagda_var remoteManager =
	  Dagda::_narrow(ORBMgr::stringToObject(pb->parameters[i].desc.dataManager));
    if (pb->parameters[i].desc.mode==DIET_VOLATILE) {
      remoteManager->lclRemData(pb->parameters[i].desc.id.idNumber);
    }
  }
}
