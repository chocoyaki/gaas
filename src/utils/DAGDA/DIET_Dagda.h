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
 * Revision 1.9  2008/10/22 15:07:28  bdepardo
 * Fixed header:
 *  - now CVS will write the log
 *  - this removes a warning during compilation
 *
 *
 ***********************************************************/
#ifndef __DIET_DAGDA_H__
#define __DIET_DAGDA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* If we don't know the type of the data to download. */
#define DIET_UNKNOWN_TYPE DIET_DATA_TYPE_COUNT
/* To add a data to the platform. */
int dagda_put_data(void* value, diet_data_type_t type,
	diet_base_type_t base_type, diet_persistence_mode_t mode,
	size_t nb_r, size_t nb_c, diet_matrix_order_t order, char* path, char** ID);
/* To get a data from the platform. */
int dagda_get_data(const char* dataID, void** value, diet_data_type_t type,
	diet_base_type_t* base_type, size_t* nb_r, size_t* nb_c,
	diet_matrix_order_t* order, char** path);
/* To remove a data from the platform */
int dagda_delete_data(char* dataID);

/* Asynchronous versions. */
/* Put a data. Return the thread ID. */
unsigned int dagda_put_data_async(void* value, diet_data_type_t type,
	  diet_base_type_t base_type, diet_persistence_mode_t mode,
	  size_t nb_r, size_t nb_c, diet_matrix_order_t order, char* path);
/* Get a data. Return the thread ID. */
unsigned int dagda_get_data_async(char* ID, diet_data_type_t type);

/* Wait functions. */
/* Wait for the end of a data transfer from here. */
int dagda_wait_put(unsigned int threadID, char** ID);
/* Wait for the end of a data transfer to here. */
int dagda_wait_get(unsigned int threadID, void** value, diet_base_type_t* base_type,
    size_t* nb_r, size_t* nb_c, diet_matrix_order_t* order, char** path);

/* Add a data asynchronously without waiting possibility. */
/* As is, this function does not make sense. Should be modified later
   to return the ID immediately. */
int dagda_add_data(void* value, diet_data_type_t type,
  diet_base_type_t base_type, diet_persistence_mode_t mode,
  size_t nb_r, size_t nb_c, diet_matrix_order_t order, char* path);
/* Load a data asynchronously on this data manager without waiting possibility. */
int dagda_load_data(char* ID, diet_data_type_t type);

/* Asks to the nodes to save their data on checkpoint file. */
int dagda_save_platform();

/* Creates a data alias. */
int dagda_data_alias(const char* id, const char* alias);
/* Get the id of a data from its alias. */
int dagda_id_from_alias(const char* alias, char** id);

/* Data replication following a wildcard rule. */
int dagda_replicate_data(const char* id, const char* rule);

/* Create a container */
int dagda_create_container(char** ID);

/* Initialize a container (ID already defined) */
int dagda_init_container(diet_data_t* profile_data);

/* Add an element to a container *
 * The container must be either created or initialized before, so that
 * it is declared on the local dagda manager */
int dagda_add_container_element(const char* idContainer, const char* idElement, int index);

/* Get all IDs of the elements of a container *
 * The container must be either created or initialized before, so that
 * it is declared on the local dagda manager */
int dagda_get_container_elements(const char* idContainer, diet_container_t* content);

/* Put macros */
#define dagda_put_scalar(value, base_type, mode, ID) \
  dagda_put_data(value, DIET_SCALAR, base_type, mode, 0, 0, \
    (diet_matrix_order_t) 0, NULL, ID)

#define dagda_put_vector(value, base_type, mode, size, ID) \
  dagda_put_data(value, DIET_VECTOR, base_type, mode, 0, size, \
    (diet_matrix_order_t) 0, NULL, ID)

#define dagda_put_matrix(value, base_type, mode, nb_rows, nb_cols, order, ID) \
  dagda_put_data(value, DIET_MATRIX, base_type, mode, nb_rows, nb_cols, \
    order, NULL, ID)

#define dagda_put_string(value, mode, ID) \
  dagda_put_data(value, DIET_STRING, DIET_CHAR, mode, 0, 0, \
    (diet_matrix_order_t) 0, NULL, ID)

#define dagda_put_paramstring(value, mode, ID) \
  dagda_put_data(value, DIET_PARAMSTRING, DIET_CHAR, mode, 0, 0, \
    (diet_matrix_order_t) 0, NULL, ID)

#define dagda_put_file(path, mode, ID) \
  dagda_put_data(NULL, DIET_FILE, DIET_CHAR, mode, 0, 0, \
    (diet_matrix_order_t) 0, path, ID)

