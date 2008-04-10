/****************************************************************************/
/* Basic Workflow Scheduler                                                 */
/* This scheduler use only the data dependencies to execute the workflow    */
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
 * Revision 1.1  2006/04/14 14:04:51  aamar
 * The basis scheduler (random scheduling).
 *
 ****************************************************************************/

#ifndef _SIMPLEWFSCHED_HH_
#define _SIMPLEWFSCHED_HH_

#include "AbstractWfSched.hh"

class BasicWfSched : public AbstractWfSched {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
  BasicWfSched();

  virtual
  ~BasicWfSched();

  /**
   * The scheduler execution method *
   * implicitely called by the runtime *
   */
  virtual 
  void execute();
  
  /**
   * Get the ready nodes for execution
   * 
   * @param dag Dag reference
   * @param readyNodes Map that will contain the ready nodes  
   */
  virtual void 
  getReadyNodes(Dag*& dag, std::vector<Node *>& readyNodes);
  
private:
};


#endif   /* not defined _SIMPLEWFSCHED_HH */



