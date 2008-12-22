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
 * Revision 1.46  2008/12/22 13:38:54  bdepardo
 * Added diet_wait_batch_job_completion to explicitely wait for the end of a
 * batch job.
 *
 * Revision 1.45  2008/12/08 15:31:42  bdepardo
 * Added the possibility to remove a service given its profile description.
 * So now, one is able to remove a service given either the real profile,
 * or the profile description.
 *
 * Revision 1.44  2008/11/18 10:11:07  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 * Revision 1.43  2008/06/25 09:52:45  bisnard
 * - Estimation vector sent with solve request to avoid storing it
 * for each submit request as it depends on the parameters value. The
 * estimation vector is used by SeD to updates internal Gantt chart and
 * provide earliest finish time to submitted requests.
 * ==> added parameter to diet_call_common & diet_call_async_common
 *
 * Revision 1.42  2008/06/10 21:01:27  ycaniou
 * Cosmetics
 *
 * Revision 1.41  2008/05/28 12:30:34  rbolze
 * change unit of the startTime value store in the diet_job_t structure.
 * it was in second, now it is in millisecond
 *
 * Revision 1.40  2008/05/19 14:45:07  bisnard
 * jobs added to the queue during submit instead of solve
 *
 * Revision 1.39  2008/05/16 12:25:05  bisnard
 * API give status of all jobs running or waiting on the SeD
 * (used to compute earliest finish time)
 *
 * Revision 1.38  2008/05/11 16:19:48  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.37  2008/05/05 13:54:17  bisnard
 * new computation time estimation get/set functions
 *
 * Revision 1.36  2008/04/07 15:33:40  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.35  2008/04/07 13:11:44  ycaniou
 * Correct "deprecated conversion from string constant to 'char*'" warnings
 * First attempt to code functions to dynamicaly get batch information
 * 	(e.g.,  getNbMaxResources(), etc.)
 *
 * Revision 1.34  2008/01/01 19:45:22  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 * Revision 1.33  2007/12/18 13:04:27  glemahec
 * This commit adds the "diet_estimate_waiting_jobs" function to obtain the
 * number of jobs waiting in the FIFO queue when using the max concurrent
 * jobs limit. This function has to be used in the SeD plugin schedulers.
 *
 * Revision 1.32  2007/04/16 22:43:42  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.31  2006/09/18 19:41:57  ycaniou
 * Removed from the server API diet_submit_batch() which is integrated in
 *   diet_submit_parallel(): it handles parallel AND batch jobs
 * Added diet_profile_desc_set_sequential(), which is unecessary because the
 *   default in DIET, but the pendant of diet_profile_desc_set_parallel()
 *
 * Revision 1.30  2006/09/11 11:16:58  ycaniou
 * - client have now 3 kind of submission (default is conserved)
 * - server can be parallel in addition to batch (not yet tested)
 *
 * Revision 1.29  2006/08/30 11:56:13  ycaniou
 * Commit the changements on the API for batch/parallel submissions
 *
 * Revision 1.28  2006/06/30 15:47:42  ycaniou
 * Coquilles, and batch stuff
 *
 * Revision 1.27  2006/05/12 07:42:35  eboix
 *  * FIX: static libraries and static linking of examples should now be effective
 *    - include/DIET_server.h: diet_generic_desc_set is now simply defined
 *      (as opposed to declared AND then further defined) which clarifies
 *      its inline status.
 *    - Cmake/ConfigureCCompiler.cmake handling of C compiler specifities now
 *      leave outside of DIET's central CMakeLists.txt.
 *    - CMakeLists.txt: now handles the non compliance of GCC to ISO-C99's
 *      inline semantics.                                      --- Injay2461
 *
 * Revision 1.26  2006/04/12 16:13:11  ycaniou
 * Commentaries C++->C to avoid compilation warnings
 *
 * Revision 1.25  2006/01/25 21:07:59  pfrauenk
 * CoRI - plugin scheduler: the type diet_est_tag_t est replace by int
 *        some new fonctions in DIET_server.h to manage the estVector
 *
 * Revision 1.24  2006/01/19 21:18:47  pfrauenk
 * CoRI: adding 2 new functions - correction of some comments
 *
 * Revision 1.23  2005/12/20 07:52:44  pfrauenk
 * CoRI functionality added: FAST is hided, information about number of processors,
 * amount of available memory and of free cpu are provided
 *
 * Revision 1.22  2005/08/31 14:37:07  alsu
 * New plugin scheduling interface:
 *  - prototype of diet_perfmetric_t changed; now takes estVector_t as an
 *    argument, rather than emitting it as a return value
 *  - new estimation vector interface, creating separate "tag-spaces" for
 *    scalar and array data
 *
 * Revision 1.21  2005/08/30 12:49:57  ycaniou
 * Added the  diet_submit_call_t in DIET_server.h and the dietJobID field in
 *   the diet_profile_t
 * Added some comments in batch.tex
 *
 * Revision 1.20  2005/05/15 15:31:44  alsu
 * - aggregator structures and interfaces
 * - simplified estimation vector interface
 *
 * Revision 1.19  2005/05/02 14:51:53  ycaniou
 * The client API has now the functions diet_call_batch() and diet_call_async_batch(). The client has also the possibility to modify the profile so that it is a batch, parallel or if he wants a special number of procs.
 * Changes in diet_profile_t and diet_profile_desc_t structures
 * Functions to register a parallel or batch problem.
 * The SeD developper must end its profile solve function by a call to diet_submit_batch().
 *
 * Revision 1.18  2004/12/15 18:09:58  alsu
 * cleaner, easier to document interface: changing diet_perfmetric_t back
 * to the simpler one-argument (of type diet_profile_t) version, and
 * hiding a copy of the pointer back to the SeD in the profile.
 *
 * Revision 1.17  2004/12/08 15:02:51  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.16.2.5  2004/11/26 15:19:01  alsu
 * "length" field of string arguments calculated automatically now
 *
 * Revision 1.16.2.4  2004/11/06 16:22:55  alsu
 * estimation vector access functions now have parameter-based default
 * return values
 *
 * Revision 1.16.2.3  2004/11/02 00:27:52  alsu
 * adding a setEstimation interface for the estimation vector (to be used
 * in the case where a value for a particular tag should replace all
 * instances of that tag)
 *
 * Revision 1.16.2.2  2004/10/27 22:35:50  alsu
 * include
 *
 * Revision 1.16.2.1  2004/10/26 14:12:52  alsu
 * (Tag: AS-plugin-sched)
 *  - branch created to avoid conflicting with release 1.2 (imminent)
 *  - initial commit on branch, new dynamic performance info structure in
 *    the profile
 *
 * Revision 1.16  2004/05/24 20:54:41  alsu
 * replacing diet_service_table_set_perfmetric with shorter, less
 * confusing function name diet_service_use_perfmetric
 *
 * Revision 1.15  2004/05/18 21:24:46  alsu
 * - adding diet_perfmetric_t type
 * - adding diet_service_table_set_perfmetric function
 *
 * Revision 1.14  2003/08/09 17:30:38  pcombes
 * Include path in the diet_profile_desc structure.
 *
 * Revision 1.13  2003/06/03 18:33:56  pcombes
 * Update comments.
 *
 * Revision 1.11  2003/02/07 17:04:12  pcombes
 * Refine convertor API: arg_idx is splitted into in_arg_idx and out_arg_idx.
 *
 * Revision 1.10  2003/02/04 10:08:22  pcombes
 * Apply Coding Standards
 *
 * Revision 1.8  2003/01/23 18:37:30  pcombes
 * API 0.6.4: change _set "dummy" arguments
 *
 * Revision 1.7  2003/01/22 17:14:09  pcombes
 * API 0.6.4 : istrans -> order (row- or column-major)
 *
 * Revision 1.6  2003/01/17 18:08:43  pcombes
 * New API (0.6.3): structures are not hidden, but the user can ignore them.
 *
 * Revision 1.3  2002/10/15 18:36:04  pcombes
 * Remove the descriptors set functions.
 * Add convertors in API :
 *     Convertors let DIET build the sequence of arguments, that the
 *     correspunding solver needs, from the client sequence of arguments
 *     (which might match another service declared with the same solver).
 * For compatibility with old API, just add NULL for the convertor argument in
 * diet_service_table_add. But all the solvers needed before in programs using
 * DIET can be transformed into convertors.
 ****************************************************************************/


