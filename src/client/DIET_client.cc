/****************************************************************************/
/* DIET client interface                                                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*    - Christophe PERA (pera.christophe@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.75  2006/06/16 10:37:32  abouteil
 * Chandra&Toueg&Aguilera fault detector support added
 *
 * Revision 1.74  2006/04/14 14:18:50  aamar
 * Two major modifications for workflow support:
 * a. API extension for workflow support:
 *        diet_error_t diet_wf_call(diet_wf_desc_t* profile);
 *        void set_sched (struct AbstractWfSched * sched);
 *        void diet_wf_free(diet_wf_desc_t * profile);
 *        void set_madag_sched(int b);
 *        int _diet_wf_scalar_get(const char * id, void** value);
 *        int _diet_wf_string_get(const char * id, char** value);
 * b. Modify the diet_initialize function to get the MA_DAG reference
 *    is the entry if set in the client config file.
 *
 * Revision 1.73  2006/01/31 16:43:03  mjan
 * Update on deployment of DIET/JuxMem via GoDIET
 *
 * Revision 1.72  2006/01/31 10:07:40  mjan
 * Update on the API of JuxMem
 *
 * Revision 1.71  2006/01/24 06:33:53  hdail
 * Fix made to code to fix bug whereby on certain systems (like Orsay G5K)
 * client successfully schedules job 3 times in a row but fails at submission
 * each time.  I have commented out contract checking as it was causing the
 * problem, adds overhead to job submission, and has not been really used in
 * DIET for several years.  Can easily be uncommented when someone decides to
 * do research on contract checking.  Fix tested at Orsay.
 *
 * Revision 1.70  2006/01/20 10:14:37  eboix
 *    - Odd references to acDIET_config.h changed to DIET_config.h  --- Injay2461
 *
 * Revision 1.69  2006/01/13 10:40:53  mjan
 * Updating DIET for next JuxMem (0.2)
 *
 * Revision 1.68  2005/11/10 14:37:51  eboix
 *     Clean-up of Cmake/DIET_config.h.in and related changes. --- Injay2461
 *
 * Revision 1.67  2005/09/05 16:07:33  hdail
 * Addition of hostname and location information to client and in submit call.
 *
 * Revision 1.66  2005/08/31 14:49:17  alsu
 * New plugin scheduling interface: new estimation vector interface in
 * the client contract-checking code
 *
 * Revision 1.65  2005/08/30 12:41:37  ycaniou
 * Some changes. Management of dietJobID.
 *
 * Revision 1.64  2005/06/14 16:17:12  mjan
 * Added support of DIET_FILE inside JuxMem-DIET for TLSE code
 *
 * Revision 1.57  2005/05/15 15:44:46  alsu
 * minor changes from estimation vector reorganization
 *
 * Revision 1.56  2005/04/28 13:07:05  eboix
 *     Inclusion of CORBA.h substitued with omniORB4/CORBA.h. --- Injay 2461
 *
 * Revision 1.55  2005/04/27 01:14:26  ycaniou
 * Added the diet_call_batch() and diet_call_async_batch() in the API and
 * modified the respective solve functions if the client wants to perform
 * a batch submission
 ****************************************************************************/


#include "DIET_client.h"

#include <omniORB4/CORBA.h>
#include <iostream>
#include <fstream>
using namespace std;
#include <unistd.h> // For gethostname()
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "DIET_config.h"
#include "debug.hh"
#include "est_internal.hh"
#include "DIET_data_internal.hh"
#if HAVE_FD
#include "fd/fd.h"
#endif
#if HAVE_JUXMEM
#include "JuxMem.hh"
#endif // HAVE_JUXMEM
#include "marshalling.hh"
#include "MasterAgent.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "SeD.hh"
#include "statistics.hh"

#include "CallAsyncMgr.hh"
#include "CallbackImpl.hh"


// for workflow support
#ifdef HAVE_WORKFLOW
#include "MaDag.hh"
#include "WfExtReader.hh"
#include "SimpleWfSched.hh"
static AbstractWfSched * defaultWfSched = NULL;
static WfExtReader * reader = NULL;
static Dag * dag = NULL;
static bool use_ma_dag = false;
static bool use_ma_dag_sched = true;
#endif
//****

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

#ifdef HAVE_FD
#ifndef DIET_DEFAULT_FD_QOS
/* set a default QoS for fault detector 
 * 30s detection time, 60s false detection, 1 month between two errors
 */
#define DIET_DEFAULT_FD_QOS 30.0, 2678400.0, 60.0
#endif

/* Recovery after failure func */
static void diet_call_failure_recover(fd_handle fd)
{

}
#endif /* HAVE_FD */

/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/** The Master Agent reference */
static MasterAgent_var MA = MasterAgent::_nil();
static omni_mutex      MA_MUTEX;

/** Error rate for contract checking */
#define ERROR_RATE 0.1

/** Maximum servers to be answered */
static unsigned long MAX_SERVERS = 10;

/** IOR reference sent to the SeD to let him contact the callback server */
static char* REF_CALLBACK_SERVER;
/** Flag for using the asynchronous API (set at configuration time) */
static size_t USE_ASYNC_API = 1;

