/**
 * @file  DIET_data_internal.hh
 *
 * @brief  DIET data internal specification
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */





#ifndef _DIET_DATA_INTERNAL_HH_
#define _DIET_DATA_INTERNAL_HH_

#include "common_types.hh"
#include "DIET_data.h"
#include "DIET_server.h"


/****************************************************************************/
/* Useful functions for data descriptors manipulation                       */
/* <implemented in DIET_data.cc>                                            */
/****************************************************************************/

/** Return the size in bytes of a DIET base type. */
size_t
type_sizeof(const diet_base_type_t type);

/** Compute data size in base elements (base type elements). */
size_t
macro_data_sizeof(const diet_data_desc_t *desc);

/** Compute data size in bytes from their descriptor. */
size_t
data_sizeof(const diet_data_desc_t *desc);

/** Compute data size in bytes from their descriptor. */
size_t
data_sizeof(const corba_data_desc_t *desc);

/**
 * Alter a scalar descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
scalar_set_desc(diet_data_desc_t *desc, char *const id,
                const diet_persistence_mode_t mode,
                const diet_base_type_t base_type, void *const value);
/**
 * Alter a vector descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
vector_set_desc(diet_data_desc_t *desc, char *const id,
                const diet_persistence_mode_t mode,
                const diet_base_type_t base_type, const size_t size);
/**
 * Alter a matrix descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
matrix_set_desc(diet_data_desc_t *desc, char *const id,
                const diet_persistence_mode_t mode,
                const diet_base_type_t base_type, const size_t nb_r,
                const size_t nb_c, const diet_matrix_order_t order);
/**
 * Alter a string descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
string_set_desc(diet_data_desc_t *desc, char *const id,
                const diet_persistence_mode_t mode, const size_t length);

int
paramstring_set_desc(diet_data_desc_t *desc,
                     char *const id,
                     const diet_persistence_mode_t mode,
                     const size_t length,
                     const char *const param);

/**
 * Alter a file descriptor. Also computes the file size ...
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
file_set_desc(diet_data_desc_t *desc, char *const id,
              const diet_persistence_mode_t mode, char *const path);

/**
 * Alter a container descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
container_set_desc(diet_data_desc_t *desc, char *const id,
                   const diet_persistence_mode_t mode, const size_t size);


/****************************************************************************/
/* Useful functions for profile manipulation                                */
/* <implemented in DIET_data.cc>                                            */
/****************************************************************************/

/** Return true if p1 is exactly identical to p2. */
int
profile_desc_match(const corba_profile_desc_t *p1,
                   const corba_profile_desc_t *p2);

/**
 * Return true if sv_profile describes a service that matches the problem that
 * pb_desc describes.
 */
int
profile_match(const corba_profile_desc_t *sv_profile,
              const corba_pb_desc_t *pb_desc);

/**
 * Return true if sv_profile describes a service that matches the problem that
 * pb and path describe.
 */
int
profile_match(const corba_profile_desc_t *sv_profile,
              const char *path, const corba_profile_t *pb);

#endif  // _DIET_DATA_INTERNAL_HH_
