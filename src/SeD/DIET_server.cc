/**
 * @file DIET_server.cc
 *
 * @brief  DIET server interface
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include "deltacloud_config.h"

#include <iostream>
#include <cstdlib>
#include <csignal>

#include "DIET_server.h"
#include "DIET_grpc.h"

#include "OSIndependance.hh"
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

#ifdef DIET_USE_DELTACLOUD
#include "Tools.hh"
#include "boost/foreach.hpp"
#endif

#define BEGIN_API extern "C" {
#define END_API }   // extern "C"

extern unsigned int TRACE_LEVEL;


template <typename C>
class CStringInserter {
public:
explicit
CStringInserter(C &c): c_(c) {
}

void
operator()(const char *cstr) {
  c_.push_back(strdup(cstr));
}

void
operator()(std::ostringstream &oss) {
  char *cstr = strdup(oss.str().c_str());
  c_.push_back(cstr);
}

private:
C &c_;
};


BEGIN_API

/****************************************************************************/
/* DIET service table                                                       */
/****************************************************************************/

static ServiceTable *SRVT;

/* We need to keep a pointer to the SeD Impl in order to be able to dynamically
 * add/remove services
 */
static SeDImpl *sedImpl = NULL;

#ifdef HAVE_ALT_BATCH
static diet_server_status_t st = SERIAL;
#endif


int
diet_service_table_init(int maxsize) {
  SRVT = new ServiceTable(maxsize);
  return 0;
}

int
diet_convertor_check(const diet_convertor_t *const cvt,
                     const diet_profile_desc_t *const profile);

static diet_perfmetric_t current_perfmetric_fn = NULL;
diet_perfmetric_t
diet_service_use_perfmetric(diet_perfmetric_t perfmetric_fn) {
  diet_perfmetric_t tmp_fn = current_perfmetric_fn;
  current_perfmetric_fn = perfmetric_fn;
  return (tmp_fn);
}

int
diet_service_table_add(const diet_profile_desc_t *const profile,
                       const diet_convertor_t *const cvt,
                       diet_solve_t solve_func) {
  int res;
  corba_profile_desc_t corba_profile;
  const diet_convertor_t *actual_cvt(NULL);

  if (SRVT == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": service table not yet initialized", 1);
  }

  mrsh_profile_desc(&corba_profile, profile);
  if (cvt) {
    /* Check the convertor */
    if (diet_convertor_check(cvt, profile)) {
      ERROR_DEBUG(
        "the convertor for profile " << profile->path
                                     <<
        " is not valid. Please correct above errors first", 1);
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
  if (sedImpl) {
    sedImpl->addService(corba_profile);
  }

  if (!cvt) {
    /*
    ** free it only when the incoming parameter was null (i.e., in
    ** the case where we allocated it
    */
    diet_convertor_free((diet_convertor_t *) actual_cvt);
  }


  return res;
} // diet_service_table_add


int
diet_service_table_remove(const diet_profile_t *const profile) {
  return ((SeDImpl *) profile->SeDPtr)->removeService(profile);
}

int
diet_service_table_remove_desc(const diet_profile_desc_t *const profile) {
  return sedImpl->removeServiceDesc(profile);
}

int
diet_service_table_lookup_by_profile(const diet_profile_t *const profile) {
  /* Called from diet_estimate_fast */
  int refNum;
  corba_profile_desc_t corbaProfile;
  diet_profile_desc_t profileDesc;

  if (profile == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL profile", -1);
  }

  if (SRVT == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": service table not yet initialized", -1);
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
      (diet_arg_desc_t *) calloc(numArgs, sizeof(diet_arg_desc_t));
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
} // diet_service_table_lookup_by_profile

void
diet_print_service_table() {
  if (SRVT == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": service table not yet initialized", );
  }

  SRVT->dump(stdout);
}

/****************************************************************************/
/* DIET service profile descriptor                                          */
/****************************************************************************/

