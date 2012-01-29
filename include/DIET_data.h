/**
* @file DIET_data.h
*
* @brief  DIET data interface
*
* @author - Philippe COMBES (Philippe.Combes@ens-lyon.fr)
*
* @section Licence
*   |LICENCE|

   \details

   \page DIETDataPage DIET data Interface

   It is important that \c DIET can manipulate data to optimize copies and
   memory allocation, to estimate data transfer and computation time, etc.
   Therefore the data must be fully described in terms of their data types and
   various attributes associated with these types.

   @section DataTypes Data types

   \c DIET defines a precise set of data types to be used to describe the
   arguments of the services (on the server side) and of the problems
   (on the client side).

   To keep \c DIET type descriptions generic, two main sets are used:
   base and composite types.

   \subsection BaseType Base types

   Base types are defined in an enum type \c diet_base_type_t and have the
   following semantics:

   \latexonly
   \begin{center}
   \footnotesize
   \begin{tabular}{|l|l|c|}
   \hline
   \textbf{Type}&\textbf{Description}&\textbf{Size in octets}\\
   \hline
   \textsf{DIET\_CHAR}     & Character                &  1\\
   \textsf{DIET\_SHORT}    & Signed short integer     &  2\\
   \textsf{DIET\_INT}      & Signed integer           &  4\\
   \textsf{DIET\_LONGINT}  & Long signed integer      &  8\\
   \textsf{DIET\_FLOAT}    & Simple precision real    &  4\\
   \textsf{DIET\_DOUBLE}   & Double precision real    &  8\\
   \hline\hline
   \textsf{DIET\_SCOMPLEX} & Simple precision complex &  8\\
   \textsf{DIET\_DCOMPLEX} & Double precision complex & 16\\
   \hline
   \end{tabular}
   \end{center}
   \endlatexonly
   \htmlonly
   <table border>
   <tr>
   <td><b> Type </b></td>
   <td><b> Description </b></td>
   <td><b> Size in octets </b></td>
   </tr>

   <tr>
   <td> DIET_CHAR </td> <td> Character </td> <td> 1 </td>
   </tr>
   <tr>
   <td> DIET_SHORT </td> <td> Signed short integer </td> <td> 1 </td>
   </tr>
   <tr>
   <td> DIET_INT </td> <td> Signed integer </td> <td> 4 </td>
   </tr>
   <tr>
   <td> DIET_LONGINT </td> <td> Long signed integer </td> <td> 8 </td>
   </tr>
   <tr>
   <td> DIET_FLOAT </td> <td> Simple precision real </td> <td> 4 </td>
   </tr>
   <tr>
   <td> DIET_DOUBLE </td> <td> Double precision real </td> <td> 8 </td>
   </tr>
   <tr>
   <td> DIET_SCOMPLEX </td> <td> Simple precision complex </td> <td> 8 </td>
   </tr>
   <tr>
   <td> DIET_DCOMPLEX </td> <td> Double precision complex </td> <td> 16 </td>
   </tr>

   </table>
   \endhtmlonly

   \subsection CompositeTypes Composite types

   Composite types are defined in an enum type \c diet_type_t:
   \latexonly
   \begin{center}
   \footnotesize
   \begin{tabular}{|l|l|}
   \hline
   \textbf{Type}&\textbf{Possible base types}\\
   \hline
   \textsf{DIET\_SCALAR} & all base types\\
   \textsf{DIET\_VECTOR} & all base types\\
   \textsf{DIET\_MATRIX} & all base types\\
   \textsf{DIET\_STRING} & \textsf{DIET\_CHAR}\\
   \textsf{DIET\_PARAMSTRING} & \textsf{DIET\_CHAR}\\
   \textsf{DIET\_FILE}   & \textsf{DIET\_CHAR}\\
   \textsf{DIET\_CONTAINER}   & all base types\\
   \hline
   \end{tabular}
   \end{center}
   \endlatexonly
   \htmlonly
   <table border>
   <tr>
   <td><b> Type </b></td>
   <td><b> Possible base type </b></td>
   </tr>

   <tr>
   <td> DIET_SCALAR </td> <td> all base types </td>
   </tr>
   <tr>
   <td> DIET_VECTOR </td> <td> all base types </td>
   </tr>
   <tr>
   <td> DIET_MATRIX </td> <td> all base types </td>
   </tr>
   <tr>
   <td> DIET_STRING </td> <td> DIET_CHAR </td>
   </tr>
   <tr>
   <td> DIET_PARAMSTRING </td> <td> DIET_CHAR </td>
   </tr>
   <tr>
   <td> DIET_FILE </td> <td> DIET_CHAR </td>
   </tr>
   <tr>
   <td> DIET_CONTAINER </td> <td> all base types </td>
   </tr>
   </table>
   \endhtmlonly

   Each of these types requires specific parameters to completely describe the
   data.

   \subsection PersistentMode Persistence mode
   Persistence mode is defined in an enum type \c diet_persistence_mode_t

   \latexonly
   \begin{center}
   \footnotesize
   \begin{tabular}{|l|l|}
   \hline
   \textbf{mode}&\textbf{Description}\\
   \hline
   \textsf{DIET\_VOLATILE} & not stored\\
   \textsf{DIET\_PERSISTENT\_RETURN} & stored on server, movable and copy back
   to client\                                                   \
   \textsf{DIET\_PERSISTENT} & stored on server and movable\\
   \textsf{DIET\_STICKY} & stored and non movable\\
   \hline\hline
   \textsf{DIET\_STICKY\_RETURN} & stored, non movable and copy back to client\\
   \hline
   \end{tabular}
   \end{center}
   \endlatexonly

   \htmlonly
   <table border>
   <tr>
   <td>Mode</td><td>Description</td>
   </tr>
   <tr>
   <td>DIET_VOLATILE</td><td>not stored</td>
   </tr>
   <tr>
   <td>DIET_PERSISTENT_RETURN</td><td>stored on server, movable and copy back
   to client</td>
   </tr>
   <tr>
   <td>DIET_PERSISTENT</td><td>stored on server and movable</td>
   </tr>
   <tr>
   <td>DIET_STICKY</td><td>stored and non movable</td>
   </tr>
   <tr>
   <td>DIET_STICKY_RETURN</td><td>stored, non movable and copy back to client
   </td>
   </tr>
   </table>
   \endhtmlonly
   @remark \c DIET_STICKY_RETURN only works with \b dagda.

   @section DataDescription Data description

   Each parameter of a client problem is manipulated by \c DIET using the
   following
   structure:
   \code
   typedef struct diet_arg_s diet_arg_t;
   struct diet_arg_s{
   diet_data_desc_t desc;
   void            *value;
   };
   typedef diet_arg_t diet_data_t;
   \endcode

   The second field is a pointer to the memory zone where the parameter
   data are stored. The first one consists of a complete \c DIET data
   description, which is better described by a figure than with C code,
   since it can be set and accessed through API functions.

   @section DataManagement Data management

   \subsection DataIdentifier Data identifier

   The data identifier is generated by the MA. The data identifier is a
   string field that contains the MA name, the number of the session plus
   the number of the data in the problem (incremental) plus the string
   ``id''.  This is the \c id field of the \c diet_data_desc_t structure.

   \code
   typedef struct {
   char* id;
   diet_persistence_mode_t  mode;
   ....
   } diet_data_desc_t;
   \endcode

   For example, \b id.MA1.1.1 will identify the first data
   in the first session submitted on the Master Agent \b MA1.


   @remark the field ``id'' of the identifier will be next replaced by a
   client identifier. This is not implemented yet.

   \subsection DataFile Data file

   The name of the file is generated by a Master Agent. It is created
   during the \c diet_initialize() call. The name of the file is
   the aggregation of the string ID_FILE plus the name of the MA plus
   the number of the session.

   A file is created only when there are some persistent data in the
   session.

   For example, \b ID_FILE.MA1.1 means the identifiers
   of the persistent data stored are in the file corresponding to the
   first session in the Master Agent \b MA1.

   The file is stored in the \c /tmp directory.

   @remark for the moment, when a data item is erased from the platform, the
   file isn't updated.

   @section ManipulatingDIETStructures Manipulating \c DIET structures

   The user will notice that the API to the \c DIET data structures consists of
   modifier and accessor functions only: no allocation function is required,
   since \c diet_profile_alloc allocates all
   necessary memory for all argument \b descriptions. This avoids the
   temptation for the user to allocate the memory for these data structures
   twice (would lead to \c DIET errors while reading profile arguments).

   Moreover, the user should know that arguments of the \b *_set functions
   that are passed by pointers are \b not copied, in order to save memory.
   This is true for the \e value arguments, but also for the \e path in
   \c diet_file_set. Thus, the user keeps ownership of the memory zones
   pointed at by these pointers, and he/she must be very careful not to alter
   it during a call to \c DIET.

   \subsection SetFunctions Set functions

   \code
   // On the server side, these functions should not be used on arguments,
   but only
   // on convertors (see section 5.5).
   // If mode                                is DIET_PERSISTENCE_MODE_COUNT,
   // or if base_type                        is DIET_BASE_TYPE_COUNT,
   // or if order                            is DIET_MATRIX_ORDER_COUNT,
   // or if size, nb_rows, nb_cols or length is 0,
   // or if path                             is NULL,
   // then the corresponding field is not modified

   int
   diet_scalar_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
   diet_base_type_t base_type);
   int
   diet_vector_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
   diet_base_type_t base_type, size_t size);

   // Matrices can be stored by rows or by columns
   typedef enum {
   DIET_COL_MAJOR = 0,
   DIET_ROW_MAJOR,
   DIET_MATRIX_ORDER_COUNT
   } diet_matrix_order_t;

   int
   diet_matrix_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
   diet_base_type_t base_type,
   size_t nb_rows, size_t nb_cols, diet_matrix_order_t order);
   int
   diet_string_set(diet_arg_t* arg, char* value, diet_persistence_mode_t mode);

   // The file size is computed and stocked in a field of arg
   // ! Warning ! The path is not duplicated !!!
   int
   diet_file_set(diet_arg_t* arg, char* path, diet_persistence_mode_t mode);
   \endcode

   \subsection AccessFunctions Access functions

   \code
   // A NULL pointer is not an error (except for arg): it is simply IGNORED.
   // For instance, diet_scalar_get(arg, &value, NULL),
   // will only set the value to the value field of the (*arg) structure.
   //
   // NB: these are macros that let the user not worry about casting (int **)
   // or (double **) etc. into (void **).

   //
   // Type: int diet_scalar_get((diet_arg_t *), (void *),
   //                           (diet_persistence_mode_t *))

   #define diet_scalar_get(arg, value, mode) \
   _scalar_get(arg, (void *)value, mode)

   // Type: int diet_vector_get((diet_arg_t *), (void **),
   //                           (diet_persistence_mode_t *), (size_t *))

   #define diet_vector_get(arg, value, mode, size) \
   _vector_get(arg, (void **)value, mode, size)

   // Type: int diet_matrix_get((diet_arg_t *), (void **),
   //                           (diet_persistence_mode_t *),
   //                           (size_t *), (size_t *), (diet_matrix_order_t *))

   #define diet_matrix_get(arg, value, mode, nb_rows, nb_cols, order) \
   _matrix_get(arg, (void **)value, mode, nb_rows, nb_cols, order)

   // Type: int diet_string_get((diet_arg_t *), (char **),
   //                           (diet_persistence_mode_t *))

   #define diet_string_get(arg, value, mode) \
   _string_get(arg, (char **)value, mode)

   // Type: int diet_file_get((diet_arg_t *), (char **),
   //                         (diet_persistence_mode_t *), (size_t *))

   #define diet_file_get(arg, path, mode, size) \
   _file_get(arg, (char **)path, mode, size)
   \endcode

   @section DataManagementFunctions Data Management functions


   @arg The \b store_id method is used to store the identifier of persistent
   data. It also accepts a description of the data stored.
   This method has to be called after the \c diet_call() so that the identifier
   exists.
   \code
   store_id(char* argID, char *msg);
   \endcode

   @arg The \c diet_use_data method allows the client to use a data item that
   is already stored in the platform.
   \code
   diet_use_data(diet_arg_t* arg, char* argID);
   \endcode

   This function replaces the set functions.

   @remark a mechanism for data identifier publication has not been
   implemented yet. So, exchanges of identifiers between end-users that
   want to share data must be done explicitly.

   @remark The \c diet_free_persistent_data method allows the
   client to remove a persistent data item from the platform.
   \code
   diet_free_persistent_data(char *argID);
   \endcode

   \code
   // Add handler argID and text message msg in the identifier file

   void
   store_id(char* argID, char* msg);


   // sets only identifier : data is present inside the platform

   void
   diet_use_data(diet_arg_t* arg, char* argID);

   // Free persistent data identified by argID

   int
   diet_free_persistent_data(char* argID);
   \endcode

   \subsection FreeFunctions Free functions

   The amount of data  pointed at by value fields should be freed through a
   \c DIET
   API function:

   \code
   // Free the amount of data pointed at by the value field of an argument.
   // This should be used ONLY for VOLATILE data,
   //    - on the server for IN arguments that will no longer be used
   //    - on the client for OUT arguments, after the problem has been solved,
   //      when they will no longer be used.
   // NB: for files, this function removes the file and frees the path (since
   //     it has been dynamically allocated by DIET in both cases)

   int
   diet_free_data(diet_arg_t* arg);

   \endcode

   @section ProblemDescription Problem description


   For \c DIET to match the client problem with a service, servers and clients
   must ``speak the same language'', \e ie they must use the same problem
   description. A unified way to describe problems is to use a name and define
   its profile with the type \c diet_profile_t:
   \code
   typedef struct {
   char*       pb_name;
   int         last_in, last_inout, last_out;
   diet_arg_t *parameters;
   } diet_profile_t;
   \endcode

   The field \e parameters consists of a \c diet_arg_t array of size
   \f$last\_out + 1\f$. Arguments can be
   @arg \b IN: The data are sent to the server. The memory is allocated by the
   user.
   @arg \b INOUT: The data are allocated by the user as for the IN
   arguments, then sent to the server and brought back into the same memory
   zone after the computation has completed, without any copy.
   Thus freeing this memory at the client while the computation is performed
   on the server would result in a segmentation fault when the data are
   brought back onto the client.
   @arg \b OUT: The data are created on the server and brought back into a
   newly allocated zone on the client. This allocation is performed by
   \c DIET. After the call has returned, the user can find the result in
   the zone pointed at by the \e value field. Of course, \c DIET
   cannot guess how long the user will need these data, so the
   user must free the memory him/herself with \e diet_free_data.

   The fields \e last_in, \e last_inout and \e last_out of the
   \c diet_profile_t structure respectively point at the indexes in the
   \e parameters array of the last IN, INOUT and OUT arguments.

   Functions to create and destroy such profiles are defined with the
   prototypes below:
   \code
   diet_profile_t *diet_profile_alloc(char* pb_name, int last_in,
   int last_inout, int last_out);
   int diet_profile_free(diet_profile_t *profile);
   \endcode

   The values of \e last_in, \e ast_inout and \e last_out
   are respectively:

   @arg \e last_in : \f$-1\f$ + number of input data.
   @arg \e last_inout : \e last_in \f$+\f$ number of inout data.
   @arg \e last_out : \e last_inout \f$+\f$ number of out data.

   \mainpage

   Here is the documentation of the DIET API.
   It is composed of:
   @arg DIET_client.h : the DIET Client API allowing users to access the DIET
   middleware and the services made available through the DIET middleware
   @arg DIET_server.h: the DIET Server API allowing programmers to make
   applications as services available for clients through the DIET middleware
   @arg DIET_admin.h: the DIET administration API allowing programmers
   dynamically change the shape of the DIET hierarchy.
   @arg DIET_data.h: the Data API allowing the users and the server
   programmers to set and get data to/from clients and servers programs.
   @arg DIET_Dagda.h: the DAGDA API allowing the programmers to directly
   manage their data.
   @arg DIET_grpc.h: the GRPC API compliant with the GridRPC paradigm of
   defined by the Grid RPC working group of the Open Grid Forum.
*/


