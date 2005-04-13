/****************************************************************************/
/* JNI SeD implementation source code                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr                        */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

#include <iostream>
using namespace std;
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "DIET_server.h"
#include "debug.hh"
#include "marshalling.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "SeDImpl.hh"
#include "DietLogComponent.hh"
#include "MonitoringThread.hh"

#if ! HAVE_JUXMEM
#include "DataMgrImpl.hh"   // DTM header file
#endif // ! HAVE_JUXMEM

#include "dmat_jxta_progs.h"

#include "jni.h"
#include "JXTASeD.h"

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

/* DIET_SeD parameters */

char* uuid;
int argc;
char **argv;

/** The DIET SeD object*/
SeDImpl* SeD;

#if ! HAVE_JUXMEM
/** The data Mgr object for DTM */
DataMgrImpl* dataMgr;
#endif // ! HAVE_JUXMEM

void SeDRPCsWait (void* par);
long int strToLong (char*);
double strToDouble (char*);
char *itoa (int);

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

static diet_perfmetric_t current_perfmetric_fn = NULL;
diet_perfmetric_t
diet_service_use_perfmetric(diet_perfmetric_t perfmetric_fn)
{
  diet_perfmetric_t tmp_fn = current_perfmetric_fn;
  current_perfmetric_fn = perfmetric_fn;
  return (tmp_fn);
}


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
  res = SRVT->addService(&corba_profile,
                         actual_cvt,
                         solve_func,
                         NULL,
                         current_perfmetric_fn);
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

END_API

/* This server can offer 5 services (chosen by at launch time): */
/*   - T = matrix translation    */
/*   - MatSUM = matrix addition  */
/*   - SqMatSUM and SqMatSUM_opt */
/*   - MatPROD = matrix product  */

#define NB_SRV 5
static const char* SRV[NB_SRV] =
  {"T", "MatPROD", "MatSUM", "SqMatSUM", "SqMatSUM_opt"};

/*
 * SOLVE FUNCTIONS
 */

int
solve_T(diet_profile_t* pb)
{
  size_t* m = NULL;
  size_t* n = NULL;
  double* A = NULL;
  diet_matrix_order_t o;
  int res;

  printf("Solve T ...");

  diet_scalar_get(diet_parameter(pb,0), &m, NULL);
  diet_scalar_get(diet_parameter(pb,1), &n, NULL);
  diet_matrix_get(diet_parameter(pb,2), &A, NULL, NULL, NULL, &o);
  
  if ((res = T(*m, *n, A, (o == DIET_ROW_MAJOR))))
    return res;
  
  // no need to set order
  diet_matrix_desc_set(diet_parameter(pb,2), *n, *m, DIET_MATRIX_ORDER_COUNT);
  
  diet_free_data(diet_parameter(pb,0));
  diet_free_data(diet_parameter(pb,1));

  printf(" done\n");
  return 0;
}

int
solve_MatSUM(diet_profile_t* pb)
{
  size_t mA, nA, mB, nB, mC, nC;
  char tA, tB;
  diet_matrix_order_t oA, oB, oC;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  int arg_idx, res;
  
  printf("Solve MatSUM ...");


#if defined(__FAST_0_4__)
  /**
   * In FAST 0.4.x, base/plus is declared to the LDIF base with only the two
   * matrix dimensions as arguments. But the library expects the user to give
   * two matrices, and it extracts itself their dimensions.
   */
  arg_idx = 0;

#else  // __FAST_0_4__
  /**
   * In FAST 0.8 and later, base/plus is also declared to the LDIF base with
   * only the two matrix dimensions as arguments. But there is no more
   * extraction of the dimensions, and then the two first arguments must be
   * these dimensions.
   */
  arg_idx = 2;

#endif // __FAST_0_4__

  diet_matrix_get(diet_parameter(pb,arg_idx), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb,(arg_idx+1)), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if ((mA != mB) || (nA != nB)) {
    fprintf(stderr, "MatSUM error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
	    (long)mA, (long)nA, (long)mB, (long)nB);
    return 1;
  }
  diet_matrix_get(diet_parameter(pb,(arg_idx+2)), &C, NULL, &mC, &nC, &oC);

  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    res = MatSUM(tB, tA, nA, mA, B, A, C);
  } else {
    res = MatSUM(tA, tB, mA, nA, A, B, C);
  }
  if (res)
    return res;
  
  diet_free_data(diet_parameter(pb,0));
  diet_free_data(diet_parameter(pb,1));
