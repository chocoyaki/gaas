/****************************************************************************/
/* The MA DAG round robbin scheduler                                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id:
 * $Log:
 ****************************************************************************/

#ifndef _ROUNDROBBIN_MADAG_SCHED_HH_
#define _ROUNDROBBIN_MADAG_SCHED_HH_

#include "MaDag_sched.hh"

class RoundRobbin_MaDag_Sched : public MaDag_sched {
public:
  RoundRobbin_MaDag_Sched();

  virtual ~RoundRobbin_MaDag_Sched();

  virtual wf_node_sched_seq_t * 
  schedule(const wf_response_t * wf_response,
	   WfReader& reader);

  virtual wf_node_sched_seq_t * 
  reSchedule(const wf_response_t * wf_response,
	     WfReader& reader);

private:
};


#endif   /* not defined _ROUNDROBBIN_MADAG_SCHED_HH */