diet_profile_desc_t *
diet_profile_desc_alloc(const char *path, int last_in,
                        int last_inout, int last_out) {
  diet_profile_desc_t *desc(NULL);
  diet_arg_desc_t *param_desc(NULL);

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
  desc->path = strdup(path);
  desc->last_in = last_in;
  desc->last_inout = last_inout;
  desc->last_out = last_out;
  desc->param_desc = param_desc;
#if defined HAVE_ALT_BATCH
  // By default, the profile is registered in the server as sequential
  diet_profile_desc_set_sequential(desc);
  desc->parallel_environment = NULL;
#endif
  return desc;
} // diet_profile_desc_alloc

int
diet_profile_desc_free(diet_profile_desc_t *desc) {
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
} // diet_profile_desc_free


/****************************************************************************/
/* DIET aggregation                                                         */
/****************************************************************************/
diet_aggregator_desc_t *
diet_profile_desc_aggregator(diet_profile_desc_t *profile) {
  if (profile == NULL) {
    WARNING(__FUNCTION__ << ": NULL profile");
    return (NULL);
  }
  return (&(profile->aggregator));
}

int
diet_aggregator_set_type(diet_aggregator_desc_t *agg,
                         diet_aggregator_type_t atype) {
  if (agg == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (atype != DIET_AGG_DEFAULT &&
      atype != DIET_AGG_PRIORITY
      /* New : For scheduler load support. */
#ifdef USERSCHED
      && atype != DIET_AGG_USER
#endif
      /*************************************/
      ) {
    ERROR_DEBUG(__FUNCTION__ << ": unknown aggregation type (" << atype << ")", 0);
  }
  if (agg->agg_method != DIET_AGG_DEFAULT) {
    WARNING(__FUNCTION__ <<
            ": overriding previous aggregation type (" <<
            agg->agg_method);
  }
  agg->agg_method = atype;
  memset(&(agg->agg_specific), 0, sizeof(agg->agg_specific));
  return (1);
} // diet_aggregator_set_type
static int
__diet_agg_pri_add_value(diet_aggregator_priority_t *priority, int value) {
  if (priority->p_numPValues == 0) {
    if ((priority->p_pValues = (int *) calloc(1, sizeof(int))) == NULL) {
      return (0);
    }
    priority->p_pValues[0] = value;
    priority->p_numPValues = 1;
  } else {
    int *newArray = (int *) realloc(priority->p_pValues,
                                    (priority->p_numPValues + 1) * sizeof(int));
    if (newArray == NULL) {
      return (0);
    }
    priority->p_pValues = newArray;
    priority->p_pValues[priority->p_numPValues] = value;
    priority->p_numPValues++;
  }

  return (1);
} // __diet_agg_pri_add_value

int
diet_aggregator_priority_max(diet_aggregator_desc_t *agg, int tag) {
  if (agg == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR_DEBUG(__FUNCTION__ << ": aggregator not a priority list", 0);
  }
  if (!__diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                tag)) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          tag <<
          ")", 0);
  }
  return (1);
} // diet_aggregator_priority_max

int
diet_aggregator_priority_min(diet_aggregator_desc_t *agg, int tag) {
  if (agg == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR_DEBUG(__FUNCTION__ << ": aggregator not a priority list", 0);
  }
  if (!__diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                -tag)) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          -tag <<
          ")", 0);
  }
  return (1);
} // diet_aggregator_priority_min

int
diet_aggregator_priority_maxuser(diet_aggregator_desc_t *agg, int val) {
  if (agg == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR_DEBUG(__FUNCTION__ << ": aggregator not a priority list", 0);
  }
  if (!__diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                EST_USERDEFINED + val)) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          (EST_USERDEFINED + val) <<
          ")", 0);
  }
  return (1);
} // diet_aggregator_priority_maxuser

