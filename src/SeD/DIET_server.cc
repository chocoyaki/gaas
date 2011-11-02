/**
* @file DIET_server.cc
* 
* @brief  DIET server interface 
* 
* @author - Philippe COMBES (Philippe.Combes@ens-lyon.fr)  
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.108  2011/05/12 15:35:50  bdepardo
 * Reduced variables scope
 *
 * Revision 1.107  2011/05/10 14:33:52  bdepardo
 * Fixed a bug in diet_get_SeD_services()
 *
 * Revision 1.106  2011/05/09 15:00:25  bdepardo
 * Fixed a bug in diet_get_SeD_services
 *
 * Revision 1.105  2011/05/09 13:49:44  bdepardo
 * Error code returned when an error occurs is now GRPC_SERVER_NOT_FOUND.
 * Catch exceptions
 *
 * Revision 1.104  2011/05/09 13:10:11  bdepardo
 * Added method diet_get_SeD_services to retreive the services of a SeD given
 * its name
 *
 * Revision 1.103  2011/04/05 14:01:06  bdepardo
 * IOR is printed only when the tracelevel is at least TRACE_MAIN_STEPS
 *
 * Revision 1.102  2011/03/21 08:27:39  bdepardo
 * Correctly register the logcomponent into the ORB, and correclty detroy it.
 *
 * Revision 1.101  2011/03/20 18:48:17  bdepardo
 * Be more robust when logComponent initialization fails
 *
 * Revision 1.100  2011/03/01 13:37:51  bdepardo
 * SIGTERM can now also be used to properly terminate DIET
 *
 * Revision 1.99  2011/02/24 17:52:11  bdepardo
 * Fixed compilation problem when using ACK_FILE
 *
 * Revision 1.98  2011/02/24 16:57:01  bdepardo
 * Use new parser
 *
 * Revision 1.97  2011/02/22 23:26:36  bdepardo
 * Reduce variable scope
 *
 * Revision 1.96  2011/02/09 09:36:35  bdepardo
 * Quick hack, for the SeD to work with the new parser
 *
 * Revision 1.95  2011/01/20 17:36:59  bdepardo
 * Removed memory leak
 *
 * Revision 1.94  2010/12/17 09:47:59  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.93  2010/11/24 12:30:42  bdepardo
 * Changed logComponenent initialization, now uses the real SeD name.
 *
 * Revision 1.92  2010/08/13 14:22:36  dloureir
 * Missing case in profile deallocation where last_out = -1 and desc->param_desc was null
 *
 * Revision 1.91  2010/07/12 16:14:09  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.90  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.89  2010/03/31 19:37:54  bdepardo
 * Changed "\n" into std::endl
 *
 * Revision 1.88  2010/03/08 13:33:40  bisnard
 * initialize DietLogComponent for DAGDA agent
 *
 * Revision 1.87  2010/03/05 02:38:03  ycaniou
 * Integration of SGE (still not complete) + fixes
 *
 * Revision 1.86  2010/01/14 11:06:10  bdepardo
 * Compiles with gcc 4.4
 *
 * Revision 1.85  2009/11/30 17:58:08  bdepardo
 * New methods to remove the SeD in a cleaner way.
 *
 * Revision 1.84  2009/11/27 03:24:30  ycaniou
 * Add user_command possibility before the end of Batch prologue (only
 * to be used for batch dependent code!)
 * Memory leak/segfault--
 * New easy Batch basic example
 * Management of OAR2_X Batch scheduler
 *
 * Revision 1.83  2009/10/26 09:13:43  bdepardo
 * When using dynamic hierarchy, catches SIGINT for clean termination.
 *
 * Revision 1.82  2009/06/23 09:28:27  bisnard
 * new API method for EFT estimation
 *
 * Revision 1.81  2008/12/22 13:38:55  bdepardo
 * Added diet_wait_batch_job_completion to explicitely wait for the end of a
 * batch job.
 *
 * Revision 1.80  2008/12/08 15:31:42  bdepardo
 * Added the possibility to remove a service given its profile description.
 * So now, one is able to remove a service given either the real profile,
 * or the profile description.
 *
 * Revision 1.79  2008/11/18 10:13:56  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 * Revision 1.78  2008/06/01 09:12:37  rbolze
 * remove free on logService pointer
 *
 * Revision 1.77  2008/05/31 08:43:55  rbolze
 * add some free on unused pointers (LogService)
 *
 * Revision 1.76  2008/05/16 12:25:55  bisnard
 * API give status of all jobs running or waiting on the SeD
 * (used to compute earliest finish time)
 *
 * Revision 1.75  2008/05/11 16:19:48  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.74  2008/05/05 13:54:19  bisnard
 * new computation time estimation get/set functions
 *
 * Revision 1.73  2008/04/07 15:33:42  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.72  2008/04/07 13:11:44  ycaniou
 * Correct "deprecated conversion from string constant to 'char*'" warnings
 * First attempt to code functions to dynamicaly get batch information
 *      (e.g.,  getNbMaxResources(), etc.)
 *
 * Revision 1.71  2008/01/14 11:16:33  glemahec
 * The servers can now use DAGDA as data manager.
 *
 * Revision 1.70  2008/01/13 21:20:39  glemahec
 * Adds a return in diet_estimate_waiting_jobs function to avoid a warning.
 *
 * Revision 1.69  2008/01/01 19:40:35  ycaniou
 * Modifications for batch management
 *
 * Revision 1.68  2007/12/18 13:04:27  glemahec
 * This commit adds the "diet_estimate_waiting_jobs" function to obtain the
 * number of jobs waiting in the FIFO queue when using the max concurrent
 * jobs limit. This function has to be used in the SeD plugin schedulers.
 *
 * Revision 1.67  2007/12/07 08:44:41  bdepardo
 * Added AckFile support in CMake files.
 * No longer need to add -DADAGE to use it, instead -DHAVE_ACKFILE is automatically added when the option is selected.
 * /!\ Parsing problem on Mac: do not recognize the parameter ackFile within the configuration file.
 *
 * Revision 1.66  2007/07/31 14:36:51  bdepardo
 * Added the support for the option ackFile to touch a file at the end of the initialization.
 * Currently needs to use -D ADAGE when compiling in order to use this feature.
 *
 * Revision 1.65  2007/04/30 13:53:22  ycaniou
 * Cosmetic changes (indentation) and small changes for Cori_Batch
 *
 * Revision 1.64  2007/04/16 22:43:43  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.63  2007/03/27 08:01:54  glemahec
 * Adds the support of the new aggregator type (DIET_AGG_USER) in the diet_aggregator_set_type method (precedently was controlling if the type was DIET_AGG_DEFAULT or DIET_AGG_PRIORITY. Just adds DIET_AGG_USER in the test).
 *
 * Revision 1.62  2006/12/14 11:41:15  aamar
 * Making the SeD display its IOR when starting.
 *
 * Revision 1.61  2006/11/16 09:55:52  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.60  2006/09/18 19:46:06  ycaniou
 * Corrected a bug in file_transfer:server.c
 * Corrected memory leaks due to incorrect free of char *
 * ServiceTable prints if service is sequential or parallel
 * Fully complete examples, whith a batch, a parallel and a sequential server and
 *  a unique client
 *
 * Revision 1.59  2006/09/11 11:06:39  ycaniou
 * Added new function in API: diet_submit_parallel() which is the pendant to
 *   diet_submit_batch() for parallel jobs. NOT YET TESTED!
 *
 * Revision 1.58  2006/08/27 18:40:10  ycaniou
 * Modified parallel submission API
 * - client: diet_call_batch() -> diet_parallel_call()
 * - SeD: diet_profile_desc_set_batch() -> [...]_parallel()
 * - from now, internal fields are related to parallel not batch
 * and corrected a bug:
 * - 3 types of submission: request among only seq, only parallel, or all
 *   available services (second wasn't implemented, third bug)
 *
 * Revision 1.57  2006/07/25 14:34:38  ycaniou
 * Use TRACE_TIME to precise time of downloading, submitting and uploading
 *   datas
 * Use a chained list (and not an array anymore) to manage the correspondance
 *   between DIET requests and batch jobs.
 * Changed the prototype of solve_batch: reqID is in the profile when batch mode
 *   is enabled.
 *
 * Batch management for sync. calls is now fully operationnal (at least for oar;)
 *
 * Revision 1.56  2006/07/11 08:59:09  ycaniou
 * .Batch queue is now read in the serveur config file (only one queue
 * supported).
 * .Transfered perf evaluation in diet server (still dummy function)
 *
 * Revision 1.55  2006/06/30 15:41:46  ycaniou
 * DIET is now capable to submit batch Jobs in synchronous mode. Still some
 *   tuning to do (hard coded NFS path for OAR, tests for synchro between
 *   SeD and the batch job in regard to delete files.., more examples).
 *
 * Put the Data transfer section (JuxMem and DTM) before and after the call to
 * the SeD solve, in inline functions
 *   - downloadSyncSeDData()
 *   - uploadSyncSeDData()
 *
 * Revision 1.54  2006/06/16 10:37:32  abouteil
 * Chandra&Toueg&Aguilera fault detector support added
 *
 * Revision 1.53  2006/05/22 19:55:59  hdail
 * Introduced uniform output format for SeD configuration option output at
 * launch time.
 *
 * Revision 1.52  2006/02/24 10:47:45  rbolze
 * remove this line :
 *   //diet_est_set(ev, EST_TIMESINCELASTSOLVE, timeSinceLastSolve);
 * diet_est_set() function is reserved for SeD developper
 *
 * Revision 1.51  2006/01/31 16:43:02  mjan
 * Update on deployment of DIET/JuxMem via GoDIET
 *
 * Revision 1.50  2006/01/25 21:07:59  pfrauenk
 * CoRI - plugin scheduler: the type diet_est_tag_t est replace by int
 *        some new fonctions in DIET_server.h to manage the estVector
 *
 * Revision 1.48  2006/01/19 21:18:47  pfrauenk
 * CoRI: adding 2 new functions - correction of some comments
 *
 * Revision 1.47  2006/01/13 10:40:39  mjan
 * Updating DIET for next JuxMem (0.2)
 *
 * Revision 1.46  2005/12/20 07:52:44  pfrauenk
 * CoRI functionality added: FAST is hided, information about number of processors,
 * amount of available memory and of free cpu are provided
 *
 * Revision 1.44  2005/08/31 14:41:49  alsu
 * New plugin scheduling interface: implementation of the new public
 * estimation vector interface functions
 *
 * Revision 1.43  2005/08/30 09:20:20  ycaniou
 * Corrected things in DIET_server.cc (diet_submit_batch...)
 * Link libDIET with Elagi and Appleseeds only if BATCH is asked in the
 *   configuration (corrected the Makefile)
 * Changed things in SeDImpl.[ch] for batch submission to work. Only synchronous
 *   mode made.
 ****************************************************************************/

