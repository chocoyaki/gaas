/****************************************************************************/
/* DIET server interface                                                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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
 * Revision 1.49  2006/01/20 10:14:37  eboix
 *    - Odd references to acDIET_config.h changed to DIET_config.h  --- Injay2461
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
 * Revision 1.45  2005/11/10 14:37:51  eboix
 *     Clean-up of Cmake/DIET_config.h.in and related changes. --- Injay2461
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
using namespace std;
#include <unistd.h>  // For gethostname()
#include <stdlib.h>

#include "DIET_config.h"
#include "DIET_server.h"

#include "debug.hh"
#include "est_internal.hh"
#include "marshalling.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "SeDImpl.hh"
#include "Vector.h"
#include "DietLogComponent.hh"
#include "MonitoringThread.hh"

#if HAVE_CORI
#include "CORIMgr.hh"
#else 
#include "FASTMgr.hh"
#endif //HAVE_CORI

#if HAVE_JUXMEM
#include "JuxMem.hh"
#else
#include "DataMgrImpl.hh"
#endif // HAVE_JUXMEM

#if HAVE_BATCH
#include "strseed.h"
#endif

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"
 
extern unsigned int TRACE_LEVEL;

BEGIN_API

/****************************************************************************/
/* DIET service table                                                       */
/****************************************************************************/

static ServiceTable* SRVT;

int
diet_service_table_init(int maxsize)
{
  SRVT = new ServiceTable(maxsize);
  return 0;
}

int
diet_convertor_check(const diet_convertor_t* const cvt,
                     const diet_profile_desc_t* const profile);

static diet_perfmetric_t current_perfmetric_fn = NULL;
diet_perfmetric_t
diet_service_use_perfmetric(diet_perfmetric_t perfmetric_fn)
{
  diet_perfmetric_t tmp_fn = current_perfmetric_fn;
  current_perfmetric_fn = perfmetric_fn;
  return (tmp_fn);
}

int
diet_service_table_add(const diet_profile_desc_t* const profile,
                       const diet_convertor_t*    const cvt,
                       diet_solve_t                     solve_func)
{
  int res;
  corba_profile_desc_t corba_profile;
  const diet_convertor_t*    actual_cvt(NULL);

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
#if HAVE_BATCH
    // Must I add something about convertors ??
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
  if (!cvt) {
    /*
    ** free it only when the incoming parameter was null (i.e., in
    ** the case where we allocated it
    */
    diet_convertor_free((diet_convertor_t*) actual_cvt);
  }
  return res;
}
/* Unused !!!
   int
   diet_service_table_lookup(const diet_profile_desc_t* const profile)
   {
   int refNum;
   corba_profile_desc_t corbaProfile;
   
   if (profile == NULL) {
   ERROR(__FUNCTION__ << ": NULL profile", -1);
   }
   
   if (SRVT == NULL) {
   ERROR(__FUNCTION__ << ": service table not yet initialized", -1);
   }
   
   mrsh_profile_desc(&corbaProfile, profile);
   refNum = SRVT->lookupService(&corbaProfile);
   
   return (refNum); 
   } 
*/


