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
 * Revision 1.1  2006/04/14 13:43:33  aamar
 * header of the MA DAG CORBA object.
 *
 ****************************************************************************/

#ifndef _MADAG_IMPL_HH_
#define _MADAG_IMPL_HH_

#include "MaDag.hh"
#include "MasterAgent.hh"
#include <string>

class MaDag_impl : public POA_MaDag  {
public:
  MaDag_impl(const char * name);

  /** Workflow submission function. */
  virtual wf_node_sched_seq_t * 
  submit_wf (const corba_wf_desc_t& wf_desc);

  /**
   * Another workflow submission function *
   * return only the ordering of the nodes execution *
   */
  virtual wf_ordering_t *
  get_wf_ordering(const corba_wf_desc_t& wf_desc);


private:
  std::string myName;
  MasterAgent_var parent;
};


#endif   /* not defined _MADAG_IMPL_HH */



