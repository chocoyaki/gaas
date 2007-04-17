/****************************************************************************/
/* The base abstract class for multi-workflow scheduler                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2007/04/17 13:34:54  ycaniou
 * Error in debug.tex header
 * Removes some warnings during doc generation
 *
 * Revision 1.3  2006/11/27 09:53:00  aamar
 * Correct headers of source files used in workflow support.
 *
 ****************************************************************************/

#ifndef _ABSWFMETASCHED_HH_
#define _ABSWFMETASCHED_HH_

#include "MaDag_sched.hh"
#include "MetaDag.hh"
#include "CltReoMan.hh"

class AbsWfMetaSched {
public:
  AbsWfMetaSched();
  
  virtual ~AbsWfMetaSched();

  /**
   * set the scheduler used by the MA DAG
   * @param sched the base scheduler to be used
   */
  virtual void 
  setSched(MaDag_sched * sched) = 0;

  /**
   * Workflow submission function. 
   * @param wf_desc workflow string description
   * @param dag_id the dag ID
   */
  virtual wf_sched_response_t * 
  submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
	     MasterAgent_var parent,
	     const bool used) = 0;

  /**
   * Add a new client reference
   * @param clt the new client.
   * @param dag the dag identifier.
   */
  virtual void
  addClient(CltReoMan_var clt, const string dag);

  /**
   * Remove a client from myClients map
   * @param dag id
   */
  virtual void
  removeClient(const string dag);

  /**
   * Set the node state as done
   * @param dagId the DAG identifier
   * @param nodeId the node identifier in the DAG
   */
  virtual void 
  setNodeAsDone(const char* dagId, const char* nodeId);

protected:

  /**
   * A map of dag with corresponding client ref
   */
  std::map<string, CltReoMan_var> myClients;

  /**
   * The Meta-Dag
   */
  MetaDag * myMetaDag;
  

  /**
   * The Wf meta-scheduler scheduler
   */
  MaDag_sched * mySched; 

  /**
   * Nodes state 
   * true if node is done
   * false otherwise
   */
  map<Node *, bool> nodesStates;

protected:
  /**
   * extract only the response for the specified dag
   * @param dag_id the dag identifier
   * @param wf_resp the global scheduling for all the meta-dag
   */
  static wf_sched_response_t * 
  extract(int dag_id, wf_sched_response_t * wf_resp);
};


#endif   /* not defined _ABSWFMETASCHED_HH */