#include <iostream>
#include <unistd.h>  // For gethostname()
#include <cstdlib>
#include <csignal>

#include "DIET_server.h"
#include "DIET_grpc.h"

#include "debug.hh"
#include "est_internal.hh"
#include "marshalling.hh"
#include "ORBMgr.hh"
#include "SeDImpl.hh"
#include "Vector.h"
#include "configuration.hh"

#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#include "MonitoringThread.hh"
#endif

#include "CORIMgr.hh"

#include "DagdaImpl.hh"
#include "DagdaFactory.hh"

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

extern unsigned int TRACE_LEVEL;

BEGIN_API

/****************************************************************************/
/* DIET service table                                                       */
/****************************************************************************/

static ServiceTable* SRVT;

/* We need to keep a pointer to the SeD Impl in order to be able to dynamically
 * add/remove services
 */
static SeDImpl * sedImpl = NULL;

#ifdef HAVE_ALT_BATCH
static diet_server_status_t st = SERIAL;
#endif

int
diet_service_table_init(int maxsize) {
  SRVT = new ServiceTable(maxsize);
  return 0;
}

int
diet_convertor_check(const diet_convertor_t* const cvt,
                     const diet_profile_desc_t* const profile);

static diet_perfmetric_t current_perfmetric_fn = NULL;
diet_perfmetric_t
diet_service_use_perfmetric(diet_perfmetric_t perfmetric_fn) {
  diet_perfmetric_t tmp_fn = current_perfmetric_fn;
  current_perfmetric_fn = perfmetric_fn;
  return (tmp_fn);
}