#ifndef _DIET_DATA_H_
#define _DIET_DATA_H_
#ifdef __WIN32__
   #define DIET_API_LIB __declspec(dllexport)
#else /* __WIN32__ */
   #define DIET_API_LIB
#endif /* __WIN32__ */
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /**
     @todo don't know was it is ...
  */
#define EST_INVALI2D 1;

  /***************************************************************************/
  /* Aggregator types                                                        */
  /***************************************************************************/

  /**
     Enumeration used to specify whether the provided aggregator is the base
     one, the priority one (using default aggregation macros, or a user
     provided one.

     @arg The aggregator defined on the SeD level must be used with
     DIET_AGG_PRIORITY
     @arg The aggregator defined on the Agent level must be used with
     DIET_AGG_USER
  */
  typedef enum {
    DIET_AGG_DEFAULT = 0 /*!< default aggregator type*/,
     /*!< priority aggregator to use with SeD level aggregators */
    DIET_AGG_PRIORITY
    /* New : The aggregator to use DIET agents scheduler dynamic loading. */
#ifdef USERSCHED
     /*!< The aggregator to use DIET agents scheduler dynamic loading */
    , DIET_AGG_USER
#endif
  } diet_aggregator_type_t;

  /***************************************************************************/
  /* Base and data types                                                     */
  /***************************************************************************/

  /**
     Enumeration defining the base \c DIET types
  */
  typedef enum {
    DIET_CHAR = 0, /*!< character base type */
    DIET_SHORT, /*!< signed short integer base type */
    DIET_INT, /*!< signed integer base type */
    DIET_LONGINT, /*!< signed long integer base type */
    DIET_FLOAT, /*!< simple precision real base type */
    DIET_DOUBLE, /*!< double precision real base type */
    DIET_SCOMPLEX, /*!< simple precision complex base type */
    DIET_DCOMPLEX, /*!< double precision complex base type */
    DIET_BASE_TYPE_COUNT /*!< \c DIET base types count */
  } diet_base_type_t;

  /**
     Enumeration defining the composite \c DIET types
  */
  typedef enum {
    DIET_SCALAR = 0, /*!< scalar value (all base types)*/
    DIET_VECTOR, /*!< vector of values (all base types)*/
    DIET_MATRIX, /*!< matrix of values (all base types)*/
    DIET_STRING, /*!< type representing a string (array of char)*/
    /*!< type representing a string passed in parameter (array of char)*/
    DIET_PARAMSTRING,
     /*!< type representing a file (the corresponding base type is char)*/
    DIET_FILE,
    DIET_CONTAINER, /*!< type representing a container (all base types)*/
    DIET_DATA_TYPE_COUNT /*!< \c DIET composite types count */
  } diet_data_type_t;


  /***************************************************************************/
  /* Various persistence modes for data :                                    */
  /*   - DIET_VOLATILE:                                                      */
  /*        No persistency at all.                                           */
  /*   - DIET_PERSISTENT_RETURN: (valid for INOUT and OUT arguments only)    */
  /*        Data are saved on the server and a copy is sent back to the      */
  /*        client after the computation is complete.                        */
  /*   - DIET_PERSISTENT:                                                    */
  /*        Data are saved on the server and nothing is brought back to the  */
  /*        client.                                                          */
  /*   - DIET_STICKY:                                                        */
  /*        Data are saved on the server, they cannot been moved from there  */
  /*        to another server, and thus cannot be sent back to the client.   */
  /***************************************************************************/

  /**
     Enumeration representing the persistent modes available within \c DIET
  */
  typedef enum {
    DIET_VOLATILE = 0, /*!< No persistency at all. */
    /*!< (valid for INOUT and OUT arguments only) Data are saved on the server
      and a copy is sent back to the client after the computation is
      complete. */
    DIET_PERSISTENT_RETURN,
    /*!< Data are saved on the server and nothing is brought back to the
      client. */
    DIET_PERSISTENT,
    /*!< Data are saved on the server, they cannot been moved from there
      to another server, and thus cannot be sent back to the client. */
    DIET_STICKY,
    /*!< (valid only when DAGDA activated) The data is stored, non movable
      and copy back to client */
    DIET_STICKY_RETURN,
    DIET_PERSISTENCE_MODE_COUNT /*!< \c DIET persistence mode count */
  } diet_persistence_mode_t;


  /***************************************************************************/
  /* Argument type - a structure with two fields:                            */
  /*  desc  : a descriptor of the argument                                   */
  /*  value : a (void*) pointer to the memory zone where the data are stored.*/
  /***************************************************************************/

  /**
     Argument type - a structure with two fields:

     @arg desc  : a descriptor of the argument
     @arg value : a (void*) pointer to the memory zone where the data are
     stored.
  */
  typedef struct diet_arg_s diet_arg_t;

  /*!
    This function returns true if arg persistence mode is sticky or persistent.
    Type: int diet_is_persistent ((diet_arg_t))

    @warning no error checking is realized concerning the input argument
    (whether it is null,
    or desc is null or the mode is correctly defined)
  */