int
diet_service_table_lookup_by_profile(const diet_profile_t* const profile)
{
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
#if HAVE_BATCH
    /* In case of client explicitly ask for a batch resolution */
    profileDesc.batch_flag = profile->batch_flag ;
#endif
    int numArgs = profile->last_out + 1;
    profileDesc.param_desc =
      (diet_arg_desc_t*) calloc (numArgs, sizeof (diet_arg_desc_t));
    for (int argIter = 0 ; argIter < numArgs ; argIter++) {
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
diet_print_service_table()
{
  if (SRVT == NULL) {
    ERROR(__FUNCTION__ << ": service table not yet initialized",);
  }

  SRVT->dump(stdout);
}


/****************************************************************************/
/* DIET service profile descriptor                                          */
/****************************************************************************/

diet_profile_desc_t*
diet_profile_desc_alloc(const char* path,
			int last_in, int last_inout, int last_out)
{
  diet_profile_desc_t* desc(NULL);
  diet_arg_desc_t*     param_desc(NULL);

  if ((last_in < -1) || (last_inout < -1) || (last_out < -1))
    return NULL;
  if (last_out == -1)
    param_desc = NULL;
  else {
    param_desc = new diet_arg_desc_t[last_out + 1];
    if (!param_desc)
      return NULL;
  }
  desc = new diet_profile_desc_t;
  if (!desc)
    return NULL;
  desc->aggregator.agg_method = DIET_AGG_DEFAULT;
  desc->path       = strdup(path);
  desc->last_in    = last_in;
  desc->last_inout = last_inout;
  desc->last_out   = last_out;
  desc->param_desc = param_desc;
  return desc;
}

int
diet_profile_desc_free(diet_profile_desc_t* desc)
{
  int res = 0;

  if (!desc)
    return 1;
  free(desc->path);
  if ((desc->last_out > -1) && desc->param_desc) {
    delete[] desc->param_desc;
    res = 0;
  } else {
    res = 1;
  }
  delete desc;
  return res;
}
  

/****************************************************************************/
/* DIET aggregation                                                         */
/****************************************************************************/
diet_aggregator_desc_t*
diet_profile_desc_aggregator(diet_profile_desc_t* profile)
{
  if (profile == NULL) {
    WARNING(__FUNCTION__ << ": NULL profile\n");
    return (NULL);
  }
  return (&(profile->aggregator));
}
int
diet_aggregator_set_type(diet_aggregator_desc_t* agg,
                         diet_aggregator_type_t atype)
{
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator\n", 0);
  }
  if (atype != DIET_AGG_DEFAULT &&
      atype != DIET_AGG_PRIORITY) {
    ERROR(__FUNCTION__ << ": unknown aggregation type (" << atype << ")\n", 0);
  }
  if (agg->agg_method != DIET_AGG_DEFAULT) {
    WARNING(__FUNCTION__ <<
            ": overriding previous aggregation type (" <<
            agg->agg_method <<
            ")\n");
  }
  agg->agg_method = atype;
  memset(&(agg->agg_specific), 0, sizeof (agg->agg_specific));
  return (1);
}
static int
__diet_agg_pri_add_value(diet_aggregator_priority_t* priority, int value)
{
  if (priority->p_numPValues == 0) {
    if ((priority->p_pValues = (int *) calloc(1, sizeof (int))) == NULL) {
      return (0);
    }
    priority->p_pValues[0] = value;
    priority->p_numPValues = 1;
  }
  else {
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
diet_aggregator_priority_max(diet_aggregator_desc_t* agg,
                             int tag)
{
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator\n", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR(__FUNCTION__ << ": aggregator not a priority list\n", 0);
  }
  if (! __diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                 tag)) {
    ERROR(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          tag <<
          ")\n", 0);
  }
  return (1);
}
int
diet_aggregator_priority_min(diet_aggregator_desc_t* agg,
                             int tag)
{
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator\n", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR(__FUNCTION__ << ": aggregator not a priority list\n", 0);
  }
  if (! __diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                 -tag)) {
    ERROR(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          -tag <<
          ")\n", 0);
  }
  return (1);
}