#ifdef HAVE_WORKFLOW
/** The MA DAG reference */
static MaDag_var MA_DAG = MaDag::_nil();
#endif

/****************************************************************************/
/* Manage MA name and Session Number for data persistency issue             */
/****************************************************************************/

  char* MA_Name;
  
  int num_Session;

  char file_Name[256];

#if HAVE_ALTPREDICT
  /** Location ID and hostname to be used in performance prediction */
  char *clientHostname;
  char *clientLocationID;
#endif

#if HAVE_JUXMEM
  JuxMem::Wrapper * juxmem;
#endif // HAVE_JUXMEM

/****************************************************************************/
/* Initialize and Finalize session                                          */
/****************************************************************************/

BEGIN_API

diet_error_t
diet_initialize(char* config_file_name, int argc, char* argv[])
{
  char*  MA_name;
  int    res(0);
  int    myargc(0);
  char** myargv(NULL);
  void*  value(NULL);
  char*  userDefName;

#ifdef HAVE_WORKFLOW
  char*  MA_DAG_name(NULL);
#endif // HAVE_WORKFLOW

  MA_MUTEX.lock();
  
  if (!CORBA::is_nil(MA)) {
    WARNING(__FUNCTION__ << ": diet_finalize has not been called");
    MA_MUTEX.unlock();
    return 0;
  }
  
  /* Set arguments for ORBMgr::init */
  if (argc) {
    myargc = argc;
    myargv = (char**)malloc(argc * sizeof(char*));
    for (int i = 0; i < argc; i++)
      myargv[i] = argv[i];
  }
  
  /* Parsing */
  Parsers::Results::param_type_t compParam[] = {Parsers::Results::MANAME};
  
  if ((res = Parsers::beginParsing(config_file_name))) {
    MA_MUTEX.unlock();
    return res;
  }
  res = Parsers::parseCfgFile(false,
                              1,
                              (Parsers::Results::param_type_t*) compParam);
  if (res) {
    Parsers::endParsing();
    MA_MUTEX.unlock();
    return res;
  }

  /* Some more checks */
  userDefName = (char*) 
    Parsers::Results::getParamValue(Parsers::Results::NAME);

  value = Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (value != NULL)
    WARNING("parsing " << config_file_name << ": no need "
            << "to specify a parent for a client - ignored");

  value = Parsers::Results::getParamValue(Parsers::Results::AGENTTYPE);
  if (value != NULL)
    WARNING("parsing " << config_file_name
            << ": agentType is useless for a client - ignored");
    
  /* Get the traceLevel */
  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    char *  level = (char *) calloc(48, sizeof(char*)) ;
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = "-ORBtraceLevel";
    sprintf(level, "%u", TRACE_LEVEL - TRACE_MAX_VALUE);
    myargv[myargc + 1] = (char*)level;
    myargc = tmp_argc;
  }

#if HAVE_ALTPREDICT
  char *tempLocID = (char*) 
      Parsers::Results::getParamValue(Parsers::Results::LOCATIONID);
  if (tempLocID != NULL) {
    clientLocationID = CORBA::string_dup(tempLocID);
  } else {
    clientLocationID = CORBA::string_dup("");
  }
#endif // HAVE_ALTPREDICT
  
  /* Get the USE_ASYNC_API flag */
  value = Parsers::Results::getParamValue(Parsers::Results::USEASYNCAPI);
  if (value != NULL)
    USE_ASYNC_API = *(size_t *)(value); 

  if (USE_ASYNC_API == 1) {
    /* Initialize the ORB */
    if (ORBMgr::init(myargc, (char**)myargv)) {
      ERROR("ORB initialization failed", 1);
    }
    // Create sole instance of synchronized CallAsyncMgr class
    CallAsyncMgr::Instance();
    // Create servant callback object
    CallbackImpl* cb = new CallbackImpl();
    // activate servant callback
    if (ORBMgr::activate(cb) != 0) return -1;
    CORBA::Object_var obj = cb->_this();
    // create corba client callback serveur reference ...
    REF_CALLBACK_SERVER = ORBMgr::getIORString(obj);
    //This is done in the activate() of the newest ORBMgr version
    //cb->_remove_ref();
    if (REF_CALLBACK_SERVER == NULL) return -1;
  }
  else {
    if (ORBMgr::init(myargc, (char**)myargv, false)) {
      ERROR("ORB initialization failed", 1);
    }
  }

#if HAVE_JUXMEM
  juxmem = new JuxMem::Wrapper(NULL);
#endif // HAVE_JUXMEM

#if HAVE_CKPT


#endif

  /* Find Master Agent */
  MA_name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::MANAME);
   cout << "MA NAME PARSING = " << MA_name << endl;
   MA_Name = CORBA::string_dup(MA_name);
  MA = MasterAgent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT, MA_name));
  if (CORBA::is_nil(MA)) {
    ERROR("cannot locate Master Agent " << MA_name, 1);
  }

  /* Initialize statistics module */
  stat_init();

  /* We do not need the parsing results any more */
  Parsers::endParsing();

