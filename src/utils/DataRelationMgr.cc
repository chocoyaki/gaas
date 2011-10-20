/***********************************************************/
/* Class used to manage relationships between data items   */
/* (used for containers in DAGDA)                          */
/*                                                         */
/*  Author(s):                                             */
/*    - Benjamin Isnard (benjamin.isnard@ens-lyon.fr       */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2010/07/30 14:44:26  glemahec
 * Temporary corrections for the new compilation process. Parallel compilation is still broken and there is a big mess on the CMakeLists files...
 *
 * Revision 1.5  2009/10/02 07:43:21  bisnard
 * reduced trace verbosity for containers
 *
 * Revision 1.4  2009/09/25 12:51:31  bisnard
 * fixed bug in case of containers containing empty elements as first elements
 *
 * Revision 1.3  2009/03/27 08:06:20  bisnard
 * new method getRelationNb
 *
 * Revision 1.2  2009/01/16 13:36:54  bisnard
 * make thread-safe using mutex
 *
 * Revision 1.1  2008/09/09 10:07:36  bisnard
 * new class used for container mgmt using Dagda agent
 *
 *
 ***********************************************************/

#include "DataRelationMgr.hh"
#include "debug.hh"
#include <iostream>

DataRelationMgr::DataRelationMgr() { }

void
DataRelationMgr::addRelation(const std::string& dataID1,
                             const std::string& dataID2,
                             long index,
                             long flag) {
  dataRelationValue_t value = { dataID2, index, flag};
  myLock.lock(); /** LOCK */
  myMap.insert(make_pair(dataID1, value));
  myLock.unlock(); /** UNLOCK */
}

void
DataRelationMgr::remRelation(const std::string& dataID1,
                             long index) {
  myLock.lock(); /** LOCK */
  for (std::multimap<std::string, dataRelationValue_t>::iterator iter = myMap.lower_bound(dataID1);
       iter != myMap.upper_bound(dataID1);
       ++iter) {
    if (iter->second.index == index) {
      myMap.erase(iter);
      break;
    }
  }
  myLock.unlock(); /** UNLOCK */
}

void
DataRelationMgr::remAllRelation(const std::string& dataID, bool reverse) {
  // Delete links FROM the item
  myLock.lock();
  int count = myMap.erase(dataID);
  myLock.unlock();
  TRACE_TEXT(TRACE_ALL_STEPS, "Removed " << count << " relations for key "
             << dataID << "\n");
  // Delete links TO the item
  if (reverse) {
    // TODO delete all links TO the item
  }
}

/**
 * Get the list of relations for a given dataID
 * Note: the SeqString and SeqLong must be pre-allocated by the caller
 */
void
DataRelationMgr::getRelationList(const std::string& dataID,
                                 SeqString& dataIDList,
                                 SeqLong& flagList,
                                 bool ordered) {
  int ix = 0;
  int listSize = dataIDList.length();
  myLock.lock();
  for (std::multimap<std::string, dataRelationValue_t>::iterator iter = myMap.lower_bound(dataID);
       iter != myMap.upper_bound(dataID);
       ++iter) {
    int jx = ordered ? iter->second.index : ix++;
    if (jx >= listSize) {
      INTERNAL_ERROR("Mismatching container element index with nb of relationships", 1);
    }
    dataIDList[jx] = CORBA::string_dup(iter->second.ID.c_str());
    flagList[jx]   = iter->second.flag;
  }
  myLock.unlock();
}

unsigned int
DataRelationMgr::getRelationNb(const std::string& dataID) {
  unsigned int ix = 0;
  myLock.lock();
  for (std::multimap<std::string, dataRelationValue_t>::iterator iter = myMap.lower_bound(dataID);
       iter != myMap.upper_bound(dataID);
       ++iter) { ++ix; }
  myLock.unlock();
  return ix;
}

unsigned int
DataRelationMgr::getRelationMaxIndex(const std::string& dataID) {
  int max = 0;
  myLock.lock();
  for (std::multimap<std::string, dataRelationValue_t>::iterator iter = myMap.lower_bound(dataID);
       iter != myMap.upper_bound(dataID);
       ++iter) {
    if (iter->second.index > max)
      max = (unsigned int) iter->second.index;
  }
  myLock.unlock();
  return max;
}

/**
 * Display all the relations for debug
 */
void
DataRelationMgr::displayContent() {
  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    std::cout << "Content of relationship DB: \n";
    myLock.lock();
    std::multimap<std::string, dataRelationValue_t>::iterator iter =
      myMap.begin();
    for (; iter != myMap.end(); ++iter) {
      std::cout << " | " << iter->first
                << " | " << iter->second.ID
                << " | " << iter->second.index
                << " | " << iter->second.flag << "\n";
    }
    myLock.unlock();
  }
}
