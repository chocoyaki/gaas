/****************************************************************************/
/* DIET master agent implementation source code                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2003/05/10 08:53:34  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.1  2003/04/10 13:01:32  pcombes
 * Replace MasterAgent_impl.cc. Apply CS. Use ChildID, NodeDescription, Parsers,
 * Schedulers and TRACE_LEVEL. Update submit.
 * Multi-MA parts are still to be updated.
 ****************************************************************************/

#include "MasterAgentImpl.hh"
#include "debug.hh"
#include "Parsers.hh"

#include <iostream>
using namespace std;
#include <stdio.h>


extern unsigned int TRACE_LEVEL;


MasterAgentImpl::MasterAgentImpl() : AgentImpl()
{
  reqIDCounter = 0;
} // MasterAgentImpl

MasterAgentImpl::~MasterAgentImpl()
{
#if HAVE_MULTI_MA
  MAList.emptyIt();
#endif // HAVE_MULTI_MA
} // MasterAgentImpl::~MasterAgentImpl()


/**
 * Launch this agent (initialization + registration in the hierarchy).
 */
int
MasterAgentImpl::run()
{
  int res = this->AgentImpl::run();

  if (res)
    return res;
#if HAVE_MULTI_MA
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "Getting MAs references ...\n";

  updateRefs();

  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "Getting MAs references ... done.\n";
#endif // HAVE_MULTI_MA

  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "\nMaster Agent " << this->myName << " started.\n\n";

  return 0;
} // run()




/****************************************************************************/
/* Submission                                                               */
/****************************************************************************/

/** Problem submission : remotely called by client. */
corba_response_t*
MasterAgentImpl::submit(const corba_pb_desc_t& pb_profile,
			CORBA::ULong maxServers)
{
  corba_request_t   creq;
  Request*          req(NULL);
  corba_response_t* resp(NULL);

  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "MA::submit(" << pb_profile.path <<", " << maxServers << ")\n";

  /* Initialize the corba request structure */
  creq.reqID = reqIDCounter++; // thread safe
  creq.pb = pb_profile;

  /* Initialize the request with a global scheduler */
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "Initialize the request " << creq.reqID << ".\n";
  req = new Request(&creq, GlobalScheduler::chooseGlobalScheduler(&creq));
		    
  /* Forward request and schedule the responses */
  resp = findServer(req, maxServers);

  // Constructor initializes sequences with length == 0
  if ((resp != NULL) && (resp->servers.length() != 0)) {
    
    resp->servers.length(MIN(resp->servers.length(), maxServers));
    if (TRACE_LEVEL >= TRACE_ALL_STEPS)
      cout << "Decision signaled" << endl;

  } else if (TRACE_LEVEL >= TRACE_MAIN_STEPS) {
    cout << "No server found for problem " << creq.pb.path << ".\n";
  }
	      
  reqList[creq.reqID] = NULL;
  delete req;
  
  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "************************************************************\n";
  return resp;
} // submit(const corba_pb_desc_t& pb, ...)



#if HAVE_MULTI_MA

/* Update MAs references */
void
MasterAgentImpl::updateRefs()
{
  dietMADescListIterator* iter = new dietMADescListIterator(knownMAs);
  CORBA::Object_var obj;

  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "MA::updateRefs()\n";

  while (iter->next()) {
    if (TRACE_LEVEL >= TRACE_ALL_STEPS)
      cout << "Resolving " << ((dietMADescListElt *)(iter->curr()))->MA.name
	   << "...";
    obj = getAgentReference(((dietMADescListElt *)(iter->curr()))->MA.name);
    if (CORBA::is_nil(obj)) {
      if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	cout << " Not found" << endl;
    } else {
      if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	cout << " Found" << endl;
      ((dietMADescListElt *)(iter->curr()))->MA.ior = Agent::_narrow(obj);
      if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	cout << "Shaking hand..." << endl;
      try {
	/* retirer par flemme de changer dietMADescListElt */
	/*	(((dietMADescListElt *)
		(iter->curr()))->MA.ior->handShake(_this(),myName);*/
	if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	  cout << "Success" << endl;
      }
      catch (CORBA::COMM_FAILURE& ex) {
	if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	  cout << "obsolete reference" << endl;
	((dietMADescListElt *)(iter->curr()))->MA.ior = Agent::_nil();
      }
    }

  }
} // updateRefs()



/****************************************************************************/
/* MAs handshake                                                            */
/****************************************************************************/

CORBA::Long
MasterAgentImpl::handShake(MasterAgent_ptr me, const char* myName)
{
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << myName << " is shaking my hand" << endl;

  bool MAFound = false;
  MAList::Iterator* iter = knownMAs.getIterator();

  while (!MAFound && iter->hasCurrent()) {
    if (!strcmp(iter->getCurrent().getName(), myName)) {
      iter->setCurrent(MADescription(MasterAgent::_duplicate(me), myName));
      if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	cout << "Reference updated" << endl;
      MAFound = true;
    }
  }
  delete(iter);

  if(!MAFound)
    knownMAs.addElement(MADescription(MasterAgent::_duplicate(me), myName));

  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "Reference created" << endl;

  if (MAFound)
    return 0;
  else
    return 1;
} // handShake(MasterAgent_ptr me, const char* myName)

#endif // HAVE_MULTI_MA
