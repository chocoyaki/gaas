/****************************************************************************/
/* DIET data internal specification                                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2008/09/10 09:04:27  bisnard
 * new diet type for containers
 *
 * Revision 1.4  2006/11/16 09:55:55  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.3  2005/04/08 13:08:12  hdail
 * Remove very old sections of code surrounded by DEVELOPPING_DATA_PERSISTENCY
 * but where top of file has included #define DEVELOPPING_DATA_PERSISTENCY 0
 * for a long time.  Verified with Bruno that this is not his code.
 *
 * Revision 1.2  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.1.2.1  2004/11/24 09:30:15  alsu
 * - adding new datatype DIET_PARAMSTRING, which allows users to define
 *   strings for which the value is important for performance evaluation
 *   (and which is consequently stored in the argument description, much
 *   like what is done for DIET_SCALAR arguments)
 * - adding functions to access the type-specific data structures stored
 *   in the diet_data_desc_t.specific union (for use in custom
 *   performance metrics to access data such as those that are described
 *   above)
 *
 * Revision 1.1  2003/12/01 14:49:31  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 ****************************************************************************/

/**
 * Here are the data manipulation functions that must remain hidden to the
 * common user.
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
macro_data_sizeof(const diet_data_desc_t* desc);

/** Compute data size in bytes from their descriptor. */
size_t
data_sizeof(const diet_data_desc_t* desc);

/** Compute data size in bytes from their descriptor. */
size_t
data_sizeof(const corba_data_desc_t* desc);

/**
 * Alter a scalar descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
scalar_set_desc(diet_data_desc_t* desc, char* const id,
                const diet_persistence_mode_t mode,
                const diet_base_type_t base_type, void* const value);
/**
 * Alter a vector descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
vector_set_desc(diet_data_desc_t* desc, char* const id,
                const diet_persistence_mode_t mode,
                const diet_base_type_t base_type, const size_t size);
/**
 * Alter a matrix descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
matrix_set_desc(diet_data_desc_t* desc, char* const id,
                const diet_persistence_mode_t mode,
                const diet_base_type_t base_type, const size_t nb_r,
                const size_t nb_c, const diet_matrix_order_t order);
/**
 * Alter a string descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
string_set_desc(diet_data_desc_t* desc, char* const id,
                const diet_persistence_mode_t mode, const size_t length);

int
paramstring_set_desc(diet_data_desc_t* desc,
                     char* const id,
                     const diet_persistence_mode_t mode,
                     const size_t length,
                     const char* const param);

/**
 * Alter a file descriptor. Also computes the file size ...
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
file_set_desc(diet_data_desc_t* desc, char* const id,
              const diet_persistence_mode_t mode, char* const path);

/**
 * Alter a container descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
container_set_desc(diet_data_desc_t* desc, char* const id,
                   const diet_persistence_mode_t mode, const size_t size);


/****************************************************************************/
/* Useful functions for profile manipulation                                */
/* <implemented in DIET_data.cc>                                            */
/****************************************************************************/

/** Return true if p1 is exactly identical to p2. */
int
profile_desc_match(const corba_profile_desc_t* p1,
                   const corba_profile_desc_t* p2);

/**
 * Return true if sv_profile describes a service that matches the problem that
 * pb_desc describes.
 */
int
profile_match(const corba_profile_desc_t* sv_profile,
              const corba_pb_desc_t*      pb_desc);

/**
 * Return true if sv_profile describes a service that matches the problem that
 * pb and path describe.
 */
int
profile_match(const corba_profile_desc_t* sv_profile,
              const char* path, const corba_profile_t* pb);

#endif // _DIET_DATA_INTERNAL_HH_