#define diet_is_persistent(arg)                                         \
  (((arg).desc.mode > DIET_VOLATILE) && ((arg).desc.mode <= DIET_STICKY))

  /***************************************************************************/
  /* Profile descriptor                                                      */
  /***************************************************************************/

  /**
     \struct diet_profile_t DIET_data.h
     This structure corresponds to the descriptor of a profile
  */
  typedef struct {
    /*!< pb_name corresponds to the name of the service to be called */
    char* pb_name;
    int last_in; /*!< \f$-1\f$ + number of input data */
    int last_inout; /*!< \e last_in \f$+\f$ number of inout data */
    int last_out; /*!< \e last_inout \f$+\f$ number of out data */
    diet_arg_t* parameters; /*!< an array of parameters for the service */
    const void* SeDPtr; /*!< pointer to SeD object, to be used in
                          performance estimation and for batch submission */
    int dietReqID; /*!< request ID of the service call */
#if defined HAVE_ALT_BATCH
    /* if 0, select seq AND parallel tasks for the request
       if 1, select only seq tasks
       if 2, parallel only */
    unsigned short int parallel_flag; /*!< flag defining the parallel aspects:
                                         @arg if 0, select seq AND parallel
                                         tasks for the request
                                         @arg if 1, select only seq tasks
                                         @arg if 2, parallel only
                                       */
    /*!< number of processors to be requested for the parallel call */
    int nbprocs;
    /*!< number of processes to be executed by the parallel call */
    int nbprocess;
    /*!< walltime in seconds of the parallel call to realize */
    unsigned long walltime;
#endif
  } diet_profile_t;

  /**
     Function used to allocate a DIET profile with memory space for its
     arguments.

     Once the profile is allocated, please use set functions on each parameter.
     For example, the nth argument is a matrix:
     \code
     diet_matrix_set(diet_parameter(profile, n), mode, value, btype,
     nb_r, nb_c, order);
     \endcode

     @remark mode is the persistence mode of the parameter.
     Since a profile will not be freed until profile_free is called, it is
     possible to refer to each parameter for data handles

     @param pb_name is deep-copied.
     @param last_in : \f$-1\f$ + number of input data. If no IN argument,
     please give -1 for last_in.
     @param last_inout : \e last_in \f$+\f$ number of inout data.
     If no INOUT argument, please give last_in for last_inout.
     @param last_out : \e last_inout \f$+\f$ number of out data.
     If no OUT argument, please give last_inout for last_out.
     @return allocated profile
  */
  DIET_API_LIB diet_profile_t*
  diet_profile_alloc(const char* pb_name, int last_in,
                     int last_inout, int last_out);

  /**
     Function used to free the profile and the inner data

     @param profile the profile to deallocate
     @return 0 in every case

     @todo is it a normal case to always return 0 (in that case we'd bether
     return void) or could something else be returned (like a diet_error_t)

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_profile_free(diet_profile_t* profile);

#if defined HAVE_ALT_BATCH

  /**
     Function used to set the parallel flag of the profile to tell that the
     call must be parallel

     @param profile the profile to be modified
     @return 0 in every case

     @todo is it a normal case to always return 0 (in that case we'd bether
     return void) or could something else be returned (lire a diet_error_t)

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_profile_set_parallel(diet_profile_t * profile);

  /**
     Function used to set the parallel flag of the profile to tell that the
     call must be sequential

     @param profile the profile to be modified
     @return 0 in every case

     @todo is it a normal case to always return 0 (in that case we'd bether
     return void) or could something else be returned (lire a diet_error_t)

     @todo error codes defined in DIET_grpc.h should better be used.
     This needs to defined new error codes
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_profile_set_sequential(diet_profile_t * profile);

  /**
     Function used to set the number of processors needed for the execution of
     the selected service in the profile.

     @param profile the profile to be modified
     @param nbprocs the number of processors needed to execute the service
     @return Error value telling whether the number of processors has be
     correctly set
     @arg 0 if nbprocs > 0
     @arg 1 otherwise

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_profile_set_nbprocs(diet_profile_t * profile, int nbprocs);
#endif

  /**
     Function used to get a pointer on the nth parameter of a profile

     Here is the signature of the function: (diet_arg_t*) diet_parameter((diet_profile_t*), (int))

     @param pt_profile profile of which we need the nth parameter
     @param n index of the requested parameter

     @return corresponding nth parameter

     @todo no error checking is realized: we do not check if the requested
     parameter is not out of the parameters array bounds*/
