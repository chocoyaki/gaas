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
DataRelationMgr::addRelation(const string& dataID1,
                             const string& dataID2,
                             long index,
                             long flag) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Adding relation cont=" << dataID1
                              << " - elt=" << dataID2 << endl);
  dataRelationValue_t value = { dataID2, index, flag};
  myLock.lock(); /** LOCK */
  myMap.insert(make_pair(dataID1, value));
  myLock.unlock(); /** UNLOCK */
}

void
DataRelationMgr::remRelation(const string& dataID1,
                             const string& dataID2,
                             long index) {
  myLock.lock(); /** LOCK */
  for (multimap<std::string, dataRelationValue_t>::iterator iter = myMap.lower_bound(dataID1);
       iter != myMap.upper_bound(dataID1);
       ++iter) {
    if ((iter->second.ID == dataID2) && (iter->second.index == index)) {
      myMap.erase(iter);
      TRACE_TEXT(TRACE_ALL_STEPS, "Removed 1 relation : key="
      << dataID1 << " value=" << dataID2 << " index=" << index << endl);
      break;
    }
  }
  myLock.unlock(); /** UNLOCK */
}

void
DataRelationMgr::remAllRelation(const string& dataID, bool reverse) {
  // Delete links FROM the item
  myLock.lock();
  int count = myMap.erase(dataID);
  myLock.unlock();
  TRACE_TEXT(TRACE_ALL_STEPS, "Removed " << count << " relations for key "
      << dataID << endl);
  // Delete links TO the item
  if (reverse) {
    // TODO delete all links TO the item
  }
}

// const int
// DataRelationMgr::getRelationNb(const string& dataID) { }

/**
 * Get the list of relations for a given dataID
 * Note: the SeqString and SeqLong must be pre-allocated by the caller
 * TODO manage exceptions
 */
void
DataRelationMgr::getRelationList(const string& dataID,
                                 SeqString& dataIDList,
                                 SeqLong& flagList,
                                 bool ordered) {
  int ix = 0;
  int listSize = dataIDList.length();
  myLock.lock();
  for (multimap<std::string, dataRelationValue_t>::iterator iter = myMap.lower_bound(dataID);
       iter != myMap.upper_bound(dataID);
       ++iter) {
    int jx = ordered ? iter->second.index : ix++;
    if (jx >= listSize) {
      INTERNAL_ERROR("Mismatching container size with nb of relationships",1);
    }
    dataIDList[jx] = CORBA::string_dup(iter->second.ID.c_str());
    flagList[jx]   = iter->second.flag;
  }
  myLock.unlock();
}

int
DataRelationMgr::getRelationNb(const string& dataID) {
  myLock.lock();
  int ix = 0;
  for (multimap<std::string, dataRelationValue_t>::iterator iter = myMap.lower_bound(dataID);
       iter != myMap.upper_bound(dataID);
       ++iter) { ++ix; }
  myLock.unlock();
  return ix;
}

/**
 * Display all the relations for debug
 */
void
DataRelationMgr::displayContent() {
  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    cout << "Content of relationship DB: " << endl;
    myLock.lock();
    for (multimap<string, dataRelationValue_t>::iterator iter = myMap.begin();
         iter != myMap.end();
         ++iter) {
      cout << " | " << iter->first << " | " << iter->second.ID
           << " | " << iter->second.index << " | " << iter->second.flag << endl;
    }
    myLock.unlock();
  }
}
