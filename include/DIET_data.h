/****************************************************************************/
/* $Id$ */
/* DIET data interface for clients as well as servers                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP - ENS Lyon (France)                 */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.1  2002/08/09 14:30:21  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 ****************************************************************************/


#ifndef _DIET_DATA_H_
#define _DIET_DATA_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



/****************************************************************************/
/* Base and data types                                                      */
/****************************************************************************/

typedef enum {
  DIET_CHAR = 0,
  DIET_BYTE,
  DIET_INT,
  DIET_LONGINT,
  DIET_FLOAT,
  DIET_DOUBLE,
  DIET_SCOMPLEX,
  DIET_DCOMPLEX,
  DIET_BASE_TYPE_COUNT
} diet_base_type_t;

typedef enum {
  DIET_SCALAR = 0,
  DIET_VECTOR,
  DIET_MATRIX,
  DIET_STRING,
  DIET_FILE,
  DIET_DATA_TYPE_COUNT
} diet_data_type_t;


/****************************************************************************/
/* Base and data types                                                      */
/****************************************************************************/

typedef enum {
  DIET_VOLATILE = 0,
  DIET_PERSISTENT,
  DIET_STICKY
} diet_persistence_mode_t;



/****************************************************************************/
/* Data descriptions                                                        */
/****************************************************************************/


/*----[ scalar - specific ]-------------------------------------------------*/
struct diet_scalar_specific {
  void *value;
};

/*----[ vector - specific ]-------------------------------------------------*/
struct diet_vector_specific {
  size_t size;
};

/*----[ matrix - specific ]-------------------------------------------------*/
struct diet_matrix_specific {
  size_t nb_r;
  size_t nb_c;
  int    istrans; /* whether the matrix is transposed or not */
};

/*----[ string - specific ]-------------------------------------------------*/
struct diet_string_specific {
  size_t length;
};

/*----[ file - specific ]---------------------------------------------------*/
struct diet_file_specific {
  int   size;
  char *path;
};

/*----[ data - generic ]----------------------------------------------------*/
struct diet_data_generic {
  diet_data_type_t type;
  diet_base_type_t base_type;
};

/*----[ data description ]--------------------------------------------------*/
typedef struct {
  diet_persistence_mode_t  mode;
  struct diet_data_generic generic;
  union {
    struct diet_scalar_specific scal;
    struct diet_vector_specific vect;
    struct diet_matrix_specific mat;
    struct diet_string_specific str;
    struct diet_file_specific   file;
  } specific;
} diet_data_desc_t;


/****************************************************************************/
/* Data type                                                                */
/****************************************************************************/

typedef struct {
  diet_data_desc_t desc;
  void            *value;
} diet_data_t;

typedef struct {
  size_t length;
  diet_data_t *seq;
} diet_data_seq_t;



/****************************************************************************/
/* Useful functions for data descriptors                                    */
/****************************************************************************/

/* There should be no use of allocating and freeing functions */

/*----[ Descriptors altering ]----------------------------------------------*/
/* Each -1 (or NULL for pointers) argument does not alter the corresponding
   field. */

int scalar_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, void *value);
int vector_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, size_t size);
int matrix_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, size_t nb_r, size_t nb_c,
		    int istrans );
int string_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    size_t length);
/* Computes the file size */
int file_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		  char *path);

inline int generic_desc_set(struct diet_data_generic *desc,
			    diet_data_type_t type, diet_base_type_t base_type)
{
  if (!desc)
    return 1;
  desc->type      = type;
  desc->base_type = base_type;
  return 0;
}



/****************************************************************************/
/* Useful functions for data                                                */
/****************************************************************************/

/* There should be no use of allocating functions */

/*----[ Data altering ]-----------------------------------------------------*/
/* Each -1 (or NULL for pointers) argument does not alter the corresponding
   field. */

int scalar_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
	       diet_base_type_t base_type);
int vector_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
	       diet_base_type_t base_type, size_t size);
int matrix_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
	       diet_base_type_t base_type, size_t nb_r, size_t nb_c, int istrans);
int string_set(diet_data_t *data, char *value, diet_persistence_mode_t mode,
	       size_t length);
/* Computes the file size */
int file_set(diet_data_t *data, diet_persistence_mode_t mode, char *path);





#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DIET_DATA_H_