#define diet_parameter(pt_profile, n) &((pt_profile)->parameters[(n)])


  /****************************************************************************/
  /* Utils functions for setting parameters of a problem description          */
  /****************************************************************************/

  /*
   * On the server side, these functions should not be used on arguments, but only
   * on convertors.
   * If mode                             is DIET_PERSISTENCE_MODE_COUNT,
   * if base_type                        is DIET_BASE_TYPE_COUNT,
   * if order                            is DIET_MATRIX_ORDER_COUNT,
   * if size, nb_rows, nb_cols or length is 0,
   * if path                             is NULL,
   * then the correspunding field is not modified.
   */

  /**
     Function used to set a parameter in the profile for a scalar value

     @param arg argument to modify
     @param value scalar to set in the corresponding argument of the profile
     @param mode persistence mode
     @param base_type base type of the parameter

     @return error value telling if the alteration has been correctly realized
     @arg 0 if the alteration was successful
     @arg 1 otherwise : mainly if the description is NULL or not defined
     correctly

     @remark if the persistence mode is DIET_PERSISTENCE_MODE_COUNT,
     the original mode is not modified
     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_scalar_set(diet_arg_t* arg, void* value,
                  diet_persistence_mode_t mode, diet_base_type_t base_type);
  /**
     Function used to set a parameter in the profile for a vector value

     @param arg argument to modify
     @param value vector to set in the corresponding argument of the profile
     @param mode persistence mode
     @param base_type base type of the vector elements
     @param size the size of the vector

     @return error value telling if the alteration of the profile has been
     correctly realized
     @arg 0 if the alteration was successful
     @arg 1 otherwise : mainly if the description is NULL or not defined
     correctly

     @remark if the persistence mode is DIET_PERSISTENCE_MODE_COUNT,
     the original mode is not modified
     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_vector_set(diet_arg_t* arg, void* value,
                  diet_persistence_mode_t mode, diet_base_type_t base_type,
                  size_t size);

  /**
     Matrices can be stored by rows or by columns.
  */
  typedef enum {
    DIET_COL_MAJOR = 0, /*!< the matrix is stored column wise */
    DIET_ROW_MAJOR, /*!< The matrix is stored row wise */
    DIET_MATRIX_ORDER_COUNT /*!< matrix order count */
  } diet_matrix_order_t;

  /**
     Function used to set a parameter in the profile for a matrix value

     @param arg argument to modify
     @param value vector to set in the corresponding argument of the profile
     @param mode persistence mode
     @param base_type base type of the matrix elements
     @param nb_rows the number of rows of the matrix
     @param nb_cols the number of columns of the matrix
     @param order order of the matrix

     @sa diet_matrix_order_t for information on matrix orders

     @return error value telling if the alteration of the profile has been
     correctly realized
     @arg 0 if the alteration was successful
     @arg 1 otherwise : mainly if the description is NULL or not defined
     correctly

     @remark if the persistence mode is DIET_PERSISTENCE_MODE_COUNT,
     the original mode is not modified
     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_matrix_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
                  diet_base_type_t base_type, size_t nb_rows,
                  size_t nb_cols, diet_matrix_order_t order);

  /**
     Function used to set a parameter in the profile for a string value

     @param arg argument to modify
     @param value string to set in the corresponding argument of the profile
     @param mode persistence mode

     @return error value telling if the alteration of the profile has been
     correctly realized
     @arg 0 if the alteration was successful
     @arg 1 otherwise : mainly if the description is NULL or not defined
     correctly

     @remark if the persistence mode is DIET_PERSISTENCE_MODE_COUNT,
     the original mode is not modified
     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_string_set(diet_arg_t* arg, char* value, diet_persistence_mode_t mode);

  /**
     Function used to set a parameter in the profile for a parameter string
     value.
     The difference with a classical string is based on the fact that this
     paramstring is
     sent with the request and not only for the resolution for the service call.
     The paramstring can be used to send information to performance metrics
 or agent schedulers.

     @param arg argument to modify
     @param value string to set in the corresponding argument of the profile
     @param mode persistence mode

     @return error value telling if the alteration of the profile has been
     correctly realized
     @arg 0 if the alteration was successful
     @arg 1 otherwise : mainly if the description is NULL or not defined
     correctly

     @remark if the persistence mode is DIET_PERSISTENCE_MODE_COUNT,
     the original mode is not modified
     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_paramstring_set(diet_arg_t* arg, char* value,
                       diet_persistence_mode_t mode);

  /**
     Function used to set a parameter in the profile for a file value.
     This function computes the file size.

     @param arg argument to modify
     @param path file to set in the corresponding argument of the profile
     @param mode persistence mode
     @return the following error values could be returned:
     @arg 0 if the alteration was successful
     @arg 1 otherwise : mainly if the description is NULL or not defined
     correctly
     @arg -1 if the underlying stat() call failed. The corresponding error
     (retrieve with errno) could then be the following:
     @arg EACCES Search permission is denied for one of the directories in
     the path prefix of path. (See also path_resolution(2).)
     @arg EBADF filedes is bad.
     @arg EFAULT Bad address.
     @arg ELOOP Too many symbolic links encountered while traversing the path.
     @arg ENAMETOOLONG File name too long.
     @arg ENOENT A component of the path path does not exist, or the path is
     an empty string.
     @arg ENOMEM Out of memory (i.e. kernel memory).
     @arg ENOTDIR A component of the path is not a directory.

     @warning The path is not duplicated
     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_file_set(diet_arg_t* arg, const char* path,
                diet_persistence_mode_t mode);

  /**
     Function used to set a parameter in the profile for a container value.

     @param arg argument to modify
     @param mode persistence mode

     @return error value telling if the alteration of the profile has been
     correctly realized
     @arg 0 if the alteration was successful
     @arg 1 otherwise : mainly if the description is NULL or not defined
     correctly

     @remark if the persistence mode is DIET_PERSISTENCE_MODE_COUNT, the
     original mode is not modified
     @todo why is diet_container_set available outside a dagda ifdef?
     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_container_set(diet_arg_t* arg, diet_persistence_mode_t mode);

  /**
     Function used to set only identifier : data is present inside the platform

     @param arg argument to modify
     @param id id of the data to set in the profile for the corresponding argument
  */
  DIET_API_LIB void
  diet_use_data(diet_arg_t* arg, const char* id);

  /****************************************************************************/
  /* Utils for getting argument descriptions and values                       */
  /*    (parameter extraction on server in solve functions)                   */
  /*    (parameter extraction on client at results return)                    */
  /****************************************************************************/

  /*
   * A NULL pointer is not an error (except for arg): it is simply IGNORED.
   * For instance,
   *   diet_scalar_get(arg, &value, NULL),
   * will only set the value to the value field of the (*arg) structure.
   *
   * NB: these are macros that let the user not worry about casting its (int**)
   * or (double**) etc. into (void**).
   */

  /**
     Function used to get a scalar value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode

     Type: int diet_scalar_get((diet_arg_t*), (void**), (diet_persistence_mode_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).
  */
