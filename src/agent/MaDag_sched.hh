/****************************************************************************/
/* The MA DAG scheduler interface                                           */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id: 
 * $Log: 
 ****************************************************************************/

#ifndef MADAG_SCHED_HH
#define MADAG_SCHED_HH

#include "response.hh"
#include "WfExtReader.hh"

class MaDag_sched {
public:  
  MaDag_sched();

  virtual ~MaDag_sched();

  virtual wf_node_sched_seq_t
  schedule(const wf_response_t * wf_response,
	   WfExtReader& reader) = 0;

  virtual wf_node_sched_seq_t
  schedule(const wf_response_t * wf_response,
	   Dag * dag) = 0;

  virtual wf_node_sched_seq_t 
  reSchedule(const wf_response_t * wf_response,
	     WfExtReader& reader) = 0;

  virtual wf_node_sched_seq_t 
  reSchedule(const wf_response_t * wf_response,
	     Dag * dag) = 0;

  virtual wf_node_sched_t
  schedule(const wf_response_t * response, 
	   Node * n) = 0;

  virtual double
  getAFT(string nodeId) = 0;

};

#endif