int
diet_aggregator_priority_maxuser(diet_aggregator_desc_t* agg, int val)
{
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator\n", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR(__FUNCTION__ << ": aggregator not a priority list\n", 0);
  }
  if (! __diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                 EST_USERDEFINED + val)) {
    ERROR(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          (EST_USERDEFINED + val) <<
          ")\n", 0);
  }
  return (1);
}
int
diet_aggregator_priority_minuser(diet_aggregator_desc_t* agg, int val)
{
  if (agg == NULL) {
    ERROR(__FUNCTION__ << ": NULL aggregator\n", 0);
  }
  if (agg->agg_method != DIET_AGG_PRIORITY) {
    ERROR(__FUNCTION__ << ": aggregator not a priority list\n", 0);
  }
  if (! __diet_agg_pri_add_value(&(agg->agg_specific.agg_specific_priority),
                                 -(EST_USERDEFINED + val))) {
    ERROR(__FUNCTION__ <<
          ": failure adding value to priority list (" <<
          -(EST_USERDEFINED + val) <<
          ")\n", 0);
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
   Internally, when a client requests for a declared service, the correspunding
   convertor is used to generate the actual profile : this allows evaluation
   (cf. below)
*/

int
diet_arg_cvt_set(diet_arg_convertor_t* arg_cvt, diet_convertor_function_t f,
		 int in_arg_idx, diet_arg_t* arg, int out_arg_idx)
{
  if (!arg_cvt)
    return 1;
  arg_cvt->f           = f;
  arg_cvt->in_arg_idx  = in_arg_idx;
  arg_cvt->arg         = arg;
  arg_cvt->out_arg_idx = out_arg_idx;
  return 0;
}

diet_convertor_t*
diet_convertor_alloc(const char* path,
		     int last_in, int last_inout, int last_out)
{
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
diet_convertor_free(diet_convertor_t* cvt)
{
  int res = 0;

  if (!cvt)
    return 1;
  free(cvt->path);
  if ((cvt->last_out <= -1) || !(cvt->arg_convs)) {
    res = 1;
  } else {
#if 0
    for (int i = 0; i < cvt->last_out; i++) {
      if (cvt->arg_convs[i].arg)
	free(cvt->arg_convs[i].arg);
    }
#endif
    delete [] cvt->arg_convs;
  }
  delete cvt;
  return res;
}

int
diet_convertor_check(const diet_convertor_t* const cvt,
                     const diet_profile_desc_t* const profile)
{
  int res = 0;

#define CHECK_ERROR(formatted_text)                         \
  if (res == 0)                                             \
    cerr << "DIET ERROR while checking the convertor from " \
	 << profile->path << " to " << cvt->path << ":\n";  \
  cerr << formatted_text << ".\n";                  \
  res = 1;


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
	CHECK_ERROR("- the argument convertor " << i << " references no "
		    << "argument ;\n  it should reference either an index of "
		    << "the profile, or a constant argument");
      }
    } else if (cvt->arg_convs[i].arg != NULL) {
      CHECK_ERROR("- the argument convertor " << i << " references too many "
		  << "arguments ;\n  it should reference either an index of "
		  << "the profile, or a constant argument");
    }
  }
  return res;
}

/****************************************************************************/
/* DIET server call                                                         */
/****************************************************************************/