#define diet_scalar_get(arg, value, mode)       \
  _scalar_get(arg, (void**)value, mode)

  /**
     Function used to get a vector value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode
     @arg size the size of the vector to get

     Type: int diet_vector_get((diet_arg_t*), (void**),(diet_persistence_mode_t*), (size_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
#define diet_vector_get(arg, value, mode, size) \
  _vector_get(arg, (void**)value, mode, size)

  /**
     Function used to get a matrix value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode
     @arg nb_rows the number of rows of the matrix
     @arg nb_cols the number of columns of the matrix
     @arg order the matrix order

     Type: int diet_matrix_get((diet_arg_t*), (void**), (diet_persistence_mode_t*), (size_t*), (size_t*), (diet_matrix_order_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
#define diet_matrix_get(arg, value, mode, nb_rows, nb_cols, order)      \
  _matrix_get(arg, (void**)value, mode, nb_rows, nb_cols, order)

  /**
     Function used to get a matrix value with an id.

     @arg arg argument from which we want to get a value
     @arg id id of the matrix
     @arg value the value where to store the element
     @arg mode persistence mode
     @arg nb_rows the number of rows of the matrix
     @arg nb_cols the number of columns of the matrix
     @arg order the matrix order

     Type: int diet_matrix_get((diet_arg_t*), (char*), (void**), (diet_persistence_mode_t*), (size_t*), (size_t*), (diet_matrix_order_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

     @warning _matrix_get_with_id() is never defined in the code !

  */
