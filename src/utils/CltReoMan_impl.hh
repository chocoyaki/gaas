/****************************************************************************/
/* The CORBA implementation of client reordering object                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id: 
 * $Log: 
 ****************************************************************************/

#ifndef _CLTREOMAN_IMPL_HH_
#define _CLTREOMAN_IMPL_HH_

#include "CltReoMan.hh"
#include "AbstractWfSched.hh"
#include "Dag.hh"
#include "Thread.hh"
#include <string>
#include "MasterAgent.hh"

class AbstractWfSched;

class CltReoMan_impl : public POA_CltReoMan,
		       public PortableServer::RefCountServantBase
{
  class ReoManThread : public Thread {
  public:
    ReoManThread(CltReoMan_impl * parent);  

    virtual void *
    run();
  private:
    CltReoMan_impl * myParent;
  };

public:
  CltReoMan_impl(const char * name, const MasterAgent_var& MA);

  virtual ~CltReoMan_impl();

  virtual void
  remainingSched(const wf_node_sched_seq_t& wf_sched);
  
  virtual char *
  getremainingDag();

  virtual void
  ping();

  /**
   * set the Dag reference
   * called by the DIET_client library when the reordering mechanism
   * is activated
   */
  void 
  setDag(Dag * dag);

  /**
   * set the used scheduler reference
   * called by the DIET_client library when the reordering mechanism
   * is activated
   */
  void 
  setScheduler(AbstractWfSched * sched);

  /**
   * start the ORB and activate the Client Reordering Manager
   */
  void 
  activate();

  void
  done();

  /**
   * Rescheduling methods
   */
  void
  reSchedule();

  /**
   * get if the reordering is enabled in the client side
   */
  bool
  isEnabled();

  /**
   * Enable/Disable the reordering (disabled by default)
   */
  void 
  setEnable(bool b);

  /**
   * Return the IOR of the CltReoMan
   */
  const char *
  getRef();

private:
  /**
   * Client Reordering Manager ID
   */
  std::string myId;

  /**
   * The DAG reference
   */
  Dag * myDag;

  /**
   * The used scheduler reference
   */
  AbstractWfSched * myWfSched;

  /**
   * The thread responible of starting ORB
   */
  ReoManThread * myThread;

  /**
   * The Master Agent reference
   */
  MasterAgent_var myMA;

  /**
   * enabled or not the reordering in the client side
   */
  bool enable;
};


#endif   /* not defined _CLTREOMAN_IMPL_HH */