#if HAVE_ALTPREDICT
  char *tmpHostName = new char[256];
  /** Get localhost name for performance prediction */
  if (gethostname(tmpHostName, 256)) {
    TRACE_TEXT(TRACE_MAIN_STEPS, 
      "Could not get hostname on client - may affect performance prediction");
    clientHostname = CORBA::string_dup("");
  } else {
    clientHostname = CORBA::string_dup(tmpHostName);
  }
#endif // HAVE_ALTPREDICT

  //create_file();
  MA_MUTEX.unlock();

  /** get Num session*/
  num_Session = MA->get_session_num();

#ifdef HAVE_WORKFLOW
  // Workflow parsing
  /* Find the MA_DAG */
  MA_DAG_name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::MADAGNAME);
  if (MA_DAG_name != NULL) {
    cout << "MA DAG NAME PARSING = " << MA_DAG_name << endl;
    MA_DAG_name = CORBA::string_dup(MA_DAG_name);
    MA_DAG = MaDag::_narrow(ORBMgr::getObjReference(ORBMgr::MA_DAG, MA_DAG_name));
    if (CORBA::is_nil(MA_DAG)) {
      ERROR("cannot locate MA DAG " << MA_DAG_name, 1);
    }
    else
      use_ma_dag = true;
  }
#endif

  return 0;
}

diet_error_t
diet_finalize()
{
  stat_finalize();
  if (USE_ASYNC_API == 1) {

    CallAsyncMgr * caMgr = CallAsyncMgr::Instance();
    while (caMgr->areThereWaitRules() > 0) {
      omni_thread::sleep(1);
      // must be replace by a call to waitall
      // must be a call to diet_finalize_force ....
      // Maybe we must split async api from sync api ...
    }
    caMgr->release();
  }
  ORBMgr::destroy();
  MA_MUTEX.lock();
  MA = MasterAgent::_nil();
  MA_MUTEX.unlock();

#if HAVE_JUXMEM
  delete(juxmem);
#endif // HAVE_JUXMEM
  
  /* end fileName */
  // *fileName='\0';
  return 0;
}

END_API


/****************************************************************************/
/* Data handles                                                             */
/****************************************************************************/

BEGIN_API

diet_data_handle_t*
diet_data_handle_malloc()
{
  return new diet_data_handle_t;
}

void*
diet_get(diet_data_handle_t* handle)
{
  return handle->value;
}

int
diet_free(diet_data_handle_t* handle)
{
  delete handle;
  return 0;
}

END_API

/**************************************************************** 
 *    get available Services in the DIET Platform
 ***************************************************************/

/* Only return the name of services available on the platform */
char**
get_diet_services(int *services_number){
  CORBA::Long length;                
  SeqCorbaProfileDesc_t* profileList = MA->getProfiles(length);
  *services_number= (int)length;        
  char** services_list = (char**)calloc(length+1,sizeof(char*));
  fflush(stdout);
  for(int i=0;i<*services_number;i++){
    services_list[i]= CORBA::string_dup((*profileList)[i].path);
  }    
  return services_list;
}


/**************************************************************** 
 *   creation of the file that stores identifiers               * 
 ***************************************************************/

void create_file()
{
sprintf(file_Name,"/tmp/ID_FILE.%s.%d",MA_Name,num_Session);
create_header();

}

void create_header()
{
  char* header_id = "Data Handle   ";
  char* header_msg = "Description \n";
 
    ofstream f(file_Name,ios_base::app|ios_base::ate);
    int cpt = strlen(header_id);
    f.write(header_id,cpt);
    for(int i = 0; i < 10; i++) {
      f.put(' ');
    }
    cpt = strlen(header_msg);
    f.write(header_msg,cpt);
    f.close();
}

/**************************************************************** 
 *   Add handler id with message msg in the file                * 
 ***************************************************************/

void store_id(char* argID, char* msg)
{
 
  size_t cpt;
 
  char* msg1 = new char[strlen(msg)+2];
  cpt=strlen(argID);
  ofstream f(file_Name,ios_base::app|ios_base::ate);
  f.write(argID,cpt);
  for(int i = 0; i < 10; i++) {
    f.put(' ');
  }
  sprintf(msg1,"%s\n",msg);
  cpt = strlen(msg1);
  f.write(msg1,cpt);
  f.close();
}


/******************************************************************
 *  Free persistent data identified by argID                     * 
 *****************************************************************/
int 
diet_free_persistent_data(char* argID)
{
  if(MA->diet_free_pdata(argID)!=0) {
    return 1;
  } else {
    cerr << "UNKNOWN DATA" << endl;
    return 0;
  }
}


/****************************************************************************/
/* GridRPC call functions                                                   */
/****************************************************************************/


/****************************************
 * Request submission
 ****************************************/

