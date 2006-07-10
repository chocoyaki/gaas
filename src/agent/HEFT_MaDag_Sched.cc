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

#include "HEFT_MaDag_Sched.hh"
#include "WfExtReader.hh"

HEFT_MaDag_Sched::HEFT_MaDag_Sched() {
}

HEFT_MaDag_Sched::~HEFT_MaDag_Sched() {
}

wf_node_sched_seq_t * 
HEFT_MaDag_Sched::schedule(const wf_response_t * wf_response,
			   WfReader& reader) {
  return NULL;
}

wf_node_sched_seq_t * 
HEFT_MaDag_Sched::reSchedule(const wf_response_t * wf_response,
			     WfReader& reader) {
  return NULL;
}

