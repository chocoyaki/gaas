/**
 * @file DIET_Dagda.cc
 *
 * @brief Dagda specific functions
 *
 * @author Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "DIET_Dagda.hh"
#include "DIET_uuid.hh"
#include "DagdaFactory.hh"

#if DAGDA_PROGRESSION
#include "Transfers.hh"
#include <stdexcept>
#endif

extern "C" {
#include "DIET_Dagda.h"
} // extern "C"

#include "MasterAgent.hh"
#include "ORBMgr.hh"
#include "marshalling.hh"
#include <omniORB4/CORBA.h>
#include "debug.hh"
#include "configuration.hh"
#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif

#define BEGIN_API extern "C" {
#define END_API }    // extern "C"

void
display_profile(corba_profile_t *p) {
  std::cout << " DISPLAY CORBA PROFILE: \n"
            << p->last_in << " / "
            << p->last_inout << " / "
            << p->last_out << " / "
            << p->dietReqID << " / "
            << p->clientName << "\n";
  for (int i = 0; i <= p->last_out; ++i) {
    std::cout << "Parameter " << i << " : "
              << p->parameters[i].desc.id.idNumber << " / "
              << p->parameters[i].desc.mode << " / "
              << p->parameters[i].desc.base_type << " / "
              << p->parameters[i].desc.dataManager << "\n";
  }
} // display_profile

/* Profile marshalling used by DAGDA on the client side. */
extern "C" DIET_API_LIB void
dagda_mrsh_profile(corba_profile_t *corba_profile, diet_profile_t *profile,
                   MasterAgent_var &MA) {
  DagdaImpl *dataManager = DagdaFactory::getDataManager();
  std::string dataManagerName = dataManager->getID();

  corba_profile->parameters.length(profile->last_out + 1);

#if defined HAVE_ALT_BATCH
  corba_profile->parallel_flag = profile->parallel_flag;
  corba_profile->nbprocs = profile->nbprocs;
  corba_profile->nbprocess = profile->nbprocess;
  corba_profile->walltime = profile->walltime;
#endif
  corba_profile->last_in = profile->last_in;
  corba_profile->last_inout = profile->last_inout;
  corba_profile->last_out = profile->last_out;
  corba_profile->dietReqID = profile->dietReqID;

  corba_profile->clientName = CORBA::string_dup(dataManagerName.c_str());

  for (int i = 0; i <= profile->last_out; ++i) {
    bool haveID = false;
    corba_data_t data;
    // Does the data need an ID ? Yes: Get one from the MA.
    // Else get the data description from the platform.
    if (profile->parameters[i].desc.id != NULL) {
      if (strlen(profile->parameters[i].desc.id) != 0) {
        // The data is stored on the platform. Get its description.
        data.desc = *MA->get_data_arg(profile->parameters[i].desc.id);
        haveID = true;
      }
    }

    // This is a new data. It needs an ID and its data manager is
    // this client data manager.
    if (i <= profile->last_inout && !haveID) {
      corba_data_t *storedData;
      size_t size;
      char *dataID = MA->get_data_id();

      // Set the ID and the data manager
      profile->parameters[i].desc.id = dataID;
      mrsh_data_desc(&data.desc, &profile->parameters[i].desc);
      data.desc.dataManager = CORBA::string_dup(dataManagerName.c_str());
      TRACE_TEXT(
        TRACE_ALL_STEPS, "Dagda marshall: param " << i << " : new ID "
                                                  << dataID <<
        " for IN/INOUT parameter created\n");

      // Add the data in the client data manager.
      // And get the pointer on it.
      storedData = dataManager->addData(data);

      if (profile->parameters[i].desc.generic.type != DIET_FILE) {
        size = data_sizeof(&profile->parameters[i].desc);
      } else {
        size = 0;
      }
      // Data in the profile and in the data manager share the same
      // pointer. The last parameter  of "replace" is the "release"
      // parameter. It is set to 0 to avoid  double free.
      if (profile->parameters[i].value != NULL) {
        storedData->value.replace(size, size,
                                  (CORBA::Char *) profile->parameters[i].value,
                                  i <= profile->last_in ? 0 : 1);
      }
      // Only scalar values are sent into the profile.
      // For other types, the peer has to download them.
      if (profile->parameters[i].desc.generic.type == DIET_SCALAR) {
        corba_profile->parameters[i] = *storedData;
      } else {
        corba_profile->parameters[i].desc = storedData->desc;
      }

      dataManager->unlockData(dataID);
      continue;
    }
    // Out data needing an ID.
    if (i > profile->last_inout && !haveID) {
      char *dataID = MA->get_data_id();

      profile->parameters[i].desc.id = dataID;
      mrsh_data_desc(&corba_profile->parameters[i].desc,
                     &profile->parameters[i].desc);
      TRACE_TEXT(
        TRACE_ALL_STEPS, "Dagda marshall: param " << i << " : new ID "
                                                  << corba_profile->
        parameters[i].desc.id.idNumber
                                                  <<
        " for OUT parameter created\n");
    }
    // The data is on the platform. Set its description.
    if (haveID) {
      corba_profile->parameters[i].desc = data.desc;
      TRACE_TEXT(
        TRACE_ALL_STEPS, "Dagda marshall: param " << i
                                                  << " : using ID " <<
        data.desc.id.idNumber << "\n");
    }
  } // end for
    // display_profile(corba_profile);
} // dagda_mrsh_profile

