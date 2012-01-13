/**
 * @file CacheAlgorithms.cc
 *
 * @brief  Cache replacements algorithms used by DAGDA when we remove data
 *
 * @author  Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include <iostream>
#include <string>
#include <map>

#include "AdvancedDagdaComponent.hh"
#include "DIET_data_internal.hh"
#include "debug.hh"

#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif
using namespace std;

extern "C" DIET_API_LIB int
LRU(AdvancedDagdaComponent *manager, size_t size, dagda_object_type_t type) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Needs more space for the data:" <<
             " Tries to remove one using LRU." << endl);
  std::map<std::string, corba_data_t>::iterator it;
  clock_t leastRecent = 0;
  std::string found;

  for (it = manager->getData()->begin(); it != manager->getData()->end(); ++it)
    if (it->second.desc.mode != DIET_STICKY && DGD_OBJ_TYPE(it->second) ==
        type) {
      if ((leastRecent == 0 || leastRecent >
           manager->getLastUsageTime(it->first.c_str()))
          && data_sizeof(&it->second.desc) >= size) {
        leastRecent = manager->getLastUsageTime(it->first.c_str());
        found = it->first;
      }
    }
  if (found != "") {
    manager->remData(found.c_str());
    return 0;
  } else {cerr << "No sufficient space found..." << endl;
  }
  return 1;
} // LRU

int
LFU(AdvancedDagdaComponent *manager, size_t size, dagda_object_type_t type) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Needs more space for the data:" <<
             " Tries to remove one using LFU policy." << endl);
  std::map<std::string, corba_data_t>::iterator it;
  unsigned long usageMin = 0;
  std::string found;

  for (it = manager->getData()->begin(); it != manager->getData()->end(); ++it)
    if (it->second.desc.mode != DIET_STICKY && DGD_OBJ_TYPE(it->second) ==
        type) {
      if ((usageMin == 0 || usageMin > manager->getNbUsage(it->first.c_str()))
          && data_sizeof(&it->second.desc) >= size) {
        usageMin = manager->getNbUsage(it->first.c_str());
        found = it->first;
      }
    }
  if (found != "") {
    manager->remData(found.c_str());
    return 0;
  } else {cerr << "No sufficient space found..." << endl;
  }
  return 1;
} // LFU

int
FIFO(AdvancedDagdaComponent *manager, size_t size, dagda_object_type_t type) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Needs more space for the data:" <<
             " Tries to remove one using FIFO policy." << endl);
  std::map<std::string, corba_data_t>::iterator it;
  clock_t registerTime = 0;
  std::string found;

  for (it = manager->getData()->begin(); it != manager->getData()->end(); ++it)
    if (it->second.desc.mode != DIET_STICKY && DGD_OBJ_TYPE(it->second) ==
        type) {
      if ((registerTime == 0 || registerTime >
           manager->getRegisterTime(it->first.c_str()))
          && data_sizeof(&it->second.desc) >= size) {
        registerTime = manager->getRegisterTime(it->first.c_str());
        found = it->first;
      }
    }
  if (found != "") {
    manager->remData(found.c_str());
    return 0;
  } else {cerr << "No sufficient space found..." << endl;
  }
  return 1;
} // FIFO