int
diet_service_table_add(const diet_profile_desc_t* const profile,
                       const diet_convertor_t* const cvt,
                       diet_solve_t solve_func) {
  int res;
  corba_profile_desc_t corba_profile;
  const diet_convertor_t* actual_cvt(NULL);

  if (SRVT == NULL) {
    ERROR(__FUNCTION__ << ": service table not yet initialized", 1);
  }

  mrsh_profile_desc(&corba_profile, profile);
  if (cvt) {
    /* Check the convertor */
    if (diet_convertor_check(cvt, profile)) {
      ERROR("the convertor for profile " << profile->path
            << " is not valid. Please correct above errors first", 1);
    }
    actual_cvt = cvt;
  } else {
    actual_cvt = diet_convertor_alloc(profile->path, profile->last_in,
                                      profile->last_inout, profile->last_out);
#if defined HAVE_ALT_BATCH
    // TODO: Must I add something about convertors ??
#endif

    for (int i = 0; i <= profile->last_out; i++)
      diet_arg_cvt_set(&(actual_cvt->arg_convs[i]),
                       DIET_CVT_IDENTITY, i, NULL, i);
  }
  res = SRVT->addService(&corba_profile,
                         actual_cvt,
                         solve_func,
                         NULL,
                         current_perfmetric_fn);


  /** if the SeD is already running, we need to inform our parent
   * that we added a new service.
   */
  if (sedImpl)
    sedImpl->addService(corba_profile);

  if (!cvt) {
    /*
    ** free it only when the incoming parameter was null (i.e., in
    ** the case where we allocated it
    */
    diet_convertor_free((diet_convertor_t*) actual_cvt);
  }


  return res;
}


int
diet_service_table_remove(const diet_profile_t* const profile) {
  return ((SeDImpl*) profile->SeDPtr)->removeService(profile);
}

int
diet_service_table_remove_desc(const diet_profile_desc_t* const profile) {
  return sedImpl->removeServiceDesc(profile);
}

int
diet_service_table_lookup_by_profile(const diet_profile_t* const profile) {
  /* Called from diet_estimate_fast */
  int refNum;
  corba_profile_desc_t corbaProfile;
  diet_profile_desc_t profileDesc;

  if (profile == NULL) {
    ERROR(__FUNCTION__ << ": NULL profile", -1);
  }

  if (SRVT == NULL) {
    ERROR(__FUNCTION__ << ": service table not yet initialized", -1);
  }

  { /* create the corresponding profile description */
    profileDesc.path = strdup(profile->pb_name);
    profileDesc.last_in = profile->last_in;
    profileDesc.last_inout = profile->last_inout;
    profileDesc.last_out = profile->last_out;
#if defined HAVE_ALT_BATCH
    profileDesc.parallel_flag = profile->parallel_flag;
#endif
    int numArgs = profile->last_out + 1;
    profileDesc.param_desc =
      (diet_arg_desc_t*) calloc (numArgs, sizeof (diet_arg_desc_t));
    for (int argIter = 0; argIter < numArgs; argIter++) {
      profileDesc.param_desc[argIter] =
        (profile->parameters[argIter]).desc.generic;
    }

    profileDesc.aggregator.agg_method = DIET_AGG_DEFAULT;
  }

  mrsh_profile_desc(&corbaProfile, &profileDesc);
  refNum = SRVT->lookupService(&corbaProfile);

  { /* deallocate the dynamic parts of the created profile description */
    free(profileDesc.path);
    free(profileDesc.param_desc);
  }

  return (refNum);
}

