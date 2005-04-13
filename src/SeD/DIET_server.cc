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
 * Revision 1.34  2005/04/13 08:46:29  hdail
 * Beginning of adoption of new persistency model: DTM is enabled by default and
 * JuxMem will be supported via configure flags.  DIET will always provide at
 * least one type of persistency.  As a first step, persistency across DTM and
 * JuxMem is not supported so all persistency handling should be surrounded by
 *     #if HAVE_JUXMEM
 *       // JuxMem code
 *     #else
 *       // DTM code
 *     #endif
 * This check-in prepares for the JuxMem check-in by cleaning up old
 * DEVELOPPING_DATA_PERSISTENCY flags and surrounding DTM code with
 * #if ! HAVE_JUXMEM / #endif flags to be replaced by above format by Mathieu's
 * check-in.  Currently the HAVE_JUXMEM flag is set in SeDImpl.hh - to be replaced
 * by Mathieu's check-in of a configure system for JuxMem.
 *
 * Revision 1.33  2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.32  2004/12/22 06:49:25  alsu
 * rewriting the diet_service_table_lookup_by_profile to convert the
 * incoming diet_profile_t* parameter to a diet_profile_desc_t to do the
 * lookup.  This works around the behavior of mrsh_pb_desc, which is not
 * sufficient for the service table lookup, as it does not copy the type
 * and base type fields.
 *
 * Revision 1.31  2004/12/15 18:09:58  alsu
 * cleaner, easier to document interface: changing diet_perfmetric_t back
 * to the simpler one-argument (of type diet_profile_t) version, and
 * hiding a copy of the pointer back to the SeD in the profile.
 *
 * Revision 1.30  2004/12/08 15:02:51  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.29.2.3  2004/11/26 15:19:30  alsu
 * - diet_estimate_fast (available to perf metrics) to get and store FAST
 *   performance values
 * - diet_estimate_lastexec (available to perf metrics) to get and store
 *   time elapsed since SeD-level last-solve timestamp (to enable the RR
 *   scheduler)
 * - diet_service_table_lookup functions (C expression of the
 *   SeviceTable::lookupService methods)
 * - various additions of const to enforce const-ness
 *
 * Revision 1.29.2.2  2004/10/27 22:35:50  alsu
 * include
 *
 * Revision 1.29.2.1  2004/10/26 14:12:52  alsu
 * (Tag: AS-plugin-sched)
 *  - branch created to avoid conflicting with release 1.2 (imminent)
 *  - initial commit on branch, new dynamic performance info structure in
 *    the profile
 *
 * Revision 1.29  2004/10/14 15:02:17  hdail
 * Allow user to provide name for SeD.  If provided, name is given to LogService
 * and can be used by GoDIET to verify launch of each SeD.
 *
 * Revision 1.28  2004/09/14 12:39:27  hdail
 * - Changed cleanup of desc->param_desc from free to delete[] to match alloc.
 * - Changed cleanup of desc from free to delete to match alloc.
 *
 * Revision 1.27  2004/07/05 14:56:13  rbolze
 * correct bug on 64 bit plat-form, when parsing cfg file :
 * remplace size_t by unsigned int for config options
 ****************************************************************************/

#include <iostream>
using namespace std;
#include <unistd.h>
#include <stdlib.h>

#include "DIET_server.h"

#include "debug.hh"
#include "marshalling.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "SeDImpl.hh"
#include "Vector.h"
#include "FASTMgr.hh"
#include "DietLogComponent.hh"
#include "MonitoringThread.hh"

#if ! HAVE_JUXMEM
#include "DataMgrImpl.hh"
#endif // ! HAVE_JUXMEM

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

#define HOSTNAME_BUFLEN 256

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

int
diet_service_table_lookup(const diet_profile_desc_t* const profile)
{
  int refNum;
  corba_profile_desc_t corbaProfile;

  if (profile == NULL) {
    ERROR(__FUNCTION__ << ": null profile", -1);
  }

  mrsh_profile_desc(&corbaProfile, profile);
  refNum = SRVT->lookupService(&corbaProfile);

  return (refNum);
}