diet_error_t
request_submission(diet_profile_t* profile,
                   SeD_var& chosenServer,
                   diet_reqID_t& reqID)
{
  static int nb_tries(3);
  int server_OK(0), subm_count, data_OK(0);
  corba_pb_desc_t corba_pb;
  corba_response_t* response(NULL);
  char* bad_id(NULL);
  char statMsg[128];

  chosenServer = SeD::_nil();
 
  if (mrsh_pb_desc(&corba_pb, profile)) {
    ERROR("profile is wrongly built", 1);
  }

  /* Request submission : try nb_tries times */

  stat_in("Client","request_submission");
  subm_count = 0;
  do {
    response = NULL;

#if ! HAVE_JUXMEM
    /* data property base_type and type retrieval : used for scheduler*/
    int i = 0;

    for (i = 0, data_OK = 0 ;
         (i <= corba_pb.last_out && data_OK == 0) ;
         i++) {
      char* new_id = strdup(corba_pb.param_desc[i].id.idNumber);
 
      if(strlen(new_id) != 0) {
        corba_data_desc_t *arg_desc = new corba_data_desc_t;
        arg_desc = MA->get_data_arg(new_id);
        char *tmp="-1";
        if( strcmp(CORBA::string_dup(arg_desc->id.idNumber),tmp) ==0) {
          bad_id = new_id;
          data_OK = 1;
        } else {
          const_cast<corba_data_desc_t&>(corba_pb.param_desc[i]) = *arg_desc;
        }
      }
    }
#endif // ! HAVE_JUXMEM

    if(data_OK == 0) {
          
      /* Submit to the agent. */
      try {
#if ! HAVE_ALTPREDICT
        response = MA->submit(corba_pb, MAX_SERVERS);
#else // HAVE_ALTPREDICT
        response = MA->submit(corba_pb, MAX_SERVERS,
                              clientHostname, clientLocationID);
#endif // HAVE_ALTPREDICT
      } catch (CORBA::Exception& e) {
        CORBA::Any tmp;
        tmp <<= e;
        CORBA::TypeCode_var tc = tmp.type();
        if (response)
          delete response;
        ERROR("caught a CORBA exception (" << tc->name()
              << ") while submitting problem", 1);
      }

      /* Check response */
      if (!response || response->servers.length() == 0) {
        WARNING("no server found for problem " << corba_pb.path);
        server_OK = -1;

      } else {

        if (TRACE_LEVEL >= TRACE_MAIN_STEPS) {
          TRACE_TEXT(TRACE_MAIN_STEPS,
                     "The Master Agent found the following server(s):\n");
          for (size_t i = 0; i < response->servers.length(); i++) {
            TRACE_TEXT(TRACE_MAIN_STEPS,
                       "    " << response->servers[i].loc.hostName << ":"
                       << response->servers[i].loc.port << "\n");
          }
        }
#if 0
        /** Contract checking has been commented out in Jan 2006 by Holly
         * because noone has used it for years.  Leaving code here as 
         * background info for someone pursuing research in contract
         * checking. */

        /* Check the contracts of the servers answered. */
        server_OK = 0;
        while ((size_t) server_OK < response->servers.length()) {
          try {
            int           idx       = server_OK;
            SeD_ptr       server    = response->servers[idx].loc.ior;

            estVector_t ev = &(response->servers[idx].estim);
            CORBA::Double totalTime =
              diet_est_get_internal(ev, EST_TOTALTIME, HUGE_VAL);

            if (server->checkContract(response->servers[idx].estim,
                                      corba_pb)) {
              server_OK++;
              continue;
            }

            ev = &(response->servers[idx].estim);
            CORBA::Double newTotalTime =
              diet_est_get_internal(ev, EST_TOTALTIME, HUGE_VAL);
            if ((totalTime == newTotalTime) ||
                ((newTotalTime - totalTime) <
                 (ERROR_RATE * MAX(totalTime, newTotalTime)))) {
              break;
            }
            server_OK++;
          } catch (CORBA::Exception& e) {
            server_OK++;
            continue;
          }
        }
        if ((size_t) server_OK == response->servers.length()) {
          server_OK = -1;
        }
#endif    // Contract checking commented out
        server_OK = 0;    // Use this when no contract checking
      } // end else  [if (!response || response->servers.length() == 0)]
    } // end if data ok
  } while ((response) && (response->servers.length() > 0) &&
           (server_OK == -1) && (++subm_count < nb_tries) && (data_OK == 0));

  if(data_OK == 1) {
    ERROR (" data with ID " <<  bad_id << " not inside the platform.", 1);
    delete (bad_id);
  } else {
  
    if (!response || response->servers.length() == 0) {
      if (response) {
        delete response;
      }
      ERROR("unable to find a server", 1);
    }
    if (server_OK == -1) {
      delete response;
      ERROR("unable to find a server after " << nb_tries << " tries."
          << "The platform might be overloaded, try again later please", 1);
    }

    if (server_OK >= 0) {
      chosenServer = response->servers[server_OK].loc.ior;
      reqID = response->reqID;
#ifdef HAVE_FD
      fd_handle fd = fd_make_handle();
      fd_set_qos(fd, DIET_DEFAULT_FD_QOS);
      fd_set_service(fd, response->servers[server_OK].loc.hostName, 1);
      fd_observe(fd);
#endif
    }
    sprintf(statMsg, "request_submission %ld", (unsigned long) reqID);
    stat_out("Client",statMsg);
#ifdef HAVE_BATCH
    profile->dietJobID = reqID ;
#endif
  }

  return 0;
}
  
  
/****************************************
 * Synchronous call
 ****************************************/

/**
 * This function unifies DIET and GridRPC APIs.
 * It is designed to be called from diet_call, grpc_call and grpc_call_argstack.
 */