/* Data download function used by DAGDA. */
extern "C" DIET_API_LIB void
dagda_download_SeD_data(diet_profile_t *profile,
                        corba_profile_t *pb) {
  DagdaImpl *dataManager = DagdaFactory::getDataManager();

  corba_data_t data;
  corba_data_t *inserted;
  size_t size;
  CORBA::Char *ptr;
  char buffer[1024];
  bool mvFileError;

  // Free the remote volatile data.
  for (int i = 0; i <= pb->last_in; ++i) {
    Dagda_var remoteManager =
      ORBMgr::getMgr()->resolve<Dagda,
                                Dagda_var>(DAGDACTXT,
                                           std::string(
                                             pb->parameters[i].desc.dataManager));
    if (pb->parameters[i].desc.mode == DIET_VOLATILE) {
      remoteManager->lclRemData(pb->parameters[i].desc.id.idNumber);
    }
  }
  // Downloads the INOUT data from the SeD to the client.
  for (int i = pb->last_in + 1; i <= pb->last_inout; ++i) {
    // Get a reference to the data manager of the data.
    std::string remoteManagerName(pb->parameters[i].desc.dataManager);
    Dagda_var remoteManager =
      ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, remoteManagerName);

    // The data needs to be downloaded.
    inserted = dataManager->getData(pb->parameters[i].desc.id.idNumber);
    std::string path(inserted->desc.specific.file().path);

    dataManager->lclAddData(remoteManagerName.c_str(), pb->parameters[i]);

    // The files are transmitted separately.
    if (pb->parameters[i].desc.specific._d() == DIET_FILE) {
      mvFileError = false;
      size_t fileSize = inserted->desc.specific.file().size;
      if (unlink(path.c_str()) == -1) {
        WARNING("Cannot remove " << path.c_str() << " for replacing");
        perror("INOUT file error: ");
      }

      if (rename(inserted->desc.specific.file().path,
                 path.c_str()) == -1) {
        std::ifstream input(inserted->desc.specific.file().path);
        std::ofstream output(path.c_str());

        if (!input.is_open() || !output.is_open()) {
          WARNING("Error: cannot update file " << path.c_str());
          perror("INOUT file error: ");
          continue;
        }
        while (!input.eof() && !input.bad()) {
          input.read(buffer, 1024);
          output.write(buffer, input.gcount());
          if (output.fail() || output.bad()) {
            WARNING("Error: cannot write data into " << path.c_str());
            mvFileError = true;
            break;
          }
        }
        if (mvFileError) {
          WARNING("Error moving file " << inserted->desc.specific.file().path);
          WARNING("This file wont be removed");
          continue;
        }
        if (unlink(inserted->desc.specific.file().path) == -1) {
          WARNING("Error removing " << inserted->desc.specific.file().path);
          perror("");
        } else {
          inserted->desc.specific.file().path = CORBA::string_dup(path.c_str());
        }
      } else {
        inserted->desc.specific.file().path = CORBA::string_dup(path.c_str());
      }
      inserted->desc.specific.file().size = fileSize;
    }
    // Give back the pointer to the client.
    size = inserted->value.length();
    ptr = inserted->value.get_buffer(true);
    // Update pointer in DAGDA
    inserted->value.replace(size, size, ptr, 0);
    unmrsh_data_desc(&profile->parameters[i].desc, &inserted->desc);

    // Remove the remote volatile data
    if (pb->parameters[i].desc.mode == DIET_VOLATILE) {
      remoteManager->lclRemData(pb->parameters[i].desc.id.idNumber);
    }
  }
  // Download the OUT data.
  for (int i = pb->last_inout + 1; i <= pb->last_out; ++i) {
    std::string remoteManagerName(pb->parameters[i].desc.dataManager);
    Dagda_var remoteManager =
      ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, remoteManagerName);

    if (pb->parameters[i].desc.mode != DIET_PERSISTENT &&
        pb->parameters[i].desc.mode != DIET_STICKY) {
      if (pb->parameters[i].desc.specific._d() != DIET_SCALAR) {
        dataManager->lclAddData(remoteManagerName.c_str(), pb->parameters[i]);
        inserted = dataManager->getData(pb->parameters[i].desc.id.idNumber);
        size = inserted->value.length();
        unmrsh_data_desc(&profile->parameters[i].desc, &inserted->desc);
        ptr = inserted->value.get_buffer(true);
        profile->parameters[i].value = ptr;
        inserted->value.replace(size, size, ptr, 0);
      } else {
        data.desc = pb->parameters[i].desc;
        size = pb->parameters[i].value.length();
        inserted = dataManager->addData(data);
        ptr = pb->parameters[i].value.get_buffer(true);
        inserted->value.replace(size, size, ptr, 0);
        unmrsh_data_desc(&profile->parameters[i].desc, &inserted->desc);
        profile->parameters[i].value = ptr;
        dataManager->unlockData(pb->parameters[i].desc.id.idNumber);
      }
    }
    if (pb->parameters[i].desc.mode == DIET_VOLATILE) {
      remoteManager->lclRemData(pb->parameters[i].desc.id.idNumber);
    }
  }
} // dagda_download_SeD_data


extern "C" DIET_API_LIB diet_error_t
dagda_get_data_desc(corba_pb_desc_t &corba_pb, MasterAgent_var &MA) {
  // Retrieves the information about a data stored on the platform.
  for (int i = 0; i <= corba_pb.last_out; ++i) {
    if (strlen(corba_pb.param_desc[i].id.idNumber) != 0) {
      if (!MA->dataLookUp(corba_pb.param_desc[i].id.idNumber)) {
        ERROR_DEBUG(" data with ID " << corba_pb.param_desc[i].id.idNumber
                               << " not inside the platform.", 1);
      } else {
        const_cast<corba_data_desc_t &>(corba_pb.param_desc[i]) =
          *MA->get_data_arg(corba_pb.param_desc[i].id.idNumber);
      }
    }
  }
  return 0;
} // dagda_get_data_desc

/* !!! Important note to programers: !!!
   - The CORBA::sequence::get_buffer(orphan == true) makes the next
   call to get_buffer returns NULL and gives the pointer control
   to the user;
   - The CORBA::sequence::replace(size, maxSize, ptr, release == false)
   gives the pointer control to the user;
   These two parameters mean the opposite. Be careful to have it in mind.
 */