int
diet_aggregator_priority_minuser(diet_aggregator_desc_t *agg, int val) {
  if (agg == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL aggregator", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR_DEBUG(__FUNCTION__ << ": aggregator not a priority list", 0);
  }
  if (!__diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                -(EST_USERDEFINED + val))) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          -(EST_USERDEFINED + val) <<
          ")", 0);
  }
  return (1);
} // diet_aggregator_priority_minuser

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
diet_arg_cvt_set(diet_arg_convertor_t *arg_cvt, diet_convertor_function_t f,
                 int in_arg_idx, diet_arg_t *arg, int out_arg_idx) {
  if (!arg_cvt) {
    return 1;
  }

  arg_cvt->f = f;
  arg_cvt->in_arg_idx = in_arg_idx;
  arg_cvt->arg = arg;
  arg_cvt->out_arg_idx = out_arg_idx;
  return 0;
} // diet_arg_cvt_set

diet_convertor_t *
diet_convertor_alloc(const char *path, int last_in,
                     int last_inout, int last_out) {
  diet_convertor_t *res = new diet_convertor_t;
  res->path = strdup(path);
  res->last_in = last_in;
  res->last_inout = last_inout;
  res->last_out = last_out;
  res->arg_convs = new diet_arg_convertor_t[last_out + 1];
  for (int i = 0; i < last_out; i++) {
    res->arg_convs[i].f = DIET_CVT_COUNT;
    res->arg_convs[i].arg = NULL;
  }
  return res;
} // diet_convertor_alloc

int
diet_convertor_free(diet_convertor_t *cvt) {
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
} // diet_convertor_free

int
diet_convertor_check(const diet_convertor_t *const cvt,
                     const diet_profile_desc_t *const profile) {
  int res = 0;

#define CHECK_ERROR(formatted_text)                                     \
  if (res == 0) {                                                       \
    std::cerr << "DIET ERROR while checking the convertor from "        \
              << profile->path << " to " << cvt->path << ":\n"          \
              << formatted_text << ".\n";                               \
  } res = 1;

  for (int i = 0; i <= cvt->last_out; i++) {
    int in, out;

    if ((((int) cvt->arg_convs[i].f) < 0)
        || (cvt->arg_convs[i].f >= DIET_CVT_COUNT)) {
      CHECK_ERROR(
        "- the argument convertor " << i << " has got a wrong "
                                    << "convertor function (" <<
        cvt->arg_convs[i].f << ")");
    }
    in = cvt->arg_convs[i].in_arg_idx;
    out = cvt->arg_convs[i].out_arg_idx;
    if ((in < 0) || (in > profile->last_out) ||
        (out < 0) || (out > profile->last_out)) {
      if (cvt->arg_convs[i].arg == NULL) {
        CHECK_ERROR(
          "- the argument convertor " << i
                                      << " references no argument;\n"
                                      <<
          " it should reference either an index of "
                                      <<
          "the profile, or a constant argument");
      }
    } else if (cvt->arg_convs[i].arg != NULL) {
      CHECK_ERROR(
        "- the argument convertor " << i
                                    <<
        " references too many arguments;\n"
                                    <<
        " it should reference either an index of "
                                    <<
        "the profile, or a constant argument");
    }
  }
  return res;
} // diet_convertor_check

/****************************************************************************/
/* DIET server call                                                         */
/****************************************************************************/