#ifndef _DIET_SERVER_H_
#define _DIET_SERVER_H_

#include "DIET_data.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/
/* DIET aggregator descriptor                                               */
/****************************************************************************/

typedef struct {
  char __dummy;
} diet_aggregator_default_t;

typedef struct {
  int p_numPValues;
  int* p_pValues;
} diet_aggregator_priority_t;

typedef struct {
  diet_aggregator_type_t agg_method;
  union {
    diet_aggregator_default_t agg_specific_default;
    diet_aggregator_priority_t agg_specific_priority;
  } agg_specific;
} diet_aggregator_desc_t;

/****************************************************************************/
/* DIET service profile descriptor                                          */
/****************************************************************************/

typedef struct diet_data_generic diet_arg_desc_t;

typedef struct {
  char*            path;
  int              last_in, last_inout, last_out;
  diet_arg_desc_t* param_desc;

#if defined HAVE_ALT_BATCH
  unsigned short int parallel_flag ;
  int nbprocs ;
#endif

  /* aggregator description, used when declaring a service */
  diet_aggregator_desc_t aggregator;
} diet_profile_desc_t;

/**
 * Type:
 *  (diet_arg_desc_t*) diet_param_desc ( (diet_profile_desc_t*), (size_t) )
 * Pointer to the nth parameter of a profile description
 */