void
diet_print_service_table() {
  if (SRVT == NULL) {
    ERROR(__FUNCTION__ << ": service table not yet initialized",);
  }

  SRVT->dump(stdout);
}

/****************************************************************************/
/* DIET service profile descriptor                                          */
/****************************************************************************/

diet_profile_desc_t*
diet_profile_desc_alloc(const char* path, int last_in,
                        int last_inout, int last_out) {
  diet_profile_desc_t* desc(NULL);
  diet_arg_desc_t* param_desc(NULL);

  if ((last_in < -1) || (last_inout < -1) || (last_out < -1)) {
    return NULL;
  }
  if (last_out == -1) {
    param_desc = NULL;
  } else {
    param_desc = new diet_arg_desc_t[last_out + 1];
    if (!param_desc) {
      return NULL;
    }
  }
  desc = new diet_profile_desc_t;
  if (!desc) {
    if (param_desc != NULL) {
      delete[] param_desc;
    }
    return NULL;
  }
  desc->aggregator.agg_method = DIET_AGG_DEFAULT;
  desc->path       = strdup(path);
  desc->last_in    = last_in;
  desc->last_inout = last_inout;
  desc->last_out   = last_out;
  desc->param_desc = param_desc;
#if defined HAVE_ALT_BATCH
  // By default, the profile is registered in the server as sequential
  diet_profile_desc_set_sequential(desc);
  desc->parallel_environment = NULL;
#endif
  return desc;
}

int
diet_profile_desc_free(diet_profile_desc_t* desc) {
  if (!desc) {
    return 1;
  }
  free(desc->path);

  if ((desc->last_out > -1) && desc->param_desc) {
    delete[] desc->param_desc;
    delete desc;
    return 0;
  }

  if ((desc->last_out == -1) && (desc->param_desc == NULL)) {
    delete desc;
    return 0;
  }

  delete desc;
  return 1;
}


/****************************************************************************/
/* DIET aggregation                                                         */
/****************************************************************************/
diet_aggregator_desc_t*
diet_profile_desc_aggregator(diet_profile_desc_t* profile) {
  if (profile == NULL) {
    WARNING(__FUNCTION__ << ": NULL profile");
    return (NULL);
  }
  return (&(profile->aggregator));
}

int
diet_aggregator_set_type(diet_aggregator_desc_t* agg,
                         diet_aggregator_type_t atype) {
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (atype != DIET_AGG_DEFAULT &&
      atype != DIET_AGG_PRIORITY
      /* New : For scheduler load support. */
#ifdef USERSCHED
      && atype != DIET_AGG_USER
#endif
      /*************************************/
) {
    ERROR(__FUNCTION__ << ": unknown aggregation type (" << atype << ")", 0);
  }
  if (agg->agg_method != DIET_AGG_DEFAULT) {
    WARNING(__FUNCTION__ <<
            ": overriding previous aggregation type (" <<
            agg->agg_method);
  }
  agg->agg_method = atype;
  memset(&(agg->agg_specific), 0, sizeof (agg->agg_specific));
  return (1);
}
static int
__diet_agg_pri_add_value(diet_aggregator_priority_t* priority, int value) {
  if (priority->p_numPValues == 0) {
    if ((priority->p_pValues = (int *) calloc(1, sizeof (int))) == NULL) {
      return (0);
    }
    priority->p_pValues[0] = value;
    priority->p_numPValues = 1;
  } else {
    int *newArray = (int *) realloc(priority->p_pValues,
                                    (priority->p_numPValues+1) * sizeof (int));
    if (newArray == NULL) {
      return (0);
    }
    priority->p_pValues = newArray;
    priority->p_pValues[priority->p_numPValues] = value;
    priority->p_numPValues++;
  }

  return (1);
}

int
diet_aggregator_priority_max(diet_aggregator_desc_t* agg, int tag) {
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR(__FUNCTION__ << ": aggregator not a priority list", 0);
  }
  if (! __diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                 tag)) {
    ERROR(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          tag <<
          ")", 0);
  }
  return (1);
}

int
diet_aggregator_priority_min(diet_aggregator_desc_t* agg, int tag) {
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR(__FUNCTION__ << ": aggregator not a priority list", 0);
  }
  if (! __diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                 -tag)) {
    ERROR(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          -tag <<
          ")", 0);
  }
  return (1);
}

int
diet_aggregator_priority_maxuser(diet_aggregator_desc_t* agg, int val) {
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR(__FUNCTION__ << ": aggregator not a priority list", 0);
  }
  if (! __diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                 EST_USERDEFINED + val)) {
    ERROR(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          (EST_USERDEFINED + val) <<
          ")", 0);
  }
  return (1);
}

int
diet_aggregator_priority_minuser(diet_aggregator_desc_t* agg, int val) {
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR(__FUNCTION__ << ": aggregator not a priority list", 0);
  }
  if (! __diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                 -(EST_USERDEFINED + val))) {
    ERROR(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          -(EST_USERDEFINED + val) <<
          ")", 0);
  }
  return (1);
}

/****************************************************************************/
/* DIET problem evaluation                                                  */
/****************************************************************************/

