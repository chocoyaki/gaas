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
 * Revision 1.53  2004/12/15 13:53:22  sdahan
 * - the trace function are now thread safe.
 * - add "extern unsigned int TRACE_LEVEL" in debug.hh
 *
 * Revision 1.52  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.51  2004/11/25 11:40:32  hdail
 * Add request ID to statistics output to allow tracing of stats for each request.
 *
 * Revision 1.50.2.3  2004/11/30 20:04:26  alsu
 * math.h needed for HUGE_VAL
 *
 * Revision 1.50.2.2  2004/11/06 16:23:37  alsu
 * - minor bug for a zero-parameter service
 * - changes for new parameter-based default values for estVector access
 *   functions
 *
 * Revision 1.50.2.1  2004/11/02 00:42:03  alsu
 * modifying the "contract checking" code to use new estimation
 * structure.
 *
 * Revision 1.50  2004/10/15 13:04:29  bdelfabr
 * request_submission modified to avoid mismatch data identifers
 *
 * Revision 1.49  2004/10/15 08:19:13  hdail
 * Removed references to corba_response_t->sortedIndexes - no longer useful.
 *
 * Revision 1.48  2004/10/06 18:37:07  rbolze
 * fix a bug due to the last commit
 *
 * Revision 1.47  2004/10/06 16:43:17  rbolze
 * implement function that calls Ma to get the list of services in the platform
 *
 * Revision 1.46  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.45  2004/09/29 12:54:16  bdelfabr
 * english please
 *
 * Revision 1.44  2004/07/29 18:52:11  rbolze
 * Change solve function now , DIET_client send the reqID of the request when
 * he call the solve function.
 * Nothing is change for DIET's API
 *
 * Revision 1.43  2004/05/04 11:46:20  bdelfabr
 * id files are created only in case of persistent data
 *
 * Revision 1.42  2004/03/01 18:52:24  rbolze
 * make some change relative to the new ORBMgr
 *
 * Revision 1.41  2004/02/27 10:23:28  bdelfabr
 * adding calls in order to use identifier instead of diet_..._set when data is already inside the platform
 *
 * Revision 1.40  2004/01/16 14:41:42  sdahan
 * In the diet_call_common function, catch the CORBA::MARSHAL exception
 * during the chosenServer->solve() call. Now, when the giopMaxMsgSize is
 * too small there is an informational error instead of a simple abore.
 *
 * Revision 1.39  2003/12/01 14:49:31  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.38  2003/10/03 12:41:26  mcolin
 * Fix memory management in the list of arguments
 *
 * Revision 1.37  2003/09/28 22:11:53  ecaron
 * Take into account the new API of statistics module
 *
 * Revision 1.36  2003/09/25 10:03:38  cpera
 * Change return function value according to GridRPC API and delete debug messages.
 *
 * Revision 1.35  2003/09/22 21:08:15  pcombes
 * Update to changes of ORBMgr.
 *
 * Revision 1.34  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.33  2003/07/09 17:08:44  pcombes
 * Better management of the variable MA (with mutex) and the handles.
 *
 * Revision 1.32  2003/07/04 09:48:02  pcombes
 * Make diet_initialize thread-safe. Use new ERROR and WARNING macros.
 *
 * Revision 1.26  2003/06/02 08:08:11  cpera
 * Beta version of asynchronize DIET API.
 *
 * Revision 1.25  2003/05/12 14:15:58  ecaron
 * Update call to stat in according to perl script file (extract.pl)
 *
 * Revision 1.24  2003/05/10 08:54:08  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.13  2002/11/22 13:36:12  lbertsch
 * Added alpha linux support
 * Added a package for statistics and some traces
 ****************************************************************************/


#include "DIET_client.h"

#include <CORBA.h>
#include <iostream>
#include <fstream>
using namespace std;
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


#include "com_tools.hh"
#include "debug.hh"
#include "DIET_data_internal.hh"
#include "marshalling.hh"
#include "MasterAgent.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "SeD.hh"
#include "statistics.hh"

#include "CallAsyncMgr.hh"
#include "CallbackImpl.hh"


#define BEGIN_API extern "C" {
#define END_API   } // extern "C"



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



/****************************************************************************/
/* Manage MA name and Session Number for data persistency issue             */
/****************************************************************************/

  char* MA_Name;
  
  int num_Session;
  
  char file_Name[150];


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
  value = Parsers::Results::getParamValue(Parsers::Results::NAME);
  if (value != NULL)
    WARNING("parsing " << config_file_name
            << ": it is useless to name a client - ignored");
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
  /** get Num session*/

  //create_file();
  MA_MUTEX.unlock();
  num_Session=MA->get_session_num();
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
    for(int i = 0; i < 10;i++) 
      f.put(' ');
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
 
  char* msg1 = new char[strlen(msg)+1];
  cpt=strlen(argID);
  ofstream f(file_Name,ios_base::app|ios_base::ate);
  f.write(argID,cpt);
  for(int i = 0;i < 10;i++) 
    f.put(' ');
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

  if(MA->diet_free_pdata(argID)!=0)
    return 1;
  else {
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
  //corba_pb_desc_t& corba_pb = *(new corba_pb_desc_t());
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

    /* data property base_type and type retrieval : used for scheduler*/

    int i = 0;
//     do {
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
        }
        else {
          const_cast<corba_data_desc_t&>(corba_pb.param_desc[i]) = *arg_desc;
        }
      }
