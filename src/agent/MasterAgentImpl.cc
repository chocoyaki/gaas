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
 * Revision 1.10  2004/03/01 18:46:08  rbolze
 * add logservice
 *
 * Revision 1.9  2004/02/27 10:25:11  bdelfabr
 * methods for data id creation and  methods to retrieve data descriptor are added
 *
 * Revision 1.8  2003/09/28 22:06:11  ecaron
 * Take into account the new API of statistics module
 *
 * Revision 1.7  2003/09/22 21:19:49  pcombes
 * Set all the modules and their interfaces for data persistency.
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
#include "statistics.hh"
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
  this->reqIDCounter = 0;
  this->num_session = 0;
  this->num_data = 0;
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
  /* num_session thread safe*/
  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "\nMaster Agent " << this->myName << " started.");
  return 0;
} // run(char* configFileName)



/**
 * Returns the identifier of a data by agreggation of numsession and numdata
 */
char * 
MasterAgentImpl::get_data_id()
{
  char* id = new char[100];
  (this->num_data)++;
  sprintf(id,"id.%s.%d.%d",myName,(int)(num_session), (int)(num_data));
  return CORBA::string_dup(id);
} // get_data_id()
 
                                                                                                       
 

/****************************************************************************/
/* Submission                                                               */
/****************************************************************************/

/**
 * Invoke Loc Manager method to get data presence information (call by client)
 */
CORBA::ULong 
MasterAgentImpl::dataLookUp(const char* argID){
  if(locMgr->dataLookUp(strdup(argID))==0)
    return 0;
  else
    return 1;
} // dataLookUp(const char* argID)

/**
 * invoke loc Manager method to get data descriptor of the data identified by argID 
 */
corba_data_desc_t* 
MasterAgentImpl::get_data_arg(const char* argID)
{
  corba_data_desc_t* resp = new corba_data_desc_t;
   resp = locMgr->set_data_arg(argID);  
   return resp;
}

/** Problem submission : remotely called by client. */
corba_response_t*
MasterAgentImpl::submit(const corba_pb_desc_t& pb_profile,
			CORBA::ULong maxServers)
{
  corba_request_t   creq;
  Request*          req(NULL);
  corba_response_t* resp(NULL);

  MA_TRACE_FUNCTION(pb_profile.path <<", " << maxServers);

  /* Initialize statistics module */
  stat_init();
  stat_in(this->myName,"start request");

  /* Initialize the corba request structure */
  creq.reqID = reqIDCounter++; // thread safe
 
  creq.pb = pb_profile;

#if HAVE_LOGSERVICE
  if (dietLogComponent!=NULL) {
    dietLogComponent->logAskForSeD(&creq);
  }
#endif
  /* Initialize the request with a global scheduler */
  TRACE_TEXT(TRACE_ALL_STEPS, "Initialize the request " << creq.reqID << ".\n");	    
  req = new Request(&creq, GlobalScheduler::chooseGlobalScheduler(&creq));

  /* Forward request and schedule the responses */

  resp = findServer(req, maxServers);

  resp->myID = (ChildID) -1;
  // Constructor initializes sequences with length == 0
  if ((resp != NULL) && (resp->servers.length() != 0)) {
    resp->servers.length(MIN(resp->servers.length(), maxServers));
    TRACE_TEXT(TRACE_ALL_STEPS, "Decision signaled.\n");
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS,
	       "No server found for problem " << creq.pb.path << ".\n");
  }
  
  reqList[creq.reqID] = NULL;

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logSedChosen(&creq, resp);
  }
#endif

  delete req;
  
  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "**************************************************\n");
  stat_out(this->myName,"stop request");

  return resp;
} // submit(const corba_pb_desc_t& pb, ...)


CORBA::Long 
MasterAgentImpl::get_session_num()
{
 (this->num_session)++;
  return num_session; 

}//get_session_num()

/**
 invoked by client : frees persistent data identified by argID, if not exists return NULL
*/
CORBA::Long
MasterAgentImpl::diet_free_pdata(const char* argID)
{
  if(this->dataLookUp(ms_strdup(argID)) == 0) {
    locMgr->rm_pdata(ms_strdup(argID));
    return 1;
  }
  else 
    return 0;
} //diet_free_pdata(const char* argID)


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
      TRACE_TEXT(TRACE_ALL_STEPS," found \n");
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
