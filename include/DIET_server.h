/**
* @file DIET_server.h
*
* @brief  DIET server interface
*
* @author Philippe COMBES (Philippe.Combes@ens-lyon.fr)
*
* @section Licence
*   |LICENSE|

   @details

   A \c DIET server program is the link between the \c DIET Server Deamon
   (SeD) and the libraries that implement the service to offer.

   \section ProgramStructure Structure of the program

   As for the client side, the \c DIET SeD is a library. So the server
   developer needs to define the \c main function. Within the
   \c main, the \c DIET server will be launched with a call to
   \c diet_SeD which will never return (except if some errors
   occur).

   \code
   #include <stdio.h>
   #include <stdlib.h>

   #include "DIET_server.h"
   \endcode

   The second step is to define a function whose prototype is "\c DIET-normalized"
   and which will be able to convert the function into the library function prototype.
   Let us consider a library function with the following prototype:
   \code
   int service(int arg1, char *arg2, double *arg3);
   \endcode

   This function cannot be called directly by \c DIET, since such a prototype is hard
   to manipulate dynamically. The user must define a "solve" function whose
   prototype only consists of a \c diet_profile_t.
   This function will be called by the \diet \sed through a pointer.

   \code
   int solve_service(diet_profile_t *pb)
   {
   int    *arg1;
   char   *arg2;
   double *arg3;

   diet_scalar_get(diet_parameter(pb, 0), &arg1, NULL);
   diet_string_get(diet_parameter(pb, 1), &arg2, NULL);
   diet_scalar_get(diet_parameter(pb, 2), &arg3, NULL);
   return service(*arg1, arg2, arg3);
   }
   \endcode

   Several API functions help the user to write this "solve"
   function, particularly for getting IN arguments as well as setting
   OUT arguments.

   \subsubsection GettingArguments Getting IN, INOUT and OUT arguments

   The \c diet_*_get functions defined in \c DIET_data.h are still
   usable here. Do not forget that the necessary memory space for OUT arguments is
   allocated by \diet. So the user should call the \c diet_*_get functions
   to retrieve the pointer to the zone his/her program should write to.

   \subsubsection SettingArguments Setting INOUT and OUT arguments

   To set INOUT and OUT arguments, use the \c diet_*_desc_set defined
   in \c DIET_server.h, these are helpful for writing "solve"
   functions only. Using these functions, the server developer must keep in
   mind the fact that he cannot alter the memory space pointed to by
   value fields on the server. Indeed, this would make \c DIET confused
   about how to manage the data (And the server developer
   should not be confused by the fact that
   \c diet_scalar_desc_set uses a value, since scalar values
   are copied into the data descriptor.).

   \code
   //
   // If value                 is NULL,
   // or if order              is DIET_MATRIX_ORDER_COUNT,
   // or if nb_rows or nb_cols is 0,
   // or if path               is NULL,
   // then the corresponding field is not modified.


   int
   diet_scalar_desc_set(diet_data_t* data, void* value);

   // No use of diet_vector_desc_set: size should not be altered by server

   // You can alter nb_r and nb_c, but the total size must remain the same
   int
   diet_matrix_desc_set(diet_data_t* data,
   size_t nb_r, size_t nb_c, diet_matrix_order_t order);

   // No use of diet_string_desc_set: length should not be altered by server

   int
   diet_file_desc_set(diet_data_t* data, char* path);
   \endcode


   \section ServerAPI Server API

   \subsubsection DefiningServices Defining services

   First, declare the service(s) that will be offered (It is
   possible to declare several services for one single SeD.).
   Each service is described by a profile description called
   \c diet_profile_desc_t since the service does not specify
   the sizes of the data. The \c diet_profile_desc_t type is
   defined in \c DIET_server.h, and is very similar to
   \c diet_profile_t. The difference is that the prototype is
   described with the generic parts of \e diet_data_desc only,
   whereas the client description uses full \e diet_data.

   \code
   struct diet_data_generic {
   diet_data_type_t type;
   diet_base_type_t base_type;
   };

   file DIET_server.h:
   typedef struct diet_data_generic diet_arg_desc_t;

   typedef struct {
   char*            path;
   int              last_in, last_inout, last_out;
   diet_arg_desc_t* param_desc;
   } diet_profile_desc_t;

   diet_profile_desc_t* diet_profile_desc_alloc(const char* path, int last_in, int last_inout, int last_out);
   int diet_profile_desc_free(diet_profile_desc_t* desc);

   \endcode

   Each profile can be allocated with \c diet_profile_desc_alloc with the
   same semantics as for \c diet_profile_alloc. Every argument of the
   profile will then be set with \c diet_generic_desc_set.

   \subsubsection DeclaringServices Declaring services

   Every service must be added in the service table before the server is
   launched:

   \code
   typedef int (* diet_solve_t)(diet_profile_t *);
   int diet_service_table_init(int max_size);
   int diet_service_table_add(diet_profile_desc_t *profile,
   diet_convertor_t    *cvt,
   diet_solve_t         solve_func);
   void diet_print_service_table();
   \endcode

   The parameter \c diet_solve_t \c solve_func is the type of the
   \c solve_service function: a function pointer used by \c DIET to launch the
   computation.

   The parameter \c diet_convertor_t \c *cvt is to be used in combination
   with FAST (if available). It is there to allow profile conversion (for
   multiple services, or when differences occur between the \diet and the FAST
   profile).

*/