diet_error_t
diet_call_common(diet_profile_t* profile, SeD_var& chosenServer)
{
  diet_error_t res(0);
  int solve_res(0);
  diet_reqID_t reqID;
  corba_profile_t corba_profile;

  char statMsg[128];
  stat_in("Client","diet_call");

  if (CORBA::is_nil(chosenServer)) {
    if ((res = request_submission(profile, chosenServer, reqID))) {
      return res;
    }
    if (CORBA::is_nil(chosenServer)) {
      return 1;
    }
  }

#if HAVE_JUXMEM 
  int i = 0;
  for (i = 0; i <= profile->last_inout; i++) {
    /**
     * If there is no data id (for both IN or INOUT case), this a new
     * data.  Therefore, attach the user input and do a msync on
     * JuxMem of the data.  Of course do that only if data are
     * persistent! Else let CORBA move them
     */
    if (profile->parameters[i].desc.id == NULL &&
	profile->parameters[i].desc.mode == DIET_PERSISTENT) {
      profile->parameters[i].desc.id = juxmem->attach(profile->parameters[i].value, 
						      data_sizeof(&(profile->parameters[i].desc)), 
						      1, 1, EC_PROTOCOL, BASIC_SOG);
      TRACE_TEXT(TRACE_MAIN_STEPS, "A data space with ID = " << profile->parameters[i].desc.id << " for IN data has been attached inside JuxMem!\n");
      /* The local memory is flush inside JuxMem */
      juxmem->msync(profile->parameters[i].value);      
      profile->parameters[i].value = NULL;
    }
  }
#endif // HAVE_JUXMEM

  /* Send Datas */
  if (mrsh_profile_to_in_args(&corba_profile, profile)) {
    ERROR("profile is wrongly built", 1);
  }
  
  int j = 0;
  bool found = false;
  while ((j <= corba_profile.last_out) && (found == false)) {
    if (diet_is_persistent(corba_profile.parameters[j])) {
// && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
       found = true;
    }
    j++;
  }
  if(found == true){
     create_file();
  }

#if ! HAVE_JUXMEM
  /* data property base_type and type retrieval : used for scheduler */
  for(int i = 0;i <= corba_profile.last_out;i++) {
    char* new_id = strdup(corba_profile.parameters[i].desc.id.idNumber);
    if(strlen(new_id) != 0) {
      corba_data_desc_t* arg_desc = new corba_data_desc_t;
      arg_desc = MA->get_data_arg(new_id);
      const_cast<corba_data_desc_t&>(corba_profile.parameters[i].desc) = *arg_desc;
    }
  }  
  
  /* generate new ID for data if not already existant */
  for(int i = 0;i <= corba_profile.last_out;i++) {
    if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) && 
        (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT) &&
        (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
    {
      char* new_id = MA->get_data_id(); 
      corba_profile.parameters[i].desc.id.idNumber = new_id;
    }
  }
#endif

  /* Computation */
  sprintf(statMsg, "computation %ld", (unsigned long) reqID);
  try {
    stat_in("Client",statMsg);

    TRACE_TEXT(TRACE_MAIN_STEPS, "Calling the ref Corba of the SeD\n");
#if HAVE_FD
    fd_set_transition_handler(diet_call_failure_recover);
#endif
    solve_res = chosenServer->solve(profile->pb_name, corba_profile,reqID);
    stat_out("Client",statMsg);
   } catch(CORBA::MARSHAL& e) {
    ERROR("got a marchal exception\n"
          "Maybe your giopMaxMsgSize is too small",1) ;
  }

  /* reaffect identifier */
  for(int i = 0;i <= profile->last_out;i++) {
    if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) && 
        (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT)) {
      profile->parameters[i].desc.id = strdup(corba_profile.parameters[i].desc.id.idNumber);
    }
  }

  if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
    INTERNAL_ERROR("returned profile is wrongly built", 1);
  }

#if HAVE_JUXMEM
  for (i = profile->last_inout + 1; i <= profile->last_out; i++) {
    /**
     * Retrieve INOUT or OUT data only if DIET_PERSISTENT_RETURN.
     * Note that for INOUT data, the value can be already initialized.
     * In this case, JuxMem will use this address to store the
     * data. If value is NULL, a memory area will allocated.
     */
    if (profile->parameters[i].desc.id != NULL &&
	profile->parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {
      juxmem->mmap(profile->parameters[i].value, data_sizeof(&(profile->parameters[i].desc)), profile->parameters[i].desc.id, 0);
      if (i <= profile->last_inout) {
	TRACE_TEXT(TRACE_MAIN_STEPS, "Retrieving IN_OUT data with ID = " << profile->parameters[i].desc.id << " from JuxMem ...\n");
      } else {
	TRACE_TEXT(TRACE_MAIN_STEPS, "Retrieving OUT data with ID = " << profile->parameters[i].desc.id << " from JuxMem ...\n");
      }
      juxmem->acquireRead(profile->parameters[i].value);
      juxmem->release(profile->parameters[i].value);
    }
  }
#endif // HAVE_JUXMEM

  sprintf(statMsg, "diet_call %ld", (unsigned long) reqID);
  stat_out("Client",statMsg);
  return solve_res;
}

