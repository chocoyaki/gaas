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
 * Revision 1.1  2006/04/14 14:03:17  aamar
 * Abstract class that defines the workflow scheduler interface.
 *
 ****************************************************************************/

#ifndef _ABSTRACTWFSCHED_HH_
#define _ABSTRACTWFSCHED_HH_

#include "response.hh"

#include "Dag.hh"
#include "Thread.hh"

class AbstractWfSched {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
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
   * Set the workflow submission response *
   * implicitely called by the runtime *
   */
  void 
  setResponse(wf_response_t * response);
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
private:
  /*********************************************************************/
  /* private methods                                                    */
  /*********************************************************************/
};

#endif   /* not defined _ABSTRACTWFSCHED_HH */