#define diet_matrix_get_with_id(arg, id, value, mode, nb_rows, nb_cols, order) \
  _matrix_get_with_id(arg, id, (void**)value, mode, nb_rows, nb_cols, order)

  /**
     Function used to get a string value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode

     Type: int diet_string_get((diet_arg_t*), (char**), (diet_persistence_mode_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
#define diet_string_get(arg, value, mode)       \
  _string_get(arg, (char**)value, mode)

  /**
     Function used to get a paramstring value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode

     Type: int diet_paramstring_get((diet_arg_t*), (char**), (diet_persistence_mode_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
#define diet_paramstring_get(arg, value, mode)  \
  _paramstring_get(arg, (char**) value, mode)

  /**
     Function used to get a file value.

     @arg arg argument from which we want to get a value
     @arg mode persistence mode
     @arg size the size ofthe file to get
     @arg path the path where to store the element

     Type: int diet_file_get((diet_arg_t*), (char**), (diet_persistence_mode_t*), (size_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
#define diet_file_get(arg, path, mode, size)    \
  _file_get(arg, (char**)path, mode, size)

  /*
    type-specific data descriptor access functions
  */

  /**
     Type of a scalar description
  */
  typedef const struct diet_scalar_specific* diet_scalar_desc_t;
  /**
     Type of a vector description
  */
  typedef const struct diet_vector_specific* diet_vector_desc_t;
  /**
     Type of a matrix description
  */
  typedef const struct diet_matrix_specific* diet_matrix_desc_t;
  /**
     Type of a string description
  */
  typedef const struct diet_string_specific* diet_string_desc_t;
  /**
     Type of a paramstring description
  */
  typedef const struct diet_paramstring_specific* diet_paramstring_desc_t;
  /**
     Type of a file decription
  */
  typedef const struct diet_file_specific* diet_file_desc_t;
  /**
     Type of a container description

     @todo is it normal that a type specific to Dagda is not located inside
     ifdefs?
  */
  typedef const struct diet_container_specific* diet_container_desc_t;

  /**
     Function used to retrieve a description of a scalar value from an argument

     @param arg arugment from which we want to retrieve the scalar description
     @return scalar description or \c NULL if the underlying composite type
     is not the good one
  */
  DIET_API_LIB diet_scalar_desc_t
  diet_scalar_get_desc(diet_arg_t* arg);

  /**
     Function used to retrieve a description of a vector value from an argument

     @param arg arugment from which we want to retrieve the vector description
     @return vector description or \c NULL if the underlying composite type is
     not the good one
  */
  DIET_API_LIB diet_vector_desc_t
  diet_vector_get_desc(diet_arg_t* arg);

  /**
     Function used to retrieve a description of a matrix value from an argument

     @param arg arugment from which we want to retrieve the matrix description
     @return matrix description or \c NULL if the underlying type is not the
     good one
  */
  DIET_API_LIB diet_matrix_desc_t
  diet_matrix_get_desc(diet_arg_t* arg);

  /**
     Function used to retrieve a description of a string value from an argument

     @param arg arugment from which we want to retrieve the string description
     @return string description or \c NULL if the underlying type is not the
     good one
  */
  DIET_API_LIB diet_string_desc_t
  diet_string_get_desc(diet_arg_t* arg);

  /**
     Function used to retrieve a description of a paramstring value from
     an argument

     @param arg arugment from which we want to retrieve the paramstring
     description
     @return paramstring description or \c NULL if the underlying type is
     not the good one
  */
  DIET_API_LIB diet_paramstring_desc_t
  diet_paramstring_get_desc(diet_arg_t* arg);

  /**
     Function used to retrieve a description of a file value from an argument

     @param arg arugment from which we want to retrieve the file description
     @return file description or \c NULL if the underlying type is not the
     good one
  */
  DIET_API_LIB diet_file_desc_t
  diet_file_get_desc(diet_arg_t* arg);

  /*
    Free the amount of data pointed at by the value field of an argument.
    This should be used ONLY for VOLATILE data,
    - on the server for IN arguments that will no longer be used
    - on the client for OUT arguments, after the problem has been solved,
    when they will no longer be used.
    NB: for files, this function removes the file and frees the path (since
    it has been dynamically allocated by DIET in both cases)
  */

  /**
     This function is used to free the amount of data pointed at by the value field of an argument.
     This should be used ONLY for VOLATILE data,
     @arg on the server for IN arguments that will no longer be used
     @arg on the client for OUT arguments, after the problem has been solved,
     when they will no longer be used.
     @remark for files, this function removes the file and frees the path (since
     it has been dynamically allocated by DIET in both cases)

     @param arg argument we want to free the data
     @return error value telling whether the data deallocation was successful or not:
     @arg 3 if we were trying to free persistent data
     @arg 2 if something went wrong when deallocating a file (when the data is a file). A error message explaining the error will be printed.
     @arg 1 if the path of the file to deallocate was NULL, or if the value of a scalar/vector/string/paramstring/matrix was NULL
     @arg 0 otherwise

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes
  */
  DIET_API_LIB int
  diet_free_data(diet_arg_t* arg);

  /****************************************************************************/
  /* The following part is useless for users, but may be read for more        */
  /* information on data structures.                                          */
  /****************************************************************************/


  /*
   * Utils for getting argument descriptions and values
   *    (the effective get function)
   */

  /**
     \internal
     Function used to get a scalar value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode

     Type: int diet_scalar_get((diet_arg_t*), (void**), (diet_persistence_mode_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
  DIET_API_LIB int
  _scalar_get(diet_arg_t* arg, void** value, diet_persistence_mode_t* mode);

  /**
     \internal
     Function used to get a vector value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode
     @arg size the size of the vector to get

     Type: int diet_vector_get((diet_arg_t*), (void**),(diet_persistence_mode_t*), (size_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
  DIET_API_LIB int
  _vector_get(diet_arg_t* arg, void** value, diet_persistence_mode_t* mode,
              size_t* size);

  /**
     \internal

     Function used to get a matrix value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode
     @arg nb_rows the number of rows of the matrix
     @arg nb_cols the number of columns of the matrix
     @arg order the matrix order

     Type: int diet_matrix_get((diet_arg_t*), (void**), (diet_persistence_mode_t*), (size_t*), (size_t*), (diet_matrix_order_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
  DIET_API_LIB int
  _matrix_get(diet_arg_t* arg, void** value, diet_persistence_mode_t* mode,
              size_t* nb_rows, size_t *nb_cols, diet_matrix_order_t* order);

  /**
     \internal
     Function used to get a string value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode

     Type: int diet_string_get((diet_arg_t*), (char**), (diet_persistence_mode_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
  DIET_API_LIB int
  _string_get(diet_arg_t* arg, char** value, diet_persistence_mode_t* mode);

  /**
     \internal
     Function used to get a paramstring value.

     @arg arg argument from which we want to get a value
     @arg value the value where to store the element
     @arg mode persistence mode

     Type: int diet_paramstring_get((diet_arg_t*), (char**), (diet_persistence_mode_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc.,
     depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg):
     it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its
     \c (int**) or \c (double**) etc. into \c (void**).
  */
  DIET_API_LIB int
  _paramstring_get(diet_arg_t* arg, char** value,
                   diet_persistence_mode_t* mode);

  /**
     \internal
     Function used to get a file value.

     @arg arg argument from which we want to get a value
     @arg mode persistence mode
     @arg size the size ofthe file to get
     @arg path the path where to store the element

     Type: int diet_file_get((diet_arg_t*), (char**), (diet_persistence_mode_t*), (size_t*))

     @return error value returned by the macro:
     @arg 0 if the get function is successful
     @arg 1 if base_type was not set properly or the argument is null

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @remark \c (void**) means \c (int**), \c (double**), \c (float**), etc., depending on the
     base C type of users's data.
     @remark A \c NULL pointer is not an error (except for arg): it is simply IGNORED.
     For instance,
     \code
     diet_scalar_get(arg, &value, NULL)
     \endcode

     will only set the value to the value field of the \c (*arg) structure.

     @remark these are macros that let the user not worry about casting its \c (int**)
     or \c (double**) etc. into \c (void**).

  */
  DIET_API_LIB int
  _file_get(diet_arg_t* arg, char** path, diet_persistence_mode_t* mode,
            size_t* size);




  /*
   * Data descriptions
   * The user can read this to understand how data structures are built
   * internally, but all functions and macros defined above should be
   * sufficient.
   */

  /*----[ scalar - specific ]-------------------------------------------------*/
  /**
     Structure representing the specific elements of a scalar
  */
  struct diet_scalar_specific {
    void* value; /*!< scalar value itself */
  };

  /*----[ vector - specific ]-------------------------------------------------*/

  /**
     Structure representing the specific elements of a vector
  */
  struct diet_vector_specific {
    size_t size; /*!< size of the vector */
  };

  /*----[ matrix - specific ]-------------------------------------------------*/
  /**
     Structure representing the specific elements of a matrix

     @sa diet_matrix_order_t for more information about diet matrix orders
  */
  struct diet_matrix_specific {
    size_t              nb_r; /*!< number of rows */
    size_t              nb_c; /*!< number of columns */
    diet_matrix_order_t order; /*!< order of the matrix */
  };

  /*----[ string - specific ]-------------------------------------------------*/
  /**
     Structure representing the specific elements of a string

  */
  struct diet_string_specific {
    size_t length; /*!< length of a string */
  };

  /**
     Structure representing the specific elements of a param string

  */
  struct diet_paramstring_specific {
    size_t length; /*!< length of a string */
    char*  param; /*!< the string itself */
  };

  /*----[ file - specific ]---------------------------------------------------*/
  /**
     Structure representing the specific elements of a file

  */
  struct diet_file_specific {
    int   size; /*!< the size of the file */
    char* path; /*!< the path of the file */
  };

  /*----[ container - specific ]---------------------------------------------------*/
  /**
     Structure representing the specific elements of a container

  */
  struct diet_container_specific {
    size_t  size; /*!< nb of elements of the container (not used) */
  };

  /*----[ data - generic ]----------------------------------------------------*/

  /**
     Structure representing the generic elements of a data : its base composite types

  */
  struct diet_data_generic {
    diet_data_type_t type; /*!< composite type of a data */
    diet_base_type_t base_type; /*!< base type of the data */
  };

  /*----[ data description ]--------------------------------------------------*/
  /**
     Structure representing the description of a data

  */
  typedef struct {
    char* id;     /*!< id of the data. Allocated at the creation
                    of the encapsulating data_handle */
    diet_persistence_mode_t  mode; /*!< persistence mode of the data */
    struct diet_data_generic generic; /*!< generic elements of a data */
    union {
      struct diet_scalar_specific scal; /*!< specific elements of a scalar
                                          value */
      struct diet_vector_specific vect; /*!< specific elements of a vector
                                          value */
      struct diet_matrix_specific mat;  /*!< specific elements of a matrix
                                          value */
      struct diet_string_specific str;  /*!< specific elements of a string
                                          value */
      struct diet_paramstring_specific pstr; /*!< specific elements of a
                                               paramstring value */
      struct diet_file_specific file; /*!< specific elements of a file value */
      struct diet_container_specific cont; /*!< specific elements of a
                                             container value */
    } specific; /*!< specific element of the data */
  } diet_data_desc_t;

  /**
     Structure representing an argument of a profile
  */
  struct diet_arg_s {
    diet_data_desc_t desc; /*!< description of a data */
    void* value; /*!< value of an argument */
  };

  /**
     diet_data_t is the same as diet_arg_t, so far ...
  */
  typedef struct diet_arg_s diet_data_t;

  /**
   * containers
   */

  /**
     Structure describing a container
  */
  typedef struct {
    char*   id; /*!< id of a container */
    size_t  size; /*!< size of the container */
    char**  elt_ids; /*!< elements of the container */
  } diet_container_t;

  /*
   * estimation tags
   * WARNING : dependency changes to this enum must be propagated to
   * DietLogComponent::getEstimationTags
   */

  /*typedef enum {*/

  /**
     Tag for invalid parameter
  */