#ifndef _DIET_SERVER_H_
#define _DIET_SERVER_H_

#include "DIET_data.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /***************************************************************************/
  /* DIET aggregator descriptor                                              */
  /***************************************************************************/

  /**
     Structure representing the default \c DIET aggregator
  */
  typedef struct {
    char __dummy; /*!< dummy value */
  } diet_aggregator_default_t;

  /**
     Structure representing the DIET aggregator priority
  */
  typedef struct {
    int p_numPValues; /*!< number of priority values */
    int* p_pValues; /*!< an array of priority values */
  } diet_aggregator_priority_t;

  /**
     Structure describing a diet_aggregator
  */
  typedef struct {
    diet_aggregator_type_t agg_method; /*!< aggregation method */
    union {
      /*!< specific elements of a default aggregator */
      diet_aggregator_default_t agg_specific_default;
      /*!< specific elements of a priority aggregator */
      diet_aggregator_priority_t agg_specific_priority;
    } agg_specific;
  } diet_aggregator_desc_t;

  /***************************************************************************/
  /* DIET service profile descriptor                                         */
  /***************************************************************************/

  /**
     Structure of generic info a data in \c DIET
  */
  typedef struct diet_data_generic diet_arg_desc_t;

  /**
     Structure representing a profile description
  */
  typedef struct {
    char* path; /*!< The name of the service */
    int last_in; /*!< \f$-1\f$ + number of input data */
    int last_inout; /*!< \e last_in \f$+\f$ number of inout data */
    int last_out; /*!< \e last_inout \f$+\f$ number of out data */
    diet_arg_desc_t* param_desc; /*!< array of parameter descriptions */

#if defined HAVE_ALT_BATCH
    /*!< Parallel environment. Only usefull for SGE.
      Seems to be dependant on local configuration...
      I have seen mpich, mpich2, lam, ompi, mpichgm, openmp */
    char * parallel_environment;
    /*!< flag defining the parallel aspects:
      @arg if 0, select seq AND parallel tasks for the request
      @arg if 1, select only seq tasks
      @arg if 2, parallel only
    */
    unsigned short int parallel_flag;
    int nbprocs; /*!< Number of processors available? */
#endif
    /* aggregator description, used when declaring a service */
    /*!< aggregator description, used when declaring a service */
    diet_aggregator_desc_t aggregator;
  } diet_profile_desc_t;

  /**
     Function used to get a pointer to the nth parameter of a profile
     description

     Type: (diet_arg_desc_t*) diet_param_desc ((diet_profile_desc_t*), (size_t))

     /arg pt_prof_desc pointer to profile description
     /arg n index in the profile description we wan to retrieve

     Returns the corresponding parameter of the profile description

     @warning no error checking is done on array bounds.
     @todo perhaps a transformation to a function would be better with a
     error checking on array bounds to manage negative values of out of bound
     values
  */