BEGIN_API

/**
 * Request + computation submissions.
 */
diet_error_t
diet_call(diet_profile_t* profile)
{
  SeD_var chosenServer = SeD::_nil();
  return diet_call_common(profile, chosenServer);
}

#ifdef HAVE_BATCH
diet_error_t
diet_call_batch(diet_profile_t* profile)
{
  diet_profile_set_batch(profile) ;
  return diet_call(profile) ;
}
#endif

END_API


/****************************************
 * Asynchronous call
 ****************************************/

/**
 * This function unifies DIET and GridRPC APIs.
 * It is designed to be called from diet_call_async, grpc_call-async
 * and grpc_call_argstack_async.
 */
diet_error_t
diet_call_async_common(diet_profile_t* profile,
                       SeD_var& chosenServer, diet_reqID_t* reqID)
{
   corba_profile_t corba_profile;
   CallAsyncMgr* caMgr;
   diet_error_t res(0);
  
  if (!reqID) {
    ERROR(__FUNCTION__ << ": 2nd argument has not been allocated", 1);
  }

  stat_in("Client","diet_call_async");


  try {

    if (CORBA::is_nil(chosenServer)) {
      if ((res = request_submission(profile, chosenServer, *reqID)))
        return res;
      if (CORBA::is_nil(chosenServer))
        return 1;
    }
    
    if (mrsh_profile_to_in_args(&corba_profile, profile)) {
      ERROR("profile is wrongly built", 1);
    }


    int j = 0;
    bool found = false;
    while ((j <= corba_profile.last_out) && (found == false)) {
      if (diet_is_persistent(corba_profile.parameters[j])) {
  // && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
        found = true;
      }
      j++;
    }
    if(found == true){
      create_file();
    }
 
    /* data property base_type and type retrieval : used for scheduler */
    for(int i = 0;i <= corba_profile.last_out;i++) {
      char* new_id = strdup(corba_profile.parameters[i].desc.id.idNumber);
      if(strlen(new_id) != 0) {
        corba_data_desc_t *arg_desc = new corba_data_desc_t;
        arg_desc = MA->get_data_arg(new_id);
        const_cast<corba_data_desc_t&>(corba_profile.parameters[i].desc) = *arg_desc;
      }
    }  
    
    
    /* generate new ID for data if not already existant */
    for(int i = 0;i <= corba_profile.last_out;i++) {
      if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) && 
          (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT) 
     && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber)))) 
      {
        char* new_id = MA->get_data_id(); 
        corba_profile.parameters[i].desc.id.idNumber = new_id;
      }
    }
    
    // get sole CallAsyncMgr singleton
    caMgr = CallAsyncMgr::Instance();
    // create corba client callback server...
    if (caMgr->addAsyncCall(*reqID, profile) != 0) {
      return 1;
    }

    stat_in("Client","computation_async");
    chosenServer->solveAsync(profile->pb_name, corba_profile, 
                             *reqID, REF_CALLBACK_SERVER);

    stat_out("Client","computation_async");

    if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
      INTERNAL_ERROR("returned profile is wrongly built", 1);
    }
   
  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      WARNING("exception caught in " << __FUNCTION__ << '(' << p << ')');
    } else {
      WARNING("exception caught in " << __FUNCTION__ << '(' << tc->id() << ')');
    }
    *reqID = -1;
    return 1;
  }
  catch (...) {
    WARNING("exception caught in " << __FUNCTION__);
    *reqID = -1;
    return 1;
  }
  
  stat_out("Client","diet_call_async");
  return 0;
}

BEGIN_API
/**
 * Request + asynchronous computation submissions.
 */
diet_error_t
diet_call_async(diet_profile_t* profile, diet_reqID_t* reqID)
{
  SeD_var chosenServer = SeD::_nil();
  return diet_call_async_common(profile, chosenServer, reqID);
}
END_API


/****************************************************************************/
/* Asynchronous control and wait functions                                  */
/****************************************************************************/

BEGIN_API

/****************************************************************************  
 * diet_probe GridRPC function.
 * This function probes status of an asynchronous request.
 * return reqID status.                   
 ****************************************************************************/
int
diet_probe(diet_reqID_t reqID)
{
  return CallAsyncMgr::Instance()->getStatusReqID(reqID);
}
  
/****************************************************************************  
 * diet_cancel GridRPC function.
 * This function erases all persistent data that are manipulated by the reqID
 * request. Do not forget to calget_data_handle on data you would like
 * to save.                   
 ****************************************************************************/
int
diet_cancel(diet_reqID_t reqID)
{
  return CallAsyncMgr::Instance()->deleteAsyncCall(reqID);
}

int
diet_wait(diet_reqID_t reqID)
{
  try {
      // Create ruleElements table ...
    ruleElement * simpleWait = new ruleElement[1];
    simpleWait[0].reqID = reqID;
    simpleWait[0].op = WAITOPERATOR(SOLE);
    Rule * rule = new Rule;
    rule->length = 1;
    rule->ruleElts = simpleWait;
    rule->status = STATUS_RESOLVING;
    
    // get lock on condition/waitRule
    return CallAsyncMgr::Instance()->addWaitRule(rule);
    // NOTES: Be careful, there may be others rules
    // using some of this reqID(AsyncCall)
    // So, be careful using diet_cancel
  }
  catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    } else {
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')'); 
    }
  }  
  catch (const exception& e) {
    ERROR(__FUNCTION__ << ": unexpected exception (what="
          << e.what() << ')', STATUS_ERROR);
  }
  return STATUS_ERROR;
}

