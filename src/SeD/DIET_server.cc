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
 * Revision 1.17  2003/09/18 09:47:19  bdelfabr
 * adding data persistence
 *
 * Revision 1.16  2003/08/09 17:28:25  pcombes
 * Make diet_profile_desc_alloc and diet_profile_alloc homogenous.
 *
 * Revision 1.15  2003/07/25 20:22:48  pcombes
 * Add macros BEGIN_API and END_API to refine what is to be put in extern "C".
 *
 * Revision 1.14  2003/07/04 09:47:57  pcombes
 * Use new ERROR, WARNING and TRACE macros.
 *
 * Revision 1.13  2003/06/23 13:35:06  pcombes
 * useAsyncAPI should be replaced by a "useBiDir" option. Remove it so far.
 *
 * Revision 1.12  2003/06/02 09:26:10  cpera
 * Beta version of asynchronize DIET API.
 *
 * Revision 1.11  2003/05/10 08:54:41  pcombes
 * New format for configuration files, new Parsers.
 ****************************************************************************/


#include <iostream>
using namespace std;
#include <unistd.h>
#include <stdlib.h>

#include "debug.hh"
#include "DIET_server.h"
#include "marshalling.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "common_types.hh"
#include "ServiceTable.hh"
#include "SeDImpl.hh"
#include "dataMgr.hh"
#include "dataMgrImpl.hh"

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
diet_convertor_check(diet_convertor_t* cvt, diet_profile_desc_t* profile);
int
diet_service_table_add(diet_profile_desc_t* profile,
		       diet_convertor_t*    cvt,
		       diet_solve_t         solve_func)
{
  int res;
  corba_profile_desc_t corba_profile;
  diet_convertor_t*    actual_cvt(NULL);

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
  res = SRVT->addService(&corba_profile, actual_cvt, solve_func, NULL);
  if (!cvt)
    diet_convertor_free(actual_cvt);
  return res;
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
  if ((desc->last_out > -1) && desc->param_desc) {
    free(desc->param_desc);
    res = 0;
  } else {
    res = 1;
  }
  free(desc);
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
diet_convertor_check(diet_convertor_t* cvt, diet_profile_desc_t* profile)
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
  dataMgrImpl *Data; 
  int    res(0);
  int    myargc;
  char** myargv;

 /* Set arguments for ORBMgr::init */

  myargc = argc;
  myargv = (char**)malloc(argc * sizeof(char*));
  for (int i = 0; i < argc; i++)
    myargv[i] = argv[i];


  /* Parsing */

  Parsers::Results::param_type_t compParam[] = {Parsers::Results::PARENTNAME};

  if ((res = Parsers::beginParsing(config_file_name)))
    return res;
  if ((res =
       Parsers::parseCfgFile(true, 1,
			     (Parsers::Results::param_type_t*)compParam))) {
    Parsers::endParsing();
    return res;
  }

  /* Some more checks */
  char* name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::NAME);
  if (name != NULL)
    WARNING("parsing " << config_file_name
	    << ": it is useless to name an SeD - ignored");
  name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::MANAME);
  if (name != NULL)
    WARNING("parsing " << config_file_name
	    << ": no need to specify an MA name for an SeD - ignored");


  /* Get listening port */

  size_t* port = (size_t*)
    (Parsers::Results::getParamValue(Parsers::Results::ENDPOINT));
  if (port != NULL) {
    char   endPoint[48];
    int    tmp_argc = myargc + 2;
    realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = "-ORBendPoint";
    sprintf(endPoint, "giop:tcp::%u", *port);
    myargv[myargc + 1] = (char*)endPoint;
    myargc = tmp_argc;
  }

  /* Get the traceLevel */

  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    char   level[48];
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
  /* SeD creation */
  SeD = new SeDImpl();
  Data = new dataMgrImpl(SeD);
  /* Activate SeD */
  ORBMgr::activate(SeD);
  SeD->refData(Data);
  ORBMgr::activate(Data);
  if (SeD->run(SRVT)) {
   ERROR("unable to launch the SeD", 1);
  }
  if (Data->run()) {
     ERROR("unable to launch the DataManager", 1);
  }

  /* We do not need the parsing results any more */
  Parsers::endParsing();

  /* Wait for RPCs : */
  ORBMgr::wait();
  
  return 0;
}


} // extern "C"

