/***********************************************************/
/* Dagda specific functions.                               */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@in2p3.fr)                   */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log $
 *
 ************************************************************/
#include "DIET_Dagda.hh"
#include "DagdaFactory.hh"
extern "C" {
#include "DIET_Dagda.h"
} // extern "C"
#include "MasterAgent.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "marshalling.hh"
#include <omniORB4/CORBA.h>
#include "debug.hh"

#include <string>
#include <sstream>
#include <iostream>
#include <map>

#if HAVE_ADVANCED_UUID
#include <uuid/uuid.h>
#endif

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

using namespace std;

void display_profile(corba_profile_t* p) {
  cout << " DISPLAY CORBA PROFILE: " << endl
      << p->last_in << " / "
      << p->last_inout << " / "
      << p->last_out << " / "
      << p->dietReqID << " / "
      << p->clientIOR << endl;
  for (int i=0; i<=p->last_out; ++i) {
    cout << "Parameter " << i << " : "
        << p->parameters[i].desc.id.idNumber << " / "
        << p->parameters[i].desc.mode << " / "
        << p->parameters[i].desc.base_type << " / "
        << p->parameters[i].desc.dataManager << endl;
  }
}

/* Profile marshalling used by DAGDA on the client side. */
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
    if (profile->parameters[i].desc.id != NULL) {
      if (strlen(profile->parameters[i].desc.id)!=0) {
        // The data is stored on the platform. Get its description.
        data.desc = *MA->get_data_arg(profile->parameters[i].desc.id);
        haveID = true;
      }
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
      TRACE_TEXT(TRACE_ALL_STEPS,"Dagda marshall: param " << i << " : new ID "
          << dataID << " for IN/INOUT parameter created" << endl);

      // Add the data in the client data manager.
      // And get the pointer on it.
      storedData = dataManager->addData(data);

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
      TRACE_TEXT(TRACE_ALL_STEPS,"Dagda marshall: param " << i << " : new ID "
          << corba_profile->parameters[i].desc.id.idNumber
          << " for OUT parameter created" << endl);
    }
        // The data is on the platform. Set its description.
    if (haveID) {
      corba_profile->parameters[i].desc=data.desc;
      TRACE_TEXT(TRACE_ALL_STEPS,"Dagda marshall: param " << i << " : using ID "
          << data.desc.id.idNumber << endl);
    }
  } // end for
//   display_profile(corba_profile);
}

