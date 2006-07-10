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

#include "CltReoMan_impl.hh"
#include "ORBMgr.hh"
#include "debug.hh"
#include "WfReader.hh"

CltReoMan_impl::CltReoMan_impl(const char * name, const MasterAgent_var& MA) :
  myId(name),
  myThread(NULL),
  myMA(MA) {
}

CltReoMan_impl::~CltReoMan_impl() {
  if (myThread != NULL)
    delete (myThread);
}

void
CltReoMan_impl::remainingSched(const wf_node_sched_seq_t& wf_sched) {
}

void
CltReoMan_impl::getremainingDag() {
}

/**
 * set the Dag reference
 * called by the DIET_client library when the reordering mechanism
 * is activated
 */
void 
CltReoMan_impl::setDag(Dag * dag) {
  this->myDag = dag;
}

/**
 * set the used scheduler reference
 * called by the DIET_client library when the reordering mechanism
 * is activated
 */
void 
CltReoMan_impl::setScheduler(AbstractWfSched * sched) {
  this->myWfSched = sched;
}

/**
 * start the ORB and activate the Client Reordering Manager
 */
void 
CltReoMan_impl::activate() {
  myThread = new ReoManThread(this);
  myThread->start();
}


void
CltReoMan_impl::done() {
  if (myThread != NULL) {
    delete (myThread);
    myThread = NULL;
  }
}


void 
CltReoMan_impl::reSchedule() {
  wf_response_t * response = NULL;
  cout << "The ReoMan tries to reschedule to following sub dag" << endl;
  cout << myDag->toXML(false).c_str() << endl;
  WfReader * tmpReader = new WfReader(myDag->toXML(false).c_str());
  tmpReader->setup();
  
  // create the profile sequence
  corba_pb_desc_seq_t pb_seq;
  unsigned int len = 0;
  tmpReader->pbReset();
  while (tmpReader->hasPbNext()) {
    len ++;
    pb_seq.length(len);
    pb_seq[len-1] = *(tmpReader->pbNext());
  }

  // delete the temporary reader
  delete (tmpReader);

  // call the master agent
  // and send the workflow description
  cout << "Try to send the workflow description (as a profile list) to" <<
    " the master agent ...";
  if (myMA) {
    //    response = MA->submit_wf(*corba_profile);
    response = myMA->submit_pb_set(pb_seq);
    cout << " done" << endl;
    if (! response->complete) {
      cout << "One ore more services are missing" << endl
	   << "The Workflow cannot be executed" << endl;
      return;
    }
  }
  else {
    // 
    cout << " The MA is unavailable !!! " << endl;
    return;
  }

  cout<< "Received response length " << response->wfn_seq_resp.length()
      << endl;

  myWfSched->setResponse(response);
  myWfSched->reSchedule();
}


void *
CltReoMan_impl::ReoManThread::run() {
  ORBMgr::activate((CltReoMan_impl*)myParent);
  
  /* Bind the Client Reordering Manager to its name 
     in the CORBA Naming Service */
  if (ORBMgr::bindObjToName(myParent->_this(), 
			    ORBMgr::CLT_REO_MAN, 
			    myParent->myId.c_str())) {
    cout << "could not declare myself as " << myParent->myId << endl;
  }
  else {
    cout << "binding Client Reordering Manager "<< 
      myParent->myId << " to naming service" << endl;
  }
      
  /* Wait for RPCs (blocking call): */
  if (ORBMgr::wait()) {
    WARNING("Error while exiting the ORBMgr::wait() function");
  }
      
  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  //  ORBMgr::destroy();

  return NULL;
}

CltReoMan_impl::ReoManThread::ReoManThread(CltReoMan_impl * parent) : 
  Thread(true),
  myParent(parent) {
}

