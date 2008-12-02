/****************************************************************************/
/* The class used to temporary store data handles during the instanciation  */
/* and build or keep the hierarchical structure of data handles             */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/12/02 10:07:07  bisnard
 * new classes for functional workflow instanciation and execution
 *
 */

#ifndef _FNODEPORTBUFFER_HH_
#define _FNODEPORTBUFFER_HH_

#include <map>

#include "FNodePort.hh"
#include "FDataHandle.hh"

/**
 * Class FNodePortBuffer
 *
 * This class is used to store the data handles to be processed by a given
 * FNode port.
 *
 * Actors in the FNodePortBuffer scenario:
 *  - data producer: inserts data handles in the buffer
 *  - data handles: reference to a virtual or real data ie a data item before
 *                  or after it has been produced by the workflow
 *  - data consumer: retrieves data handles from the buffer
 *
 * The data producer (eg a FNode output port or a data source) insert data
 * handles in the buffer. These data handles correspond to data items that
 * have a fixed depth (0 if scalar data or a positive integer if container).
 * The purpose of the buffer is to provide to its consumer (the parent port)
 * data items that either correspond in a one-to-one fashion to the input item
 * or that group or split input items if the output depth has a different
 * value from the input depth.
 *
 */

class FNodePortBuffer {

  public:

    FNodePortBuffer(unsigned int dataDepth);

    /**
     * Data handle insertion
     */
    void
    insert(FDataHandle* dataHdl);

    /**
     * Data handle retrieval
     */
    FDataHandle*
    get(const FDataTag& tag);

    /**
     * Get the list of data handles (begin)
     */

    map<FDataTag,FDataHandle*>::iterator
    begin();

    /**
     * Get the list of data handles (end)
     */

    map<FDataTag,FDataHandle*>::iterator
    end();

  protected:

    FDataHandle *
    getParent(FDataHandle* childHdl,
              unsigned int parentDepth);

    /**
     * The data buffer
     */
    map<FDataTag,FDataHandle*> myData;

    /**
     * The depth of data items in the data buffer
     */
    unsigned int myDepth;

}; // end class FNodePortBuffer

#endif // _FNODEPORTBUFFER_HH_
