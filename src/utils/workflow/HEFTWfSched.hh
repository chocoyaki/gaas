/****************************************************************************/
/* HEFT Workflow Scheduler                                                  */
/* This scheduler use the HEFT heuristic to execute the workflow            */
/* The order between ready nodes is random                                  */
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
 * Revision 1.2  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 ****************************************************************************/

#ifndef _HEFT_SCHED_HH_
#define _HEFT_SCHED_HH_

#include "AbstractWfSched.hh"
#include "SeD.hh"

class HEFTWfSched : public AbstractWfSched {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
  HEFTWfSched();

  virtual
  ~HEFTWfSched();

  /**
   * The scheduler execution method *
   * implicitely called by the runtime *
   */
  virtual 
  void execute();

  /**
   * Rescheduling methods
   */
  virtual void
  reSchedule();

  void
  init();
  
  /**
   * Get the ready nodes for execution
   * 
   * @param dag Dag reference
   * @param readyNodes Map that will contain the ready nodes  
   */
  virtual void 
  getReadyNodes(Dag*& dag, std::vector<Node *>& readyNodes);

private:
  /**
   * rank the node upward 
   */
  void
  rank(Node * n);

  // the average wi
  map<string, double> WI;
  // the availabilty of SeD
  map<string, double> avail;
  // AFT and AST map
  map<string, double> AFT;
  map<string, double> AST;
};


#endif   /* not defined _HEFT_WFSCHED_HH */



