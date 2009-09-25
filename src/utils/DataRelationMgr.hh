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

#ifndef __DATARELATIONMGR_HH__
#define __DATARELATIONMGR_HH__

#include "DIET_data.h"
#include "common_types.hh"

#include <string>
#include <map>

using namespace std;

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
    void addRelation(const string& dataID1,
                     const string& dataID2,
                     long index,
                     long flag = 0);

    /**
     * Remove the relationship given parent data id and index
     * @param dataID1 id of first item
     * @param index   index of the link
     */
    void remRelation(const string& dataID1,
                     long index);

    /**
     * Remove all relationships for a given item
     * @param dataID  id of the item
     * @param reverse if true, will consider links in BOTH WAYS
     */
    void remAllRelation(const string& dataID, bool reverse = false);

    /**
     * Get the total nb of relations for a given dataID
     * @param dataID  id of the container
     */
    unsigned int
        getRelationNb(const string& dataID);

    /**
     * Get the maximum index value for a given dataID (indexes start from 0)
     * @param dataID  id of the container
     */
    unsigned int
        getRelationMaxIndex(const string& dataID);

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
     * @param dataIDList  a sequence of string containing the IDs
     * @param flagList    a sequence of integers containing the flags
     * @param ordered     if false, will not use the index field to sort
     */
    void getRelationList(const string& dataID,
                         SeqString& dataIDList,
                         SeqLong& flagList,
                         bool ordered = true);

    /**
     * Display all relationships (for debug)
     */
    void displayContent();

  protected:

    typedef struct {
      string  ID;
      long    index;
      long    flag;
    } dataRelationValue_t;

    multimap<string, dataRelationValue_t> myMap;

    /**
     * Critical section
     */
    omni_mutex myLock;

}; // end class DataRelationMgr

#endif