int
diet_service_table_lookup_by_profile(const diet_profile_t* const profile)
{
  int refNum;
  corba_profile_desc_t corbaProfile;
  diet_profile_desc_t profileDesc;

  if (profile == NULL) {
    ERROR(__FUNCTION__ << ": null profile", -1);
  }

  { /* create the corresponding profile description */
    profileDesc.path = strdup(profile->pb_name);
    profileDesc.last_in = profile->last_in;
    profileDesc.last_inout = profile->last_inout;
    profileDesc.last_out = profile->last_out;
    int numArgs = profile->last_out + 1;
    profileDesc.param_desc =
      (diet_arg_desc_t*) calloc (numArgs, sizeof (diet_arg_desc_t));
    for (int argIter = 0 ; argIter < numArgs ; argIter++) {
      profileDesc.param_desc[argIter] =
        (profile->parameters[argIter]).desc.generic;
    }
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
#if ! HAVE_JUXMEM
  DataMgrImpl* dataMgr; 
#endif // ! HAVE_JUXMEM

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
    // size_t --> unsigned int
  unsigned int* ULSptr;
  int outBufferSize;
    // size_t --> unsigned int
  unsigned int* OBSptr;
  int flushTime;
    // size_t --> unsigned int
  unsigned int* FTptr;

    // size_t --> unsigned int
  ULSptr = (unsigned int*)Parsers::Results::getParamValue(
              Parsers::Results::USELOGSERVICE);
  useLS = false;
  if (ULSptr == NULL) {
    cout << "WARNING: useLogService not configured. Disabled by default\n";
  } else {
    if (*ULSptr) {
      useLS = true;
    }
  }

  if (useLS) {
    // size_t --> unsigned int
    OBSptr = (unsigned int*)Parsers::Results::getParamValue(
  	       Parsers::Results::LSOUTBUFFERSIZE);
    if (OBSptr != NULL) {
      outBufferSize = (int)(*OBSptr);
    } else {
      outBufferSize = 0;
      WARNING("lsOutbuffersize not configured, using default");
    }

    // size_t --> unsigned int
    FTptr = (unsigned int*)Parsers::Results::getParamValue(
  	       Parsers::Results::LSFLUSHINTERVAL);
    if (FTptr != NULL) {
      flushTime = (int)(*FTptr);
    } else {
      flushTime = 10000;
      WARNING("lsFlushinterval not configured, using default");
    }
  }

  if (useLS) {
    TRACE_TEXT(TRACE_MAIN_STEPS, "LogService enabled\n");
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
      dietLogComponent = NULL; // this should never happen;
    }
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS, "LogService disabled\n");
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

#if ! HAVE_JUXMEM
  /* Set-up and activate Data Manager for DTM usage */
  dataMgr = new DataMgrImpl();
  dataMgr->setDietLogComponent(dietLogComponent);
  ORBMgr::activate(dataMgr);
  if (dataMgr->run()) {
    ERROR("unable to launch the DataManager", 1);
  }
  SeD->linkToDataMgr(dataMgr);
#endif // ! HAVE_JUXMEM

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
diet_estimate_fast(estVector_t ev,
                   const diet_profile_t* const profilePtr)
//                    int stRef)
{
  char hostnameBuf[HOSTNAME_BUFLEN];
  int stRef;

  if (gethostname(hostnameBuf, HOSTNAME_BUFLEN-1)) {
    ERROR("error getting hostname", -1);
  }

  stRef = diet_service_table_lookup_by_profile(profilePtr);
  FASTMgr::estimate(hostnameBuf,
                    profilePtr,
                    SRVT,
                    (ServiceTable::ServiceReference_t) stRef,
                    ev);
                    
  return (0);
}

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
  estVector_addEstimation(ev,
                          EST_TIMESINCELASTSOLVE,
                          timeSinceLastSolve);
  return (0);
}

END_API