extern "C" DIET_API_LIB void
dagda_download_data(diet_profile_t &profile, corba_profile_t &pb) {
  DagdaImpl *dataManager = DagdaFactory::getDataManager();
  corba_data_t data;
  corba_data_t *inserted;
  size_t size;
  profile.parameters = new diet_data_t[pb.last_out + 1];
  profile.last_in = pb.last_in;
  profile.last_inout = pb.last_inout;
  profile.last_out = pb.last_out;
  profile.dietReqID = pb.dietReqID;
#if defined HAVE_ALT_BATCH
  profile.parallel_flag = pb.parallel_flag;
  profile.nbprocs = pb.nbprocs;
  profile.nbprocess = pb.nbprocess;
  profile.walltime = pb.walltime;
#endif
  for (int i = 0; i <= pb.last_inout; ++i) {
    if (!dataManager->pfmIsDataPresent(pb.parameters[i].desc.id.idNumber)) {
      std::string remoteManagerName(pb.parameters[i].desc.dataManager);

      // The data is not yet registered here.
      if (pb.parameters[i].desc.specific._d() != DIET_SCALAR) {
        // Data is transmitted from the remote manager.
        dataManager->lclAddData(remoteManagerName.c_str(), pb.parameters[i]);
        inserted = dataManager->getData(pb.parameters[i].desc.id.idNumber);
        size = inserted->value.length();
        unmrsh_data_desc(&profile.parameters[i].desc, &inserted->desc);
        // (1):
        // IN value: The data manager still have control on the value pointer.
        // INOUT value: The data manager gives the pointer control to the SeD.
        // get_buffer(boolean orphan) :
        // orphan == true => inserted->value = NULL && param.value = ptr
        // The SeD can modify *ptr.
        // orphan == false => inserted->value = param.value = ptr
        // The SeD must NOT to modify *ptr !
        profile.parameters[i].value = inserted->value.get_buffer(
          i > pb.last_in ? true : false);
      } else {
        // Scalar values are transmitted in the profile.
        data.desc = pb.parameters[i].desc;
        size = pb.parameters[i].value.length();
        inserted = dataManager->addData(data);
        // If the data is INOUT, we leave its management to the SeD developper.
        // Otherwise, the data should not be modified.
        // To modify it is a major bug.
        // The corba profile leaves the pointer control to the data manager.
        inserted->value.replace(size, size,
                                pb.parameters[i].value.get_buffer(true), true);
        unmrsh_data_desc(&profile.parameters[i].desc, &inserted->desc);
        // See (1) below.
        profile.parameters[i].value = inserted->value.get_buffer(
          i > pb.last_in ? true : false);
        dataManager->unlockData(pb.parameters[i].desc.id.idNumber);
        dataManager->useMemSpace(size);
      }
    } else {
      // Data is present. We use it.

      char *bestSource = dataManager->getBestSource(
        dataManager->getID(),
        pb.parameters[i].desc.id.
        idNumber);
      if (!dataManager->lclIsDataPresent(pb.parameters[i].desc.id.idNumber)) {
        TRACE_TEXT(
          TRACE_MAIN_STEPS, "Uses the remote data " <<
          pb.parameters[i].desc.id.idNumber << "\n");
        corba_data_t data;
        Dagda_var source = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(
          DAGDACTXT, bestSource);
        data.desc = *source->lclGetDataDesc(pb.parameters[i].desc.id.idNumber);

        dataManager->lclAddData(bestSource, data);
      }
      if ((pb.parameters[i].desc.specific._d() == DIET_CONTAINER)
          && (strcmp(dataManager->getID(), bestSource) != 0)) {
        // original container should be removed to avoid incoherencies
        Dagda_var src = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT,
                                                                    bestSource);
        src->lclRemData(pb.parameters[i].desc.id.idNumber);
      }
      inserted = dataManager->getData(pb.parameters[i].desc.id.idNumber);
      size = inserted->value.length();
      unmrsh_data_desc(&profile.parameters[i].desc, &inserted->desc);
      // See (1) below.
      profile.parameters[i].value = inserted->value.get_buffer(
        i > pb.last_in ? true : false);
    }
  }
  for (int i = pb.last_inout + 1; i <= pb.last_out; ++i) {
    size_t outDataSize = data_sizeof(&pb.parameters[i].desc);
    unmrsh_data_desc(&profile.parameters[i].desc, &pb.parameters[i].desc);
    profile.parameters[i].value = new CORBA::Char[outDataSize];
  }
} // dagda_download_data

extern "C" DIET_API_LIB void
dagda_upload_data(diet_profile_t &profile, corba_profile_t &pb) {
  DagdaImpl *manager = DagdaFactory::getDataManager();
  for (int i = 0; i <= pb.last_in; ++i)
    pb.parameters[i].desc.dataManager = CORBA::string_dup(manager->getID());
  for (int i = profile.last_in + 1; i <= profile.last_out; ++i) {
    // marshalling of the data
    corba_data_t data;
    corba_data_t *inserted;
    size_t size = data_sizeof(&profile.parameters[i].desc);
    std::string origName(pb.parameters[i].desc.dataManager);

    mrsh_data_desc(&data.desc, &profile.parameters[i].desc);

    if (i <= pb.last_inout && data.desc.specific._d() == DIET_FILE) {
      // We cannot delete an INOUT file but we have to update the used
      // disk space.
      size_t previousSize;
      previousSize =
        (manager->getData(data.desc.id.idNumber))->desc.specific.file().size;
      manager->freeDiskSpace(previousSize);
    }

    if (data.desc.specific._d() != DIET_CONTAINER) {
      inserted = manager->addData(data);
    } else { // container is not modified in the diet profile but in dagda
      inserted = manager->getData(data.desc.id.idNumber);
    }

    if (data.desc.specific._d() != DIET_FILE && data.desc.specific._d() !=
        DIET_CONTAINER) {
      // The data manager obtains the pointer control. (release = true).
      inserted->value.replace(size, size,
                              (CORBA::Char *) profile.parameters[i].value,
                              true);
      if ((data.desc.specific._d() == DIET_SCALAR &&
           pb.parameters[i].desc.mode == DIET_VOLATILE) ||
          pb.parameters[i].desc.mode == DIET_PERSISTENT_RETURN ||
          pb.parameters[i].desc.mode == DIET_STICKY_RETURN) {
        pb.parameters[i] = *inserted;
      }
    }
    pb.parameters[i].desc = inserted->desc;

    if (i > pb.last_inout || data.desc.specific._d() == DIET_FILE) {
      if (data.desc.specific._d() == DIET_FILE) {
        manager->useDiskSpace(inserted->desc.specific.file().size);
      } else {
        manager->useMemSpace(inserted->value.length());
      }
    }

    manager->unlockData(data.desc.id.idNumber);

    // if (manager->pfmIsDataPresent(data.desc.id.idNumber) && (data.desc.specific._d() != DIET_CONTAINER))
    if (manager->pfmIsDataPresent(data.desc.id.idNumber)) {
      manager->pfmUpdateData(manager->getID(), data);
    }
  }
} // dagda_upload_data

