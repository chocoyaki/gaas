/***********************************************************/
/* Dagda API.                                              */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@in2p3.fr)                   */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id:
/* $Log
/*
************************************************************/

/* To add a data to the platform. */
int dagda_add_data(void* value, diet_data_type_t type,
	diet_base_type_t base_type, diet_persistence_mode_t mode,
	size_t nb_r, size_t nb_c, diet_matrix_order_t order, char* path, char** ID);
/* To get a data from the platform. */
int dagda_get_data(char* dataID, void** value, diet_data_type_t type,
	diet_base_type_t* base_type, size_t* nb_r, size_t* nb_c,
	diet_matrix_order_t* order, char** path);

#define dagda_add_scalar(value, base_type, mode, ID) \
  dagda_add_data(value, DIET_SCALAR, base_type, mode, 0, 0, \
    (diet_matrix_order_t) 0, NULL, ID)
  
#define dagda_add_vector(value, base_type, mode, size, ID) \
  dagda_add_data(value, DIET_VECTOR, base_type, mode, 0, size, \
    (diet_matrix_order_t) 0, NULL, ID)
  
#define dagda_add_matrix(value, base_type, mode, nb_rows, nb_cols, order, ID) \
  dagda_add_data(value, DIET_MATRIX, base_type, mode, nb_rows, nb_cols, \
    order, NULL, ID)
  
#define dagda_add_string(value, mode, ID) \
  dagda_add_data(value, DIET_STRING, DIET_CHAR, mode, 0, 0, \
    (diet_matrix_order_t) 0, NULL, ID)
  
#define dagda_add_paramstring(value, mode, ID) \
  dagda_add_data(value, DIET_PARAMSTRING, DIET_CHAR, mode, 0, 0, \
    (diet_matrix_order_t) 0, NULL, ID)
  
#define dagda_add_file(path, mode, ID) \
  dagda_add_data(NULL, DIET_FILE, DIET_CHAR, mode, 0, 0, \
    (diet_matrix_order_t) 0, path, ID)

#define dagda_get_scalar(ID, value, base_type) \
  dagda_get_data(ID, (void*) (value), DIET_SCALAR, base_type, NULL, NULL, NULL, NULL)

#define dagda_get_vector(ID, value, base_type, size) \
  dagda_get_data(ID, (void*) (value), DIET_VECTOR, base_type, NULL, size, NULL, NULL)

#define dagda_get_matrix(ID, value, base_type, nb_r, nb_c, order, NULL) \
  dagda_get_data(ID, (void*) (value), DIET_MATRIX, base_type, nb_r, nb_c, order, NULL)

#define dagda_get_string(ID, value) \
  dagda_get_data(ID, (void*) (value), DIET_STRING, DIET_CHAR, 0, 0, NULL, NULL)

#define dagda_get_paramstring(ID, value) dagda_get_string(ID, value)

#define dagda_get_file(ID, path) \
  dagda_get_data(ID, NULL, DIET_FILE, DIET_CHAR, 0, 0, NULL, (char**) (path))
