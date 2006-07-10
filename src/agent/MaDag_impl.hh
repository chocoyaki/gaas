/****************************************************************************/
/* [Documentation Here!]                                                    */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2006/07/10 10:00:00  aamar
 * - Adding the function remainingDag to the MA DAG interface
 * - Round Robbin and HEFT scheduling
 *
 * Revision 1.1  2006/04/14 13:43:33  aamar
 * header of the MA DAG CORBA object.
 *
 ****************************************************************************/

#ifndef _MADAG_IMPL_HH_
#define _MADAG_IMPL_HH_

#include <string>

#include "MaDag.hh"
#include "MasterAgent.hh"
#include "MaDag_sched.hh"

typedef enum {
  round_robbin_sched,
  heft_sched
} madag_sched_t;

class MaDag_impl : public POA_MaDag,
		   public PortableServer::RefCountServantBase {
public:
  MaDag_impl(const char * name);

  virtual ~MaDag_impl();

  /** Workflow submission function. */
  virtual wf_node_sched_seq_t * 
  submit_wf (const corba_wf_desc_t& wf_desc);

  /**
   * Another workflow submission function *
   * return only the ordering of the nodes execution *
   */
  virtual wf_ordering_t *
  get_wf_ordering(const corba_wf_desc_t& wf_desc);

  /**
   * set the remaining dag
   */
  virtual void
  remainingDag(const char * dag_descr);

  /**
   * set the scheduler for the MA DAG
   */
  void
  set_sched(const madag_sched_t madag_sched);

private:
  std::string myName;
  MasterAgent_var parent;
  MaDag_sched * mySched;
};


#endif   /* not defined _MADAG_IMPL_HH */



