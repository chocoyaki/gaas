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
 * Revision 1.3  2010/07/20 09:20:11  bisnard
 * integration with eclipse gui and with dietForwarder
 *
 * Revision 1.2  2010/07/12 16:14:13  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
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
    
  DagNode* getNode() const { return myNode; }

  /**
   * Set the SeD reference on which the node should be executed
   * @param sed the SeD reference
   * @param reqID the request ID (of previous submit request)
   * @param ev  the Estimation vector for this SeD (required to call diet_call_common)
   */
  void setSeD(const char* sed, const unsigned long reqID, corba_estimation_t& ev);
        
  /**
   * Returns true if sed is defined
   */
  bool isSeDDefined() const { return isSeDDefinedFlag; }
    
  /**
   * Returns the sed host name
   */
  string getSeDName() const;
    
  /**
   * Returns the request id
   */
  unsigned long getReqId() const { return myReqID; }
    
  /**
   * Returns a description of this object (for events)
   */
  virtual string toString() const;

  /**
   * Run method
   */
  virtual void * run();

  /**
   * Node execution method (VIRTUAL)
   */
  virtual void
  execNode() = 0;

  virtual void
  finishNode();


protected:

  DagNode*            myNode;
  DagScheduler *      myDagScheduler;
  bool                isSeDDefinedFlag;
  char*               myChosenServer;
  corba_estimation_t  myEstimVect;
  unsigned long       myReqID;
  bool                isSuccessfulFlag;

};

#endif
