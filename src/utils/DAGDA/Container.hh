/***********************************************************/
/* Container implementation.                               */
/*                                                         */
/*  Author(s):                                             */
/*    - Benjamin Isnard (benjamin.isnard@ens-lyon.fr       */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/09/09 10:05:14  bisnard
 * container mgmt using Dagda agent
 *
 *
 ***********************************************************/

#ifndef __CONTAINER_HH__
#define __CONTAINER_HH__

#include "DIET_data.h"
#include "DIET_Dagda.h"
#include "DagdaImpl.hh"
#include "DataRelationMgr.hh"
#include <map>

class Container {

  public:

    /**
     * Constructor by ID
     * Access an existing container on the local Data Mgr
     */
    Container(const char* dataID);

    /**
     * Destructor
     */
    ~Container();

    /**
     * Add a data element to the container
     * (the element must already be registered in a Data Mgr that is
     * not necessarily the local Data Mgr)
     * @param dataID  the element data identifier
     * @param index   the index of the element
     * @param flag    the flag of the relation to the container
     * @param setSize   if yes, will increase the size of the container
     */
    void addData(const char* dataID, long index, long flag, bool setSize=true);

    /**
     * Remove a data element from the container
     */
    void remData(const char* dataID, long flag);

    /**
     * Get the size (nb of elements) of the container
     * Note: currently only used to allocate the sequences returned by getAllElements
     */
    int size();

    /**
     * Get all the elements of a container
     * @param dataIDSeq a sequence of strings (not pre-allocated)
     * @param flagSeq a sequence of long integers (not pre-allocated)
     * @param ordered sort the elements in the initial order
     */
    void getAllElements(SeqString& dataIDSeq, SeqLong& flagSeq, bool ordered);

    /**
     * Send the container to a remote Data Mgr
     * @param dest  the destination dagda agent
     */
    char* send(Dagda_ptr dest);

  private:

    /**
     * The DAGDA ID of the container
     */
    std::string  myID;

    /**
     * The Data Mgr that contains the container
     */
    DagdaImpl * myMgr;

    /**
     * The data relation Mgr that manages the container-elements relationship
     */
    DataRelationMgr*  myRelMgr;

    /**
     * Nb of elements in the container
     */
    int nbOfElements;


}; // end class Container

#endif