/* The server may declare several services for only one underlying routine.
   Thus, diet_convertors are useful to translate the various declared profiles
   into the actual profile of the underlying routine, ie the profile that is
   used for the FAST benches.
   Internally, when a client requests for a declared service, the corresponding
   convertor is used to generate the actual profile: this allows evaluation
   (cf. below)
*/

int
diet_arg_cvt_set(diet_arg_convertor_t* arg_cvt, diet_convertor_function_t f,
                 int in_arg_idx, diet_arg_t* arg, int out_arg_idx) {
  if (!arg_cvt) {
    return 1;
  }

  arg_cvt->f           = f;
  arg_cvt->in_arg_idx  = in_arg_idx;
  arg_cvt->arg         = arg;
  arg_cvt->out_arg_idx = out_arg_idx;
  return 0;
}

diet_convertor_t*
diet_convertor_alloc(const char* path, int last_in,
                     int last_inout, int last_out) {
  diet_convertor_t* res = new diet_convertor_t;
  res->path       = strdup(path);
  res->last_in    = last_in;
  res->last_inout = last_inout;
  res->last_out   = last_out;
  res->arg_convs  = new diet_arg_convertor_t[last_out + 1];
  for (int i = 0; i < last_out; i++) {
    res->arg_convs[i].f   = DIET_CVT_COUNT;
    res->arg_convs[i].arg = NULL;
  }
  return res;
}

int
diet_convertor_free(diet_convertor_t* cvt) {
  int res = 0;

  if (!cvt) {
    return 1;
  }
  free(cvt->path);
  if ((cvt->last_out <= -1) || !(cvt->arg_convs)) {
    res = 1;
  } else {
#if 0
    for (int i = 0; i < cvt->last_out; i++) {
      if (cvt->arg_convs[i].arg) {
        free(cvt->arg_convs[i].arg);
      }
    }
#endif
    delete [] cvt->arg_convs;
  }
  delete cvt;
  return res;
}

int
diet_convertor_check(const diet_convertor_t* const cvt,
                     const diet_profile_desc_t* const profile) {
  int res = 0;

#define CHECK_ERROR(formatted_text)                                     \
  if (res == 0) {                                                       \
    std::cerr << "DIET ERROR while checking the convertor from "        \
              << profile->path << " to " << cvt->path << ":\n"          \
              << formatted_text << ".\n";                               \
  } res = 1;

  for (int i = 0; i <= cvt->last_out; i++) {
    int in, out;

    if ((((int)cvt->arg_convs[i].f) < 0)
        || (cvt->arg_convs[i].f >= DIET_CVT_COUNT)) {
      CHECK_ERROR("- the argument convertor " << i << " has got a wrong "
                  << "convertor function (" << cvt->arg_convs[i].f << ")");
    }
    in  = cvt->arg_convs[i].in_arg_idx;
    out = cvt->arg_convs[i].out_arg_idx;
    if ((in < 0) || (in > profile->last_out) ||
        (out < 0) || (out > profile->last_out)) {
      if (cvt->arg_convs[i].arg == NULL) {
        CHECK_ERROR("- the argument convertor " << i
                    << " references no argument;\n"
                    << " it should reference either an index of "
                    << "the profile, or a constant argument");
      }
    } else if (cvt->arg_convs[i].arg != NULL) {
      CHECK_ERROR("- the argument convertor " << i
                  << " references too many arguments;\n"
                  << " it should reference either an index of "
                  << "the profile, or a constant argument");
    }
  }
  return res;
}

/****************************************************************************/
/* DIET server call                                                         */
/****************************************************************************/

int
diet_SeD(const char* config_file_name, int argc, char* argv[]) {
  SeDImpl* SeD;
  int    myargc;
  char** myargv;
#ifdef USE_LOG_SERVICE
  DietLogComponent* dietLogComponent;     /* LogService */
  MonitoringThread* monitoringThread;
#endif

  DagdaImpl* dataManager;

  if (SRVT == NULL) {
    ERROR(__FUNCTION__ << ": service table not yet initialized", 1);
  }

  /* Set arguments for ORBMgr::init */
  myargc = argc;
  myargv = (char**)malloc(argc * sizeof(char*));
  for (int i = 0; i < argc; i++)
    myargv[i] = argv[i];

  /* Get configuration file parameters */
  FileParser fileParser;
  try {
    fileParser.parseFile(config_file_name);
  } catch (...) {
    ERROR("while parsing " << config_file_name, DIET_FILE_IO_ERROR);
  }
  CONFIGMAP = fileParser.getConfiguration();
  // FIXME: should we also parse command line arguments?


  /* Check the parameters */
  std::string tmpString;
  if (!CONFIG_STRING(diet::PARENTNAME, tmpString)) {
    ERROR("No parentName found in the configuration", GRPC_CONFIGFILE_ERROR);
  }
  if (CONFIG_STRING(diet::MANAME, tmpString)) {
    WARNING("No need to specify an MA name for a SeD - ignored");
  }


  /* Get listening port & hostname */
  int port;
  std::string host;
  bool hasPort = CONFIG_INT(diet::DIETPORT, port);
  bool hasHost = CONFIG_STRING(diet::DIETHOSTNAME, host);
  if (hasPort || hasHost) {
    std::ostringstream endpoint;
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = strdup("-ORBendPoint");

    endpoint << "giop:tcp:" << host << ":";
    if (hasPort) {
      endpoint << port;
    }

    myargv[myargc + 1] = strdup(endpoint.str().c_str());
    myargc = tmp_argc;
  }

  /* Get the traceLevel */
  unsigned long tmpTraceLevel = TRACE_DEFAULT;
  CONFIG_ULONG(diet::TRACELEVEL, tmpTraceLevel);
  TRACE_LEVEL = tmpTraceLevel;
  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    char *  level = (char *) calloc(48, sizeof(char*));
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = strdup("-ORBtraceLevel");
    sprintf(level, "%u", TRACE_LEVEL - TRACE_MAX_VALUE);
    myargv[myargc + 1] = (char*)level;
    myargc = tmp_argc;
  }

  /* ORB initialization */
  try {
    ORBMgr::init(myargc, (char**)myargv);
  } catch (...) {
    ERROR("ORB initialization failed", 1);
  }


  /* SeD creation */
  SeD = new SeDImpl();
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "## SED_IOR " << ORBMgr::getMgr()->getIOR(SeD->_this()) << "\n");
  fsync(1);
  fflush(NULL);