int
diet_SeD(char* config_file_name, int argc, char* argv[])
{
  SeDImpl* SeD;
  int    res(0);
  int    myargc;
  char** myargv;
  char*  userDefName;
  char*  name;
  DietLogComponent* dietLogComponent;     /* LogService */
  MonitoringThread* monitoringThread;
#if HAVE_JUXMEM
  JuxMem::Wrapper* juxmem;
#else
  DataMgrImpl* dataMgr; 
#endif // HAVE_JUXMEM

  if (SRVT == NULL) {
    ERROR(__FUNCTION__ << ": service table not yet initialized", 1);
  }

  /* Set arguments for ORBMgr::init */
  myargc = argc;
  myargv = (char**)malloc(argc * sizeof(char*));
  for (int i = 0; i < argc; i++)
    myargv[i] = argv[i];

  /* Parsing */
  Parsers::Results::param_type_t compParam[] = {Parsers::Results::PARENTNAME};

  if ((res = Parsers::beginParsing(config_file_name))) {
    return res;
  }
  if ((res =
       Parsers::parseCfgFile(true, 1,
			     (Parsers::Results::param_type_t*)compParam))) {
    Parsers::endParsing();
    return res;
  }

  /* Some more checks */

  // NOTE: if non-NULL, userDefName will be used for LogService
  userDefName = (char*)
    Parsers::Results::getParamValue(Parsers::Results::NAME);

  name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::MANAME);
  if (name != NULL)
    WARNING("parsing " << config_file_name
	    << ": no need to specify an MA name for an SeD - ignored");

  /* Get listening port & hostname */

  // size_t --> unsigned int
  unsigned int* port = (unsigned int*) 
    (Parsers::Results::getParamValue(Parsers::Results::DIETPORT));
  char* host = (char*)
    (Parsers::Results::getParamValue(Parsers::Results::DIETHOSTNAME));
  if ((port != NULL)|| (host !=NULL)) {
    char *  endPoint = (char *) calloc(48, sizeof(char*)) ;
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = "-ORBendPoint";
    if (port == NULL) {
	    sprintf(endPoint, "giop:tcp:%s:", host);
    } else if (host == NULL)  {
	    sprintf(endPoint, "giop:tcp::%u", *port);
    } else {
	    sprintf(endPoint, "giop:tcp:%s:%u", host,*port);
    }	    
    myargv[myargc + 1] = (char*)endPoint;
    myargc = tmp_argc;
  }

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

  /* ORB initialization */

  if (ORBMgr::init(myargc, (char**)myargv)) {
    ERROR("ORB initialization failed", 1);
  }

  /* DietLogComponent creation for LogService usage */
  bool useLS;
  unsigned int* ULSptr;
  int outBufferSize;
  unsigned int* OBSptr;
  int flushTime;
  unsigned int* FTptr;

  ULSptr = (unsigned int*)Parsers::Results::getParamValue(
              Parsers::Results::USELOGSERVICE);
  useLS = false;
  if (ULSptr != NULL) {
    if (*ULSptr) {
      useLS = true;
    }
  }

  if (useLS) {
    OBSptr = (unsigned int*)Parsers::Results::getParamValue(
  	       Parsers::Results::LSOUTBUFFERSIZE);
    if (OBSptr != NULL) {
      outBufferSize = (int)(*OBSptr);
    } else {
      outBufferSize = 0;
      TRACE_TEXT(TRACE_ALL_STEPS,
            "lsOutbuffersize not configured, using default");
    }

    FTptr = (unsigned int*)Parsers::Results::getParamValue(
  	       Parsers::Results::LSFLUSHINTERVAL);
    if (FTptr != NULL) {
      flushTime = (int)(*FTptr);
    } else {
      flushTime = 10000;
      TRACE_TEXT(TRACE_ALL_STEPS,
            "lsFlushinterval not configured, using default");
    }
  }

  if (useLS) {
    TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: enabled\n");
    char* parentName;
    parentName = (char*)Parsers::Results::getParamValue
                          (Parsers::Results::PARENTNAME);

    if (userDefName != NULL){
      dietLogComponent = new DietLogComponent(userDefName, outBufferSize);
    } else {
      dietLogComponent = new DietLogComponent("", outBufferSize);
    }

    ORBMgr::activate(dietLogComponent);
    if (dietLogComponent->run("SeD", parentName, flushTime) != 0) {
      // delete(dietLogComponent); // DLC is activated, do not delete !
      WARNING("Could not initialize DietLogComponent");
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled\n");
      dietLogComponent = NULL; // this should never happen;
    }
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled\n");
    dietLogComponent = NULL;
  }

  // Just start the thread, as it might not be FAST-related
  monitoringThread = new MonitoringThread(dietLogComponent);

  /* SeD creation */
  SeD = new SeDImpl();

  /* Set SeD to use LogService object */
  SeD->setDietLogComponent(dietLogComponent);

  /* Activate SeD */
  ORBMgr::activate(SeD);
  if (SeD->run(SRVT)) {
    ERROR("unable to launch the SeD", 1);
  }
  
#if HAVE_JUXMEM
  /** JuxMem creation */
  juxmem = new JuxMem::Wrapper(userDefName);
  SeD->linkToJuxMem(juxmem);
#else
  /* Set-up and activate Data Manager for DTM usage */
  dataMgr = new DataMgrImpl();
  dataMgr->setDietLogComponent(dietLogComponent);
  ORBMgr::activate(dataMgr);
  if (dataMgr->run()) {
    ERROR("unable to launch the DataManager", 1);
  }
  SeD->linkToDataMgr(dataMgr);
#endif // HAVE_JUXMEM

  /* We do not need the parsing results any more */
  Parsers::endParsing();

  /* Wait for RPCs : */
  ORBMgr::wait();

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  ORBMgr::destroy();
  
  return 0;
}

int
diet_est_set(estVector_t ev, int userTag, double value)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")\n", -1);
  }

  return (diet_est_set_internal(ev, userTag + EST_USERDEFINED, value));
}