/*****************************************************************************
 * diet_wait_and GridRPC function
 * return error status 
 * three args :
 *      1 - reqID table.
 *      2 - size of the table.
 *****************************************************************************/
int
diet_wait_and(diet_reqID_t* IDs, size_t length)
{
  request_status_t rst = STATUS_ERROR;
  try {
    // Create ruleElements table ...
    ruleElement * simpleWait = new ruleElement[length];
    for (size_t k = 0; k < length; k++) {
      simpleWait[k].reqID = IDs[k];
      simpleWait[k].op = WAITOPERATOR(AND);
    }
    Rule * rule = new Rule;
    rule->length = length;
    rule->ruleElts = simpleWait;
    
    // get lock on condition/waitRule
    return CallAsyncMgr::Instance()->addWaitRule(rule);
    // NOTES: Be careful, there may be others rules using some of this
    // reqID(AsyncCall) So, careful using diet_cancel
  }
  catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    } else {
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
    }
  }  
  catch (const exception& e) {
    ERROR(__FUNCTION__ << ": unexpected exception (what=" << e.what() << ')',
          STATUS_ERROR);
  }
  return rst;
}

/*****************************************************************************
 * diet_wait_or GridRPC function
 * return error status 
 * three args :
 *      1 - reqID table.
 *      2 - size of the table.
 *      3 - received reqID
 *****************************************************************************/
int
diet_wait_or(diet_reqID_t* IDs, size_t length, diet_reqID_t* IDptr)
{
  request_status_t rst = STATUS_ERROR;
  try {
    // Create ruleElements table ...
    ruleElement * simpleWait = new ruleElement[length];
    for (unsigned int k = 0; k < length; k++) {
      simpleWait[k].reqID = IDs[k];
      simpleWait[k].op = WAITOPERATOR(OR);
    }
    Rule * rule = new Rule;
    rule->length = length;
    rule->ruleElts = simpleWait;
    
    // get lock on condition/waitRule
    // and manage return rule status
    switch (CallAsyncMgr::Instance()->addWaitRule(rule)) {
    case STATUS_DONE:
      for (unsigned int k = 0; k < length; k++) {
        if (CallAsyncMgr::Instance()->getStatusReqID(IDs[k]) == STATUS_DONE) {
          *IDptr = IDs[k];
          return STATUS_DONE;
        }
      }
      return STATUS_ERROR;
    case STATUS_CANCEL:
      return STATUS_CANCEL;
    case STATUS_ERROR:
      return STATUS_ERROR;
    default:
      return -1; // Unexpected error, no value describing it
      // NOTES: Be careful, there may be others rules
      // using some of this reqID(AsyncCall)
      // So, careful using diet_cancel
    }
  }
  catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    } else {
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
    }
  } catch (const exception& e) {
    ERROR(__FUNCTION__ << ": unexpected exception (what="
          << e.what() << ')', STATUS_ERROR);
  }
  return rst;
}

/*****************************************************************************
 * diet_wait_all GridRPC function
 * return error status
 *****************************************************************************/
int
diet_wait_all()
{
  return CallAsyncMgr::Instance()->addWaitAllRule();
}

/*****************************************************************************
 * diet_wait_any GridRPC function
 * return the ID of the received request
 *****************************************************************************/
int
diet_wait_any(diet_reqID_t* IDptr)
{
  return CallAsyncMgr::Instance()->addWaitAnyRule(IDptr);
}

// for workflow support
#ifdef HAVE_WORKFLOW
/*****************************************/
/* send a workflow description to the MA */
/*****************************************/
diet_error_t
diet_wf_call_ma(diet_wf_desc_t* profile) {
  diet_error_t res(0);
  corba_wf_desc_t  * corba_profile = new corba_wf_desc_t;
  wf_response_t * response;
  bool user_sched = true;
  cout << "marshalling the workflow description ..." ;
  mrsh_wf_desc(corba_profile, profile);
  cout << " done" << endl;

  reader = new WfExtReader(profile->abstract_wf);
  reader->setup();

  // Since the WfExtReader don't allocate profile before execution
  // we can't use it to ask the Master Agent is the profile set is
  // executable by the availble services
  // So a temporary basic reader is used instead
  WfReader * tmpReader = new WfReader(profile->abstract_wf);
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
  if (MA) {
    //    response = MA->submit_wf(*corba_profile);
    response = MA->submit_pb_set(pb_seq);
    cout << " done" << endl;
    if (! response->complete) {
      cout << "One ore more services are missing" << endl
	   << "The Workflow cannot be executed" << endl;
      return -1;
    }
  }
  else {
    // 
    cout << " The MA is unavailable !!! " << endl;
    return -1;
  }

  cout<< "Received response length " << response->wfn_seq_resp.length()
      << endl;

  // Execution du workflow
  // ...
  dag = reader->getDag();

  // Not used (TO REMOVE). It is the scheduler which execute the DAG
  dag->exec();

  cout << "The dag contains " << dag->size() << " nodes" << endl;

  if (defaultWfSched == NULL) {
    defaultWfSched = new SimpleWfSched();
    user_sched = false;
  }


  defaultWfSched->setDag (dag);
  defaultWfSched->setResponse(response);
  defaultWfSched->execute();

  if (! user_sched) {
    delete defaultWfSched;
    defaultWfSched = NULL;
  }

  return res;
}