#define EST_INVALID -1
  /**
     Tag for the total time
  */
#define EST_TOTALTIME 1
  /**
     Tag for the communication time
  */
#define EST_COMMTIME 2
  /**
     Tag for the computation time
  */
#define EST_TCOMP 3
  /**
     Tag for the time since last solve
  */
#define EST_TIMESINCELASTSOLVE 4

#define EST_COMMPROXIMITY 5
#define EST_TRANSFEREFFORT 6
  /**
     Tag for the number of free cpus
  */
#define EST_FREECPU 7
  /**
     Tag for the quantity of free memory
  */
#define EST_FREEMEM 8
  /**
     Tag for the number of cpus
  */
#define EST_NBCPU 9
  /**
     Tag for the cpu speed
  */
#define EST_CPUSPEED 10
  /**
     Tag for the total quantity of memory
  */
#define EST_TOTALMEM 11
  /**
     Tag for the average quantity of free memory
     @warning not yet implemented
  */
#define EST_AVGFREEMEM 12
  /**
     Tag for the average number of free cpus
  */
#define EST_AVGFREECPU 13
  /**
     Tag for the bogomips
  */
#define EST_BOGOMIPS 14
  /**
     Tag for the quantity of cache memory of the cpu
  */
#define EST_CACHECPU 15
  /**
     Tag for the total size of the disk
  */
#define EST_TOTALSIZEDISK 16
  /**
     Tag for the free size of the disk
  */
#define EST_FREESIZEDISK 17
  /**
     Tag for the read access speed of the disk
  */
#define EST_DISKACCESREAD 18
  /**
     Tag for the write access speed of the disk
  */
#define EST_DISKACCESWRITE 19
  /**
     Tag to get all infos
  */
#define EST_ALLINFOS 20
  /**
     Tag Added to obtain the queue size from the SeD.
  */
#define EST_NUMWAITINGJOBS 21
  /********** HAVE_ALT_BATCH ************/
  /**
     Tag to get the server type
  */
#define EST_SERVER_TYPE 22
  /**
     Tag for the number of nodes if it's a parallel machine, or the number of machines if it's a cluster
  */
#define EST_PARAL_NBTOT_RESOURCES 23
  /**
     Tag for the number of idle nodes if it's a parallel machine, or the number of idle machines if it's a cluster
  */
#define EST_PARAL_NBTOT_FREE_RESOURCES 24
  /**
     Tag for the number of nodes if it's a parallel machine, or the number of machines if it's a cluster, all of this in the default queue
  */
#define EST_PARAL_NB_RESOURCES_IN_DEFAULT_QUEUE 25
  /**
     Tag for the number of free nodes if it's a parallel machine, or the number of free machines if it's a cluster, all of this in the default queue
  */
#define EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE 26
  /**
     Tag for the maximum walltime
  */
#define EST_PARAL_MAX_WALLTIME 27
  /**
     Tag for the maximum number of processors
  */
#define EST_PARAL_MAX_PROCS 28
  /********** !HAVE_ALT_BATCH ************/
  /**
     Tag Added to obtain the earliest finish time (used by workflows)
  */
#define EST_EFT 29
  /*
    Tag specified to define the start point where to add new estimations values
    @warning do not add new values before this tag
  */
#define EST_USERDEFINED 30
  /*} diet_est_tag_t;*/

  /**
   * Incomplete type for an estimation vector
   */
  typedef struct corba_estimation_t *estVector_t;
  /**
   * Incomplete constant type for an estimation vector
   */
  typedef const struct corba_estimation_t *estVectorConst_t;

  /**
     estimation source tags

     tags used by the plug-in CoRI to choose the appropriated programm
     for collecting information about the machine

     @warning for the moment the collectors for Ganglia or Nagios are not implemented
     @todo collectors for Ganglia and Nagios are not yet implemented
  */

  typedef enum {
    EST_COLL_EASY, /*!< source tag for CoRI Easy */
    EST_COLL_FAST, /*!< source tag for FAST */
    EST_COLL_GANGLIA, /*!< source tag for Ganglia */
    EST_COLL_NAGIOS, /*!< source tag for Nagios */
    /****** HAVE_ALT_BATCH ******/
    EST_COLL_BATCH /*!< source tag for Batch schedulers */
    /***** !HAVE_ALT_BATCH ******/
  } diet_est_collect_tag_t;

#ifdef HAVE_WORKFLOW

  /**
     Enumeration corresponding to the workflow level
  */
  typedef enum { DIET_WF_DAG, /*!< level for a DAG */
                 DIET_WF_FUNCTIONAL /*!< level for a functional workflow */
  } wf_level_t;

  /**
     Structure describing a workflow
  */
  typedef struct {
    char * abstract_wf; /*!< path the the abstract workflow */
    char * name; /*!< name of the workflow */
    wf_level_t level; /*!< level of the workflow */
    int wfReqID; /*!< request id of the workflow */
    char * dataFile; /*!< path to the data file */
    char * transcriptFile; /*!< path to the transcript file */
  } diet_wf_desc_t;

  /**
     Function used to allocate a workflow diet profile
     @param wf_file_name is the file name that contains the workflow description
     @param wf_name is the name of the workflow (used in logs)
     @param wf_level is the type of workflow
     @return the allocated workflow profile
  */
  DIET_API_LIB diet_wf_desc_t*
  diet_wf_profile_alloc(const char* wf_file_name,
                        const char* wf_name,
                        wf_level_t wf_level);

  /**
     Function used to set the name of the file containing data used by a functional workflow

     @param profile is the functional wf profile ref
     @param data_file_name is the full name of the file containing data (in XML)
  */
  DIET_API_LIB void
  diet_wf_set_data_file(diet_wf_desc_t * profile,
                        const char * data_file_name);

  /**
     Function used to Store the description of input and output data of a functional workflow
     (data items of 'source' and 'sink' nodes of the workflow)
     The description (in XML) contains the data IDs therefore the sources can
     be re-used for another execution of the workflow.
     @param profile is the functional wf profile ref
     @param data_file_name  output file (overwritten if existing)
     @return error value specifying if the data has been saved correctly or not:
     @arg 0 on success
     @arg 1 if the profile is null, the workflow file malformed, the data_file_name is NULL or if the write operation failed

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !
  */
  DIET_API_LIB int
  diet_wf_save_data_file(diet_wf_desc_t * profile,
                         const char * data_file_name);

  /**
     Function used to set the name of the file used to read the execution transcript
     of a workflow (dag or functional). The workflow engine will not
     execute again tasks when their results are already existing on
     the DIET platform.

     @param profile is the dag or functional wf profile ref
     @param transcript_file_name is the path of the file to read

  */
  DIET_API_LIB void
  diet_wf_set_transcript_file(diet_wf_desc_t * profile,
                              const char * transcript_file_name);

  /**
     Function used to store the execution transcript of the workflow (after execution)
     This file is used to restart the workflow from where it stopped
     (if results of all tasks are still available on the DIET platform)

     @param profile is the dag or functional wf profile ref
     @param transcript_file_name output file (overwritten if existing)

     @return error value specifying if the data has been saved correctly or not:
     @arg 0 on success
     @arg 1 if the profile is null, the workflow file malformed, the transcript_file_name is NULL or if the write operation failed

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !
  */
  DIET_API_LIB int
  diet_wf_save_transcript_file(diet_wf_desc_t * profile,
                               const char * transcript_file_name);

  /**
     Function used to free ressources of a workflow

     @param profile is the dag / functional wf profile ref
  */
  DIET_API_LIB void
  diet_wf_profile_free(diet_wf_desc_t * profile);

  /**
     Function used to display all workflow outputs on standard output in a generic manner
     (may be used for dag or functional wf)

     @param profile is the dag / functional wf profile ref
     @return error value specifying if the data could be displayed correctly or not:
     @arg 0 on success
     @arg 1 if something wrong happened during the display

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @deprecated{should not be used. Use diet_wf_print_results instead}
  */
  DIET_API_LIB int
  get_all_results(diet_wf_desc_t * profile);  /* DEPRECATED */

  /**
     Function used to display all workflow outputs on standard output in a generic manner
     (may be used for dag or functional wf)

     @param profile is the dag / functional wf profile ref
     @return error value specifying if the data could be displayed correctly or not:
     @arg 0 on success
     @arg 1 if something wrong happened during the display

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  diet_wf_print_results(diet_wf_desc_t * profile);

  /**
     Function used to get a scalar value of a DAG workflow

     Type : int diet_wf_scalar_get(diet_wf_desc_t * profile, const char * id, void** value)

     @arg profile the profile of the workflow
     @arg id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @arg value the scalar value

     The returned value could be :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_scalar_get call

     @remark error checking are realized in _diet_wf_scalar_get (the underlying called function)

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

  */