int
diet_SeD(const char *config_file_name, int argc, char *argv[]) {
  SeDImpl *SeD;
  // use std::vector instead of C array
  // C++ standard guarantees that its storage is contiguous (C++ Faq 34.3)
  std::vector<char *> args, argsTmp;
  CStringInserter<std::vector<char *> > ins(args);
#ifdef USE_LOG_SERVICE
  DietLogComponent *dietLogComponent;     /* LogService */
  MonitoringThread *monitoringThread;
#endif

  DagdaImpl *dataManager;

  if (SRVT == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": service table not yet initialized", 1);
  }

  /* Set arguments for ORBMgr::init */
  for (int i = 0; i < argc; i++)
	  ins(argv[i]);

  /* Get configuration file parameters */
  FileParser fileParser;
  try {
    fileParser.parseFile(config_file_name);
  } catch (...) {
    ERROR_DEBUG("while parsing " << config_file_name, DIET_FILE_IO_ERROR);
  }
  CONFIGMAP = fileParser.getConfiguration();
  // FIXME: should we also parse command line arguments?


  /* Check the parameters */
  std::string tmpString;
  if (!CONFIG_STRING(diet::PARENTNAME, tmpString)) {
    ERROR_DEBUG("No parentName found in the configuration", GRPC_CONFIGFILE_ERROR);
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
		  ins("-ORBendPoint");
		  endpoint << "giop:tcp:" << host << ":";
		  if (hasPort) {
			  endpoint << port;
		  }
		  ins(endpoint);
	}
  else {
	  ins("-ORBendPointPublish");
	  ins("all(addr)");
  }


  /* Get the traceLevel */
  unsigned long tmpTraceLevel = TRACE_DEFAULT;
  CONFIG_ULONG(diet::TRACELEVEL, tmpTraceLevel);
  TRACE_LEVEL = tmpTraceLevel;
  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    char *level = (char *) calloc(48, sizeof(char *));
   ins("-ORBtraceLevel");
    sprintf(level, "%u", TRACE_LEVEL - TRACE_MAX_VALUE);
    ins(level);
  }

  argsTmp = args;
  /* ORB initialization */
  try {
    ORBMgr::init(argsTmp.size(), &argsTmp[0]);
  } catch (...) {
    ERROR_DEBUG("ORB initialization failed", 1);
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
  TRACE_TEXT(TRACE_MAIN_STEPS, "setServerStatus " << (int) st << "\n");
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

    dietLogComponent = new DietLogComponent(
      SeD->getName(), outBufferSize, args.size(), &args[0]);


    ORBMgr::getMgr()->activate(dietLogComponent);

    if (dietLogComponent->run("SeD", parentName.c_str(), flushTime) != 0) {
      // delete(dietLogComponent);  // DLC is activated, do not delete !
      WARNING("Could not initialize DietLogComponent");
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled\n");
      dietLogComponent = NULL;
    }
  }

  // Just start the thread, as it might not be FAST-related
  monitoringThread = new MonitoringThread(dietLogComponent);
  SeD->setDietLogComponent(dietLogComponent);
#endif // ifdef USE_LOG_SERVICE

  /* Activate SeD */
  ORBMgr::getMgr()->activate(SeD);
  if (SeD->run(SRVT)) {
    ERROR_DEBUG("unable to launch the SeD", 1);
  }

  dataManager = DagdaFactory::getSeDDataManager();
#ifdef USE_LOG_SERVICE
  dataManager->setLogComponent(dietLogComponent);  // modif bisnard_logs_1
#ifdef DIET_USE_DELTACLOUD
   if (dietLogComponent != NULL) {
     BOOST_FOREACH(wrapped_service_log entry, wrappedServicesList) {
       dietLogComponent->logVMServiceWrapped(*entry.serviceWrapper, entry.vmIP, entry.vmUserName);
     }
   }
#endif // DIET_USE_DELTACLOUD
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
  ORBMgr::kill();

  TRACE_TEXT(TRACE_ALL_STEPS, "SeD has exited\n");

  return 0;
} // diet_SeD

int
diet_est_set(estVector_t ev, int userTag, double value) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }

  return (diet_est_set_internal(ev, userTag + EST_USERDEFINED, value));
} // diet_est_set

double
diet_est_get(estVectorConst_t ev, int userTag, double errVal) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", errVal);
  }

  return (diet_est_get_internal(ev, userTag + EST_USERDEFINED, errVal));
} // diet_est_get