/**
 * First variant of using MA_DAG approach *
 * This approach use ordering and mapping provided by the MA_DAG * 
 */
diet_error_t
diet_call_wf_madag_v1(diet_wf_desc_t* profile) {
  diet_error_t res(0);
  corba_wf_desc_t  * corba_profile = new corba_wf_desc_t;
  wf_node_sched_seq_t * response;

  reader = new WfExtReader(profile->abstract_wf);
  reader->setup();

  dag = reader->getDag();

  cout << "The dag contains " << dag->size() << " nodes" << endl;
  
  cout << "marshalling the workflow description ..." ;
  mrsh_wf_desc(corba_profile, profile);
  cout << " done" << endl;
  cout << "corba_profile->abstract_wf = " <<
    corba_profile->abstract_wf << endl;
  // call the master agent
  // and send the workflow description
  cout << "Try to send the workflow description to the MA_DAG ...";
  if (MA_DAG)
    response = MA_DAG->submit_wf(*corba_profile);
  cout << " done" << endl;
  // response processing and defining scheduling strategy
  // ...

  cout << "Received response " << response->length() << endl;

  dag->setSchedResponse(response);

  if (defaultWfSched == NULL) 
    defaultWfSched = new SimpleWfSched();


  defaultWfSched->setDag(dag);
  defaultWfSched->execute();

  if (defaultWfSched)
    delete defaultWfSched;

  return res;
} // end diet_call_wf_madag_v1


/**
 * Second variant of using MA_DAG approach *
 * This approach use only the ordering provided by the MA_DAG * 
 * Currently this method is the same as diet_call_wf_madag_v1 except *
 * the call to dag->setSchedResponse(response); which implie that node *
 * chosenServer still NULL*
 */

diet_error_t
diet_call_wf_madag_v2(diet_wf_desc_t* profile) {
  diet_error_t res(0);
  corba_wf_desc_t  * corba_profile = new corba_wf_desc_t;
  wf_node_sched_seq_t * response;

  reader = new WfExtReader(profile->abstract_wf);
  reader->setup();

  dag = reader->getDag();

  cout << "The dag contains " << dag->size() << " nodes" << endl;
  
  cout << "marshalling the workflow description ..." ;
  mrsh_wf_desc(corba_profile, profile);
  cout << " done" << endl;
  cout << "corba_profile->abstract_wf = " <<
    corba_profile->abstract_wf << endl;
  // call the master agent
  // and send the workflow description
  cout << "Try to send the workflow description to the MA_DAG ...";
  if (MA_DAG)
    response = MA_DAG->submit_wf(*corba_profile);
  cout << " done" << endl;
  // response processing and defining scheduling strategy
  // ...

  cout << "Received response " << response->length() << endl;

  if (defaultWfSched == NULL) 
    defaultWfSched = new SimpleWfSched();


  defaultWfSched->setDag(dag);
  defaultWfSched->execute();

  if (defaultWfSched)
    delete defaultWfSched;

  return res;
}



diet_error_t
diet_wf_call_madag(diet_wf_desc_t* profile) {
  if (use_ma_dag_sched) {
    return diet_call_wf_madag_v1(profile);
  }
  return diet_call_wf_madag_v2(profile);
}


diet_error_t
diet_wf_call(diet_wf_desc_t* profile) {
  if (! use_ma_dag) {
    return diet_wf_call_ma(profile);
  }
  return diet_wf_call_madag(profile);
}

/**
 * terminate a workflow session *
 * and free the memory *
 */

void
diet_wf_free(diet_wf_desc_t * profile) {
  diet_wf_profile_free(profile);
  if (reader) {
    delete reader;
    reader = NULL;
  }
}

/*
 * use a custom workflow scheduler *
 */

void set_sched (struct AbstractWfSched * sched) {
  defaultWfSched = sched;
}


/*
 * define if the ma_dag return a *
 * ordering and a scheduling (b = true) *
 * or only and ordering (b = false) *
 */
void
set_madag_sched(int b) {
  use_ma_dag_sched = b;
}


/**
 * initialize the workflow support *
 */

diet_error_t
diet_init_wf(char * config_file) {
  int    res(0);
  return res;
}


int
_diet_wf_scalar_get(const char * id,
		    void** value) {
  if (dag != NULL) {
    return dag->get_scalar_output(id, value);
  }
  else
    return 1;
}


int 
_diet_wf_string_get(const char * id, 
		    char** value) {
  if (dag != NULL) {
    return dag->get_string_output(id, value);
  }
  return 0;
}

#endif // HAVE_WORKFLOW

END_API