#define diet_param_desc(pt_prof_desc, n) &((pt_prof_desc)->param_desc[(n)])

  /**
     Function used to allocate a DIET profile descriptors with memory space
     for its argument descriptors.
     @arg If no IN argument, please give -1 for last_in.
     @arg If no INOUT argument, please give last_in for last_inout.
     @arg If no OUT argument, please give last_inout for last_out.

     Once allocation is performed, please use set functions for each descriptor.
     For example, the nth argument is a matrix of doubles:
     \code
     diet_generic_desc_set(diet_param_desc(profile, n), DIET_MATRIX; DIET_DOUBLE);
     \endcode

     @param path name of the service
     @param last_in last_in : \f$-1\f$ + number of input data
     @param last_inout last_inout : \e last_in \f$+\f$ number of inout data
     @param last_out last_out : \e last_inout \f$+\f$ number of out data

     @return the newly allocated diet_profile_desc
  */
  DIET_API_LIB diet_profile_desc_t*
  diet_profile_desc_alloc(const char* path, int last_in,
                          int last_inout, int last_out);

  /**
     Function used to free the provided profile description

     @param desc diet profile description to free

     @return error value specifying whether the deallocation
     was successful or not:
     @arg 0 if everything is ok
     @arg 1 otherwise (description was NULL or no argument should be
     provided and the parameter description is not null)

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_profile_desc_free(diet_profile_desc_t* desc);

#if defined HAVE_ALT_BATCH
  /* Functions for server profile registration */
  /**
     Function used to set the sequential flag (of parallel_flag)
     on the profile description

     @param profile profile description to modify
     @return error value specifying if the operation was successful or not:
     0 in every case

     @warning function declared in DIET_server.h and implemented in
     DIET_data.cc

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_profile_desc_set_sequential(diet_profile_desc_t * profile);

  /**
     Function used to set the parallel flag (parallel_flag)
     on the profile description

     @param profile profile description to modify
     @return error value specifying if the operation was successful or not:
     0 in every case

     @warning function declared in DIET_server.h and implemented in
     DIET_data.cc

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_profile_desc_set_parallel(diet_profile_desc_t * profile);

  /**
     Function used to explicitely wait for the completion of a batch job

     @param profile on which we want to explicitely wait for the completion
     of a batch job
     @return error value specifying whether the job completion was correct or
     not:
     @arg 1 is the job was terminated, cancelled or on error
     @arg -1 if there was an error when trying to read the status,
     if the submission of the script could not be performed,
     if there was an error with I/O file,
     if the whole batch status of specific batch job status cannot be read

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_wait_batch_job_completion(diet_profile_t * profile);
#endif


  /*
   * Configure a priority aggregator
   */

  /**
     This function is used to return the aggregator of a profile

     @param profile the profile we want to get the aggregator
     @return aggregator aggregator of the profile or NULL if no aggregator
     was defined for the profile
  */
  DIET_API_LIB diet_aggregator_desc_t*
  diet_profile_desc_aggregator(diet_profile_desc_t* profile);

  /**
     Function used to set the aggregator type for a specific aggregator

     @param agg an aggregator for which we want to set the type
     @param atype type to set to the aggregator

     @return error value telling whether the type setup was
     correctly realized or not:
     @arg 0 if agg is NULL (it's an error)
     @arg 0 if the aggregation type is unknown (it's an error)
     @arg 1 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_aggregator_set_type(diet_aggregator_desc_t* agg,
                           diet_aggregator_type_t atype);
  /**
     Function used to add to the aggregator in parameter a rule corresponding
     to the selection of the maximum value of a specific element
     (corresponding to the tag in parameter)

     @param agg an aggregator for which we wan to add a priority for
     the selection of the maximum of a metric
     @param tag a tag specifying which metric to study
     @return error value telling whether the type setup was correctly realized
     or not:
     @arg 0 if the aggregator is NULL
     @arg 0 if the aggregator is not a priority list
     (in fact if the aggregation method is not DIET_AGG_PRIORITY)
     @arg 0 if the function failed in adding value to priority list
     @arg 1 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_aggregator_priority_max(diet_aggregator_desc_t* agg, int tag);

  /**
     Function used to add to the aggregator in parameter a rule corresponding
     to the selection of the minimum value of a specific element
     (corresponding to the tag in parameter)

     @param agg an aggregator for which we wan to add a priority for
     the selection of the minimum of a metric
     @param tag a tag specifying which metric to study
     @return error value telling whether the type setup was correctly realized
     or not:
     @arg 0 if the aggregator is NULL
     @arg 0 if the aggregator is not a priority list
     (in fact if the aggregation method is not DIET_AGG_PRIORITY)
     @arg 0 if the function failed in adding value to priority list
     @arg 1 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_aggregator_priority_min(diet_aggregator_desc_t* agg, int tag);

  /**
     Function used to add to the aggregator in parameter a rule corresponding
     "to the selection of the maximum value of a user-specific element
     (corresponding to the tag in parameter)

     @param agg an aggregator for which we wan to add a priority for the
     selection of the maximum of a user-provided metric
     @param tag a tag specifying which metric to study
     @return error value telling whether the type setup was correctly realized
     or not:
     @arg 0 if the aggregator is NULL
     @arg 0 if the aggregator is not a priority list
     (in fact if the aggregation method is not DIET_AGG_PRIORITY)
     @arg 0 if the function failed in adding value to priority list
     @arg 1 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_aggregator_priority_maxuser(diet_aggregator_desc_t* agg, int val);

  /**
     Function used to add to the aggregator in parameter a rule corresponding
     to the selection of the minimum value of a user-specific element
     (corresponding to the tag in parameter)

     @param agg an aggregator for which we wan to add a priority for the
     selection of the minimum of a user-provided metric
     @param tag a tag specifying which metric to study
     @return error value telling whether the type setup was correctly
     realized or not:
     @arg 0 if the aggregator is NULL
     @arg 0 if the aggregator is not a priority list
     (in fact if the aggregation method is not DIET_AGG_PRIORITY)
     @arg 0 if the function failed in adding value to priority list
     @arg 1 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_aggregator_priority_minuser(diet_aggregator_desc_t* agg, int val);

  /****************************************************************************/
  /* DIET problem evaluation                                                  */
  /****************************************************************************/

  /**
     The DIET convertor function enumeration list the available conversion
     function for parameters between input and output profiles.
  */
  typedef enum {
    DIET_CVT_IDENTITY = 0, /*!< the convertor is identical
                             to the declared profile */
    DIET_CVT_VECT_SIZE, /*!< Get the size of a vector*/
    DIET_CVT_MAT_NB_ROW, /*< Get the number of row of a matrix */
    DIET_CVT_MAT_NB_COL, /*!< Get the number of column of a matrix */
    DIET_CVT_MAT_ORDER, /*!< Get the order of a matrix */
    DIET_CVT_STR_LEN, /*!< Get the length of a string */
    DIET_CVT_FILE_SIZE, /*!< Get the size of a file */
    DIET_CVT_COUNT /*!< Base value to compare with */
  } diet_convertor_function_t;

  /**
     When the server receives data from the client:
     - If in_arg_idx is a valid argument index of the sent profile, then the
     function f is applied to the diet_arg_t indexed by in_arg_idx in this
     profile parameters array.
     - Else, f is applied to arg (for instance, if in_arg_idx is -1).
     When the server sends results to the client:
     - If out_arg_idx is a valid argument index of the sent profile, then the
     function DIET_CVT_IDENTITY is applied to the diet_arg_t indexed by
     out_arg_idx in this profile parameters array.

     NB: when DIET_CVT_IDENTITY is applied several times on an IN parameter, the
     data are duplicated, so that the server gets two (or more) copies.
  */
  typedef struct {
    diet_convertor_function_t f; /*!< convertor function */
    int in_arg_idx; /*!< index of the argument in the input profile
                      to change with the function */
    diet_arg_t* arg; /*!< DIET argument */
    int out_arg_idx; /*!< index of the argument in the output profile
                       to change with the function */
  } diet_arg_convertor_t;

  /**
     The server may declare several services for only one underlying routine.
     Thus, diet_convertors are useful to translate the various declared profiles
     into the actual profile of the underlying routine, ie the profile that is
     used for the FAST benches.
     Internally, when a client requests for a declared service, the
     corresponding convertor is used to generate the actual profile:
     this allows evaluation.

     @param arg_cvt argument convertor to which we want to set the different
     elements
     @param f convertor function to set
     @param in_arg_idx argument index in the input profile
     @param arg argument
     @param out_arg_idx argument index in the output profile

     @return error value telling whether the operation has been successful
     or not:
     @arg 1 if the argument convertor was not set
     @arg 0 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_arg_cvt_set(diet_arg_convertor_t* arg_cvt, diet_convertor_function_t f,
                   int in_arg_idx, diet_arg_t* arg, int out_arg_idx);

  /**
     This function is used to convert argument without modificiations.
     It is often used with this configuration:
     @arg f == DIET_CVT_IDENTITY
     @arg out_arg_idx == in_arg_idx
     Thus we define the following macro that sets these two arguments ...
     Type:
     (int) diet_arg_cvt_short_set((diet_arg_convertor_t*), (int), (diet_arg_t *))

     @arg arg_cvt argument convertor
     @arg arg_idx index of the argument to convert
     @arg arg the argument to convert

     The returned value tells whether the operation has been successful or not:
     @arg 1 if the argument convertor was not set
     @arg 0 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
#define diet_arg_cvt_short_set(arg_cvt, arg_idx, arg)                     \
  diet_arg_cvt_set((arg_cvt), DIET_CVT_IDENTITY, (arg_idx), (arg), (arg_idx))

  /**
     Structure corresponding to a convertor
  */
  typedef struct {
    char* path; /*!< path of the newly cerated profile */
    int last_in; /*!< \f$-1\f$ + number of input data */
    int last_inout; /*!< \e last_in \f$+\f$ number of inout data */
    int last_out; /*!< \e \f$+\f$ number of out data */
    diet_arg_convertor_t* arg_convs; /*!< argument convertor to use when
                                       converting argument with the original
                                       profile */
#if defined HAVE_ALT_BATCH
    unsigned short int batch_flag; /*!< flag defining the parallel aspects:
                                     @arg if 0,
                                     select seq AND parallel tasks for
                                     the request
                                     @arg if 1, select only seq tasks
                                     @arg if 2, parallel only
                                   */
    int nbprocs; /*!< Number of processors to request */
    unsigned long walltime; /*!< Walltime to request */
#endif
  } diet_convertor_t;

  /**
     Function used to get a pointer to the nth arg convertor of a convertor
     Type:
     (diet_arg_convertor_t*) diet_arg_conv ((diet_convertor_t*), (size_t))

     @warning user should be aware that no checking of array bound is realized
  */