#ifdef HAVE_ALT_BATCH
  /* Define the role of the SeD: batch, serial, etc. */
  SeD->setServerStatus(st);
  TRACE_TEXT(TRACE_MAIN_STEPS, "setServerStatus " << (int)st << "\n");
#endif

#ifdef USE_LOG_SERVICE
  /* DietLogComponent creation for LogService usage */
  bool useLS = false;


  CONFIG_BOOL(diet::USELOGSERVICE, useLS);
  if (!useLS) {
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService disabled\n");
    dietLogComponent = NULL;
  } else {
    int outBufferSize;
    int flushTime;

    if (!CONFIG_INT(diet::LSOUTBUFFERSIZE, outBufferSize)) {
      outBufferSize = 0;
      WARNING("lsOutbuffersize not configured, using default");
    }

    if (!CONFIG_INT(diet::LSFLUSHINTERVAL, flushTime)) {
      flushTime = 10000;
      WARNING("lsFlushinterval not configured, using default");
    }

    TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: enabled\n");
    std::string parentName;
    CONFIG_STRING(diet::PARENTNAME, parentName);

    dietLogComponent = new DietLogComponent(SeD->getName(), outBufferSize, myargc, (char**)myargv);


    ORBMgr::getMgr()->activate(dietLogComponent);

    if (dietLogComponent->run("SeD", parentName.c_str(), flushTime) != 0) {
      //      delete(dietLogComponent);  // DLC is activated, do not delete !
      WARNING("Could not initialize DietLogComponent");
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled\n");
      dietLogComponent = NULL;
    }
  }

  // Just start the thread, as it might not be FAST-related
  monitoringThread = new MonitoringThread(dietLogComponent);
  SeD->setDietLogComponent(dietLogComponent);
#endif

  /* Activate SeD */
  ORBMgr::getMgr()->activate(SeD);
  if (SeD->run(SRVT)) {
    ERROR("unable to launch the SeD", 1);
  }

  dataManager = DagdaFactory::getSeDDataManager();
#ifdef USE_LOG_SERVICE
  dataManager->setLogComponent(dietLogComponent);  // modif bisnard_logs_1
#endif

  ORBMgr::getMgr()->activate(dataManager);
  SeD->setDataManager(dataManager);

  sedImpl = SeD;

  /* Wait for RPCs : */
  ORBMgr::getMgr()->wait();

  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  SeD->removeElementClean();
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  delete ORBMgr::getMgr();

  TRACE_TEXT(TRACE_ALL_STEPS, "SeD has exited\n");

  return 0;
}

int
diet_est_set(estVector_t ev, int userTag, double value) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }

  return (diet_est_set_internal(ev, userTag + EST_USERDEFINED, value));
}

double
diet_est_get(estVectorConst_t ev, int userTag, double errVal) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", errVal);
  }

  return (diet_est_get_internal(ev, userTag + EST_USERDEFINED, errVal));
}

double
diet_est_get_system(estVectorConst_t ev, int systemTag, double errVal) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": systemTag must be non-negative (" <<
          systemTag <<
          ")", errVal);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")", errVal);
  }
  return (diet_est_get_internal(ev, systemTag, errVal));
}

int
diet_est_defined(estVectorConst_t ev, int userTag) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }

  return (diet_est_defined_internal(ev, userTag + EST_USERDEFINED));
}

int
diet_est_defined_system(estVectorConst_t ev, int systemTag) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")", -1);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")", -1);
  }

  return (diet_est_defined_internal(ev, systemTag));
}

int
diet_est_array_size(estVectorConst_t ev, int userTag) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }

  return (diet_est_array_size_internal(ev, userTag + EST_USERDEFINED));
}

int
diet_est_array_size_system(estVectorConst_t ev, int systemTag) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")", -1);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")", -1);
  }

  return (diet_est_array_size_internal(ev, systemTag));
}

int
diet_est_array_set(estVector_t ev, int userTag, int idx, double value) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ << ": idx must be non-negative (" << idx << ")", -1);
  }

  return (diet_est_array_set_internal(ev,
                                      userTag + EST_USERDEFINED,
                                      idx,
                                      value));
}