#define diet_param_desc(pt_prof_desc, n) &((pt_prof_desc)->param_desc[(n)])


/**
 * Allocate a DIET profile descriptors with memory space for its argument
 * descriptors.
 * If no IN argument, please give -1 for last_in.
 * If no INOUT argument, please give last_in for last_inout.
 * If no OUT argument, please give last_inout for last_out.
 * Once allocation is performed, please use set functions for each descriptor.
 * For example, the nth argument is a matrix of doubles:
 *  diet_generic_desc_set(diet_param_desc(profile,n), DIET_MATRIX; DIET_DOUBLE);
 */
diet_profile_desc_t*
diet_profile_desc_alloc(const char* path,
			int last_in, int last_inout, int last_out);
int
diet_profile_desc_free(diet_profile_desc_t* desc);

#if defined HAVE_ALT_BATCH
/* Functions for server profile registration */
int
diet_profile_desc_set_sequential(diet_profile_desc_t * profile) ;
int
diet_profile_desc_set_parallel(diet_profile_desc_t * profile) ;
/* Used to explicitely wait for the completion of a batch job */
int
diet_wait_batch_job_completion(diet_profile_t * profile);
#endif




/**
 * Configure a priority aggregator
 */
diet_aggregator_desc_t*
diet_profile_desc_aggregator(diet_profile_desc_t* profile);
int diet_aggregator_set_type(diet_aggregator_desc_t* agg,
                             diet_aggregator_type_t atype);
int diet_aggregator_priority_max(diet_aggregator_desc_t* agg,
                                 int tag);
int diet_aggregator_priority_min(diet_aggregator_desc_t* agg,
                                 int tag);
int diet_aggregator_priority_maxuser(diet_aggregator_desc_t* agg, int val);
int diet_aggregator_priority_minuser(diet_aggregator_desc_t* agg, int val);

/****************************************************************************/
/* DIET problem evaluation                                                  */
/****************************************************************************/
/**
 * The server may declare several services for only one underlying routine.
 * Thus, diet_convertors are useful to translate the various declared profiles
 * into the actual profile of the underlying routine, ie the profile that is
 * used for the FAST benches.
 * Internally, when a client requests for a declared service, the correspunding
 * convertor is used to generate the actual profile : this allows evaluation
 * (cf. below)
 */

typedef enum {
  DIET_CVT_IDENTITY = 0,
  DIET_CVT_VECT_SIZE,
  DIET_CVT_MAT_NB_ROW,
  DIET_CVT_MAT_NB_COL,
  DIET_CVT_MAT_ORDER,
  DIET_CVT_STR_LEN,
  DIET_CVT_FILE_SIZE,
  DIET_CVT_COUNT
} diet_convertor_function_t;

/**
 * When the server receives data from the client:
 *  - If in_arg_idx is a valid argument index of the sent profile, then the
 *    function f is applied to the diet_arg_t indexed by in_arg_idx in this
 *    profile parameters array.
 *  - Else, f is applied to arg (for instance, if in_arg_idx is -1).
 * When the server sends results to the client:
 *  - If out_arg_idx is a valid argument index of the sent profile, then the
 *    function DIET_CVT_IDENTITY is applied to the diet_arg_t indexed by
 *    out_arg_idx in this profile parameters array.
 *
 * NB: when DIET_CVT_IDENTITY is applied several times on an IN parameter, the
 *     data are duplicated, so that the server gets two (or more) copies.
 */
typedef struct {
  diet_convertor_function_t f;
  int              in_arg_idx;
  diet_arg_t*             arg;
  int             out_arg_idx;
} diet_arg_convertor_t;


int
diet_arg_cvt_set(diet_arg_convertor_t* arg_cvt, diet_convertor_function_t f,
		 int in_arg_idx, diet_arg_t* arg, int out_arg_idx);