/* Data download function used by DAGDA. */
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
  // Downloads the INOUT data from the SeD to the client.
  for (int i=pb->last_in+1; i<=pb->last_inout; ++i) {
    // Get a reference to the data manager of the data.
    Dagda_var remoteManager =
    Dagda::_narrow(ORBMgr::stringToObject(pb->parameters[i].desc.dataManager));

    if (pb->parameters[i].desc.mode != DIET_PERSISTENT &&
        pb->parameters[i].desc.mode != DIET_STICKY) {
      // The data needs to be downloaded.
      if (pb->parameters[i].desc.specific._d() != DIET_SCALAR ) {
        inserted = dataManager->getData(pb->parameters[i].desc.id.idNumber);
        size = inserted->value.length();
        CORBA::Char* value = inserted->value.get_buffer();
        char*  path = inserted->desc.specific.file().path;
        size_t fileSize = inserted->desc.specific.file().size;
        // Data downloading.
        dataManager->lclAddData(remoteManager, pb->parameters[i]);

        inserted = dataManager->getData(pb->parameters[i].desc.id.idNumber);
        // The files are transmitted separately.
        if (pb->parameters[i].desc.specific._d() != DIET_FILE) {
          // Optimisation necessaire. Voir cote serveur...
          for (size_t j=0; j<size; ++j)
            value[j]=inserted->value[j];
          inserted->value.replace(size, size, value, 0);
        }
        else {
          // INOUT file: Remove the previous one.
          unlink(inserted->desc.specific.file().path);
          rename(path, inserted->desc.specific.file().path);
          inserted->desc.specific.file().path = path;
          inserted->desc.specific.file().size = fileSize;
        }
      } else {
	// Scalar data: The value is transmitted inside the profile.
        inserted = dataManager->getData(pb->parameters[i].desc.id.idNumber);
        size = inserted->value.length();
        for (size_t j=0; j<size; ++j)
          inserted->value[j]=pb->parameters[i].value[j];
      }
    }
    // Remove the remote volatile data
    if (pb->parameters[i].desc.mode==DIET_VOLATILE)
      remoteManager->lclRemData(pb->parameters[i].desc.id.idNumber);
  }
  // Download the OUT data.
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
        inserted = dataManager->addData(data);
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
        inserted = dataManager->addData(data);

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
      if ((pb.parameters[i].desc.specific._d() == DIET_CONTAINER)
         && (strcmp(dataManager->getID(), bestSource->getID()) != 0)) {
        // original container should be removed to avoid incoherencies
        bestSource->lclRemData(pb.parameters[i].desc.id.idNumber);
      }
      inserted = dataManager->getData(pb.parameters[i].desc.id.idNumber);
      size = inserted->value.length();
      unmrsh_data_desc(&profile.parameters[i].desc, &inserted->desc);
      // See (1) below.
      profile.parameters[i].value = inserted->value.get_buffer(i>pb.last_in ? true:false);
    }
  }
  for (int i=pb.last_inout+1;i<=pb.last_out; ++i) {
    size_t outDataSize = data_sizeof(&pb.parameters[i].desc);
    unmrsh_data_desc(&profile.parameters[i].desc, &pb.parameters[i].desc);
    profile.parameters[i].value = new CORBA::Char[outDataSize];
  }
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

    if (data.desc.specific._d()!=DIET_CONTAINER)
      inserted = manager->addData(data);
    else // container is not modified in the diet profile but in dagda
      inserted = manager->getData(data.desc.id.idNumber);

    if (data.desc.specific._d()!=DIET_FILE && data.desc.specific._d()!=DIET_CONTAINER) {
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

    if (manager->pfmIsDataPresent(data.desc.id.idNumber) && (data.desc.specific._d() != DIET_CONTAINER))
      manager->pfmUpdateData(manager->_this(), data);
  }
}

// The "entry point" to the DAGDA hierarchy.
Dagda_var entryPoint = NULL;
// A reference to the MA.
MasterAgent_var masterAgent = NULL;

// Returns an new ID
// DAGDA should use the uuid library to avoid id's conflicts.
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

MasterAgent_var getMasterAgent() {
  if (masterAgent!=NULL) return masterAgent;
  char* MA_name =
    (char*) Parsers::Results::getParamValue(Parsers::Results::MANAME);
  if (MA_name==NULL) return NULL;

  MasterAgent_var MA = MasterAgent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT, MA_name));
  if (CORBA::is_nil(MA)) {
    //ERROR("cannot locate Master Agent " << MA_name, 1);
    return NULL;
  }
  masterAgent = MA;
  return masterAgent;
}

// The reference to the MA DAGDA component is obtained from the ORB
// only one time.
Dagda_var getEntryPoint() {
  if (entryPoint!=NULL) return entryPoint;

  SimpleDagdaImpl* localManager = (SimpleDagdaImpl*) DagdaFactory::getDataManager();
  Dagda_var manager;

  if (localManager->getType()==DGD_CLIENT_MNGR) {
    char* MA_name =
    (char*) Parsers::Results::getParamValue(Parsers::Results::MANAME);
    if (MA_name==NULL) return NULL;
    MasterAgent_var MA = MasterAgent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT, MA_name));
    if (CORBA::is_nil(MA)) {
      //ERROR("cannot locate Master Agent " << MA_name, 1);
      return NULL;
    }
    manager = MA->getDataManager();
    entryPoint = manager;
    return entryPoint;
  } else return NULL;
}