#define diet_arg_conv(cvt, n) &((cvt)->arg_convs[(n)])

  /**
     Function used to allocate a convertor

     @param path the name of the service corresponding to the convertor
     @param last_in last_in : \f$-1\f$ + number of input data
     @param last_inout : \e last_in \f$+\f$ number of inout data
     @param last_inout \f$+\f$ number of out data

     @return the newly allocated convertor
  */
  DIET_API_LIB diet_convertor_t*
  diet_convertor_alloc(const char* path, int last_in,
                       int last_inout, int last_out);

  /**
     Function used to free also arg field (if not NULL) of each
     diet_arg_convertor_t in arg_convs
     array.
     @param cvt the convertor to deallocate
     @return error value telling whether the deallocation was successful:
     @arg 0 if ok
     @arg 1 if all went well

     @warning Be careful to the coherence between arg_idx and arg fields !!!

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_convertor_free(diet_convertor_t* cvt);


  /***************************************************************************/
  /* DIET evaluation function prototype                                      */
  /****************************************************************************/

  /**
     DIET evaluation function prototype

     If FAST cannot evaluate the computation time of a request, for instance
     because the FAST benches have not been performed, then the service
     programmer should provide an evaluation function that uses the actual
     profile for the service to forecast the computation time.
     NB: The profile internally given to the eval function has not the value
     fields set, since evaluation takes place before data are transfered.

     @param profile profile to pass to the DIET evaluation function

     @return error value of the evaluation function
  */
  typedef int (* diet_eval_t)(diet_profile_t*, double*);


  /***************************************************************************/
  /* DIET solve function prototype                                           */
  /***************************************************************************/


  /**
     DIET solve function prototype

     @param profile profile to pass to the DIET solve function
     @return error value of the solve function
  */
  typedef int (* diet_solve_t)(diet_profile_t*);

  /***************************************************************************/
  /* DIET performance metric function prototype                              */
  /***************************************************************************/

  /**
     DIET performance metric function prototype

     @param profile profile to pass to the DIET performance metric function
     @param estimationVector estimation Vector in which values can be stored
     and from which values can be retrieved
  */
  typedef void (* diet_perfmetric_t)(diet_profile_t*, estVector_t);


  /***************************************************************************/
  /* Utils for setting (IN)OUT arguments (solve functions, after computation)*/
  /***************************************************************************/
  /**
   * These functions do not alter the data memory sizes: ie the server CANNOT
   * alter the arguments sizes, since the necessary memory space is allocated
   * by DIET, following the client specifications.
   * If value              is NULL,
   * if order              is DIET_MATRIX_ORDER_COUNT,
   * if nb_rows or nb_cols is 0,
   * if path               is NULL,
   * then the correspunding field is not modified.
   */

  /**
     Function used to set the description of a scalar

     @param data data for which we are setting the value
     @param value the value to set in the data

     @return error value telling whether the operation was successfully
     realized or not:
     @arg 1 if the function was misused (wrong type)
     @arg 1 if the function was misused (data->value is NULL)
     @arg 1 if the function was misused (wrong base type)
     @arg 0 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_scalar_desc_set(diet_data_t* data, void* value);

  /* No use of diet_vector_desc_set: size should not be altered by server */

  /**
     Function used to alter nb_r and nb_c, but the total size must remain
     the same

     @param data the data the matrix on which we are setting elements
     @param nb_r new number of row
     @param nb_c new number of column
     @param order new order for the matrix

     @return error value telling whether the operation was successfully
     realized or not:
     @arg 1 if the function was misused (the new size cannot exceed the old one)
     @arg 1 if the function was misused (wrong type)
     @arg 0 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

     @warning the new size (nb_r * nb_c) must be equal to the previous one
  */
  DIET_API_LIB int
  diet_matrix_desc_set(diet_data_t* data, size_t nb_r,
                       size_t nb_c, diet_matrix_order_t order);

  /* No use of diet_string_desc_set: length should not be altered by server */

  /**
     Function used to set the new path to the file in the data

     @param data data to modify
     @param path the new path of the data

     @return error value telling whether the operation was successfully
     realized or not:
     @arg 1 if the function was misused (wrong type)
     @arg 0 if all went well

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_file_desc_set(diet_data_t* data, char* path);

  /****************************************************************************/
  /* DIET service table API                                                   */
  /****************************************************************************/
  /**
     Function used to initialize the service table of the SeD

     @remark The service table of a SeD is unique

     @param max_size the number of services for the SeD
     @return 0 in every case
  */
  DIET_API_LIB int
  diet_service_table_init(int max_size);

  /* (cvt = NULL) is equivalent to "no conversion needed" */

  /**
     Function used to set the the perfmetric function

     @param fn performance metric function
     @return the current performance metric function
  */
  DIET_API_LIB diet_perfmetric_t
  diet_service_use_perfmetric(diet_perfmetric_t fn);

  DIET_API_LIB int
  diet_service_table_add(const diet_profile_desc_t* const profile,
                         const diet_convertor_t* const cvt,
                         diet_solve_t solve_func);

  /**
     Function to remove a service from the server.
     It also informs its parent from the removal.

     @param profile profile to remove form the the service table
     @return error value telling whether the operation was successfully
     realized or not:
     @arg -1 if the profile is NULL
     @arg -1 if the service table is not yet initialized
     @arg 1 if the server did not subscribe yet
     @arg 1 if the service reference corresponding to the profile was wrong
     @arg 1 if we are attempting to rm a service that is not in table of a
     parent agent
     @arg 1 if we are attempting to rm a service to a child which do not
     possess it

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_service_table_remove(const diet_profile_t* const profile);

  /**
     Function to remove a service from the server based on the profile
     description.
     It also informs its parent from the removal.

     @param profile profile description to remove form the the service table
     @return error value telling whether the operation was successfullly
     realized or not:
     @arg -1 if the profile is NULL
     @arg -1 if the service table is not yet initialized
     @arg 1 if the server did not subscribe yet
     @arg 1 if the service reference corresponding to the profile was wrong
     @arg 1 if we are attempting to rm a service that is not in table of a
     parent agent
     @arg 1 if we are attempting to rm a service to a child which do not
     possess it

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_service_table_remove_desc(const diet_profile_desc_t* const profile);

  /* Unused !!!
     int diet_service_table_lookup(const diet_profile_desc_t* const profile);
  */
  /**
     Function used to consult the table of the service and get back the index of
     the service of which the profile was supplied in argument

     @param profile the profile to use in order to find the sed reference
     inside the service table
     @return the index of the corresponding service or -1 if :
     @arg the profile is NULL
     @arg the service table was not yet initialized

     @todo error codes defined in DIET_grpc.h should better be used.
     ²This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_service_table_lookup_by_profile(const diet_profile_t* const profile);

  /**
     Function used to print the service table on the standard output.
  */
  DIET_API_LIB void
  diet_print_service_table();


  /****************************************************************************/
  /* DIET server call                                                         */
  /****************************************************************************/

  /* Most users should set argc to 0 and argv to NULL.
     Advanced omniORB users can set these arguments that are transmitted to
     the ORB initialization. */
 DIET_API_LIB  int
  diet_SeD(const char* config_file_name, int argc, char* argv[]);

  /****************************************************************************/
  /* DIET batch submit call                                                   */
  /****************************************************************************/
