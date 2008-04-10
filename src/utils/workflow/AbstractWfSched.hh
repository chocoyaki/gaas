/****************************************************************************/
/* Workflow scheduler (Abstract class)                                      */
/* This abstract class must be implemented to write other workflow          */
/* schedulers                                                               */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _ABSTRACTWFSCHED_HH_
#define _ABSTRACTWFSCHED_HH_

#include <map>
#include <pthread.h>
#include <semaphore.h>

#include "response.hh"

#include "Dag.hh"
#include "Thread.hh"
#include "CltReoMan_impl.hh"
#include "SeDSynchro.hh"

class CltReoMan_impl;

class AbstractWfSched {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
  AbstractWfSched();

  virtual 
  ~AbstractWfSched();

  /**
   * Workflow execution method; implemented by scheduler subclass *
   */
  virtual void
  execute() = 0;

  /**
   * Assign a dag to the scheduler *
   * implicitely called by the runtime *
   */
  void
  setDag (Dag * dag);

  /**
   * Set the workflow submission response
   * implicitely called by the runtime (when the client ask the MA directly)
   */
  void 
  setResponse(wf_response_t * response);

  /**
   * Another version of the above function *
   * Used when we use the MA DAG
   */
  void 
  setResponse(wf_sched_response_t * response);
  
  /**
   * set the client reordering manager
   */
  void setCltReoMan(CltReoMan_impl * crm);

  /**
   * Rescheduling methods
   */
  virtual void
  reSchedule();

  /**
   * Get the ready nodes for execution
   * 
   * @param dag Dag reference
   * @param readyNodes Vector that will contain the ready nodes  
   */
  
  virtual void
  getReadyNodes(Dag*& dag, std::vector<Node * >& readyNodes) = 0;
  
  /**
   * Get access to a SeD
   */
  static void
  getAccess(std::string hostName);

  /**
   * Release a SeD
   * 
   * @param node_id Identifier of the node in the top of a SeDSynchro object
   */
  static void
  pop(std::string node_id);

  /**
   * Get the nodes in the top of each SeD allocation queue
   * 
   * @param topQueues Vector that will contain nodes identifiers in the top of queues
   */
  static void
  getTopSeDQueues(std::vector<std::string>& topQueues);
  
  /**
   * Put the node in the SeD queue
   * 
   * @param node_id  Node identifier
   * @param hostName SeD hostname
   */
  static void
  allocateToSeD(std::string node_id, std::string hostName);

protected:
  /*********************************************************************/
  /* protected fields                                                  */
  /*********************************************************************/
  /**
   * Dag reference *
   */
  Dag * 
  myDag;

  /**
   * Workflow submission response *
   */
  wf_response_t response;

  /**
   * Client Reordering Manager
   */
  CltReoMan_impl * myCltReoMan;

  /**
   * Scheduler semaphore 
   * used for rescheduling suspend/resume
   */
  sem_t mySem;

  /**
   * Map of Synchronisation objects
   */
  static std::map<std::string, SeDSynchro> mySeDSynchro;


private:
  /*********************************************************************/
  /* private methods                                                    */
  /*********************************************************************/
};

#endif   /* not defined _ABSTRACTWFSCHED_HH */
