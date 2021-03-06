/**
 * @file  DataRelationMgr.hh
 *
 * @brief  Class used to manage relationships between data items (used for containers in DAGDA)
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef __DATARELATIONMGR_HH__
#define __DATARELATIONMGR_HH__

#include "DIET_data.h"
#include "common_types.hh"

#include <map>
#include <string>

/**
 * Class DataRelationMgr
 *
 * This class is used to store and manage relationships between data items of
 * a DAGDA agent. A relationship is simply a link between two data items with
 * two additionnal fields 'index' and 'flag'. 'index' is used to keep an order
 * in the links from a given item, and 'flag' is used to store specific info
 * about the linked item (e.g. size of the item).
 */

class DataRelationMgr {
public:
  DataRelationMgr();

  /**
   * Add a relationship between two data items
   * @param dataID1 id of first item
   * @param dataID2 id of second item
   * @param index   index of the link
   * @param flag    specific info about linked item
   */
  void
  addRelation(const std::string &dataID1, const std::string &dataID2,
              long index, long flag = 0);

  /**
   * Remove the relationship given parent data id and index
   * @param dataID1 id of first item
   * @param index   index of the link
   */
  void
  remRelation(const std::string &dataID1, long index);

  /**
   * Remove all relationships for a given item
   * @param dataID  id of the item
   * @param reverse if true, will consider links in BOTH WAYS
   */
  void
  remAllRelation(const std::string &dataID, bool reverse = false);

  /**
   * Get the total nb of relations for a given dataID
   * @param dataID  id of the container
   */
  unsigned int
  getRelationNb(const std::string &dataID);

  /**
   * Get the maximum index value for a given dataID (indexes start from 0)
   * @param dataID  id of the container
   */
  unsigned int
  getRelationMaxIndex(const std::string &dataID);

  /**
   * Get the relationships from a given item (using index order)
   *
   * The two sequences returned must be pre-allocated with enough space
   * to store all the relationships found
   *
   * The sequence may contain NULL elements if some indexes are missing
   * in the database.
   *
   * @param dataID      id of the item
   * @param dataIDList  a sequence of std::string containing the IDs
   * @param flagList    a sequence of integers containing the flags
   * @param ordered     if false, will not use the index field to sort
   */
  void
  getRelationList(const std::string &dataID, SeqString &dataIDList,
                  SeqLong &flagList, bool ordered = true);

  /**
   * Display all relationships (for debug)
   */
  void
  displayContent();

protected:
  typedef struct {
    std::string ID;
    long index;
    long flag;
  } dataRelationValue_t;

  std::multimap<std::string, dataRelationValue_t> myMap;

  /**
   * Critical section
   */
  omni_mutex myLock;
};  // end class DataRelationMgr

#endif /* ifndef __DATARELATIONMGR_HH__ */