// The "entry point" to the DAGDA hierarchy.
Dagda_var entryPoint = NULL;
// A reference to the MA.
MasterAgent_var masterAgent = NULL;
Agent_var parentAgent = NULL;

void
dagda_reset() {
  entryPoint = NULL;
  masterAgent = NULL;
  parentAgent = NULL;
}

// Returns an new ID
// DAGDA should use the uuid library to avoid id's conflicts.
char *
get_data_id() {
  SimpleDagdaImpl *localManager =
    (SimpleDagdaImpl *) DagdaFactory::getDataManager();
  std::ostringstream id;
  boost::uuids::uuid uuid = diet_generate_uuid();
  char *name = localManager->getID();

  if (name != NULL) {
    id << "DAGDA://id-" << uuid << "-" << name;
    CORBA::string_free(name);
  } else {
    id << "DAGDA://id-" << uuid << "-client-" << uuid;
  }

  return CORBA::string_dup(id.str().c_str());
} // get_data_id

MasterAgent_var
getMasterAgent() {
  if (masterAgent != NULL) {
    return masterAgent;
  }

  std::string MA_name;
  if (!CONFIG_STRING(diet::MANAME, MA_name)) {
    return NULL;
  }

  MasterAgent_var MA =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT, MA_name);
  if (CORBA::is_nil(MA)) {
    //ERROR_DEBUG("cannot locate Master Agent " << MA_name, 1);
    return NULL;
  }
  masterAgent = MasterAgent::_duplicate(MA);
  return MasterAgent::_duplicate(masterAgent);
} // getMasterAgent

Agent_var
getParent() {
  if (parentAgent != NULL) {
    return parentAgent;
  }

  std::string parent_name;
  if (!CONFIG_STRING(diet::PARENTNAME, parent_name)) {
    return NULL;
  }

  Agent_var Parent =
    ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, parent_name);
  if (CORBA::is_nil(Parent)) {
    WARNING("cannot locate parent Agent " << parent_name);
    return NULL;
  }
  parentAgent = Agent::_duplicate(Parent);
  return Agent::_duplicate(parentAgent);
} // getParent


// The reference to the MA DAGDA component is obtained from the ORB
// only one time.
Dagda_var
getEntryPoint() {
  if (entryPoint != NULL) {
    return entryPoint;
  }

  SimpleDagdaImpl *localManager =
    (SimpleDagdaImpl *) DagdaFactory::getDataManager();
  Dagda_var manager;

  if (localManager->getType() == DGD_CLIENT_MNGR) {
    std::string MA_name;
    if (!CONFIG_STRING(diet::MANAME, MA_name)) {
      return NULL;
    }

    MasterAgent_var MA =
      ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                              MA_name);
    if (CORBA::is_nil(MA)) {
      //ERROR_DEBUG("cannot locate Master Agent " << MA_name, 1);
      return NULL;
    }

    manager = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT,
                                                          MA->getDataManager());
    entryPoint = Dagda::_duplicate(manager);
    return Dagda::_duplicate(entryPoint);
  } else {return NULL;
  }
} // getEntryPoint

// corba_data_t initialization from explicit parameters.
size_t
corba_data_init(corba_data_t &data, diet_data_type_t type,
                diet_base_type_t base_type, diet_persistence_mode_t mode,
                size_t nb_r, size_t nb_c, diet_matrix_order_t order,
                void *value,
                char *path) {
  diet_data_t diet_data;
  DagdaImpl *manager = DagdaFactory::getDataManager();

  if (mode == DIET_VOLATILE) {
    WARNING("Trying to add a volatile data to DAGDA... The data " <<
            "will be persistent.");
    mode = DIET_PERSISTENT;
  }

  char *dataID = get_data_id();

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
    diet_data.desc.specific.str.length = strlen((char *) value) + 1;
    break;
  case DIET_PARAMSTRING:
    diet_data.desc.specific.pstr.param = (char *) value;
    diet_data.desc.specific.pstr.length = strlen((char *) value) + 1;
    break;
  case DIET_FILE:
    diet_data.desc.specific.file.path = path;
    break;
  case DIET_CONTAINER:
    break;
  default:
    WARNING("This type is not managed by DIET.");
  } // switch

  mrsh_data_desc(&data.desc, &diet_data.desc);
  data.desc.dataManager = CORBA::string_dup(manager->getID());
  return data_sizeof(&diet_data.desc);
} // corba_data_init

// This structure is used for the omni_thread outputs.
typedef struct {
  int returnedValue;
  // Put results.
  char *ID;
  // Get results.
  void *value;
  diet_base_type_t base_type;
  size_t nb_r, nb_c;
  diet_matrix_order_t order;
  char *path;
} DagdaThreadResult;

