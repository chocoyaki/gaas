/****************************************************************************/
/* Header for the admin API                                                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (Benjamin.Depardon@ens-lyon.fr)                   */
/*    - Kevin COULOMB (kevin.coulomb@-sysfera.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

#ifndef _DIET_ADMIN_H_
#define _DIET_ADMIN_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//------------------------------------------------------------------------
// Error code
//------------------------------------------------------------------------

#define DIET_SUCCESS 	  0    
#define DIET_NAMING  	  1
#define DIET_NARROW  	  2    
#define DIET_COMM_FAILURE 3
#define DIET_OMNIFATAL    4
#define DIET_UNKNOWN      5
#define DIET_SED          6
#define DIET_AGENT        7
#define DIET_SYSTEM       8

//------------------------------------------------------------------------
// Datatypes
//------------------------------------------------------------------------


/**
 * \brief Enumeration to specify the type of the component to administrate
 */
typedef enum dynamic_type_t {
  SED = 0, /*!< The element is a SeD */
  AGENT,   /*!< The element is an agent */
  LA,      /*!< The element is a local agent */
  MA       /*!< The element is a master agent */
} dynamic_type_t;

//------------------------------------------------------------------------
// API
//------------------------------------------------------------------------

/**
 * \brief Remove a component from the hierarchy
 * \param type: The type of the element to disconnect
 * \param name: The name of the component to disconnect (as recorded in the naming service)
 * \param recursive: if 0, not recursive, otherwise the children are also removed
 * \param DIET_SUCCESS on success, an error code otherwise
 */
int
diet_remove_from_hierarchy(dynamic_type_t type, const char *name, int recursive);

/**
 * \brief Change the parent of a component in the hierarchy
 * \param type: The type of the element to change the parent of
 * \param name: The name of the component to change the parent of (as recorded in the naming service)
 * \param parent_name: the name of the new parent in the hierarchy (as recorded in the naming service)
 * \param DIET_SUCCESS on success, an error code otherwise
 */
int
diet_change_parent(dynamic_type_t type, const char *name, const char *parent_name);


/**
 * \brief Disconnect an element from its parent in the hierarchy
 * \param type: The type of the element to disconnect
 * \param name: The name of the component to disconnect (as recorded in the naming service)
 * \param DIET_SUCCESS on success, an error code otherwise
 */
int
diet_disconnect_from_hierarchy(dynamic_type_t type, const char *name);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DIET_ADMIN_H_ */
