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
     * Remove a relationship between two data items
     * @param dataID1 id of first item
     * @param dataID2 id of second item
     * @param index   index of the link
     */
    void remRelation(const string& dataID1,
                     const string& dataID2,
                     long index);

    /**
     * Remove all relationships for a given item
     * @param dataID  id of the item
     * @param reverse if true, will consider links in BOTH WAYS
     */
    void remAllRelation(const string& dataID, bool reverse = false);

//     const int  getRelationNb(const string& dataID);

    /**
     * Get the relationships from a given item (using index order)
     *
     * The two sequences returned must be pre-allocated with enough space
     * to store all the relationships found
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

}; // end class DataRelationMgr

#endif
