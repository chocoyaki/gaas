/**
* @file DagNodeLauncher.hh
* 
* @brief  The virtual class used for dagNode execution 
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _DAGNODELAUNCHER_
#define _DAGNODELAUNCHER_

#include <map>
#include <string>
#include "SeDImpl.hh"
#include "Thread.hh"
#include "DagScheduler.hh"

class DagNode;

class DagNodeLauncher : public Thread {
public:
  DagNodeLauncher(DagNode * parent, DagScheduler * scheduler = NULL);

  DagNode*
  getNode() const {
    return myNode;
  }

  /**
   * Set the SeD reference on which the node should be executed
   * @param sed the SeD reference
   * @param reqID the request ID (of previous submit request)
   * @param ev  the Estimation vector for this SeD (required to call diet_call_common)
   */
  void
  setSeD(const char* sed, const unsigned long reqID, corba_estimation_t& ev);

  /**
   * Returns true if sed is defined
   */
  bool
  isSeDDefined() const {
    return isSeDDefinedFlag;
  }

  /**
   * Returns the sed host name
   */
  std::string
  getSeDName() const;

  /**
   * Returns the request id
   */
  unsigned long
  getReqId() const {
    return myReqID;
  }

  /**
   * Returns a description of this object (for events)
   */
  virtual std::string
  toString() const;

  /**
   * Run method
   */
  virtual void *
  run();

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