// DAGDA thread to put a data asynchronously in the DAGDA hierarchy.
class DagdaPutDataThread: public omni_thread {
private:
void *value;
diet_data_type_t type;
diet_base_type_t base_type;
diet_persistence_mode_t mode;
size_t nb_r, nb_c;
diet_matrix_order_t order;
char *path;
public:
DagdaPutDataThread(void *value, diet_data_type_t type,
                   diet_base_type_t base_type, diet_persistence_mode_t mode,
                   size_t nb_r, size_t nb_c, diet_matrix_order_t order,
                   char *path):
  value(value), type(type), base_type(base_type), mode(mode), nb_r(nb_r),
  nb_c(nb_c), order(order), path(path) {
}

void
startThread() {
  start_undetached();
}
protected:
void *
run_undetached(void *arg) {
  DagdaThreadResult *result = new DagdaThreadResult;

  result->returnedValue =
    dagda_put_data(value, type, base_type, mode, nb_r, nb_c, order, path,
                   &result->ID);
  return result;
}

void
run(void *arg) {
  dagda_put_data(value, type, base_type, mode, nb_r, nb_c, order, path, NULL);
}
};

// DAGDA thread to get a data asynchronously from the DAGDA hierarchy.
class DagdaGetDataThread: public omni_thread {
private:
char *dataID;
diet_data_type_t type;
public:
DagdaGetDataThread(char *dataID, diet_data_type_t type): dataID(dataID),
  type(type) {
}

void
startThread() {
  start_undetached();
}
protected:
void *
run_undetached(void *arg) {
  DagdaThreadResult *result = new DagdaThreadResult;

  result->returnedValue =
    dagda_get_data(dataID, &result->value, type, &result->base_type,
                   &result->nb_r, &result->nb_c, &result->order, &result->path);

  return result;
}

void
run(void *arg) {
  dagda_get_data(dataID, NULL, type, NULL, NULL, NULL, NULL, NULL);
}
};

// A thread pool to manage the asynchronous DAGDA operations.
class DagdaThreadPool {
public:
static DagdaThreadPool *
getInstance() {
  if (!instance) {
    instance = new DagdaThreadPool();
  }

  return instance;
}

// Creates a thread and records a reference on it.
// Put a data.
unsigned int
newPutDataThread(void *value, diet_data_type_t type,
                 diet_base_type_t base_type, diet_persistence_mode_t mode,
                 size_t nb_r, size_t nb_c,
                 diet_matrix_order_t order, char *path) {
  unsigned int id = getNextID();

  poolMutex.lock();
  pool[id] = new DagdaPutDataThread(value, type, base_type, mode,
                                    nb_r, nb_c, order, path);
  ((DagdaPutDataThread *) pool[id])->startThread();
  poolMutex.unlock();

  return id;
} // newPutDataThread

// Get a data.
unsigned int
newGetDataThread(char *dataID, diet_data_type_t type) {
  unsigned int id = getNextID();

  poolMutex.lock();
  pool[id] = new DagdaGetDataThread(dataID, type);
  ((DagdaGetDataThread *) pool[id])->startThread();
  poolMutex.unlock();

  return id;
}
// Waits the end of the transfert from the client.

int
waitDataPut(unsigned int id, char **ID) {
  int ret;
  DagdaThreadResult *result;
  omni_thread *thread;

  poolMutex.lock();
  if (pool.find(id) == pool.end()) {
    poolMutex.unlock();
    return -1;
  }

  thread = pool[id];
  poolMutex.unlock();

  thread->join((void **) &result);

  ret = result->returnedValue;
  *ID = result->ID;
  delete result;

  poolMutex.lock();
  pool.erase(id);
  poolMutex.unlock();

  return ret;
} // waitDataPut

// Waits the end of the transfert to the client.
int
waitDataGet(unsigned int id, void **value,
            diet_base_type_t *base_type, size_t *nb_r,
            size_t *nb_c, diet_matrix_order_t *order,
            char **path) {
  int ret;
  DagdaThreadResult *result;
  omni_thread *thread;

  poolMutex.lock();
  if (pool.find(id) == pool.end()) {
    poolMutex.unlock();
    return -1;
  }

  thread = pool[id];
  poolMutex.unlock();

  thread->join((void **) &result);
  ret = result->returnedValue;
  if (value != NULL) {
    *value = result->value;
  }
  if (base_type != NULL) {
    *base_type = result->base_type;
  }
  if (nb_r != NULL) {
    *nb_r = result->nb_r;
  }
  if (nb_c != NULL) {
    *nb_c = result->nb_c;
  }
  if (order != NULL) {
    *order = result->order;
  }
  if (path != NULL) {
    *path = result->path;
  }
  delete result;
  poolMutex.lock();
  pool.erase(id);
  poolMutex.unlock();
  return ret;
} // waitDataGet

// Returns a new thread id.
unsigned int
getNextID() {
  unsigned int ret;
  idMutex.lock();
  ret = nextID++;
  idMutex.unlock();
  return ret;
}

private:
static DagdaThreadPool *instance;
static unsigned int nextID;
omni_mutex idMutex;
omni_mutex poolMutex;

std::map<unsigned int, omni_thread *> pool;

DagdaThreadPool() {
}
};
// The thread pool is a singleton class.
DagdaThreadPool *DagdaThreadPool::instance = NULL;
unsigned int DagdaThreadPool::nextID = 0;