double
diet_est_array_get(estVectorConst_t ev, int userTag, int idx, double errVal) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", errVal);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ <<
          ": idx must be non-negative (" <<
          idx <<
          ")", errVal);
  }

  return diet_est_array_get_internal(ev,
                                     userTag + EST_USERDEFINED,
                                     idx,
                                     errVal);
}

double
diet_est_array_get_system(estVectorConst_t ev, int systemTag,
                          int idx, double errVal) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")", errVal);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ <<
          ": idx must be non-negative (" <<
          idx <<
          ")", errVal);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")", errVal);
  }
  return diet_est_array_get_internal(ev,
                                     systemTag,
                                     idx,
                                     errVal);
}


int
diet_est_array_defined(estVectorConst_t ev, int userTag, int idx) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ << ": idx must be non-negative (" << idx << ")", -1);
  }

  return diet_est_array_defined_internal(ev,
                                         userTag + EST_USERDEFINED,
                                         idx);
}

int
diet_est_array_defined_system(estVectorConst_t ev, int systemTag, int idx) {
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")", -1);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ << ": idx must be non-negative (" << idx << ")", -1);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")", -1);
  }
  return diet_est_array_defined_internal(ev,
                                         systemTag,
                                         idx);
}

#ifdef HAVE_ALT_BATCH
/* These two functions shall be removed and a better mechanism found
 * for example vhen and if CoRI is rewritten.
 */
estVector_t
diet_new_estVect() {
  return new corba_estimation_t();
}

void
diet_destroy_estVect(estVector_t perfVect) {
  delete perfVect;
}
#endif  // HAVE_ALT_BATCH

int
diet_estimate_cori(estVector_t ev,
                   int info_type,
                   diet_est_collect_tag_t collector_type,
                   const void * data) {
  switch (collector_type) {
  case EST_COLL_EASY:
  case EST_COLL_BATCH:
    CORIMgr::call_cori_mgr(&ev, info_type, collector_type, data);
    break;
  case EST_COLL_GANGLIA:
  case EST_COLL_NAGIOS:
  default:
    ERROR("Requested collector not implemented", -1);
  }
  return 0;
}

int
diet_estimate_cori_add_collector(diet_est_collect_tag_t collector_type,
                                 void * data) {
  return CORIMgr::add(collector_type, data);
}

void
print_message() {
  std::cerr << "=default value used\n";
}

void
diet_estimate_coriEasy_print() {
  int tmp_int = TRACE_LEVEL;
  TRACE_LEVEL = 15;

  std::cerr << "start printing CoRI values..\n";
  estVector_t vec = new corba_estimation_t();

  CORIMgr::add(EST_COLL_EASY, NULL);

  int minut = 15;

  if (diet_estimate_cori(vec, EST_AVGFREECPU, EST_COLL_EASY, &minut)) {
    print_message();
  }

  if (diet_estimate_cori(vec, EST_CACHECPU, EST_COLL_EASY, NULL)) {
    print_message();
  }

  if (diet_estimate_cori(vec, EST_NBCPU, EST_COLL_EASY, NULL)) {
    print_message();
  }

  if (diet_estimate_cori(vec, EST_BOGOMIPS, EST_COLL_EASY, NULL)) {
    print_message();
  }

  const char * tmp = "./";

  if (diet_estimate_cori(vec, EST_DISKACCESREAD, EST_COLL_EASY, tmp)) {
    print_message();
  }

  if (diet_estimate_cori(vec, EST_DISKACCESWRITE, EST_COLL_EASY, tmp)) {
    print_message();
  }

  if (diet_estimate_cori(vec, EST_TOTALSIZEDISK, EST_COLL_EASY, tmp)) {
    print_message();
  }

  if (diet_estimate_cori(vec, EST_FREESIZEDISK, EST_COLL_EASY, tmp)) {
    print_message();
  }

  if (diet_estimate_cori(vec, EST_TOTALMEM, EST_COLL_EASY, NULL)) {
    print_message();
  }

  if (diet_estimate_cori(vec, EST_FREEMEM, EST_COLL_EASY, NULL)) {
    print_message();
  }

  std::cerr << "end printing CoRI values\n";
  TRACE_LEVEL = tmp_int;

  /* FIXME (YC->ANY): release vec ? */
}


int
diet_estimate_lastexec(estVector_t ev,
                       const diet_profile_t* const profilePtr) {
  const SeDImpl* refSeD = (SeDImpl*) profilePtr->SeDPtr;
  double timeSinceLastSolve;
  const struct timeval* lastSolveStartPtr;
  struct timeval currentTime;

  /* casting away const-ness, because we know that the
   * method doesn't change the SeD
   */
  lastSolveStartPtr = ((SeDImpl*) refSeD)->timeSinceLastSolve();

  gettimeofday(&currentTime, NULL);
  timeSinceLastSolve = ((double) currentTime.tv_sec -
                        (double) lastSolveStartPtr->tv_sec +
                        (((double) currentTime.tv_usec -
                          (double) lastSolveStartPtr->tv_usec) /
                         1000000.0));
  /* store the value in the performance data array */
  diet_est_set_internal(ev, EST_TIMESINCELASTSOLVE, timeSinceLastSolve);
  return 1;
}

int
diet_estimate_comptime(estVector_t ev, double value) {
  diet_est_set_internal(ev, EST_TCOMP, value);
  return 0;
}

