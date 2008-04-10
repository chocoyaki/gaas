/****************************************************************************/
/* The CORBA implementation of client reordering object                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.4  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 ****************************************************************************/

#include "CltReoMan_impl.hh"
#include "ORBMgr.hh"
#include "debug.hh"
#include "WfParser.hh"

CltReoMan_impl::CltReoMan_impl(const char * name, const MasterAgent_var& MA) :
  myId(name),
  myThread(NULL),
  myMA(MA) {
  this->enable = false;
  this->myDag = NULL;
  this->myWfSched = NULL;
}

void
CltReoMan_impl::activate() {
  ORBMgr::activate(this);
  
  /* Bind the Client Reordering Manager to its name 
     in the CORBA Naming Service */
  /*
  if (ORBMgr::bindObjToName(_this(), 
			    ORBMgr::CLT_REO_MAN, 
			    this->myId.c_str())) {
    cout << "could not declare myself as " << this->myId << endl;
  }
  else {
    cout << "binding Client Reordering Manager "<< 
      this->myId << " to naming service" << endl;
  }
  */
}

CltReoMan_impl::~CltReoMan_impl() {
  if (myThread != NULL)
    delete (myThread);
}

void
CltReoMan_impl::remainingSched(const wf_node_sched_seq_t& wf_sched) {
  // this->myDag->setSchedResponse(&((wf_node_sched_seq_t)(wf_sched)));
}

char *
CltReoMan_impl::getremainingDag() {
  if (this->myDag == NULL) {
    WARNING ("Dag is NULL" << endl);
    return CORBA::string_dup("");
  }
  return CORBA::string_dup(this->myDag->toXML().c_str());
}

void
CltReoMan_impl::ping() {
  cout << "CltReoMan::ping" << endl;
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
  WfParser * tmpReader = new WfParser(myDag->toXML(false).c_str());
  tmpReader->setup();
  
  // create the profile sequence
  // create the profile sequence
  unsigned int len = tmpReader->pbs_list.size();
  corba_pb_desc_seq_t pbs_seq;
  pbs_seq.length(len);
  for (unsigned int ix=0; ix< len; ix++) {
    pbs_seq[ix] = tmpReader->pbs_list[ix];
  }

  unsigned int dagSize= tmpReader->getDagSize();
  // delete the temporary reader
  delete (tmpReader);

  // call the master agent
  // and send the workflow description
  cout << "Try to send the workflow description (as a profile list) to" <<
    " the master agent ...";
  if (myMA) {
    //    response = MA->submit_wf(*corba_profile);
    response = myMA->submit_pb_set(pbs_seq, dagSize, true);
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

/**
 * get if the reordering is enabled in the client side
 */
bool
CltReoMan_impl::isEnabled() {
  return this->enable;
}

/**
 * Enable/Disable the reordering (disabled by default)
 */
void 
CltReoMan_impl::setEnable(bool b) {
  this->enable = b;
}

/**
 * Return the IOR of the CltReoMan
 */
const char *
CltReoMan_impl::getRef() {
  return ORBMgr::getIORString(_this());
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