double
diet_est_get(estVectorConst_t ev, int userTag, double errVal)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")\n", errVal);
  }

  return (diet_est_get_internal(ev, userTag + EST_USERDEFINED, errVal));
}

double
diet_est_get_system(estVectorConst_t ev, int systemTag, double errVal)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": systemTag must be non-negative (" <<
          systemTag <<
          ")\n", errVal);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")\n", errVal);
  }
  return (diet_est_get_internal(ev,systemTag, errVal));
}

int
diet_est_defined(estVectorConst_t ev, int userTag)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")\n", -1);
  }

  return (diet_est_defined_internal(ev, userTag + EST_USERDEFINED));
}
int
diet_est_defined_system(estVectorConst_t ev, int systemTag)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")\n", -1);
  }
   if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")\n", -1);
  }

  return (diet_est_defined_internal(ev, systemTag ));
}

int
diet_est_array_size(estVectorConst_t ev, int userTag)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")\n", -1);
  }

  return (diet_est_array_size_internal(ev, userTag + EST_USERDEFINED));
}
int
diet_est_array_size_system(estVectorConst_t ev, int systemTag)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")\n", -1);
  }
   if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")\n", -1);
  }

  return (diet_est_array_size_internal(ev, systemTag));
}

int
diet_est_array_set(estVector_t ev, int userTag, int idx, double value)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")\n", -1);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ << ": idx must be non-negative (" << idx << ")\n", -1);
  }

  return (diet_est_array_set_internal(ev,
                                      userTag + EST_USERDEFINED,
                                      idx,
                                      value));
}

double
diet_est_array_get(estVectorConst_t ev, int userTag, int idx, double errVal)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")\n", errVal);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ <<
          ": idx must be non-negative (" <<
          idx <<
          ")\n", errVal);
  }

  return (diet_est_array_get_internal(ev,
                                      userTag + EST_USERDEFINED,
                                      idx,
                                      errVal));
}

double
diet_est_array_get_system(estVectorConst_t ev, int systemTag, int idx, double errVal)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")\n", errVal);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ <<
          ": idx must be non-negative (" <<
          idx <<
          ")\n", errVal);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")\n", errVal);
  }
  return (diet_est_array_get_internal(ev,
                                      systemTag,
                                      idx,
                                      errVal));
}


int
diet_est_array_defined(estVectorConst_t ev, int userTag, int idx)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (userTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          userTag <<
          ")\n", -1);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ << ": idx must be non-negative (" << idx << ")\n", -1);
  }

  return (diet_est_array_defined_internal(ev,
                                          userTag + EST_USERDEFINED,
                                          idx));
}