#if defined HAVE_ALT_BATCH
  /**
     Function used to submit a batch job corresponding to given @param profile service with a
     partially defined script contained in the string @param command .
     @param addon_prologue can be used for batch-dedicated (non inter-batch
     portable) script: corresponding commands are added before the termination
     of the batch prologue (e.g., QUEUE for LL).

     @param profile profile of the service to call service
     @param addon_prologue prologue to execute after the submitted job. This prologue
     is batch specific
     @param script particular script to execute (variables inseide the script will be filled automatically)
  */
  DIET_API_LIB int
  diet_submit_parallel(diet_profile_t * profile,
                       const char * addon_prologue,
                       const char * command);

  /* TODO: this will disapear when BatchSystem will implement seqFIFO,
     paralFIFO, LOADLEVELER, etc., since the definition would be
     straightforward then. */
  /**
     server status to define its type
  */
  typedef enum {
    SERIAL, /*!< serial SeD */
    BATCH, /*!< batch SeD */
    NB_SERVER_STATUS
  } diet_server_status_t;

  /**
     Function used to set the server status of the SeD

     @param status the status to set for the server

  */
  DIET_API_LIB void
  diet_set_server_status(diet_server_status_t status);

#endif



  /****************************************************************************/
  /* DIET standard estimation methods (DIET_server.cc)                        */
  /****************************************************************************/

  /**
     Function used to set a user-defined value in the estimation vector

     @param ev estimation vector where to set values
     @param userTag index in the estimation vector of the value to set
     @warning the userTag must correspond to a user defined one
     @param value the value to set

     @return error code telling whether the operation was successful or not:
     @arg -1 if the estimation vector is null
     @arg -1 if the userTag is negative

     @remark userTag should start at 0 for the first user-defined value.
     The code then translates the index to get userTag + EST_USERDEFINED.

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_est_set(estVector_t ev, int userTag, double value);

  /**
     Function used to get a user-defined value from the estimation vector

     @param ev estimation vector
     @param userTag index in the estimation vector of the value to get
     @param errVal error value to return on error

     @return the value at the userTag + EST_USERDEFINED position in the
     estimation vector.

     An error will appear if :
     @arg the estimation vector is null
     @arg the userTag is negative

     @remark userTag should start at 0 for the first user-defined value.
     The code then translates the index to get userTag + EST_USERDEFINED.

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB double
  diet_est_get(estVectorConst_t ev, int userTag, double errVal);

  /**
     Function used to get a system value from the estimation vector

     @param ev estimation vector
     @param systemTag index in the estimation vector of the value to get
     @param errVal error value to return on error

     @return the value at the systemTag position in the estimation vector.

     An error will appear if :
     @arg the estimation vector is null
     @arg the systemTag is negative
     @arg the tag is greater than EST_USERDEFINED (in that case the value to get corresponds to a user-defined one)

     @todo error codes defined in DIET_grpc.h should better be used. This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB double
  diet_est_get_system(estVectorConst_t ev, int systemTag,
                      double errVal);
  /**
     Function used to know if a user value is defined in the estimation vector

     @param ev estimation vector
     @param userTag position in the estimation vector

     @remark userTag should start at 0 for the first user-defined value.
     The code then translates the index to get userTag + EST_USERDEFINED.

     @return value telling whether the userTag is defined or not
     (an error code is returned if an error has been encountered):
     @arg 1 if the tag is defined
     @arg 0 if the tag was not defined
     @arg -1 if the estimation vector is null
     @arg -1 if the userTag is negative

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_est_defined(estVectorConst_t ev, int userTag);

  /**
     Function used to know if a a system value is defined in the estimation
     vector

     @param ev estimation vector
     @param systemTag position in the estimation vector

     @return value telling whether the the systemTag is defined or not
     (an error code is returned if an error has been encountered):
     @arg 1 if the tag is defined
     @arg 0 if the tag was not defined
     @arg -1 if the estimation vector is null
     @arg -1 if the userTag is negative

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_est_defined_system(estVectorConst_t ev, int systemTag);

  /**
     Function used to get the size of a user-defined array value in the
     estimation vector

     @param ev estimation vector
     @param userTag tag of the user-defined array

     @remark userTag should start at 0 for the first user-defined value.
     The code then translates the index to get userTag + EST_USERDEFINED.

     @return size of the array value in the estimation vector
     (an error code is returned if an error has been encountered):
     @arg The corresponding size on success
     @arg 0 if the tag does not correspond to an array or the size is 0
     @arg -1 if the estimation vector is null
     @arg -1 if the userTag is negative

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_est_array_size(estVectorConst_t ev, int userTag);

  /**
     Function used to get the size of a system array value in the estimation
     vector

     @param ev estimation vector
     @param systemTag of the array in the estimation vector

     @return size of the array value in the estimation vector
     (an error code is returned if an error has been encountered):
     @arg The corresponding size on success
     @arg 0 if the tag does not correspond to an array or the size is 0
     @arg -1 if the estimation vector is null
     @arg -1 if the userTag is negative

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_est_array_size_system(estVectorConst_t ev, int systemTag);

  /**
     Function used to set a value in a user-defined array of the estimation
     vector

     @param ev estimation vector
     @param userTag tag in the estimation vector
     @param idx index of the value to set in the array
     @param value the value to set in the array at the userTag of the
     estimation vector

     @return error code telling whether the operation was successful or not:
     @arg -1 if the estimation vector is null
     @arg -1 if the userTag is negative
     @arg -1 if index is negative

     @remark userTag should start at 0 for the first user-defined value.
     The code then translates the index to get userTag + EST_USERDEFINED.

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_est_array_set(estVector_t ev, int userTag, int idx, double value);

  /**
     Function used to get a value from an array of the estimation vector

     @param ev estimation vector
     @param userTag tag in the estimation vector
     @param idx index of the value in the array
     @param errVal error value to return if a problem

     @return value at the idx position of the array at the userTag position
     in the estimation vector or errVal if :
     @arg the estimation vector is NULL
     @arg the userTag is not defined or is negative
     @arg the idx does not corresponds to a position in the array or is negative

     @remark userTag should start at 0 for the first user-defined value.
     The code then translates the index to get userTag + EST_USERDEFINED.

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB double
  diet_est_array_get(estVectorConst_t ev, int userTag,
                     int idx, double errVal);

  /**
     Function used to get a value from an array of the estimation vector

     @param ev estimation vector
     @param systemTag tag in the estimation vector
     @param idx index of the value in the array
     @param errVal error value to return if a problem

     @return value at the idx position of the array at the systemTag position
     in the estimation vector or errVal if :
     @arg the estimation vector is NULL
     @arg the systemTag is not defined or is negative
     @arg the idx does not corresponds to a position in the array or is negative

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB double
  diet_est_array_get_system(estVectorConst_t ev, int systemTag,
                            int idx, double errVal);

  /**
     Function used to know if a value in a user-defined array is defined in
     the estimation vector

     @param ev estimation vector
     @param userTag position in the estimation vector
     @param idx index in the array

     @remark userTag should start at 0 for the first user-defined value.
     The code then translates the index to get userTag + EST_USERDEFINED.

     @return value telling whether the userTag is defined or not
     (an error code is returned if an error has been encountered):
     @arg 1 if the value at the idx position in the array at the userTag
     position is defined
     @arg 0 if the value at the idx position in the array at the userTag
     position was not defined
     @arg -1 if the estimation vector is null
     @arg -1 if the userTag value is negative
     @arg -1 if the idx value is negative

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_est_array_defined(estVectorConst_t ev, int userTag, int idx);

  /**
     Function used to know if a value in a system array is defined in the
     estimation vector

     @param ev estimation vector
     @param systemTag position in the estimation vector
     @param idx index in the array

     @return value telling whether the systemTag is defined or not
     (an error code is returned if an error has been encountered):
     @arg 1 if the value at the idx position in the array at the systemTag
     position is defined
     @arg 0 if the value at the idx position in the array at the systemTag
     position was not defined
     @arg -1 if the estimation vector is null
     @arg -1 if the systemTag value is negative
     @arg -1 if the idx value is negative

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_est_array_defined_system(estVectorConst_t ev, int systemTag, int idx);

#ifdef HAVE_ALT_BATCH
  /* These two functions shall be removed and a better mechanism found
     for example when and if CoRI is rewritten. Or clients using CoRI must
     be written in C and not in C++...
     FIXME: Do they have to be integrated to the documented API at the moment?
  */
  /**
     Function used to allocate a new estimation vector

     @remark this function shall be removed and a better mechanism found
     for example when and if CoRI is rewritten. Or clients using CoRI must
     be written in C and not in C++...
     @todo Do they have to be integrated to the documented API at the moment?

     @return new estimation vector
  */
  DIET_API_LIB estVector_t
  diet_new_estVect();

  /**
     Function used to deallocate an estimation vector

     @param perfVect the estimation vector to destroy

     @remark this function shall be removed and a better mechanism found
     for example when and if CoRI is rewritten. Or clients using CoRI must
     be written in C and not in C++...
     @todo Do they have to be integrated to the documented API at the moment?
  */
  DIET_API_LIB void
  diet_destroy_estVect(estVector_t perfVect);