/* Get the number of waiting jobs in the queue. */
/* TODO : Add to the documentation. */
int
diet_estimate_waiting_jobs(estVector_t ev,
                           const diet_profile_t* const profilePtr) {
  const SeDImpl* refSeD = (SeDImpl*) profilePtr->SeDPtr;

  if (refSeD != NULL) {
    /* casting away const-ness, because we know that the
     * method doesn't change the SeD
     */
    diet_est_set_internal(ev, EST_NUMWAITINGJOBS,
                          ((SeDImpl*) refSeD)->getNumJobsWaiting());
    return 0;
  } else {
    INTERNAL_ERROR(__FUNCTION__ <<": ref on SeD not initialized?", 1);
  }
}

/* Get the list of all running and waiting jobs */
/* (Caller is responsible for freeing the result) */
int
diet_estimate_list_jobs(jobVector_t* jv, int* jobNb,
                        const diet_profile_t* const profilePtr) {
  const SeDImpl* refSeD = (SeDImpl*) profilePtr->SeDPtr;
  /* const_cast required due to lock/unlock mutex when getting jobs */
  SeDImpl* ncrefSeD = const_cast<SeDImpl*> (refSeD);
  if (ncrefSeD != NULL) {
    *jobNb = ncrefSeD->getActiveJobVector(*jv);
    return 0;
  } else {
    INTERNAL_ERROR(__FUNCTION__ <<": ref on SeD not initialized?", 1);
  }
}

/* Get the estimated Earliest Finish Time for a job with given duration est.*/
int
diet_estimate_eft(estVector_t ev,
                  double  jobEstimatedCompTime,
                  const diet_profile_t* const profilePtr) {
  const SeDImpl* refSeD = (SeDImpl*) profilePtr->SeDPtr;
  if (refSeD == NULL) {
    ERROR(__FUNCTION__ <<": ref on SeD not initialized?", 1);
  }
  /* casting away const-ness, because we know that the
   * method doesn't change the SeD
   */
  double value = jobEstimatedCompTime + ((SeDImpl*) refSeD)->getEFT();
  diet_est_set_internal(ev, EST_EFT, value);
  return 0;
}

/****************************************************************************/
/* DIET batch submit call                                                   */
/****************************************************************************/

#ifdef HAVE_ALT_BATCH
/* TODO (YC): put me in right place in this file */
void
diet_set_server_status(diet_server_status_t status) {
  if ((status > -1) && (status<NB_SERVER_STATUS)) {
    st = status;
    switch ((int)st) {
    case BATCH:
      TRACE_TEXT(TRACE_MAIN_STEPS, "SeD is batch\n");
      break;
    case SERIAL:
      TRACE_TEXT(TRACE_MAIN_STEPS, "SeD is sequential\n");
      break;
    default:
      TRACE_TEXT(TRACE_MAIN_STEPS, "Server status list to update\n");
    }
  } else ERROR_EXIT("Server status not recognized");
}
#endif

#ifdef HAVE_ALT_BATCH
int
diet_submit_parallel(diet_profile_t * profile,
                     const char * addon_prologue,
                     const char * command) {
  return ((((SeDImpl*)profile->SeDPtr)->getBatch())->
          diet_submit_parallel(profile, addon_prologue, command));
}

/* This is to be used later: a SeD can manage a reservation as he wants. It
   can partition the reserved procs, launch several pb resolutions with
   tasks overlapping or not */
int
diet_concurrent_submit_parallel(int batchJobID, diet_profile_t * profile,
                                const char * command) {
  return (((SeDImpl*)profile->SeDPtr)->getBatch())->
    diet_submit_parallel(batchJobID, profile, command);
}

/* Used to explicitely wait for the completion of a batch job */
int
diet_wait_batch_job_completion(diet_profile_t * profile) {
  return ((SeDImpl*) profile->SeDPtr)->getBatch()->
    wait4BatchJobCompletion(((SeDImpl*) profile->SeDPtr)->
                            getBatch()->getBatchJobID(profile->dietReqID));
}
#endif

int
diet_get_SeD_services(int *services_number,
                      diet_profile_desc_t ***profiles,
                      const char *SeDName) {
  *services_number = 0;
  *profiles = NULL;

  /* Find the SeD */
  if (SeDName) {
    try {
      SeD_var sed = NULL;
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Searching SeD " << SeDName << "\n");
      sed = ORBMgr::getMgr()->resolve<SeD, SeD_ptr>(SEDCTXT, SeDName);

      if (CORBA::is_nil(sed)) {
        ERROR("Cannot locate SeD " << SeDName, GRPC_SERVER_NOT_FOUND);
      }

      // Now retreive the services
      CORBA::Long length;
      SeqCorbaProfileDesc_t* profileList = sed->getSeDProfiles(length);
      *services_number= (int)length;
      *profiles =
        (diet_profile_desc_t**)calloc(length, sizeof(diet_profile_desc_t*));

      for (int i = 0; i < *services_number; i++) {
        (*profiles)[i] = new diet_profile_desc_t;
        unmrsh_profile_desc((*profiles)[i], &((*profileList)[i]));
      }
    } catch (...) {
      // TODO catch exceptions correctly
      ERROR("An exception has been caught while searching for SeD "
            << SeDName, GRPC_SERVER_NOT_FOUND);
    }
  } else {
    ERROR("No SeDName has been given", GRPC_SERVER_NOT_FOUND);
  }

  return 0;
}

END_API