// corba_data_t initialization from explicit parameters.
size_t corba_data_init(corba_data_t& data, diet_data_type_t type,
                       diet_base_type_t base_type, diet_persistence_mode_t mode,
                       size_t nb_r, size_t nb_c, diet_matrix_order_t order, void* value, char* path) {
  diet_data_t diet_data;
  DagdaImpl*  manager = DagdaFactory::getDataManager();

  if (mode==DIET_VOLATILE) {
    WARNING("Trying to add a volatile data to DAGDA... The data " <<
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
    case DIET_CONTAINER:
      diet_data.desc.specific.cont.size = nb_c;
      break;
    default:
      WARNING("This type is not managed by DIET.");
  }

  mrsh_data_desc(&data.desc, &diet_data.desc);
  data.desc.dataManager = CORBA::string_dup(dataManagerIOR);
  return data_sizeof(&diet_data.desc);
}

// This structure is used for the omni_thread outputs.
typedef struct {
  int returnedValue;
  // Put results.
  char* ID;
  // Get results.
  void* value;
  diet_base_type_t base_type;
  size_t nb_r, nb_c;
  diet_matrix_order_t order;
  char* path;
} DagdaThreadResult;

// DAGDA thread to put a data asynchronously in the DAGDA hierarchy.
class DagdaPutDataThread : public omni_thread {
private:
  void* value;
  diet_data_type_t type;
  diet_base_type_t base_type;
  diet_persistence_mode_t mode;
  size_t nb_r, nb_c;
  diet_matrix_order_t order;
  char* path;
public:
    DagdaPutDataThread(void* value, diet_data_type_t type,
                       diet_base_type_t base_type, diet_persistence_mode_t mode,
                       size_t nb_r, size_t nb_c, diet_matrix_order_t order, char* path) :
    value(value), type(type), base_type(base_type), mode(mode), nb_r(nb_r),
    nb_c(nb_c), order(order), path(path) {

    }

  void startThread() {
    start_undetached();
  }
protected:
    void* run_undetached(void* arg) {
      DagdaThreadResult* result = new DagdaThreadResult;

      result->returnedValue =
        dagda_put_data(value, type, base_type, mode, nb_r, nb_c, order, path, &result->ID);
      return result;
    }

  void run(void* arg) {
    dagda_put_data(value, type, base_type, mode, nb_r, nb_c, order, path, NULL);
  }
};

// DAGDA thread to get a data asynchronously from the DAGDA hierarchy.
class DagdaGetDataThread : public omni_thread {
private:
  char* dataID;
  diet_data_type_t type;
public:
    DagdaGetDataThread(char* dataID, diet_data_type_t type) : dataID(dataID),
    type(type) {
    }

  void startThread() {
    start_undetached();
  }
protected:
    void* run_undetached(void* arg) {
      DagdaThreadResult* result = new DagdaThreadResult;

      result->returnedValue =
        dagda_get_data(dataID, &result->value, type, &result->base_type,
                       &result->nb_r, &result->nb_c, &result->order, &result->path);

      return result;
    }

  void run(void* arg) {
    dagda_get_data(dataID, NULL, type, NULL, NULL, NULL, NULL, NULL);
  }
};

// A thread pool to manage the asynchronous DAGDA operations.
class DagdaThreadPool {
private:
  static DagdaThreadPool* instance;
  static unsigned int nextID;
  omni_mutex idMutex;
  omni_mutex poolMutex;

  map<unsigned int, omni_thread*> pool;

  DagdaThreadPool() {

  }

public:
    static DagdaThreadPool* getInstance() {
      if (instance==NULL)
        instance = new DagdaThreadPool();
      return instance;
    }
  // Creates a thread and records a reference on it.
  // Put a data.
  unsigned int newPutDataThread(void* value, diet_data_type_t type,
                                diet_base_type_t base_type, diet_persistence_mode_t mode,
                                size_t nb_r, size_t nb_c, diet_matrix_order_t order, char* path) {
    unsigned int id = getNextID();

    poolMutex.lock();
    pool[id] = new DagdaPutDataThread(value, type, base_type, mode, nb_r, nb_c,
                                      order, path);
    ((DagdaPutDataThread*) pool[id])->startThread();
    poolMutex.unlock();

    return id;
  }
  // Get a data.
  unsigned int newGetDataThread(char* dataID, diet_data_type_t type) {
    unsigned int id = getNextID();

    poolMutex.lock();
    pool[id] = new DagdaGetDataThread(dataID, type);
    ((DagdaGetDataThread*) pool[id])->startThread();
    poolMutex.unlock();

    return id;
  }
  // Waits the end of the transfert from the client.
  int waitDataPut(unsigned int id, char** ID) {
    int ret;
    DagdaThreadResult* result;
    omni_thread* thread;

    poolMutex.lock();
    if (pool.find(id)==pool.end()) {
      poolMutex.unlock();
      return -1;
    }

    thread = pool[id];
    poolMutex.unlock();

    thread->join((void**) &result);

    ret = result->returnedValue;
    *ID = result->ID;
    delete result;

    poolMutex.lock();
    pool.erase(id);
    poolMutex.unlock();

    return ret;
  }
  // Waits the end of the transfert to the client.
  int waitDataGet(unsigned int id, void** value, diet_base_type_t* base_type,
                  size_t* nb_r, size_t* nb_c, diet_matrix_order_t* order, char** path) {
    int ret;
    DagdaThreadResult* result;
    omni_thread* thread;

    poolMutex.lock();
    if (pool.find(id)==pool.end()) {
      poolMutex.unlock();
      return -1;
    }

    thread = pool[id];
    poolMutex.unlock();

    thread->join((void**) &result);
    ret = result->returnedValue;
    if (value!=NULL)
      *value = result->value;
    if (base_type!=NULL)
      *base_type = result->base_type;
    if (nb_r!=NULL)
      *nb_r = result->nb_r;
    if (nb_c!=NULL)
      *nb_c = result->nb_c;
    if (order!=NULL)
      *order = result->order;
    if (path!=NULL)
      *path = result->path;
    delete result;
    poolMutex.lock();
    pool.erase(id);
    poolMutex.unlock();
    return ret;
  }
  // Returns a new thread id.
  unsigned int getNextID() {
    unsigned int ret;
    idMutex.lock();
    ret = nextID++;
    idMutex.unlock();
    return ret;
  }
};
// The thread pool is a singleton class.
DagdaThreadPool* DagdaThreadPool::instance = NULL;
unsigned int DagdaThreadPool::nextID = 0;

// Evaluation of the replication rule.
int eval(const char* str, long* type, char** rule, bool* replace) {
  char* strCpy = strdup(str);
  char* S = strCpy;
  char* t, *r, *repl;

  t = strsep(&strCpy, ":");
  if (strCpy==NULL) {
    free(S);
    return 1;
  }
  if (strcasecmp(t, "id")==0)
    *type = 1;
  else {
    if (strcasecmp(t, "host")==0)
      *type = 0;
    else {
      free(S);
      return 1;
    }
  }
  r = strsep(&strCpy, ":");
  if (strCpy==NULL) {
    free(S);
    return 1;
  }
  repl = strsep(&strCpy, ":");
  if (strCpy!=NULL) {
    free(S);
    return 1;
  }
  if (strcasecmp(repl, "replace")==0)
    *replace=true;
  else {
    if (strcasecmp(repl, "noreplace")==0)
      *replace=false;
    else {
      free(S);
      return 1;
    }
  }
  *rule = strdup(r);
  free(S);
  return 0;
}

BEGIN_API
int dagda_put_data(void* value, diet_data_type_t type,
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
  inserted = manager->addData(data);

  if (value!=NULL) {
    inserted->value.replace(size, size, (CORBA::Char*) value, false);
    manager->useMemSpace(inserted->value.length()); // FIX for mem usage (bi)
  }

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

int dagda_get_data(const char* dataID, void** value, diet_data_type_t type,
                   diet_base_type_t* base_type, size_t* nb_r, size_t* nb_c,
                   diet_matrix_order_t* order, char** path) {
  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl*  manager = DagdaFactory::getDataManager();
  Dagda_ptr src;
  corba_data_t data;
  corba_data_t* inserted;

  data.desc.id.idNumber = CORBA::string_dup(dataID);

  if (!manager->lclIsDataPresent(dataID)) { // added because container elts are pre-downloaded
    if (entryPoint!=NULL) {
      try {
      data.desc = *entryPoint->pfmGetDataDesc(dataID);
      src = entryPoint->getBestSource(manager->_this(), dataID);
      manager->lclAddData(src, data);
      inserted = manager->getData(dataID);
      } catch (Dagda::DataNotFound& ex) {
        return 1;
      }
    } else {
      try {
      data.desc = *manager->pfmGetDataDesc(dataID);
      src = manager->getBestSource(manager->_this(), dataID);
      manager->lclAddData(src, data);
      inserted = manager->getData(dataID);
      } catch (Dagda::DataNotFound& ex) {
        return 1;
      }
    }
  } else {
    inserted = manager->getData(dataID);
  }

  if (inserted->desc.specific._d()!=type && type!=DIET_UNKNOWN_TYPE) {
    return 1;
  }
  if (value!=NULL) *value = inserted->value.get_buffer(false);
  if (base_type!=NULL) *base_type = (diet_base_type_t) inserted->desc.base_type;
  switch (inserted->desc.specific._d()) {
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
      break;
    case DIET_CONTAINER:
      break;
    default:
      WARNING("This data type is not managed by DIET.");
  }
  return 0;
}

int dagda_delete_data(char* dataID) {
  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl*  manager = DagdaFactory::getDataManager();
  if (entryPoint != NULL) {
    entryPoint->pfmRemData(dataID);
    manager->lclRemData(dataID);
  } else
    manager->pfmRemData(dataID);
  return 0;
}

unsigned int dagda_put_data_async(void* value, diet_data_type_t type,
                                  diet_base_type_t base_type, diet_persistence_mode_t mode,
                                  size_t nb_r, size_t nb_c, diet_matrix_order_t order, char* path) {
  DagdaThreadPool* instance = DagdaThreadPool::getInstance();

  return instance->newPutDataThread(value, type, base_type, mode, nb_r, nb_c, order, path);
}

unsigned int dagda_get_data_async(char* ID, diet_data_type_t type) {
  DagdaThreadPool* instance = DagdaThreadPool::getInstance();

  return instance->newGetDataThread(ID, type);
}

int dagda_wait_put(unsigned int threadID, char** ID) {
  DagdaThreadPool* instance = DagdaThreadPool::getInstance();

  return instance->waitDataPut(threadID, ID);
}

int dagda_wait_get(unsigned int threadID, void** value, diet_base_type_t* base_type,
                   size_t* nb_r, size_t* nb_c, diet_matrix_order_t* order, char** path) {
  DagdaThreadPool* instance = DagdaThreadPool::getInstance();

  return instance->waitDataGet(threadID, value, base_type, nb_r, nb_c, order, path);
}

int dagda_add_data(void* value, diet_data_type_t type,
                   diet_base_type_t base_type, diet_persistence_mode_t mode,
                   size_t nb_r, size_t nb_c, diet_matrix_order_t order, char* path) {
  DagdaPutDataThread* thrd =
  new DagdaPutDataThread(value, type, base_type, mode, nb_r, nb_c, order, path);
  thrd->start();
  return 0;
}

int dagda_load_data(char* ID, diet_data_type_t type) {
  DagdaGetDataThread* thrd =
  new DagdaGetDataThread(ID, type);
  thrd->start();
  return 0;
}

int dagda_save_platform() {
  if (entryPoint!=NULL) {
    entryPoint->checkpointState();
    return 0;
  }
  return 1;
}

int dagda_data_alias(const char* id, const char* alias) {
  MasterAgent_var MA = getMasterAgent();

  if (MA==NULL) {
    WARNING("Try to call " << __FUNCTION__ << " outside a client.");
    return 1;
  }
  SeqString values;
  values.length(1);
  values[0] = id;

  return MA->insertData(alias, values);
}

int dagda_id_from_alias(const char* alias, char** id) {
  MasterAgent_var MA = getMasterAgent();

  if (MA==NULL) {
    WARNING("Try to call " << __FUNCTION__ << " outside a client.");
    return 1;
  }
  SeqString* attributes = MA->searchData(alias);
  if (attributes->length()==0) return 1;
  *id = (*attributes)[0];
  return 0;
}

int dagda_replicate_data(const char* id, const char* rule) {
  long target;
  char* pattern;
  bool replace;

  if (eval(rule, &target, &pattern, &replace)!=0) {
    WARNING("Error in replication rule definition.");
    return 1;
  }

  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl*  manager = DagdaFactory::getDataManager();

  if (entryPoint!=NULL) { // We are on a client.
    entryPoint->pfmReplicate(id, target, pattern, replace);
  } else
    manager->pfmReplicate(id, target, pattern, replace);
  return 0;
}


/** API for container management **/

// used to create a new container (ID to be defined) - used on Client or SeD
int dagda_create_container(char** ID) {
  return dagda_put_data(NULL, DIET_CONTAINER, DIET_CHAR, DIET_PERSISTENT,
                        0, 0, (diet_matrix_order_t) 0, NULL, ID);
}

// used to initialize a container (ID already defined) - used on Client or SeD
// will NOT download all the elements of the container
// on SERVER IN ==> it is called after the container was downloaded so after adding elements
// on SERVER OUT ==> called before adding elements
int dagda_init_container(diet_data_t *profile_data) {
  DagdaImpl*  manager    = DagdaFactory::getDataManager();
  // dagda_put_data cannot be used as it would create a new ID
  // check first if container is already present locally (eg INOUT param)
  if (manager->lclIsDataPresent(profile_data->desc.id)) {
    return 0;
  }
  // if not present => create a new entry
  corba_data_t data;
  mrsh_data_desc(&data.desc, &profile_data->desc);
  // register the container locally
  manager->addData(data);
  manager->unlockData(data.desc.id.idNumber);
}

int dagda_add_container_element(const char* idContainer, const char* idElement, int index) {
  Dagda_var   entryPoint = getEntryPoint();
  DagdaImpl*  manager    = DagdaFactory::getDataManager();
  if (entryPoint != NULL) {
    // on the client => find the data mgr that hosts the container
    if (entryPoint->pfmIsDataPresent(idContainer)) {
      corba_data_desc_t* storedDataDesc = entryPoint->pfmGetDataDesc(idContainer);
      Dagda_var srcMgr = Dagda::_narrow(ORBMgr::stringToObject(storedDataDesc->dataManager));
      // add the relationship
      srcMgr->lclAddContainerElt(idContainer, idElement, index, 0, true);
    } else {
      WARNING("Cannot find container " << idContainer << " on platform");
      throw Dagda::DataNotFound(idContainer);
    }
  } else {
    // not on the client => dagda_init_container must have been called before
    // so the container exists locally
    if (manager->lclIsDataPresent(idContainer)) {
      manager->lclAddContainerElt(idContainer, idElement, index, 0, true);
    } else {
      WARNING("Cannot find container " << idContainer << " locally");
      throw Dagda::DataNotFound(idContainer);
    }
  }
  return 0;
}

int dagda_get_container_elements(const char* idContainer, diet_container_t* content) {
  DagdaImpl*  manager = DagdaFactory::getDataManager();
  // get the sequence of IDs
  SeqString  eltIDSeq;
  SeqLong    eltFlagSeq;
  manager->lclGetContainerElts(idContainer,eltIDSeq,eltFlagSeq,true);
  // convert it to diet_container_t
  if (content != NULL) {
    content->size = eltIDSeq.length();
    content->id   = CORBA::string_dup(idContainer);
    content->elt_ids = (char**) malloc(sizeof(char*) * content->size); // FIXME not deallocated
    for (int ix=0; ix<eltIDSeq.length(); ++ix) {
      content->elt_ids[ix] = CORBA::string_dup(eltIDSeq[ix]);
    }
  } else return 1;
  return 0;
}


END_API