// Evaluation of the replication rule.
int
eval(const char *str, long *type, char **rule, bool *replace) {
  char *strCpy = strdup(str);
  char *S = strCpy;
  char *t, *r, *repl;

  t = strsep(&strCpy, ":");
  if (strCpy == NULL) {
    free(S);
    return 1;
  }
#ifdef __WIN32__
  if (stricmp(t, "id") == 0) {
#else
  if (strcasecmp(t, "id") == 0) {
#endif
    *type = 1;
  } else {
#ifdef __WIN32__
    if (stricmp(t, "host") == 0) {
#else
    if (strcasecmp(t, "host") == 0) {
#endif

      *type = 0;
    } else {
      free(S);
      return 1;
    }
  }
  r = strsep(&strCpy, ":");
  if (strCpy == NULL) {
    free(S);
    return 1;
  }
  repl = strsep(&strCpy, ":");
  if (strCpy != NULL) {
    free(S);
    return 1;
  }
#ifdef __WIN32__
  if (stricmp(repl, "replace") == 0) {
#else
  if (strcasecmp(repl, "replace") == 0) {
#endif
    *replace = true;
  } else {
#ifdef __WIN32__
    if (stricmp(repl, "noreplace") == 0) {
#else
    if (strcasecmp(repl, "noreplace") == 0) {
#endif

      *replace = false;
    } else {
      free(S);
      return 1;
    }
  }
  *rule = strdup(r);
  free(S);
  return 0;
} // eval

BEGIN_API
int
dagda_put_data(void *value, diet_data_type_t type,
               diet_base_type_t base_type, diet_persistence_mode_t mode,
               size_t nb_r, size_t nb_c,
               diet_matrix_order_t order, char *path,
               char **ID) {
  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl *manager = DagdaFactory::getDataManager();
  corba_data_t *inserted;

  corba_data_t data;
  char *dataID;

  size_t size = corba_data_init(data, type, base_type,
                                mode, nb_r, nb_c, order, value, path);
  dataID = data.desc.id.idNumber;
  inserted = manager->addData(data);

  if (value != NULL) {
    inserted->value.replace(size, size, (CORBA::Char *) value, false);
    manager->useMemSpace(inserted->value.length());  // FIX for mem usage (bi)
  }

  manager->unlockData(dataID);

  if (entryPoint != NULL) {
    entryPoint->pfmAddData(manager->getID(), data);
    // Client side. Don't need to keep the data reference.
    if (type == DIET_FILE) {
      manager->setDataStatus(dataID, Dagda::notOwner);
    }
    manager->remData(dataID);
  } else {
    manager->pfmAddData(manager->getID(), data);
  }
  if (ID != NULL) {
    *ID = CORBA::string_dup(dataID);
  }
  return 0;
} // dagda_put_data

int
dagda_get_data_description(const char *dataID, diet_data_type_t *type,
                           diet_base_type_t *base_type,
                           diet_persistence_mode_t* persistence_mode,
                           size_t *nb_r, size_t *nb_c,
                           diet_matrix_order_t *order) {
  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl *manager = DagdaFactory::getDataManager();
  char *src;
  corba_data_t data;

  data.desc.id.idNumber = CORBA::string_dup(dataID);

  if (!manager->lclIsDataPresent(dataID)) {
    // added because container elts are pre-downloaded
    if (entryPoint != NULL) {
      try {
        data.desc = *entryPoint->pfmGetDataDesc(dataID);
      } catch (CORBA::SystemException &e) {
        std::cerr << "dagda_get_data/pfmGetDataDesc: Caught a CORBA "
                  << e._name() << " exception ("
                  << e.NP_minorString() << ")\n";
        throw;
      } catch (...) {
        std::cerr << "dagda_get_data/pfmGetDataDesc: Caught exception\n";
        throw;
      }
      try {
        src = entryPoint->getBestSource(manager->getID(), dataID);

        Dagda_var source =
          ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, src);
        data.desc = *source->lclGetDataDesc(dataID);
      } catch (Dagda::DataNotFound &ex) {
        std::cerr << "dagda_get_data: data not found\n";
        return 1;
      } catch (CORBA::SystemException &e) {
        std::cerr << "dagda_get_data: Caught a CORBA "
                  << e._name() << " exception ("
                  << e.NP_minorString() << ")\n";
        return 1;
      }
    } else {
      try {
        data.desc = *manager->pfmGetDataDesc(dataID);
        src = manager->getBestSource(manager->getID(), dataID);

        Dagda_var source =
          ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, src);
        data.desc = *source->lclGetDataDesc(dataID);

      } catch (Dagda::DataNotFound &ex) {
        std::cerr << "dagda_get_data: data not found\n";
        return 1;
      }
    }
  } else {
    data.desc = manager->getData(dataID)->desc;
  }

  *base_type = (diet_base_type_t)data.desc.base_type;
  *persistence_mode = (diet_persistence_mode_t)data.desc.mode;

  switch (data.desc.specific._d()) {
  case 0:{
    *type = DIET_SCALAR;
    break;
  }
  case 1:{
    *type = DIET_VECTOR;
    *nb_c = (size_t)data.desc.specific.vect().size;
    break;
  }

  case 2:{
    *type = DIET_MATRIX;
    *nb_r = (size_t)data.desc.specific.mat().nb_r;
    *nb_c = (size_t)data.desc.specific.mat().nb_c;
    *order = (diet_matrix_order_t) data.desc.specific.mat().order;
    break;
  }

  case 3:{
    *type = DIET_STRING;
    break;
  }

  case 4:{
    *type = DIET_PARAMSTRING;
    break;
  }

  case 5:{
    *type = DIET_FILE;
    break;
  }
  case 6:{
    *type = DIET_CONTAINER;
    break;
  }
  default: break;
  }
  return 0;
} // dagda_get_data_description


int
dagda_get_data(const char *dataID, void **value, diet_data_type_t type,
               diet_base_type_t *base_type, size_t *nb_r, size_t *nb_c,
               diet_matrix_order_t *order, char **path) {
  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl *manager = DagdaFactory::getDataManager();
  char *src;
  corba_data_t data;
  corba_data_t *inserted;

  data.desc.id.idNumber = CORBA::string_dup(dataID);

  if (!manager->lclIsDataPresent(dataID)) {
    // added because container elts are pre-downloaded
    if (entryPoint != NULL) {
      try {
        data.desc = *entryPoint->pfmGetDataDesc(dataID);
      } catch (CORBA::SystemException &e) {
        std::cerr << "dagda_get_data/pfmGetDataDesc: Caught a CORBA "
                  << e._name() << " exception ("
                  << e.NP_minorString() << ")\n";
        throw;
      } catch (...) {
        std::cerr << "dagda_get_data/pfmGetDataDesc: Caught exception\n";
        throw;
      }
      try {
        src = entryPoint->getBestSource(manager->getID(), dataID);

        Dagda_var source =
          ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, src);
        data.desc = *source->lclGetDataDesc(dataID);

        manager->lclAddData(src, data);
        inserted = manager->getData(dataID);
      } catch (Dagda::DataNotFound &ex) {
        std::cerr << "dagda_get_data: data not found\n";
        return 1;
      } catch (CORBA::SystemException &e) {
        std::cerr << "dagda_get_data: Caught a CORBA "
                  << e._name() << " exception ("
                  << e.NP_minorString() << ")\n";
        return 1;
      }
    } else {
      try {
        data.desc = *manager->pfmGetDataDesc(dataID);
        src = manager->getBestSource(manager->getID(), dataID);

        Dagda_var source =
          ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, src);
        data.desc = *source->lclGetDataDesc(dataID);

        manager->lclAddData(src, data);
        inserted = manager->getData(dataID);
      } catch (Dagda::DataNotFound &ex) {
        std::cerr << "dagda_get_data: data not found\n";
        return 1;
      }
    }
  } else {
    inserted = manager->getData(dataID);
  }

  if (inserted->desc.specific._d() != type && type != DIET_UNKNOWN_TYPE) {
    std::cerr << "dagda_get_data: unknown data type\n";
    return 1;
  }

  if (value != NULL) {
    size_t size = inserted->value.length();
    if (inserted->value.release()) {
      *value = inserted->value.get_buffer(true);
      inserted->value.replace(size, size, (CORBA::Char *) *value, 0);
    } else {
      *value = inserted->value.get_buffer();
      inserted->value.replace(size, size, (CORBA::Char *) *value, 0);
    }
  }

  if (base_type != NULL) {
    *base_type = (diet_base_type_t) inserted->desc.base_type;
  }

  switch (inserted->desc.specific._d()) {
  case DIET_SCALAR:
    break;
  case DIET_VECTOR:
    if (nb_c != NULL) {
      *nb_c = inserted->desc.specific.vect().size;
    }
    break;
  case DIET_MATRIX:
    if (nb_r != NULL) {
      *nb_r = inserted->desc.specific.mat().nb_r;
    }
    if (nb_c != NULL) {
      *nb_c = inserted->desc.specific.mat().nb_c;
    }
    if (order != NULL) {
      *order = (diet_matrix_order_t) inserted->desc.specific.mat().order;
    }
    break;
  case DIET_STRING:
  case DIET_PARAMSTRING:
    break;
  case DIET_FILE:
    if (path != NULL) {
      *path = inserted->desc.specific.file().path;
    }
    break;
  case DIET_CONTAINER:
    break;
  default:
    WARNING("This data type is not managed by DIET.");
  } // switch
  return 0;
} // dagda_get_data

