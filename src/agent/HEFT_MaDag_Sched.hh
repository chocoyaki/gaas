/****************************************************************************/
/* The MA DAG HEFT scheduler                                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id:
 * $Log:
 ****************************************************************************/

#ifndef _HEFT_MADAG_SCHED_HH_
#define _HEFT_MADAG_SCHED_HH_

#include "MaDag_sched.hh"

class HEFT_MaDag_Sched : public MaDag_sched {
public:
  HEFT_MaDag_Sched();

  virtual ~HEFT_MaDag_Sched();

  virtual wf_node_sched_seq_t 
  schedule(const wf_response_t * wf_response,
	   WfExtReader& reader);

  virtual wf_node_sched_seq_t 
  schedule(const wf_response_t * wf_response,
	   Dag * dag);

  virtual wf_node_sched_seq_t 
  reSchedule(const wf_response_t * wf_response,
	     WfExtReader& reader);

  virtual wf_node_sched_seq_t 
  reSchedule(const wf_response_t * wf_response,
	     Dag* dag);

  virtual wf_node_sched_t
  schedule(const wf_response_t * response, 
	   Node * n);

  virtual double
  getAFT(string nodeId);
private:
  /**
   * rank the node upward 
   * @param n the top node to rank.
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


#endif   /* not defined _HEFT_MADAG_SCHED_HH */