/**
 * This function is often used with this configuration:
 *  - f == DIET_CVT_IDENTITY
 *  - out_arg_idx == in_arg_idx
 * Thus we define the following macro that sets these two arguments ...
 * Type:
 *  (int) diet_arg_cvt_short_set((diet_arg_convertor_t*), (int), (diet_arg_t *))
 */
#define diet_arg_cvt_short_set(arg_cvt,arg_idx,arg) \
  diet_arg_cvt_set((arg_cvt), DIET_CVT_IDENTITY, (arg_idx), (arg), (arg_idx))


typedef struct {
  char*                 path;
  int                   last_in, last_inout, last_out;
  diet_arg_convertor_t* arg_convs;
#if defined HAVE_ALT_BATCH
  unsigned short int batch_flag ;
  int nbprocs ;
  unsigned long walltime ;
#endif
} diet_convertor_t;

/**
 * Type:
 *  (diet_arg_convertor_t*) diet_arg_conv ( (diet_convertor_t*), (size_t) )
 * Pointer to the nth arg convertor of a convertor
 */
#define diet_arg_conv(cvt,n) &((cvt)->arg_convs[(n)])

diet_convertor_t*
diet_convertor_alloc(const char* path,
		     int last_in, int last_inout, int last_out);

/**
 * Free also arg field (if not NULL) of each diet_arg_convertor_t in arg_convs
 * array. Be careful to the coherence between arg_idx and arg fields !!!
 */
int
diet_convertor_free(diet_convertor_t* cvt);


/****************************************************************************/
/* DIET evaluation function prototype                                       */
/****************************************************************************/
/**
 * If FAST cannot evaluate the computation time of a request, for instance
 * because the FAST benches have not been performed, then the service programmer
 * should provide an evaluation function that uses the actual profile fo the
 * service to forecast the computation time.
 * NB: The profile internally given to the eval function has not the value
 * fields set, since evaluation takes place before data are transfered.
 */

typedef int (* diet_eval_t)(diet_profile_t*, double*);


/****************************************************************************/
/* DIET solve function prototype                                            */
/****************************************************************************/

typedef int (* diet_solve_t)(diet_profile_t*);

/****************************************************************************/
/* DIET performance metric function prototype                               */
/****************************************************************************/

typedef void (* diet_perfmetric_t)(diet_profile_t*, estVector_t);


/****************************************************************************/
/* Utils for setting (IN)OUT arguments (solve functions, after computation) */
/****************************************************************************/
/**
 * These functions do not alter the data memory sizes: ie the server CANNOT
 * alter the arguments sizes, since the necessary memory space is allocated by
 * DIET, following the client specifications.
 * If value              is NULL,
 * if order              is DIET_MATRIX_ORDER_COUNT,
 * if nb_rows or nb_cols is 0,
 * if path               is NULL,
 * then the correspunding field is not modified.
 */

int
diet_scalar_desc_set(diet_data_t* data, void* value);

  /* No use of diet_vector_desc_set: size should not be altered by server */

  /* You can alter nb_r and nb_c, but the total size must remain the same */
int
diet_matrix_desc_set(diet_data_t* data,
		     size_t nb_r, size_t nb_c, diet_matrix_order_t order);

  /* No use of diet_string_desc_set: length should not be altered by server */

int
diet_file_desc_set(diet_data_t* data, char* path);

/****************************************************************************/
/* DIET service table API                                                   */
/****************************************************************************/
/* No need to reference the service table since it is unique for the SeD.   */

int diet_service_table_init(int max_size);
/* (cvt = NULL) is equivalent to "no conversion needed" */
diet_perfmetric_t diet_service_use_perfmetric(diet_perfmetric_t fn);
int diet_service_table_add(const diet_profile_desc_t* const profile,
                           const diet_convertor_t*    const cvt,
                           diet_solve_t               solve_func);

#ifdef HAVE_DAGDA
/* Function to remove a service from the server.
 * It also informs its parent from the removal.
 */
int diet_service_table_remove(const diet_profile_t* const profile);
int diet_service_table_remove_desc(const diet_profile_desc_t* const profile);
#endif
  /* Unused !!!
int diet_service_table_lookup(const diet_profile_desc_t* const profile);
  */
int diet_service_table_lookup_by_profile(const diet_profile_t* const profile);
void diet_print_service_table();


/****************************************************************************/
/* DIET server call                                                         */
/****************************************************************************/