#define diet_wf_scalar_get(profile, id, value)          \
  _diet_wf_scalar_get(profile, id, (void**)value)


  /**
     \internal
     Function used to get a scalar value of a DAG workflow

     @param profile the profile of the workflow
     @param id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @param value the scalar value

     @return error value telling whether the recuperation of the container value succeeded or not :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_scalar_get call

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !
     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  _diet_wf_scalar_get(diet_wf_desc_t * profile,
                      const char * id,
                      void** value);

  /**
     Function used to get a string value of a DAG workflow

     Type : int diet_wf_string_get(diet_wf_desc_t * profile, const char * id, char** value)

     @arg profile the profile of the workflow
     @arg id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @arg value the string value

     The returned value could be :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_string_get call

     @remark error checking are realized in _diet_wf_string_get (the underlying called function)

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

  */
#define diet_wf_string_get(profile, id, value)          \
  _diet_wf_string_get(profile, id, (char**)value)

  /**
     \internal
     Function used to get a string value of a DAG workflow

     @param profile the profile of the workflow
     @param id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @param value the string value

     @return error value telling whether the recuperation of the container value succeeded or not :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_string_get call

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !

  */
  DIET_API_LIB int
  _diet_wf_string_get(diet_wf_desc_t * profile,
                      const char * id,
                      char** value);

  /**
     Function used to get a file value of a DAG workflow

     Type : int diet_wf_file_get(diet_wf_desc_t * profile, const char * id, size_t* size, char** path)

     @arg profile the profile of the workflow
     @arg id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @arg size the size of the file
     @arg path the path to the file

     The returned value could be :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_file_get call

     @remark error checking are realized in _diet_wf_file_get (the underlying called function)

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

  */
#define diet_wf_file_get(profile, id, size, path)       \
  _diet_wf_file_get(profile, id, size, (char**)path)


  /**
     \internal
     Function used to get a file value of a DAG workflow

     @param profile the profile of the workflow
     @param id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @param size the size of the file
     @param path the path to the file

     @return error value telling whether the recuperation of the file value succeeded or not :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_file_get call

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  _diet_wf_file_get(diet_wf_desc_t * profile,
                    const char * id,
                    size_t* size, char** path);

  /**
     Function used to get a matrix value of a DAG workflow

     Type : int diet_wf_matrix_get(diet_wf_desc_t * profile, const char * id, size_t* nb_rows, size_t* nb_cols, diet_matrix_order_t* order)

     @arg profile the profile of the workflow
     @arg id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @arg nb_rows number of rows of the matrix
     @arg nb_cols number of columns of the matrix
     @arg order order of the matrix

     The returned value could be :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_matrix_get call

     @remark error checking are realized in _diet_wf_matrix_get (the underlying called function)

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @sa diet_matrix_order_t structure for more information of matrix orders

  */
#define diet_wf_matrix_get(profile, id, value, nb_rows, nb_cols, order) \
  _diet_wf_matrix_get(profile, id, (void**)value, nb_rows, nb_cols, order)

  /**
     \internal
     Function used to get a matrix value of a DAG workflow

     @param profile the profile of the workflow
     @param id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @param nb_rows number of rows of the matrix
     @param nb_cols number of columns of the matrix
     @param order order of the matrix

     @return error value telling whether the recuperation of the matrix value succeeded or not :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_matrix_get call

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @sa diet_matrix_order_t structure for more information of matrix orders

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !

  */
  DIET_API_LIB int
  _diet_wf_matrix_get(diet_wf_desc_t * profile,
                      const char * id, void** value,
                      size_t* nb_rows, size_t *nb_cols,
                      diet_matrix_order_t* order);

  /**
     Function used to get a container value of a DAG workflow

     Type : int diet_wf_container_get(diet_wf_desc_t * profile, const char * id, char** dataID)

     @arg profile the profile of the workflow
     @arg id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @arg dataID data ID

     The returned value could be :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_container_get call

     @remark error checking are realized in _diet_wf_container_get (the underlying called function)

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

  */
#define diet_wf_container_get(profile, id, dataID)      \
  _diet_wf_container_get(profile, id, (char**)dataID)

  /**
     Function used to get a container value of a DAG workflow

     @param profile the profile of the workflow
     @param id The 'id' parameter is the port complete id (ex: 'nodeA#out')
     @param dataID data ID

     @return error value telling whether the recuperation of the container value succeeded or not :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid
     @arg the returned error value of the underlying diet_container_get call

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

  */
  DIET_API_LIB int
  _diet_wf_container_get(diet_wf_desc_t * profile,
                         const char * id,
                         char** dataID);

  /**
     Function used to get results of a FUNCTIONAL workflow
     Use sink nodes to get access to workflow results

     @param profile is the functional wf profile ref
     @param id is the id ('name' attribute) of the sink node
     @param dataID returns the DAGDA ID of a container containing the results

     @return error value telling whether the recuperation of the sink value succeeded or not :
     @arg 0 on success
     @arg 1 if the profile is null, the workflow type is invalid, the value is invalid
     @arg the returned error value of the underlying diet_container_get call

     @todo error codes defined in DIET_grpc.h should better be used.
     @sa DIET_grpc.h for error codes

     @warning this function is declared in DIET_data.h but implemented in DIET_client.cc !
  */

  DIET_API_LIB int
  diet_wf_sink_get(diet_wf_desc_t * profile,
                   const char * id,
                   char** dataID);

#endif /* HAVE_WORKFLOW */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DIET_DATA_H_ */