double
diet_est_get_system(estVectorConst_t ev, int systemTag, double errVal) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (systemTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag must be non-negative (" <<
          systemTag <<
          ")", errVal);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag " << systemTag << " must be smaller than (" <<
          EST_USERDEFINED <<
          ")", errVal);
  }
  return (diet_est_get_internal(ev, systemTag, errVal));
} // diet_est_get_system

int
diet_est_defined(estVectorConst_t ev, int userTag) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }

  return (diet_est_defined_internal(ev, userTag + EST_USERDEFINED));
} // diet_est_defined

int
diet_est_defined_system(estVectorConst_t ev, int systemTag) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")", -1);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag " << systemTag << " must be smaller than (" <<
          EST_USERDEFINED <<
          ")", -1);
  }

  return (diet_est_defined_internal(ev, systemTag));
} // diet_est_defined_system

int
diet_est_array_size(estVectorConst_t ev, int userTag) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }

  return (diet_est_array_size_internal(ev, userTag + EST_USERDEFINED));
} // diet_est_array_size

int
diet_est_array_size_system(estVectorConst_t ev, int systemTag) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")", -1);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag " << systemTag << " must be smaller than (" <<
          EST_USERDEFINED <<
          ")", -1);
  }

  return (diet_est_array_size_internal(ev, systemTag));
} // diet_est_array_size_system

int
diet_est_array_set(estVector_t ev, int userTag, int idx, double value) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }
  if (idx < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": idx must be non-negative (" << idx << ")", -1);
  }

  return (diet_est_array_set_internal(ev,
                                      userTag + EST_USERDEFINED,
                                      idx,
                                      value));
} // diet_est_array_set

double
diet_est_array_get(estVectorConst_t ev, int userTag, int idx, double errVal) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", errVal);
  }
  if (idx < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": idx must be non-negative (" <<
          idx <<
          ")", errVal);
  }

  return diet_est_array_get_internal(ev,
                                     userTag + EST_USERDEFINED,
                                     idx,
                                     errVal);
} // diet_est_array_get

double
diet_est_array_get_system(estVectorConst_t ev, int systemTag,
                          int idx, double errVal) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (systemTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")", errVal);
  }
  if (idx < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": idx must be non-negative (" <<
          idx <<
          ")", errVal);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag " << systemTag << " must be smaller than (" <<
          EST_USERDEFINED <<
          ")", errVal);
  }
  return diet_est_array_get_internal(ev,
                                     systemTag,
                                     idx,
                                     errVal);
} // diet_est_array_get_system


int
diet_est_array_defined(estVectorConst_t ev, int userTag, int idx) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }
  if (idx < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": idx must be non-negative (" << idx << ")", -1);
  }

  return diet_est_array_defined_internal(ev,
                                         userTag + EST_USERDEFINED,
                                         idx);
} // diet_est_array_defined

int
diet_est_array_defined_system(estVectorConst_t ev, int systemTag, int idx) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")", -1);
  }
  if (idx < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": idx must be non-negative (" << idx << ")", -1);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag " << systemTag << " must be smaller than (" <<
          EST_USERDEFINED <<
          ")", -1);
  }
  return diet_est_array_defined_internal(ev,
                                         systemTag,
                                         idx);
} // diet_est_array_defined_system

int
diet_est_set_str(estVector_t ev, int userTag, const char *value) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }
  return (diet_est_set_str_internal(ev, userTag + EST_USERDEFINED, value));
}

const char*
diet_est_get_str(estVectorConst_t ev, int userTag) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", NULL);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", NULL);
  }
  return (diet_est_get_str_internal(ev, userTag + EST_USERDEFINED));
}

const char*
diet_est_get_str_system(estVectorConst_t ev, int systemTag) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", NULL);
  }
  if (systemTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag must be non-negative (" <<
          systemTag <<
          ")", NULL);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag " << systemTag << " must be smaller than (" <<
          EST_USERDEFINED <<
          ")", NULL);
  }
  return (diet_est_get_str_internal(ev, systemTag));
}

