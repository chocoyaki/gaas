/****************************************************************************/
/* The virtual class used for dagNode execution                             */
/*                                                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/09/25 12:36:18  bisnard
 * created new classes DagNodeLauncher & childs
 *
 ****************************************************************************/

#ifndef _DAGNODELAUNCHER_
#define _DAGNODELAUNCHER_

#include "SeDImpl.hh"
#include "Thread.hh"
#include "DagScheduler.hh"

class DagNode;

class DagNodeLauncher : public Thread
{

  public:

    DagNodeLauncher(DagNode * parent,
                    DagScheduler * scheduler = NULL);

    /**
     * Set the SeD reference on which the node should be executed
     * @param sed the SeD reference
     * @param reqID the request ID (of previous submit request)
     * @param ev  the Estimation vector for this SeD (required to call diet_call_common)
     */
    void
        setSeD(const SeD_var& sed, const unsigned long reqID, corba_estimation_t& ev);

    /**
      * Run method
      */
    virtual void * run();

    /**
     * Node execution method (VIRTUAL)
     */
    virtual void
        execNode(bool& successful) = 0;

    virtual void
        finishNode();


  protected:

    DagNode*      myNode;
    DagScheduler *  myDagScheduler;
    bool          isSedDefined;
    SeD_var       myChosenServer;
    corba_estimation_t   myEstimVect;
    unsigned long myReqID;

};

#endif
