/***********************************************************/
/* Dagda API.                                              */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@in2p3.fr)                   */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log$
 * Revision 1.15  2011/04/22 09:31:20  bdepardo
 * Fixed a bug in dagda_get_paramstring: it now directly uses dagda_get_data
 * instead of dagda_get_string.
 *
 * Revision 1.14  2011/03/18 17:37:45  bdepardo
 * Add dagda_reset method to reset internal variables.
 * This is used for allowing multiple consecutive diet_initialize/diet_finalize
 *
 * Revision 1.13  2010/09/03 10:09:28  bdepardo
 * Changed C++ comments into C comments to remove warnings.
 *
 * Revision 1.12  2010/07/12 16:14:12  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.11  2010/04/20 12:00:43  glemahec
 * Ajout de l option de compilation TRANSFER_PROGRESSION => Extension de l API DAGDA pour compatibilite services de gestion de fichiers.
 *
 * Revision 1.10  2009/04/17 08:50:49  bisnard
 * added handling of container empty elements
 *
 * Revision 1.9  2008/10/22 15:07:28  bdepardo
 * Fixed header:
 *  - now CVS will write the log
 *  - this removes a warning during compilation
 *
 *
 ***********************************************************/
#ifndef __DIET_DAGDA_H__
#define __DIET_DAGDA_H__
#include "DIET_data.h"
#ifdef __cplusplus
extern "C" {
#endif

  /* If we don't know the type of the data to download. */
#define DIET_UNKNOWN_TYPE DIET_DATA_TYPE_COUNT
  /**
   * @brief add a data to the platform
   *
   * @param value value to be stored
   * @param type diet data type
   * @param base_type basic data type
   * @param mode persistence mode
   * @param nb_r
   * @param nb_c
   * @param order
   * @param path
   * @param ID return data ID
   *
   * @return  0 (success), 1 (failure)
   */
  int
  dagda_put_data(void* value, diet_data_type_t type,
                 diet_base_type_t base_type, diet_persistence_mode_t mode,
                 size_t nb_r, size_t nb_c, diet_matrix_order_t order,
                 char* path, char** ID);

  /**
   * @brief get a data from platform
   *
   * @param dataID ID of the data to be retrieved
   * @param value return data value
   * @param type diet data type
   * @param base_type basic data type
   * @param nb_r
   * @param nb_c
   * @param order
   * @param path
   *
   * @return 0 (success), 1 (failure)
   */
  int
  dagda_get_data(const char* dataID, void** value,
                 diet_data_type_t type, diet_base_type_t* base_type,
                 size_t* nb_r, size_t* nb_c,
                 diet_matrix_order_t* order, char** path);

  /**
   * @brief remove a data from the platform
   * @param dataID ID of the data to be removed
   *
   * @return 0 (success)
   */
  int
  dagda_delete_data(char* dataID);

  /**
   * @brief add a data to the platform (asynchronous)
   *
   * @param value value to be stored
   * @param type diet data type
   * @param base_type basic data type
   * @param mode persistence mode
   * @param nb_r
   * @param nb_c
   * @param order
   * @param path
   *
   * @return thread ID
   */
  unsigned int
  dagda_put_data_async(void* value, diet_data_type_t type,
                       diet_base_type_t base_type, diet_persistence_mode_t mode,
                       size_t nb_r, size_t nb_c,
                       diet_matrix_order_t order, char* path);

  /**
   * @brief get a data from platform (asynchronous)
   *
   * @param ID ID of the data to be retrieved
   * @param type diet data type
   *
   * @return thread ID
   */
  unsigned int
  dagda_get_data_async(char* ID, diet_data_type_t type);

  /* Wait functions. */
  /**
   * @brief wait for the end of data transfer (put)
   * @param threadID thread ID of the transfer
   * @param ID return ID of the data transferred
   *
   * @return
   */
  int
  dagda_wait_put(unsigned int threadID, char** ID);

  /**
   * @brief wait for the end of data transfer (get)
   *
   * @param threadID thread ID of the transfer
   * @param value data to be returned
   * @param base_type basic data type
   * @param nb_r
   * @param nb_c
   * @param order
   * @param path
   *
   * @return
   */
  int
  dagda_wait_get(unsigned int threadID, void** value,
                 diet_base_type_t* base_type, size_t* nb_r,
                 size_t* nb_c, diet_matrix_order_t* order, char** path);

  /* As is, this function does not make sense. Should be modified later
     to return the ID immediately. */
  /**
   * @brief add data asynchronously without waiting
   *
   * @param value value to be added
   * @param type diet data type
   * @param base_type basic data type
   * @param mode persistence mode
   * @param nb_r
   * @param nb_c
   * @param order
   * @param path
   *
   * @return 0 (success)
   */
  int
  dagda_add_data(void* value, diet_data_type_t type,
                 diet_base_type_t base_type, diet_persistence_mode_t mode,
                 size_t nb_r, size_t nb_c,
                 diet_matrix_order_t order, char* path);

  /**
   * @brief load data asynchronously without wait
   *
   * @param ID ID of the data to be loaded
   * @param type diet data type
   *
   * @return 0 (success)
   */
  int
  dagda_load_data(char* ID, diet_data_type_t type);

  /**
   * @brief ask node to save data on checkpoint file
   *
   * @return 0 (success), 1 (failure)
   */
  int
  dagda_save_platform();

  /**
   * @brief create a data alias
   *
   * @param id
   * @param alias
   *
   * @return 1 (failure)
   */
  int
  dagda_data_alias(const char* id, const char* alias);

  /**
   * @brief get data ID from its alias
   *
   * @param alias alias of the data
   * @param id return the ID of the data
   *
   * @return 0 (success), 1 (failure)
   */
  int
  dagda_id_from_alias(const char* alias, char** id);

  /**
   * @brief replicate data according a wildcard
   *
   * @param id ID of the data to be replicate
   * @param rule wildcard rule
   *
   * @return 0 (success), 1 (failure)
   */
  int
  dagda_replicate_data(const char* id, const char* rule);

  /**
   * @brief create a container
   *
   * @param ID return ID of the container
   *
   * @return
   */
  int
  dagda_create_container(char** ID);

  /**
   * @brief initialize a container (ID already defined)
   *
   * @param profile_data
   *
   * @return 0 (success)
   */
  int
  dagda_init_container(diet_data_t* profile_data);

  /**
   * @brief add an element to a container
   * (the container must be either created/initialized before
   *  before it is registered in the local DAGDA manager)
   *
   * @param idContainer
   * @param idElement
   * @param index
   *
   * @return 0 (success), 1 (failure)
   */
  int
  dagda_add_container_element(const char* idContainer, const char* idElement,
                              int index);

  /**
   * @brief add an empty slot to a container (replaces an element)
   * The container must be either created or initialized before, so that
   * it is declared on the local dagda manager
   *
   * @param idContainer container ID
   * @param index index
   *
   * @return
   */
  int
  dagda_add_container_null_element(const char* idContainer, int index);

  /**
   * @brief Get all IDs of the elements of a container
   * The container must be either created or initialized before, so that
   * it is declared on the local dagda manager
   *
   * @param idContainer
   * @param content
   *
   * @return  0 (success), 1 (failure)
   */
  int
  dagda_get_container_elements(const char* idContainer,
                               diet_container_t* content);

  /**
   * @brief get download progress
   *
   * @param dataId ID of the data transferred
   *
   * @return progress
   */
  double
  dagda_get_progress(const char* dataId);

  /**
   *
   *
   * @param transferId
   */
  void
  dagda_rem_progress(const char* transferId);

  /**
   * @brief reset internal state of the node
   * (used for sucessive diet_initialize/diet_finalize)
   */
  void
  dagda_reset();

  /* Put macros */
/**
 * @brief convenience macro around dagda_put_data
 *
 * @param value scalar data to be uploaded
 * @param base_type data base type
 * @param mode persistence mode
 * @param ID data ID
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_put_scalar(value, base_type, mode, ID)            \
  dagda_put_data(value, DIET_SCALAR, base_type, mode, 0, 0,     \
                 (diet_matrix_order_t) 0, NULL, ID)

/**
 * @brief convenience macro around dagda_put_data
 *
 * @param value vector value to be uploaded
 * @param base_type  data base type
 * @param mode persistence mode
 * @param size vector size
 * @param ID data ID
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_put_vector(value, base_type, mode, size, ID)      \
  dagda_put_data(value, DIET_VECTOR, base_type, mode, 0, size,  \
                 (diet_matrix_order_t) 0, NULL, ID)

/**
 * @brief convenience macro around dagda_put_data
 *
 * @param value matrix data to be uploaded
 * @param base_type data base type
 * @param mode persistence mode
 * @param nb_rows matrix rows number
 * @param nb_cols matrix columns number
 * @param order matrix order
 * @param ID data ID
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_put_matrix(value, base_type, mode, nb_rows, nb_cols, order, ID) \
  dagda_put_data(value, DIET_MATRIX, base_type, mode, nb_rows, nb_cols, \
                 order, NULL, ID)

/**
 * @brief convenience macro around dagda_put_data
 *
 * @param value string data to be uploaded
 * @param mode persistence mode
 * @param ID data ID
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_put_string(value, mode, ID)                       \
  dagda_put_data(value, DIET_STRING, DIET_CHAR, mode, 0, 0,     \
                 (diet_matrix_order_t) 0, NULL, ID)

/**
 * @brief convenience macro around dagda_put_data
 *
 * @param value param string data to be uploaded
 * @param mode persistence mode
 * @param ID data ID
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_put_paramstring(value, mode, ID)                          \
  dagda_put_data(value, DIET_PARAMSTRING, DIET_CHAR, mode, 0, 0,        \
                 (diet_matrix_order_t) 0, NULL, ID)


/**
 * @brief convenience macro around dagda_put_data
 *
 * @param path file path to be uploaded
 * @param mode persistence mode
 * @param ID data ID
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_put_file(path, mode, ID)                          \
  dagda_put_data(NULL, DIET_FILE, DIET_CHAR, mode, 0, 0,        \
                 (diet_matrix_order_t) 0, path, ID)

/* Get macros */
/**
 * @brief convenience macro around dagda_get_data
 *
 * @param ID ID to the downloaded scalar
 * @param[out] value downloaded scalar
 * @param base_type base type
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_get_scalar(ID, value, base_type)                          \
  dagda_get_data(ID, (void**) (value), DIET_SCALAR, base_type, NULL, NULL, NULL, NULL)

/**
 * @brief convenience macro around dagda_get_data
 *
 * @param ID ID to the downloaded vector
 * @param[out] value downloaded vector
 * @param base_type base type
 * @param size vector size
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_get_vector(ID, value, base_type, size)                    \
  dagda_get_data(ID, (void**) (value), DIET_VECTOR, base_type, NULL, size, NULL, NULL)

/**
 * @brief convenience macro around dagda_get_data
 *
 * @param ID ID to the downloaded matrix
 * @param[out] value downloaded matrix
 * @param base_type base type
 * @param nb_r matrix rows number
 * @param nb_c matrix columns number
 * @param order matrix order
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_get_matrix(ID, value, base_type, nb_r, nb_c, order)       \
  dagda_get_data(ID, (void**) (value), DIET_MATRIX, base_type, nb_r, nb_c, order, NULL)

/**
 * @brief convenience macro around dagda_get_data
 *
 * @param ID ID to the downloaded string
 * @param[out] value downloaded string
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_get_string(ID, value)                                     \
  dagda_get_data(ID, (void**) (value), DIET_STRING, NULL, NULL, NULL, NULL, NULL)

/**
 * @brief convenience macro around dagda_get_data
 *
 * @param ID ID to the downloaded param string
 * @param[out] value downloaded param string
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_get_paramstring(ID, value)                                \
  dagda_get_data(ID, (void**) (value), DIET_PARAMSTRING, NULL, NULL, NULL, NULL, NULL)

/**
 * @brief convenience macro around dagda_get_data
 *
 * @param ID ID to the downloaded file
 * @param[out] path downloaded file path
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_get_file(ID, path)                                        \
  dagda_get_data(ID, NULL, DIET_FILE, NULL, NULL, NULL, NULL, (char**) (path))

/**
 * @brief convenience macro around dagda_get_data
 *
 * @param ID ID to the downloaded container
 *
 * @return 0 (success), 1 (failure)
 */
#define dagda_get_container(ID)                                         \
  dagda_get_data(ID, NULL, DIET_CONTAINER, NULL, NULL, NULL, NULL, NULL)

/* Asynchronous versions. */
/* Put macros */
  /* Put macros */
/**
 * @brief convenience macro around dagda_put_data_async
 *
 * @param value scalar data to be uploaded
 * @param base_type data base type
 * @param mode persistence mode
 *
 * @return thread ID
 */
#define dagda_put_scalar_async(value, base_type, mode)                  \
  dagda_put_data_async(value, DIET_SCALAR, base_type, mode, 0, 0,       \
                       (diet_matrix_order_t) 0, NULL)

/**
 * @brief convenience macro around dagda_put_data_async
 *
 * @param value vector value to be uploaded
 * @param base_type data base type
 * @param mode persistence mode
 * @param size vector size
 *
 * @return thread ID
 */
#define dagda_put_vector_async(value, base_type, mode, size)            \
  dagda_put_data_async(value, DIET_VECTOR, base_type, mode, 0, size,    \
                       (diet_matrix_order_t) 0, NULL)
/**
 * @brief convenience macro around dagda_put_data_async
 *
 * @param value matrix data to be uploaded
 * @param base_type data base type
 * @param mode persistence mode
 * @param nb_rows matrix rows number
 * @param nb_cols matrix columns number
 * @param order matrix order
 *
 * @return thread ID
 */
#define dagda_put_matrix_async(value, base_type, mode, nb_rows, nb_cols, order) \
  dagda_put_data_async(value, DIET_MATRIX, base_type, mode, nb_rows, nb_cols, order, NULL)

/**
 * @brief convenience macro around dagda_put_data_async
 *
 * @param value string data to be uploaded
 * @param mode persistence mode
 *
 * @return thread ID
 */
#define dagda_put_string_async(value, mode)                             \
  dagda_put_data_async(value, DIET_STRING, DIET_CHAR, mode, 0, 0,       \
                       (diet_matrix_order_t) 0, NULL)

/**
 * @brief convenience macro around dagda_put_data_async
 *
 * @param value param string data to be uploaded
 * @param mode persistence mode
 *
 * @return thread ID
 */
#define dagda_put_paramstring_async(value, mode)                        \
  dagda_put_data_async(value, DIET_PARAMSTRING, DIET_CHAR, mode, 0, 0,  \
                       (diet_matrix_order_t) 0, NULL)

/**
 * @brief convenience macro around dagda_put_data_async
 *
 * @param path file path to be uploaded
 * @param mode persistence mode
 *
 * @return thread ID
 */
#define dagda_put_file_async(path, mode)                        \
  dagda_put_data_async(NULL, DIET_FILE, DIET_CHAR, mode, 0, 0,  \
                       (diet_matrix_order_t) 0, path)

/* Get macros */
/**
 * @brief convenience macro around dagda_get_data_async
 *
 * @param ID ID to the downloaded scalar
 *
 * @return thread ID
 */
#define dagda_get_scalar_async(ID) dagda_get_data_async(ID, DIET_SCALAR)
/**
 * @brief convenience macro around dagda_get_data_async
 *
 * @param ID ID to the downloaded vector
 *
 * @return thread ID
 */
#define dagda_get_vector_async(ID) dagda_get_data_async(ID, DIET_VECTOR)
/**
 * @brief convenience macro around dagda_get_data_async
 *
 * @param ID ID to the downloaded matrix
 *
 * @return thread ID
 */
#define dagda_get_matrix_async(ID) dagda_get_data_async(ID, DIET_MATRIX)
/**
 * @brief convenience macro around dagda_get_data_async
 *
 * @param ID ID to the downloaded string
 *
 * @return thread ID
 */
#define dagda_get_string_async(ID) dagda_get_data_async(ID, DIET_STRING)
/**
 * @brief convenience macro around dagda_get_data_async
 *
 * @param ID ID to the downloaded param string
 *
 * @return thread ID
 */
#define dagda_get_paramstring_async(ID) dagda_get_data_async(ID, DIET_PARAMSTRING)
/**
 * @brief convenience macro around dagda_get_data_async
 *
 * @param ID ID to the downloaded file
 *
 * @return thread ID
 */
#define dagda_get_file_async(ID) dagda_get_data_async(ID, DIET_FILE)

/* Asynchronous without thread control. */
/**
 * @brief convenience macro around dagda_load_data
 *
 * @param ID ID to the downloaded scalar
 *
 * @return  0 (success)
 */
#define dagda_load_scalar(ID) dagda_load_data(ID, DIET_SCALAR)
/**
 * @brief convenience macro around dagda_load_data
 *
 * @param ID ID to the downloaded vector
 *
 * @return  0 (success)
 */
#define dagda_load_vector(ID) dagda_load_data(ID, DIET_VECTOR)
/**
 * @brief convenience macro around dagda_load_data
 *
 * @param ID ID to the downloaded matrix
 *
 * @return  0 (success)
 */
#define dagda_load_matrix(ID) dagda_load_data(ID, DIET_MATRIX)
/**
 * @brief convenience macro around dagda_load_data_string
 *
 * @param ID ID to the downloaded string
 *
 * @return  0 (success)
 */
#define dagda_load_string(ID) dagda_load_data(ID, DIET_STRING)
/**
 * @brief convenience macro around dagda_load_data
 *
 * @param ID ID to the downloaded param string
 *
 * @return  0 (success)
 */
#define dagda_load_paramstring(ID) dagda_load_data(ID, DIET_PARAMSTRING)
/**
 * @brief convenience macro around dagda_load_data
 *
 * @param ID ID to the downloaded file
 *
 * @return  0 (success)
 */
#define dagda_load_file(ID) dagda_load_data(ID, DIET_FILE)

/* Wait macros */
/**
 * @brief convenience macro around dagda_wait_get
 *
 * @param thread thread ID
 * @param ID data ID
 *
 * @return
 */
#define dagda_wait_data_ID(thread, ID) dagda_wait_put(thread, ID)

/**
 * @brief convenience macro around dagda_wait_get
 *
 * @param thread thread ID
 * @param[out] value retrieved scalar
 * @param base_type data base type
 *
 * @return
 */
#define dagda_wait_scalar(thread, value, base_type)                     \
  dagda_wait_get(thread, value, base_type, NULL, NULL, NULL, NULL)
/**
 * @brief convenience macro around dagda_wait_get
 *
 * @param thread thread ID
 * @param[out] value retrieved vector
 * @param base_type data base type
 * @param size vector size
 *
 * @return
 */
#define dagda_wait_vector(thread, value, base_type, size)               \
  dagda_wait_get(thread, value, base_type, NULL, size, NULL, NULL)
/**
 * @brief convenience macro around dagda_wait_get
 *
 * @param thread thread ID
 * @param[out] value retrieved matrix
 * @param base_type data base type
 * @param nb_r matrix rows number
 * @param nb_c matrix columns number
 * @param order matrix order
 *
 * @return
 */
#define dagda_wait_matrix(thread, value, base_type, nb_r, nb_c, order)  \
  dagda_wait_get(thread, value, base_type, nb_r, nb_c, order, NULL)
/**
 * @brief convenience macro around dagda_wait_get
 *
 * @param thread thread ID
 * @param[out] value retrieved string
 *
 * @return
 */
#define dagda_wait_string(thread, value)                        \
  dagda_wait_get(thread, value, NULL, NULL, NULL, NULL, NULL)
/**
 * @brief convenience macro around dagda_wait_get
 *
 * @param thread thread ID
 * @param[out] value retrieved param string
 *
 * @return
 */
#define dagda_wait_paramstring(thread, value)   \
  dagda_wait_string(thread, value)
/**
 * @brief convenience macro around dagda_wait_get
 *
 * @param thread thread ID
 * @param[out] path retrieved file
 *
 * @return
 */
#define dagda_wait_file(thread, path)                           \
  dagda_wait_get(thread, NULL, NULL, NULL, NULL, NULL, path)

#ifdef __cplusplus
}
#endif
#endif