int
diet_est_set_bin(estVector_t ev, int userTag, const unsigned char *buf, size_t size) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }
  return (diet_est_set_bin_internal(ev, userTag + EST_USERDEFINED, buf, size));
}

int
diet_est_get_bin(estVectorConst_t ev, int userTag, const unsigned char **buf, size_t *size) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")", -1);
  }
  return (diet_est_get_bin_internal(ev, userTag + EST_USERDEFINED, buf, size));
}

int
diet_est_get_bin_system(estVectorConst_t ev, int systemTag, const unsigned char **buf, size_t *size) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag must be non-negative (" <<
          systemTag <<
          ")", -1);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": systemTag " << systemTag << " must be smaller than (" <<
          EST_USERDEFINED <<
          ")", -1);
  }
  return (diet_est_get_bin_internal(ev, systemTag, buf, size));
}

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

int
diet_estimate_cori(estVector_t ev,
                   int info_type,
                   diet_est_collect_tag_t collector_type,
                   const void *data) {
  switch (collector_type) {
  case EST_COLL_EASY:
  case EST_COLL_BATCH:
    CORIMgr::call_cori_mgr(&ev, info_type, collector_type, data);
    break;
  case EST_COLL_GANGLIA:
  case EST_COLL_NAGIOS:
  default:
    ERROR_DEBUG("Requested collector not implemented", -1);
  }
  return 0;
} // diet_estimate_cori

int
diet_estimate_cori_add_collector(diet_est_collect_tag_t collector_type,
                                 void *data) {
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

  const char *tmp = "./";

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
} // diet_estimate_coriEasy_print


int
diet_estimate_lastexec(estVector_t ev,
                       const diet_profile_t *const profilePtr) {
  const SeDImpl *refSeD = (SeDImpl *) profilePtr->SeDPtr;
  double timeSinceLastSolve;
  const struct timeval *lastSolveStartPtr;
  struct timeval currentTime;

  /* casting away const-ness, because we know that the
   * method doesn't change the SeD
   */
  lastSolveStartPtr = ((SeDImpl *) refSeD)->timeSinceLastSolve();

  gettimeofday(&currentTime, NULL);
  timeSinceLastSolve = ((double) currentTime.tv_sec -
                        (double) lastSolveStartPtr->tv_sec +
                        (((double) currentTime.tv_usec -
                          (double) lastSolveStartPtr->tv_usec) /
                         1000000.0));
  /* store the value in the performance data array */
  diet_est_set_internal(ev, EST_TIMESINCELASTSOLVE, timeSinceLastSolve);
  return 1;
} // diet_estimate_lastexec

int
diet_estimate_comptime(estVector_t ev, double value) {
  diet_est_set_internal(ev, EST_TCOMP, value);
  return 0;
}

/* Get the number of waiting jobs in the queue. */
/* TODO : Add to the documentation. */
int
diet_estimate_waiting_jobs(estVector_t ev,
                           const diet_profile_t *const profilePtr) {
  const SeDImpl *refSeD = (SeDImpl *) profilePtr->SeDPtr;

  if (refSeD != NULL) {
    /* casting away const-ness, because we know that the
     * method doesn't change the SeD
     */
    diet_est_set_internal(ev, EST_NUMWAITINGJOBS,
                          ((SeDImpl *) refSeD)->getNumJobsWaiting());
    return 0;
  } else {
    INTERNAL_ERROR(__FUNCTION__ << ": ref on SeD not initialized?", 1);
  }
} // diet_estimate_waiting_jobs