int
diet_est_array_defined_system(estVectorConst_t ev, int systemTag, int idx)
{
  if (ev == NULL) {
    ERROR(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (systemTag < 0) {
    ERROR(__FUNCTION__ <<
          ": userTag must be non-negative (" <<
          systemTag <<
          ")\n", -1);
  }
  if (idx < 0) {
    ERROR(__FUNCTION__ << ": idx must be non-negative (" << idx << ")\n", -1);
  }
  if (systemTag >= EST_USERDEFINED) {
    ERROR(__FUNCTION__ <<
          ": systemTag "<<systemTag <<" must be smaller than (" <<
          EST_USERDEFINED<<
          ")\n", -1);
  }
  return (diet_est_array_defined_internal(ev,
                                          systemTag,
                                          idx));
}
#if HAVE_CORI

int
diet_estimate_cori(estVector_t ev,
		   int info_type,
		   diet_est_collect_tag_t collector_type,
		   void * data)
{

  if (collector_type==EST_COLL_FAST){
    //#if HAVE_FAST    
    fast_param_t fastparam={(diet_profile_t*)data,SRVT};
    //testing already here, because it is possible that an internal call use tag COMMTIME
   if ((info_type==EST_TCOMP)||
	(info_type==EST_FREECPU)||
	 (info_type==EST_FREEMEM)||
	  (info_type==EST_NBCPU)||
        (info_type==EST_ALLINFOS))
    CORIMgr::call_cori_mgr(&ev,info_type,collector_type,&fastparam);
   else {
      ERROR(__FUNCTION__ << ": info_type must be EST_TCOMP,EST_FREECPU,EST_FREEMEM, EST_NBCPU or EST_ALLINFOS!)\n", -1);
      diet_est_set_internal(ev,info_type,0);
      //fixme: set the default values for each type
   }
   //#endif //HAVE_FAST
  }
  else
       CORIMgr::call_cori_mgr(&ev,info_type,collector_type,data);   
  return 0;
}

int
diet_estimate_cori_add_collector(diet_est_collect_tag_t collector_type,
				 void* data){
  return CORIMgr::add(collector_type,NULL);
}

void 
print_message(){
  cerr<<"=default value used"<<endl;
  }

void
diet_estimate_coriEasy_print(){
  int tmp_int=TRACE_LEVEL;
  TRACE_LEVEL=15;
 
   cerr<<"start printing CoRI values.."<<endl;
   estVector_t vec=new corba_estimation_t();

   CORIMgr::add(EST_COLL_EASY,NULL);
 
   int minut=15;

   if  (diet_estimate_cori(vec,EST_AVGFREECPU,EST_COLL_EASY,&minut))
     print_message();		   
   
   if  (diet_estimate_cori(vec,EST_CACHECPU,EST_COLL_EASY,NULL))
     print_message();	 
   
   if  (diet_estimate_cori(vec,EST_NBCPU,EST_COLL_EASY,NULL))
     print_message();		   
   
   if  (diet_estimate_cori(vec,EST_BOGOMIPS,EST_COLL_EASY,NULL))
     print_message();		   
   char * tmp="./";
   if  (diet_estimate_cori(vec,EST_DISKACCESREAD,EST_COLL_EASY,tmp))
     print_message();		   
   
   if  (diet_estimate_cori(vec,EST_DISKACCESWRITE,EST_COLL_EASY,tmp))
     print_message();		   
   
   if  (diet_estimate_cori(vec,EST_TOTALSIZEDISK,EST_COLL_EASY,tmp))
     print_message();		   
   
   if  (diet_estimate_cori(vec,EST_FREESIZEDISK,EST_COLL_EASY,tmp))
     print_message();		   
   
   if  (diet_estimate_cori(vec,EST_TOTALMEM,EST_COLL_EASY,NULL))
     print_message();		   
   
   if  (diet_estimate_cori(vec,EST_FREEMEM,EST_COLL_EASY,NULL))
     print_message();	

   cerr<<"end printing CoRI values"<<endl;
   TRACE_LEVEL=tmp_int;
}

#else //HAVE_CORI

int
diet_estimate_fast(estVector_t ev,
                   const diet_profile_t* const profilePtr)
//                    int stRef)
{
  char hostnameBuf[256]; //was HOSTNAME_BUFLEN initially
  int stRef;

  if (gethostname(hostnameBuf, 255)) {
    ERROR("error getting hostname", 0);
  }

  stRef = diet_service_table_lookup_by_profile(profilePtr);
  FASTMgr::estimate(hostnameBuf,
                    profilePtr,
                    SRVT,
                    (ServiceTable::ServiceReference_t) stRef,
                    ev);
                    
  return (1);
}
  
#endif //HAVE_CORI

int diet_estimate_lastexec(estVector_t ev,
                           const diet_profile_t* const profilePtr)
{
  const SeDImpl* refSeD = (SeDImpl*) profilePtr->SeDPtr;
  double timeSinceLastSolve;
  const struct timeval* lastSolveStartPtr;
  struct timeval currentTime;

  /*
  ** casting away const-ness, because we know that the
  ** method doesn't change the SeD
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
  return (1);
}

/****************************************************************************/
/* DIET batch submit call                                                   */
/****************************************************************************/

#ifdef HAVE_BATCH
/* Make the script and submit it to the batch scheduler */
int
diet_submit_batch(diet_profile_t *profile, const char *command,
		  diet_submit_call_t call)
{
  int passed = -1 ;
  ELBASE_SchedulerServiceTypes schedulerID ;
  ELBASE_ComputeServiceTypes ELBASE_service ;
  char **attrs ;
  //  ELBASE_Process pid ;
  char *chaine = NULL ;
  char *arg = NULL ;
  char *script = NULL ;
  char *hostname = NULL ;
  //  int status ;

  schedulerID = ((SeDImpl*)(profile->SeDPtr))->getBatchSchedulerID() ;
  /* Read the walltime and the nbprocs in profile */
  /* Do we modify them? */
  /* until a correct prediction function is given, fixed values are used */
  profile->walltime=125 ;
  profile->nbprocs=2 ;
  /* Make the correct script */
  //attrs = (char**)malloc(sizeof(char*)*3) ;
  chaine = (char*)malloc(1000*sizeof(char)) ;
  sprintf(chaine,"host_count=%d "
	  "max_wall_time=%d:%d:%d",
	  profile->nbprocs,
	  (int)(profile->walltime/3600),
	  (int)((profile->walltime%3600)/60),
	  (int)(profile->walltime%60)) ;

  /*  arg = ASSTR_StrAppend(chaine," ", NULL) ;
      printf("La chaine est %s\n",arg) ; */
  
  //attrs[0] = strdup(chaine) ;
  /*  sprintf(chaine,"max_wall_time=%d:%d:%d",
	  (int)(profile->walltime/3600),
	  (int)((profile->walltime%3600)/60),
	  (int)(profile->walltime%60)) ;
	  arg = ASSTR_StrAppend(arg, chaine, " ", NULL) ;*/
  arg = chaine ;
  
  //  attrs[1] = strdup(chaine) ;
  //attrs[2] = NULL ;

  /* Attention, free memory ! */
  /*  free(arg) ; arg = NULL ; */
  
  // Fork the submission to the batch scheduler
  ELBASE_service = ELBASE_FORK ;
  
  hostname = ((SeDImpl*)profile->SeDPtr)->getLocalHostName() ;
  attrs = ASSTR_StrSplit(arg, NULL) ;
  
  switch(call) {
  case DIET_Mpich:
    arg = strdup("mpirun -np BATCH_NBNODES -machinefile BATCH_NODES");
    break ;
  case DIET_Lam:
  case DIET_Pvm:
  case DIET_Poe: // only available with Loadleveler
  case DIET_Sequential:
  case DIET_Submit_COUNT:
    TRACE_TEXT(TRACE_ALL_STEPS, "FIXME: not done!");
    exit(1) ;
    break ;
  default:
    TRACE_TEXT(TRACE_ALL_STEPS,
	       "En error occurs: requested diet_call mode not implemented!");
    exit(1) ;
  }
  arg = ASSTR_StrAppend(arg, " ", command) ;
  
//   passed = ELBASE_Submit(ELBASE_service, "localhost", schedulerID
// 			 , (const char **)attrs,
// 			 command, NULL, NULL, NULL, "dietBatchSubmit05"
// 			 , NULL, NULL,
// 			 &pid) ;

  script = 
    ELBASE_ScriptForSubmit(ELBASE_service, hostname, schedulerID
			   , (const char **)attrs
			   , arg, NULL, NULL, NULL, "dietBatchSubmit05"
			   , NULL, NULL) ;
  
  TRACE_TEXT(TRACE_ALL_STEPS,
	     "Script : " << script << "\n") ;
  
  /* Stock the task pid in relation with the profile */
  /*  ((SeDImpl*)profile->SeDPtr)-> ; */
  /* But is the pid the task ID or the submission ID ??? */

  /*
  // synchro can not be defined here, or in the profile...
  // must take place in SeDImpl::solve_batch()

  if( synchro == 1 ) {
    passed = passed  &&
      ELBASE_Poll(pid, 1, &status) &&
      status == 0 ;
  }
  */
  /*
      &&
      ELBASE_Spawn(service, server, "rm", NULL, rmArgs, NULL, NULL,
		   NULL, NULL, &pid)
      &&
      ELBASE_Poll(pid, 1, &status) &&
      status == 0;
  */

  passed = 1 ; // for testing
  return passed ;
}
#endif

END_API
