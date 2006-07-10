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
#include "WfReader.hh"

class MaDag_sched {
public:  
  MaDag_sched();

  virtual ~MaDag_sched();

  virtual wf_node_sched_seq_t * 
  schedule(const wf_response_t * wf_response,
	   WfReader& reader) = 0;

  virtual wf_node_sched_seq_t * 
  reSchedule(const wf_response_t * wf_response,
	     WfReader& reader) = 0;
};

#endif