#if !defined(__FAST_0_4__)
  diet_free_data(diet_parameter(pb,2));
  diet_free_data(diet_parameter(pb,3));
#endif // ! __FAST_0_4__

  printf(" done\n");
  return res;
}


int
solve_MatPROD(diet_profile_t* pb)
{
  size_t mA, nA, mB, nB;
  char tA, tB;
  diet_matrix_order_t oA, oB, oC;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  int arg_idx, res;
  
  printf("Solve MatPROD ...");

#if defined(__FAST_0_4__)
  /**
   * In FAST 0.4.x, base/mult is declared to the LDIF base with only the three
   * relevant matrix dimensions as arguments. But the library expects the user
   * to give two matrices, and it extracts itself their dimensions.
   */
  arg_idx = 0;

#else  // __FAST_0_4__
  /**
   * In FAST 0.8 and later, base/plus is also declared to the LDIF base with
   * only the three relevant matrix dimensions as arguments. But there is no
   * more extraction of the dimensions, and then the three first arguments must
   * be these dimensions.
   */
  arg_idx = 3;

#endif // __FAST_0_4__

  diet_matrix_get(diet_parameter(pb,arg_idx), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb,(arg_idx+1)), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if (nA != mB) {
    fprintf(stderr, "MatPROD error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
    (long)mA, (long)nA, (long)mB, (long)nB);
    return 1;
  }
  diet_matrix_get(diet_parameter(pb,(arg_idx+2)), &C, NULL, NULL, NULL, &oC);
  
  
  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    res = MatPROD(tB, tA, nB, mB, B, mA, A, C);
  } else {
    res = MatPROD(tA, tB, mA, nA, A, nB, B, C);
  }

  diet_free_data(diet_parameter(pb,0));
  diet_free_data(diet_parameter(pb,1));
#if !defined(__FAST_0_4__)
  diet_free_data(diet_parameter(pb,2));
  diet_free_data(diet_parameter(pb,3));
  diet_free_data(diet_parameter(pb,4));
#endif // ! __FAST_0_4__

  printf(" done\n");
  return res;
}

int
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg> [all | [%s][%s][%s][%s][%s] ]\n",
	  cmd, SRV[0], SRV[1], SRV[2], SRV[3], SRV[4]);
  return 1;
}

/***********
 * JNI Part
 ***********/

/* Native function "ping" */
JNIEXPORT void JNICALL 
   Java_JXTASeD_ping(JNIEnv *env, jobject obj)
{
    SeD->ping();
    return;    
}

/* Native function to start the DIET SeD object
 */
 
