/****************************************************************************/
/* Header for the admin API                                                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (Benjamin.Depardon@ens-lyon.fr)                   */
/*    - Kevin COULOMB (kevin.coulomb@-sysfera.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

BEGIN_API

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
typedef enum DynamicType {
  SED = 0, /*!< The element is a DIET SeD */
  AGENT    /*!< The element is a DIET agent */
}DynamicType;

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
removeFromHierarchy(DynamicType type, char *name, int recursive);


END_API