/* Most users should set argc to 0 and argv to NULL.
   Advanced omniORB users can set these arguments that are transmitted to
   the ORB initialization. */
int
diet_SeD(char* config_file_name, int argc, char* argv[]);

/****************************************************************************/
/* DIET batch submit call                                                   */
/****************************************************************************/
#if defined HAVE_ALT_BATCH
  /**
   * @param command is the content of the script that the SeD programmer wants
   * to submit in order to provide the service @param profile
   */
  int
  diet_submit_parallel(diet_profile_t * profile, const char * command) ;

  /* TODO: this will disapear when BatchSystem will implement seqFIFO,
     paralFIFO, LOADLEVELER, etc., since the definition would be
     straightforward then. */
  typedef enum {
    SERIAL,
    BATCH,
    NB_SERVER_STATUS
  } diet_server_status_t ;

  void
  diet_set_server_status( diet_server_status_t status ) ;

  /* TODO: erase me, i am in Cori_batch */
  /*   int */
  /*   diet_getNbMaxResources(diet_profile_t * profile) ; */

  /*   int */
  /*   diet_getNbIdleResources(diet_profile_t * profile) ; */
#endif



/****************************************************************************/
/* DIET standard estimation methods (DIET_server.cc)                        */
/****************************************************************************/
int diet_est_set(estVector_t ev, int userTag, double value);
double diet_est_get(estVectorConst_t ev, int userTag, double errVal);
double diet_est_get_system(estVectorConst_t ev,
			     int systemTag,
			     double errVal);
int diet_est_defined(estVectorConst_t ev, int userTag);
int diet_est_defined_system(estVectorConst_t ev, int systemTag);
int diet_est_array_size(estVectorConst_t ev, int userTag);
int diet_est_array_size_system(estVectorConst_t ev, int systemTag);

int diet_est_array_set(estVector_t ev, int userTag, int idx, double value);
double diet_est_array_get(estVectorConst_t ev,
                          int userTag,
                          int idx,
                          double errVal);
double diet_est_array_get_system(estVectorConst_t ev,
                          int systemTag,
                          int idx,
                          double errVal);
int diet_est_array_defined(estVectorConst_t ev, int userTag, int idx);
int diet_est_array_defined_system(estVectorConst_t ev, int systemTag, int idx);

#ifdef HAVE_ALT_BATCH
/* These two functions shall be removed and a better mechanism found
   for example vhen and if CoRI is rewritten. Or clients using CoRI must
   be written in C and not in C++...
   FIXME: Do they have to be integrated to the documented API at the moment?
*/
estVector_t
diet_new_estVect() ;

void
diet_destroy_estVect( estVector_t perfVect ) ;
#endif

int diet_estimate_cori(estVector_t ev,
		       int info_type,
		       diet_est_collect_tag_t collector_type,
		       const void * data);

int diet_estimate_cori_add_collector(diet_est_collect_tag_t collector_type,
				     void* data);

void diet_estimate_coriEasy_print();

int diet_estimate_fast(estVector_t ev, const diet_profile_t* const profilePtr);

int diet_estimate_lastexec(estVector_t ev,
                           const diet_profile_t* const profilePtr);

int diet_estimate_comptime(estVector_t ev, double value);

/* To obtain the queue size. */
int diet_estimate_waiting_jobs(estVector_t ev,
			       const diet_profile_t* const profilePtr) ;

/* To get the list of jobs currently waiting or running */
/* Note: job vector must be deleted after usage */
typedef enum {
  DIET_JOB_WAITING,
  DIET_JOB_RUNNING,
  DIET_JOB_FINISHED
} diet_job_status_t;

typedef struct {
  estVector_t           estVector;
  diet_job_status_t     status;
  double                startTime; /* in ms */
} diet_job_t;

typedef diet_job_t* jobVector_t;

int diet_estimate_list_jobs(jobVector_t* jv, int* jobNb,
                            const diet_profile_t* const profilePtr);

/****************************************************************************/
/* Inline definitions                                                       */
/****************************************************************************/


/** Utils for setting data descriptors (service construction)
 *
 * Every -1 argument implies that the correspunding field is not modified.
 */
inline int
diet_generic_desc_set( struct diet_data_generic* desc,
		       diet_data_type_t type,
                       diet_base_type_t base_type )
{
  if (!desc)
    return 1;
  if (type != DIET_DATA_TYPE_COUNT)
    desc->type      = type;
  if (base_type != DIET_BASE_TYPE_COUNT)
    desc->base_type = base_type;
  return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DIET_SERVER_H_ */