//       i++;
//     } while (( i <= corba_pb.last_out) && (data_OK == 0));
    }

    if(data_OK == 0) {
          
      /* Submit to the agent. */
      try {
        response = MA->submit(corba_pb, MAX_SERVERS);
      } catch (CORBA::Exception& e) {
        CORBA::Any tmp;
        tmp <<= e;
        CORBA::TypeCode_var tc = tmp.type();
        //delete &corba_pb;
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

        /* Check the contracts of the servers answered. */
        server_OK = 0;
        while ((size_t) server_OK < response->servers.length()) {
          try {
            int           idx       = server_OK;
            SeD_ptr       server    = response->servers[idx].loc.ior;
//             CORBA::Double totalTime = response->servers[idx].estim.totalTime;

            estVector_t ev = new_estVector();
            unmrsh_estimation_to_estVector(&(response->servers[idx].estim),
                                           ev);
            CORBA::Double totalTime =
              estVector_getEstimationValue(ev, EST_TOTALTIME, HUGE_VAL);
            free_estVector(ev);

            if (server->checkContract(response->servers[idx].estim,
                                      corba_pb)) {
              server_OK++;
              continue;
            }

//             if ((totalTime == response->servers[idx].estim.totalTime) ||
//                 ((response->servers[idx].estim.totalTime - totalTime)
//                  < (ERROR_RATE *
//                     MAX(totalTime,response->servers[idx].estim.totalTime))))
            ev = new_estVector();
            unmrsh_estimation_to_estVector(&(response->servers[idx].estim),
                                           ev);
            CORBA::Double newTotalTime =
              estVector_getEstimationValue(ev, EST_TOTALTIME, HUGE_VAL);
            free_estVector(ev);
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
      } // end else  [if (!response || response->servers.length() == 0)]
    } // end if data ok
  } while ((response) && (response->servers.length() > 0) &&
           (server_OK == -1) && (++subm_count < nb_tries) && (data_OK == 0));

  if(data_OK == 1) {
    ERROR (" data which id is " <<  bad_id << " not inside the platform.", 1);

    delete (bad_id);
  } else {
  
    if (!response || response->servers.length() == 0) {
      //delete &corba_pb;
      if (response) {
        delete response;
      }
      ERROR("unable to find a server", 1);
    }
    if (server_OK == -1) {
      //delete &corba_pb;
      delete response;
      ERROR("unable to find a server after " << nb_tries << " tries."
          << "The platform might be overloaded, try again later please", 1);
    }
  
#if HAVE_CICHLID
    static int already_initialized(0);
    char str_tmp[1000];
  
    if (!already_initialized) {
      init_communications();
      already_initialized = 1;
    }
  
    strcpy(str_tmp, response->servers[server_OK].loc.hostName);
    strcat(str_tmp, "_SeD");
    add_communication("client", str_tmp, profile_size(&corba_pb));
#endif // HAVE_CICHLID
  
    //delete &corba_pb;

    if (server_OK >= 0) {
      chosenServer = response->servers[server_OK].loc.ior;
      reqID = response->reqID;
    }
    sprintf(statMsg, "request_submission %ld", (unsigned long) reqID);
    stat_out("Client",statMsg);
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
  //char *new_id;
  char statMsg[128];
  stat_in("Client","diet_call");


  if (CORBA::is_nil(chosenServer)) {
    if ((res = request_submission(profile, chosenServer, reqID)))
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
    if (diet_is_persistent(corba_profile.parameters[j]))// && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
       found = true;
    j++;
  }
  if(found == true){
     create_file();
  }


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
    if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) && (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT) && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber)))) {
      char* new_id = MA->get_data_id(); 
      corba_profile.parameters[i].desc.id.idNumber = new_id;
    }
  }
 
  sprintf(statMsg, "computation %ld", (unsigned long) reqID);
  try {
    stat_in("Client",statMsg);
    solve_res = chosenServer->solve(profile->pb_name, corba_profile,reqID);
    stat_out("Client",statMsg);
   } catch(CORBA::MARSHAL& e) {
    ERROR("got a marchal exception\n"
          "Maybe your giopMaxMsgSize is too small",1) ;
  }

  /* reaffect identifier */
  for(int i = 0;i <= profile->last_out;i++) {
    if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) && (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT)) {
      profile->parameters[i].desc.id = strdup(corba_profile.parameters[i].desc.id.idNumber);
    }
  }
 


  if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
    INTERNAL_ERROR("returned profile is wrongly built", 1);
  }

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
    if (diet_is_persistent(corba_profile.parameters[j]))// && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
       found = true;
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
      if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) && (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT) && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber)))) {
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
    // NOTES: Be carefull, there may be others rules
    // using some of this reqID(AsyncCall)
    // So, carefull using diet_cancel
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
      return -1; // Unexcpected error, no value describing it
      // NOTES: Be carefull, there may be others rules
      // using some of this reqID(AsyncCall)
      // So, carefull using diet_cancel
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

END_API