JNIEXPORT jint JNICALL 
  Java_JXTASeD_startDIETSeD (JNIEnv *env, jobject obj, 
			     jstring config_file, 
			     jstring jxtaUuid, jobjectArray pbs)
{
  char* config_file_name;

  /* LogService components */
  DietLogComponent* dietLogComponent;
  MonitoringThread* monitoringThread;

  /* Getting arguments for DIET SeD */
  argc = env->GetArrayLength(pbs) + 2;
  argv = new char *[argc];

  /* Get config_file_name */
  argv[0] = "SeD";
  config_file_name = strdup (env->GetStringUTFChars(config_file, 0));
  argv[1] = config_file_name;
  
  /* Get uuid */
  uuid = strdup (env->GetStringUTFChars(jxtaUuid, 0));

  /* Getting services */
  for (int i = 0; i < argc - 2; i++) {
    argv [i+2] = strdup (env->GetStringUTFChars
			 (((jstring)env->GetObjectArrayElement
			   (pbs, i)), 0));
  }

  size_t i,j;
  int res(0);
  int services[NB_SRV] = {0, 0, 0, 0, 0};

  diet_profile_desc_t* profile = NULL;
  diet_convertor_t*    cvt     = NULL;
  
  if (argc < 3) {
    return usage(argv[0]);
  }  
  
  for (i = 2; i < argc; i++) {
    char* path = argv[i];
    if (!strcmp("all", path)) {
      for (j = 0; j < NB_SRV; (services[j++] = 1));
    } else {
      for (j = 0; j < NB_SRV; j++) {
	if ((services[j] = (services[j] || !strcmp(SRV[j], path))))
	  break;
      }
      if (j == NB_SRV)
	exit(usage(argv[0]));
    }
  }

  diet_service_table_init(NB_SRV);
  
  if (services[0]) {
    profile = diet_profile_desc_alloc(SRV[0], -1, 0, 0);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    /**
     * Example of convertor:
     *  solve_T assumes that the profile is (IN m, IN n, INOUT A), but we
     *  declare a simpler service as (INOUT A). The convertor will let DIET
     *  extract automatically m and n before calling solve_T.
     */
    cvt = diet_convertor_alloc("T", 1, 2, 2);
    diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL, 0);
    diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL, 0);
    diet_arg_cvt_short_set(&(cvt->arg_convs[2]), 0, NULL);
    
    diet_service_table_add(profile, cvt, solve_T);
    diet_profile_desc_free(profile);
    diet_convertor_free(cvt);
  }
  
  if (services[1] || services[2] || services[3]) {
    const char* path = "still undefined";
    profile = diet_profile_desc_alloc(path, 1, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);

#if defined(__FAST_0_4__)
    /**
     * As FAST 0.4.x performs the conversion matrices -> dimensions, there is no
     * convertor to define for the arguments. But the names of the services
     * offered differ from the names used in the LDIF base. So let us define
     * convertors that only convert the paths.
     */
    cvt = diet_convertor_alloc("still undefined", 1, 1, 2);
    diet_arg_cvt_short_set(&(cvt->arg_convs[0]), 0, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[1]), 1, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[2]), 2, NULL);

    for (i = 1; i <= 3; i++) {
      if (services[i]) {
	free(profile->path);
	profile->path = strdup(SRV[i]);
	free(cvt->path);
	cvt->path = strdup((i == 1) ? "base/mult" : "base/plus");
	if (diet_service_table_add(profile, cvt,
				   (i == 1) ? solve_MatPROD : solve_MatSUM))
	  return 1;
      }
    }
    
#else  // __FAST_0_4__
    {
      diet_convertor_t* cvt_SUM = NULL;
      /**
       * solve_MatPROD assumes that the profile is
       * (IN mA, IN nA, IN nB, IN A, IN B, OUT C), but we declare a simpler
       * service as (IN A, IN B, OUT C). The convertor will let DIET extract
       * automatically mA, nA and nB before calling solve_MatPROD.
       */
      cvt = diet_convertor_alloc("base/mult", 4, 4, 5);
      diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL, 0);
      diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL, 0);
      diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_MAT_NB_COL, 1, NULL, 1);
      diet_arg_cvt_short_set(&(cvt->arg_convs[3]), 0, NULL);
      diet_arg_cvt_short_set(&(cvt->arg_convs[4]), 1, NULL);
      diet_arg_cvt_short_set(&(cvt->arg_convs[5]), 2, NULL);

      /**
       * solve_MatSUM assumes that the profile is
       * (IN mA, IN nA, IN A, IN B, OUT C), but we declare simpler services as
       * (IN A, IN B, OUT C). The convertor will let DIET extract automatically
       * mA and nA before calling solve_MatSUM.
       */
      cvt_SUM = diet_convertor_alloc("base/plus", 3, 3, 4);
      diet_arg_cvt_set(&(cvt_SUM->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL, 0);
      diet_arg_cvt_set(&(cvt_SUM->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL, 0);
      diet_arg_cvt_short_set(&(cvt_SUM->arg_convs[2]), 0, NULL);
      diet_arg_cvt_short_set(&(cvt_SUM->arg_convs[3]), 1, NULL);
      diet_arg_cvt_short_set(&(cvt_SUM->arg_convs[4]), 2, NULL);

      for (i = 1; i <= 3; i++) {
	if (i == 2) {
	  diet_convertor_free(cvt);
	  cvt = cvt_SUM;
	}
	if (services[i]) {
	  free(profile->path);
	  profile->path = strdup(SRV[i]);
	  if (diet_service_table_add(profile, cvt,
				     (i == 1) ? solve_MatPROD : solve_MatSUM))
	    return 1;
	}
      }
    }
#endif // __FAST_0_4__

    diet_convertor_free(cvt);
    diet_profile_desc_free(profile);

  } // if (services[1] || services[2] || services[3])


  if (services[4]) {
    profile = diet_profile_desc_alloc(SRV[4], 0, 1, 1);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);

#if defined(__FAST_0_4__)
    /* Profile expected by solve_MatSUM: (IN A, IN B, OUT C) */
    cvt = diet_convertor_alloc("base/plus", 1, 1, 2);
    i = 0;