#endif

  /**
     Function used to get a value from CoRI and set it inside the estimation
     vector

     @param ev estimation vector
     @param info_type type of information to get from CoRI and to set inside
     the estimation vector
     @param collector_type collector type to use (could be EST_COLL_EASY,
     EST_COLL_FAST, EST_COLL_GANGLIA, EST_COLL_NAGIOS or EST_COLL_BATCH)
     @param data some data to be used by CoRI

     @todo add some info about what data could be depending on the
     collector_type provided

     @return error value telling whether the operation was successful or not:
     @arg -1 if info_type was not EST_TCOMP, EST_FREECPU, EST_FREEMEM,
     EST_NBCPU or EST_ALLINFOS
     @arg -1 if the requested collector not implemented (Ganglia or Nagios)

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_estimate_cori(estVector_t ev, int info_type,
                     diet_est_collect_tag_t collector_type, const void *data);

  /**
     Function used to add a collector

     @param collector_type a collector type (could be EST_COLL_EASY,
     EST_COLL_FAST, EST_COLL_GANGLIA, EST_COLL_NAGIOS or EST_COLL_BATCH)
     @param data some data to pass to the collector

     @remark Ganglia and Nagios are not yet implemented

     @return 0 in every case

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_estimate_cori_add_collector(diet_est_collect_tag_t collector_type,
                                   void* data);

  /**
     Function used to display data from CoRI Easy on the standard output
  */
  DIET_API_LIB void
  diet_estimate_coriEasy_print();

  /**
     Function used to set the time since last solve in the estimation vector

     @param ev estimation vector
     @param profilePtr pointer to a profile (to know the last execution time
     of a specific service)

     @return error code specifying if the operation was sucessful or not:
     @arg 1 on success
     @arg -1 if the estimation vector was NULL

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_estimate_lastexec(estVector_t ev,
                         const diet_profile_t* const profilePtr);

  /**
     Function used to set the computation time in the estimation vector

     @param ev estimation vector
     @param value the computation time

     @return 0 in every case

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_estimate_comptime(estVector_t ev, double value);

  /**
     Function used to obtain the queue size.

     @param ev estimation vector
     @param profilePtr

     @return error code telling whether the operation was successful or not:
     @arg 0 on success
     @arg 1 if the reference on the SeD (corresponding to the profile) was
     not initialized

     @todo error codes defined in DIET_grpc.h should better be used.
 This needs to defined new error codes
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_estimate_waiting_jobs(estVector_t ev,
                             const diet_profile_t* const profilePtr);

  /*
    Function used to get the list of jobs currently waiting or running
  */
  /* Note: job vector must be deleted after usage */
  /**
     structure representing the job status
  */
  typedef enum {
    DIET_JOB_WAITING, /*!< value for a waiting job */
    DIET_JOB_RUNNING, /*!< value for a running job */
    DIET_JOB_FINISHED /*!< value for a finished job */
  } diet_job_status_t;

  /**
     Structure for a job
  */
  typedef struct {
    estVector_t           estVector; /*!< estimation vector */
    diet_job_status_t     status; /*!< status of a job */
    double                estCompTime;  /*!< estimated computation time in ms */
    double                startTime;    /*!< start time of a job in ms */
  } diet_job_t;

  /**
     Structure representing a vector jof jobs
  */
  typedef diet_job_t* jobVector_t;

  /**
     Function used to retrieve the list of jobs queued on the SeD with their
     estimation of performance (initially added to compute EFT but this is now
     done by the function diet_estimate_eft)

     @param jv vector of jobs
     @param jobNb number of jobs
     @param profilePtr pointer to the profile

     @return error value telling whether the operation was successful or not:
     @arg 0 on success
     @arg 1 if the reference on the SeD was not initialized

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_estimate_list_jobs(jobVector_t* jv, int* jobNb,
                          const diet_profile_t* const profilePtr);

  /**
     Function used to estimate the EFT for a job. The value is then setted in
     the estimated vector

     @param ev estimation vector
     @param jobEstimatedCompTime estimated computation time of a job
     @param profilePtr pointer to the profile

     @return error value telling whether the operation was successful or not:
     @arg 0 on success
     @arg 1 if the reference on the SeD was not initialized
     @arg -1 if the estimation vector is null

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_estimate_eft(estVector_t ev, double jobEstimatedCompTime,
                    const diet_profile_t* const profilePtr);

  /****************************************************************************/
  /* Inline definitions                                                       */
  /****************************************************************************/


  /**
     Util function for setting data descriptors (service construction)

     Every -1 argument implies that the corresponding field is not modified.

     @param desc data generic description
     @param type composite data type
     @param base_type base type

     @return error value telling whether the operation was successful or not:
     @arg 1 if the description was NULL
     @arg 0 on success

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  int
  diet_generic_desc_set(struct diet_data_generic* desc,
                        diet_data_type_t type,
                        diet_base_type_t base_type);

  /****************************************************************************/
  /*    get available Services on a SeD                                       */
  /****************************************************************************/

  /**
     Function used to get the names of the services available on a given SeD.

     @param services_number after the call the value contains the number of services available on the given SeD.
     @param profiles after the call the value contains the list of services' profiles available on the given SeD. Every diet_profile_desc_t* present in this array has to be deallocated using diet_profile_desc_free(), and the array profiles has to be deallocated with free().
     @param SeDName the name of the SeD.
     @return error code
  */
  DIET_API_LIB int
  diet_get_SeD_services(int *services_number,
                        diet_profile_desc_t ***profiles,
                        const char *SeDName);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DIET_SERVER_H_ */