/* Get macros */
#define dagda_get_scalar(ID, value, base_type) \
  dagda_get_data(ID, (void**) (value), DIET_SCALAR, base_type, NULL, NULL, NULL, NULL)

#define dagda_get_vector(ID, value, base_type, size) \
  dagda_get_data(ID, (void**) (value), DIET_VECTOR, base_type, NULL, size, NULL, NULL)

#define dagda_get_matrix(ID, value, base_type, nb_r, nb_c, order) \
  dagda_get_data(ID, (void**) (value), DIET_MATRIX, base_type, nb_r, nb_c, order, NULL)

#define dagda_get_string(ID, value) \
  dagda_get_data(ID, (void**) (value), DIET_STRING, NULL, NULL, NULL, NULL, NULL)

#define dagda_get_paramstring(ID, value) dagda_get_string(ID, value)

#define dagda_get_file(ID, path) \
  dagda_get_data(ID, NULL, DIET_FILE, NULL, NULL, NULL, NULL, (char**) (path))

#define dagda_get_container(ID) \
  dagda_get_data(ID, NULL, DIET_CONTAINER, NULL, NULL, NULL, NULL, NULL)

// Asynchronous versions.
/* Put macros */
#define dagda_put_scalar_async(value, base_type, mode) \
  dagda_put_data_async(value, DIET_SCALAR, base_type, mode, 0, 0, \
    (diet_matrix_order_t) 0, NULL)

#define dagda_put_vector_async(value, base_type, mode, size) \
  dagda_put_data_async(value, DIET_VECTOR, base_type, mode, 0, size, \
    (diet_matrix_order_t) 0, NULL)

#define dagda_put_matrix_async(value, base_type, mode, nb_rows, nb_cols, order) \
  dagda_put_data_async(value, DIET_MATRIX, base_type, mode, nb_rows, nb_cols, order, NULL)

#define dagda_put_string_async(value, mode) \
  dagda_put_data_async(value, DIET_STRING, DIET_CHAR, mode, 0, 0, \
    (diet_matrix_order_t) 0, NULL)

#define dagda_put_paramstring_async(value, mode) \
  dagda_put_data_async(value, DIET_PARAMSTRING, DIET_CHAR, mode, 0, 0, \
    (diet_matrix_order_t) 0, NULL)

#define dagda_put_file_async(path, mode) \
  dagda_put_data_async(NULL, DIET_FILE, DIET_CHAR, mode, 0, 0, \
    (diet_matrix_order_t) 0, path)

/* Get macros */
#define dagda_get_scalar_async(ID) dagda_get_data_async(ID, DIET_SCALAR)
#define dagda_get_vector_async(ID) dagda_get_data_async(ID, DIET_VECTOR)
#define dagda_get_matrix_async(ID) dagda_get_data_async(ID, DIET_MATRIX)
#define dagda_get_string_async(ID) dagda_get_data_async(ID, DIET_STRING)
#define dagda_get_paramstring_async(ID) dagda_get_data_async(ID, DIET_PARAMSTRING)
#define dagda_get_file_async(ID) dagda_get_data_async(ID, DIET_FILE)

/* Asynchronous without thread control. */
#define dagda_load_scalar(ID) dagda_load_data(ID, DIET_SCALAR)
#define dagda_load_vector(ID) dagda_load_data(ID, DIET_VECTOR)
#define dagda_load_matrix(ID) dagda_load_data(ID, DIET_MATRIX)
#define dagda_load_string(ID) dagda_load_data(ID, DIET_STRING)
#define dagda_load_paramstring(ID) dagda_load_data(ID, DIET_PARAMSTRING)
#define dagda_load_file(ID) dagda_load_data(ID, DIET_FILE)

/* Wait macros */
#define dagda_wait_data_ID(thread, ID) dagda_wait_put(thread, ID)

#define dagda_wait_scalar(thread, value, base_type) \
  dagda_wait_get(thread, value, base_type, NULL, NULL, NULL, NULL)
#define dagda_wait_vector(thread, value, base_type, size) \
  dagda_wait_get(thread, value, base_type, NULL, size, NULL, NULL)
#define dagda_wait_matrix(thread, value, base_type, nb_r, nb_c, order) \
  dagda_wait_get(thread, value, base_type, nb_r, nb_c, order, NULL)
#define dagda_wait_string(thread, value) \
  dagda_wait_get(thread, value, NULL, NULL, NULL, NULL, NULL)
#define dagda_wait_paramstring(thread, value) \
  dagda_wait_string(thread, value)
#define dagda_wait_file(thread, path) \
  dagda_wait_get(thread, NULL, NULL, NULL, NULL, NULL, path)
    
#ifdef __cplusplus
}
#endif
#endif