#else  // __FAST_0_4__
    /* Profile expected by solve_MatSUM: (IN mA, IN nA, IN A, IN B, OUT C)
     * Thus, we must extract the first two dimensions. */
    cvt = diet_convertor_alloc("base/plus", 3, 3, 4);
    diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL, 0);
    diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL, 0);
    i = 2;
#endif // __FAST_0_4__
    /**
     * As SqMatSUM_opt is declared with the profile (IN A, INOUT B) (to match a
     * sub-service of the BLAS dgemm), the convertor must tell DIET to
     * "duplicate" the (i+2)th argument. Actually, "duplicate" concerns only the
     * description of the argument, not the memory space needed for the whole
     * matrices.
     */
    diet_arg_cvt_short_set(&(cvt->arg_convs[i]), 0, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[i+1]), 1, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[i+2]), 1, NULL);
    if (diet_service_table_add(profile, cvt, solve_MatSUM))
      return 1;
    diet_convertor_free(cvt);
    diet_profile_desc_free(profile);    
  }

  diet_print_service_table();

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


  /* Get listening port & hostname */

  size_t* port = (size_t*) 
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

  /* DietLogComponent creation */
  bool useLS;
  size_t* ULSptr;
  int outBufferSize;
  size_t* OBSptr;
  int flushTime;
  size_t* FTptr;

  ULSptr = (size_t*)Parsers::Results::getParamValue(
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
    OBSptr = (size_t*)Parsers::Results::getParamValue(
  	       Parsers::Results::LSOUTBUFFERSIZE);
    if (OBSptr != NULL) {
      outBufferSize = (int)(*OBSptr);
    } else {
      outBufferSize = 0;
      WARNING("lsOutbuffersize not configured, using default");
    }

    FTptr = (size_t*)Parsers::Results::getParamValue(
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

    dietLogComponent = new DietLogComponent("", outBufferSize);
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

  /* Create the SeD */
  SeD = new SeDImpl(uuid);

  /* Set up LogService */
  SeD->setDietLogComponent(dietLogComponent);

  /* Activate SeD */
  ORBMgr::activate(SeD);
  if (SeD->run(SRVT)) {
    ERROR("unable to launch the SeD", 1);
  }

#if ! HAVE_JUXMEM
  /* Set up Data Manager for DTM */
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

  cout << "DIET SeD : Creating thread for RPC's wait... ";
  omni_thread *RPCsWaitThread = omni_thread::create
    (SeDRPCsWait, (void *)0, omni_thread::PRIORITY_NORMAL);
  cout << "done." << endl;
  
  return 0;

} // startDIETSeD

void 
SeDRPCsWait (void* par)
{
  /* Wait for RPCs : */
  ORBMgr::wait();

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  ORBMgr::destroy();
} // SeDRPCsWait


JNIEXPORT jstring 
JNICALL Java_JXTASeD_solveJXTA (JNIEnv *env, jobject obj, 
				jstring pb, jstring nbRow, jstring nbCol, 
				jstring _mat1, jstring _mat2, jstring reqID)
{

  /* create C types from Java types */
  char* pbName = strdup(env->GetStringUTFChars(pb, 0));

  long int nbR, nbC, rID;
  nbR = strToLong(strdup(env->GetStringUTFChars(nbRow, 0)));
  nbC = strToLong(strdup(env->GetStringUTFChars(nbCol, 0))); 
  rID = strToLong(strdup(env->GetStringUTFChars(reqID, 0)));

  char* mat1Char = strdup(env->GetStringUTFChars(_mat1, 0));
  char* mat2Char = strdup(env->GetStringUTFChars(_mat2, 0));

  double* mat1Value = new double [nbR * nbC];
  double* mat2Value = new double [nbR * nbC];

  int k = 0;
  for (int i = 0; i < strlen(mat1Char); i++) {
    char* inter = new char [10];
    int j = 0;
    for (i = i; i < strlen(mat1Char) && mat1Char[i] != ' '; i++)
      inter[j++] = mat1Char[i];
    inter[j] = '\0';

    mat1Value[k++] = strToDouble(inter);
  }
  
  k = 0;
  for (int i = 0; i < strlen(mat2Char); i++) {
    char* inter = new char [10];
    int j = 0;
    for (i = i; i < strlen(mat2Char) && mat2Char[i] != ' '; i++)
      inter[j++] = mat2Char[i];
    inter[j] = '\0';

    mat2Value[k++] = strToDouble(inter);
  }

  /* create the 3 matrices */

  corba_matrix_specific_t mat1;
  corba_matrix_specific_t mat2;
  corba_matrix_specific_t mat3;

  mat1.nb_r = nbR;
  mat1.nb_c = nbC;
  mat1.order = 1;
  
  mat2.nb_r = nbR;
  mat2.nb_c = nbC;
  mat2.order = 1;
  
  mat3.nb_r = nbR;
  mat3.nb_c = nbC;
  mat3.order = 1;

  /* create the 3 data desc */

  corba_data_desc_t data_desc1;
  corba_data_desc_t data_desc2; 
  corba_data_desc_t data_desc3;

  data_desc1.mode = 0;
  data_desc1.base_type = 5;
  data_desc1.specific.mat(mat1);

  data_desc2.mode = 0;
  data_desc2.base_type = 5;
  data_desc2.specific.mat(mat2);

  data_desc3.mode = 0;
  data_desc3.base_type = 5;
  data_desc3.specific.mat(mat3);

  /* create the matrix containing result of computation */
  double* mat3Value = new double [nbR * nbC];
  for (int i = 0;  i < nbR * nbC; mat3Value[i++] = 0);
  
  /* sequence of datas */
  SeqCorbaData_t seqData;
  seqData.length(3);

  seqData[0].desc = data_desc1;
  seqData[0].value = SeqChar(32 * sizeof(double), 
			     16 * sizeof(double), 
			     (CORBA::Char*)mat1Value);

  seqData[1].desc = data_desc2;
  seqData[1].value = SeqChar(32 * sizeof(double), 
			     16 * sizeof(double), 
			     (CORBA::Char*)mat2Value);

  seqData[2].desc = data_desc3;
  seqData[2].value = SeqChar(32 * sizeof(double), 
			     16 * sizeof(double), 
			     (CORBA::Char*)mat3Value);

  /* create profile and solve it */
  corba_profile_t profile;
  profile.last_in = 1;
  profile.last_inout = 1;
  profile.last_out = 2;
  profile.parameters = seqData;

  long int ret = SeD->solve(pbName, profile, rID);

  /* extract and print the result */
  char *matRes = new char[nbR * nbC * sizeof (double)];
  for (int m = 0; m < nbR * nbC * sizeof(double); m++)
      matRes[m] = profile.parameters[2].value[m];

  cout << "DIET SeD : Computed matrix : " << endl;
  double *mat = (double *)matRes;
  for (int i = 0; i < nbR; i++) {
    for (int j = 0; j < nbC; j++) {
      cout << mat[j+i*nbC] << " ";
    }
    cout << endl;
  }

  /* serialize and return the result */
  char* resul = new char [500];
  resul[0] = '\0';
  for (int i = 0; i < nbR; i++) {
    for (int j = 0; j < nbC; j++) {
      strcat(resul, itoa((int)mat[j+i*nbC]));
      strcat(resul, " ");
    }
  }
  return (env->NewStringUTF(resul));

} // solveJXTA

/* parse the char * and return it as a long int */
long int 
strToLong (char *str)
{
  long int d = 0;
  int diz = 1; 
  for(int i = strlen(str) - 1; i >= 0; i--) {
    d = d + (str[i]-'0') * diz;
    diz = diz * 10;
  }
  return (d);
} // str to long

/* parse the char * (representing an integer) parameter 
 * and return it as a double */
double strToDouble (char *str)
{
  double d = 0;
  int diz = 1;
  for(int i = strlen(str) - 1; i >= 0; i--)
  {
    d = d + (str[i]-'0') * diz;
    diz = diz * 10;
  }
  return (d);
} // str to double

/* return a char * representing the parameter int value */
char *itoa (int i)
{
  char *s = new char [50];
  int tmp = i;
  int cpt = 0;
  if (tmp == 0) {
    return "0";
  }
  else {
    while (tmp >= 1) {
	s[cpt++] = tmp % 10 + '0';
	tmp-=tmp % 10;
	tmp/=10;
    }
    s[cpt] = '\0';
    char *srev = new char[strlen(s)];
    int irev = 0;
			  
    for (int i = strlen(s)- 1; i >= 0; i--)
      srev[irev++] = s[i];
    srev[irev] = '\0';
    
    return (srev);
  }
} // itoa