/* Get the list of all running and waiting jobs */
/* (Caller is responsible for freeing the result) */
int
diet_estimate_list_jobs(jobVector_t *jv, int *jobNb,
                        const diet_profile_t *const profilePtr) {
  const SeDImpl *refSeD = (SeDImpl *) profilePtr->SeDPtr;
  /* const_cast required due to lock/unlock mutex when getting jobs */
  SeDImpl *ncrefSeD = const_cast<SeDImpl *> (refSeD);
  if (ncrefSeD != NULL) {
    *jobNb = ncrefSeD->getActiveJobVector(*jv);
    return 0;
  } else {
    INTERNAL_ERROR(__FUNCTION__ << ": ref on SeD not initialized?", 1);
  }
} // diet_estimate_list_jobs

/* Get the estimated Earliest Finish Time for a job with given duration est.*/
int
diet_estimate_eft(estVector_t ev,
                  double jobEstimatedCompTime,
                  const diet_profile_t *const profilePtr) {
  const SeDImpl *refSeD = (SeDImpl *) profilePtr->SeDPtr;
  if (refSeD == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": ref on SeD not initialized?", 1);
  }
  /* casting away const-ness, because we know that the
   * method doesn't change the SeD
   */
  double value = jobEstimatedCompTime + ((SeDImpl *) refSeD)->getEFT();
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
  if ((status > -1) && (status < NB_SERVER_STATUS)) {
    st = status;
    switch ((int) st) {
    case BATCH:
      TRACE_TEXT(TRACE_MAIN_STEPS, "SeD is batch\n");
      break;
    case SERIAL:
      TRACE_TEXT(TRACE_MAIN_STEPS, "SeD is sequential\n");
      break;
    default:
      TRACE_TEXT(TRACE_MAIN_STEPS, "Server status list to update\n");
    }
  } else {ERROR_EXIT("Server status not recognized");
  }
} // diet_set_server_status
#endif // ifdef HAVE_ALT_BATCH

#ifdef HAVE_ALT_BATCH
int
diet_submit_parallel(diet_profile_t *profile,
                     const char *addon_prologue,
                     const char *command) {
  return ((((SeDImpl *) profile->SeDPtr)->getBatch())->
          diet_submit_parallel(profile, addon_prologue, command));
}

/* This is to be used later: a SeD can manage a reservation as he wants. It
   can partition the reserved procs, launch several pb resolutions with
   tasks overlapping or not */
int
diet_concurrent_submit_parallel(int batchJobID, diet_profile_t *profile,
                                const char *command) {
  return (((SeDImpl *) profile->SeDPtr)->getBatch())->
         diet_submit_parallel(batchJobID, profile, command);
}

/* Used to explicitely wait for the completion of a batch job */
int
diet_wait_batch_job_completion(diet_profile_t *profile) {
  return ((SeDImpl *) profile->SeDPtr)->getBatch()->
         wait4BatchJobCompletion(((SeDImpl *) profile->SeDPtr)->
                                 getBatch()->getBatchJobID(profile->dietReqID));
}
#endif // ifdef HAVE_ALT_BATCH

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
        ERROR_DEBUG("Cannot locate SeD " << SeDName, GRPC_SERVER_NOT_FOUND);
      }

      // Now retrieve the services
      CORBA::Long length;
      SeqCorbaProfileDesc_t *profileList = sed->getSeDProfiles(length);
      *services_number = (int) length;
      *profiles =
        (diet_profile_desc_t **) calloc(length, sizeof(diet_profile_desc_t *));

      for (int i = 0; i < *services_number; i++) {
        (*profiles)[i] = new diet_profile_desc_t;
        unmrsh_profile_desc((*profiles)[i], &((*profileList)[i]));
      }
    } catch (...) {
      // TODO catch exceptions correctly
      ERROR_DEBUG("An exception has been caught while searching for SeD "
            << SeDName, GRPC_SERVER_NOT_FOUND);
    }
  } else {
    ERROR_DEBUG("No SeDName has been given", GRPC_SERVER_NOT_FOUND);
  }

  return 0;
} // diet_get_SeD_services

END_API
