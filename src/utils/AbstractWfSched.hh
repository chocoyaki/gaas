/****************************************************************************/
/* Workflow scheduler (Abstract class)                                      */
/* This abstract class must be implemented to write other workflow          */ 
/* schedulers                                                               */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2006/07/10 11:15:29  aamar
 * Adding the rescheduling management
 *
 * Revision 1.1  2006/04/14 14:03:17  aamar
 * Abstract class that defines the workflow scheduler interface.
 *
 ****************************************************************************/

#ifndef _ABSTRACTWFSCHED_HH_
#define _ABSTRACTWFSCHED_HH_

#include <pthread.h>
#include <semaphore.h>

#include "response.hh"

#include "Dag.hh"
#include "Thread.hh"
#include "CltReoMan_impl.hh"

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
   * set the client reordering manager
   */
  void setCltReoMan(CltReoMan_impl * crm);

  /**
   * Rescheduling methods
   */
  virtual void
  reSchedule();

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
  wf_response_t * response;

  /**
   * Client Reordering Manager
   */
  CltReoMan_impl * myCltReoMan;

  /**
   * Scheduler semaphore 
   * used for rescheduling suspend/resume
   */
  sem_t mySem;
private:
  /*********************************************************************/
  /* private methods                                                    */
  /*********************************************************************/
};

#endif   /* not defined _ABSTRACTWFSCHED_HH */