int
dagda_delete_data(char *dataID) {
  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl *manager = DagdaFactory::getDataManager();
  if (entryPoint != NULL) {
    entryPoint->pfmRemData(dataID);
    manager->lclRemData(dataID);
  } else {
    manager->pfmRemData(dataID);
  }
  return 0;
} // dagda_delete_data

unsigned int
dagda_put_data_async(void *value, diet_data_type_t type,
                     diet_base_type_t base_type, diet_persistence_mode_t mode,
                     size_t nb_r, size_t nb_c,
                     diet_matrix_order_t order, char *path) {
  DagdaThreadPool *instance = DagdaThreadPool::getInstance();

  return instance->newPutDataThread(value, type, base_type, mode,
                                    nb_r, nb_c, order, path);
}

unsigned int
dagda_get_data_async(char *ID, diet_data_type_t type) {
  DagdaThreadPool *instance = DagdaThreadPool::getInstance();

  return instance->newGetDataThread(ID, type);
}

int
dagda_wait_put(unsigned int threadID, char **ID) {
  DagdaThreadPool *instance = DagdaThreadPool::getInstance();

  return instance->waitDataPut(threadID, ID);
}

int
dagda_wait_get(unsigned int threadID, void **value,
               diet_base_type_t *base_type, size_t *nb_r,
               size_t *nb_c, diet_matrix_order_t *order, char **path) {
  DagdaThreadPool *instance = DagdaThreadPool::getInstance();

  return instance->waitDataGet(threadID, value, base_type, nb_r,
                               nb_c, order, path);
}

int
dagda_add_data(void *value, diet_data_type_t type,
               diet_base_type_t base_type, diet_persistence_mode_t mode,
               size_t nb_r, size_t nb_c,
               diet_matrix_order_t order, char *path) {
  DagdaPutDataThread *thrd =
    new DagdaPutDataThread(value, type, base_type, mode,
                           nb_r, nb_c, order, path);
  thrd->start();
  return 0;
}

int
dagda_load_data(char *ID, diet_data_type_t type) {
  DagdaGetDataThread *thrd =
    new DagdaGetDataThread(ID, type);
  thrd->start();
  return 0;
}

int
dagda_save_platform() {
  if (entryPoint != NULL) {
    entryPoint->checkpointState();
    return 0;
  }
  return 1;
}

int
dagda_data_alias(const char *id, const char *alias) {
  MasterAgent_var MA = getMasterAgent();

  if (MA == NULL) {
    WARNING("Try to call " << __FUNCTION__ << " outside a client.");
    return 1;
  }
  SeqString values;
  values.length(1);
  values[0] = id;

  return MA->insertData(alias, values);
} // dagda_data_alias

