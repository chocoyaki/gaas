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
 * Revision 1.6  2003/09/18 09:47:19  bdelfabr
 * adding data persistence
 *
 * Revision 1.4  2003/07/04 09:47:59  pcombes
 * Use new ERROR, WARNING and TRACE macros.
 *
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

//#define aff_val(x)
#define aff_val(x) cout << #x << " = " << x << endl;

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define MA_TRACE_FUNCTION(formatted_text)       \
  TRACE_TEXT(TRACE_ALL_STEPS, "MA::");          \
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)

MasterAgentImpl::MasterAgentImpl() : AgentImpl()
{
  reqIDCounter = 0;
  *fatherName = '\0';
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
  TRACE_TEXT(TRACE_ALL_STEPS, "Getting MAs references ...\n");
  updateRefs();
  TRACE_TEXT(TRACE_ALL_STEPS, "Getting MAs references ... done.\n");
#endif // HAVE_MULTI_MA
  
  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "\nMaster Agent " << this->myName << " started.\n\n");
  return 0;
} // run(char* configFileName)




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

  MA_TRACE_FUNCTION(pb_profile.path <<", " << maxServers);
  /* Initialize the corba request structure */
  creq.reqID = reqIDCounter++; // thread safe
  creq.pb = pb_profile;

  /* Initialize the request with a global scheduler */
  TRACE_TEXT(TRACE_ALL_STEPS, "Initialize the request " << creq.reqID << ".\n");	    
  req = new Request(&creq, GlobalScheduler::chooseGlobalScheduler(&creq));

  /* Forward request and schedule the responses */
  resp = findServer(req, maxServers);
  // Constructor initializes sequences with length == 0
  if ((resp != NULL) && (resp->servers.length() != 0)) {
    resp->servers.length(MIN(resp->servers.length(), maxServers));
    TRACE_TEXT(TRACE_ALL_STEPS, "Decision signaled.\n");
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS,
	       "No server found for problem " << creq.pb.path << ".\n");
  }
  
  reqList[creq.reqID] = NULL;
  delete req;
  
   TRACE_TEXT(TRACE_MAIN_STEPS,
	     "**************************************************\n");
  return resp;
} // submit(const corba_pb_desc_t& pb, ...)



#if HAVE_MULTI_MA

/* Update MAs references */
void
MasterAgentImpl::updateRefs()
{
  dietMADescListIterator* iter = new dietMADescListIterator(knownMAs);
  CORBA::Object_var obj;

  MA_TRACE_FUNCTION();

  TRACE_TEXT(TRACE_ALL_STEPS, "Resolving "
	       << ((dietMADescListElt *)(iter->curr()))->MA.name << "...");
    obj = getAgentReference(((dietMADescListElt *)(iter->curr()))->MA.name);
    if (CORBA::is_nil(obj)) {
      TRACE_TEXT(TRACE_ALL_STEPS, " not found.\n");
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS,
	cout << " found" << endl;
      ((dietMADescListElt *)(iter->curr()))->MA.ior = Agent::_narrow(obj);
      TRACE_TEXT(TRACE_ALL_STEPS, "Shaking hand ...");
      try {
	/* retirer par flemme de changer dietMADescListElt */
	/*	(((dietMADescListElt *)
		(iter->curr()))->MA.ior->handShake(_this(),myName);*/
	TRACE_TEXT(TRACE_ALL_STEPS, " OK.\n");
      }
      catch (CORBA::COMM_FAILURE& ex) {
	TRACE_TEXT(TRACE_ALL_STEPS, " obsolete reference.\n");
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
  MA_TRACE_FUNCTION("with " << myName);

  bool MAFound = false;
  MAList::Iterator* iter = knownMAs.getIterator();

  while (!MAFound && iter->hasCurrent()) {
    if (!strcmp(iter->getCurrent().getName(), myName)) {
      iter->setCurrent(MADescription(me, myName));
      TRACE_TEXT(TRACE_ALL_STEPS, "Reference updated.\n")
      MAFound = true;
    }
  }
  delete(iter);

  if(!MAFound)
    knownMAs.addElement(MADescription(MasterAgent::_duplicate(me), myName));

  TRACE_TEXT(TRACE_ALL_STEPS, "Reference created.\n");

  if (MAFound)
    return 0;
  else
    return 1;
} // handShake(MasterAgent_ptr me, const char* myName)

#endif // HAVE_MULTI_MA
