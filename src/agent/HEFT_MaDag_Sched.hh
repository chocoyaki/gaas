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

  virtual wf_node_sched_seq_t * 
  schedule(const wf_response_t * wf_response,
	   WfReader& reader);

  virtual wf_node_sched_seq_t * 
  reSchedule(const wf_response_t * wf_response,
	     WfReader& reader);

private:
};


#endif   /* not defined _HEFT_MADAG_SCHED_HH */