int
dagda_id_from_alias(const char *alias, char **id) {
  MasterAgent_var MA = getMasterAgent();

  if (MA == NULL) {
    // WARNING("Try to call " << __FUNCTION__ << " outside a client.");

    Agent_var parent = getParent();
    SeqString *attributes = parent->searchData(alias);
    if (attributes->length() == 0) {
      return 1;
    }
    *id = (*attributes)[0];

    return 0;
  }
  SeqString *attributes = MA->searchData(alias);
  if (attributes->length() == 0) {
    return 1;
  }
  *id = (*attributes)[0];
  return 0;
} // dagda_id_from_alias

int
dagda_replicate_data(const char *id, const char *rule) {
  long target;
  char *pattern;
  bool replace;

  if (eval(rule, &target, &pattern, &replace) != 0) {
    WARNING("Error in replication rule definition.");
    return 1;
  }

  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl *manager = DagdaFactory::getDataManager();

  if (entryPoint != NULL) { // We are on a client.
    entryPoint->pfmReplicate(id, target, pattern, replace);
  } else {
    manager->pfmReplicate(id, target, pattern, replace);
  }
  return 0;
} // dagda_replicate_data


/** API for container management **/

// used to create a new container (ID to be defined) - used on Client or SeD
int
dagda_create_container(char **ID) {
  return dagda_put_data(NULL, DIET_CONTAINER, DIET_CHAR, DIET_PERSISTENT,
                        0, 0, (diet_matrix_order_t) 0, NULL, ID);
}

// used to initialize a container (ID already defined) - used on Client or SeD
// will NOT download all the elements of the container
// on SERVER IN ==> it is called after the container was downloaded so after
// adding elements
// on SERVER OUT ==> called before adding elements
int
dagda_init_container(diet_data_t *profile_data) {
  DagdaImpl *manager = DagdaFactory::getDataManager();
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

  return 0;
} // dagda_init_container

int
add_container_element_common(const char *idContainer, const char *idElement,
                             int index, int flag) {
  Dagda_var entryPoint = getEntryPoint();
  DagdaImpl *manager = DagdaFactory::getDataManager();
  try {
    if (entryPoint != NULL) {
      // on the client => find the data mgr that hosts the container
      if (entryPoint->pfmIsDataPresent(idContainer)) {
        corba_data_desc_t *storedDataDesc =
          entryPoint->pfmGetDataDesc(idContainer);
        Dagda_var srcMgr =
          ORBMgr::getMgr()->resolve<Dagda,
                                    Dagda_var>(DAGDACTXT,
                                               std::string(
                                                 storedDataDesc->dataManager));
        // add the relationship
        srcMgr->lclAddContainerElt(idContainer, idElement, index, flag);
      } else {
        WARNING("Cannot find container " << idContainer << " on platform");
        return 1;
      }
    } else {
      // not on the client => dagda_init_container must have been called
      // before so the container exists locally
      if (manager->lclIsDataPresent(idContainer)) {
        manager->lclAddContainerElt(idContainer, idElement, index, flag);
      } else {
        WARNING("Cannot find container " << idContainer << " locally");
        return 1;
      }
    }
  } catch (Dagda::DataNotFound &ex) {
    return 1;
  }
  return 0;
} // add_container_element_common

int
dagda_add_container_element(const char *idContainer, const char *idElement,
                            int index) {
  return add_container_element_common(idContainer, idElement, index, 0);
}

int
dagda_add_container_null_element(const char *idContainer, int index) {
  char idElement = 0;
  return add_container_element_common(idContainer, &idElement, index, 1);
}

int
dagda_get_container_elements(const char *idContainer,
                             diet_container_t *content) {
  DagdaImpl *manager = DagdaFactory::getDataManager();
  // get the sequence of IDs
  SeqString eltIDSeq;
  SeqLong eltFlagSeq;
  try {
    manager->lclGetContainerElts(idContainer, eltIDSeq, eltFlagSeq, true);
  } catch (CORBA::SystemException &e) {
    std::cerr << "dagda_get_container_elements: Caught a CORBA " <<
    e._name() << " exception ("
              << e.NP_minorString() << ")\n";
    return 1;
  } catch (...) {
    std::cerr << "dagda_get_container_elements exception!\n";
    throw;
  }
  // convert it to diet_container_t
  if (content != NULL) {
    content->size = eltIDSeq.length();
    content->id = CORBA::string_dup(idContainer);
    content->elt_ids = (char **) malloc(sizeof(char *) * content->size);  // FIXME not deallocated
    for (unsigned int ix = 0; ix < eltIDSeq.length(); ++ix) {
      if (!eltFlagSeq[ix]) { // test if this is not a null element
        content->elt_ids[ix] = CORBA::string_dup(eltIDSeq[ix]);
      } else {
        content->elt_ids[ix] = NULL;
      }
    }
  } else {return 1;
  }
  return 0;
} // dagda_get_container_elements

#if DAGDA_PROGRESSION
double
dagda_get_progress(const char *transferId) {
  Transfers *instance = Transfers::getInstance();

  try {
    return instance->getProgress(transferId);
  } catch (std::runtime_error &err) {
    return 0;  // should return a distinct value
  }
}

void
dagda_rem_progress(const char *transferId) {
  Transfers *instance = Transfers::getInstance();

  try {
    instance->remTransfer(transferId);
  } catch (std::runtime_error &err) {
    std::cerr << "Error: " << err.what() << "\n";
  }
}
#else // if DAGDA_PROGRESSION
double
dagda_get_progress(const char *transferId) {
  std::cerr << "DIET was not compile with DAGDA_PROGRESSION set.";
  std::cerr << " Please re-compile DIET activating this option.\n";
  return 0;
}

void
dagda_rem_progress(const char *transferId) {
  std::cerr << "DIET was not compile with DAGDA_PROGRESSION set.";
  std::cerr << " Please re-compile DIET activating this option.\n";
}
#endif // if DAGDA_PROGRESSION

END_API
