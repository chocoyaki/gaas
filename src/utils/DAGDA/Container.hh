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
 * Revision 1.5  2009/09/25 12:43:37  bisnard
 * modified send method to handle missing relationships
 *
 * Revision 1.4  2009/03/27 09:09:41  bisnard
 * replace container size attr by dynamic value
 *
 * Revision 1.3  2009/01/16 13:32:47  bisnard
 * replaced use of DagdaImpl ptr by dagda object ref
 * modified constructor signature
 * moved container descr update from addData to DagdaImpl
 *
 * Revision 1.2  2008/12/09 12:06:21  bisnard
 * changed container download method to transfer only the list of IDs
 * (each container element must be downloaded separately)
 *
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
    Container(const char* dataID,
              Dagda_ptr dataMgr,
              DataRelationMgr* relMgr);

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
     */
    void addData(const char* dataID, long index, long flag);

    /**
     * Remove the i-th element from the container (does not delete the element)
     * @param index   the index of the element
     */
    void remData(long index);

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
     * @param sendData  if true, will send all the elements of the container
     *                  if false, will send only the relationships
     */
    char* send(Dagda_ptr dest, bool sendData = true);

  private:

    /**
     * The DAGDA ID of the container
     */
    std::string  myID;

    /**
     * The Data Mgr that contains the container
     */
    Dagda_ptr myMgr;

    /**
     * The data relation Mgr that manages the container-elements relationship
     */
    DataRelationMgr*  myRelMgr;

    /**
     * Nb of elements in the container
     */
    int nbOfElements;

    /**
     * Not found flag
     */
    bool notFound;


}; // end class Container

#endif
