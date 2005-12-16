/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* Collecting Ressource Information with FAST                               */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $@Id$
 * $@Log$
 ****************************************************************************/

#ifndef _CORI_FAST_HH__
#define _CORI_FAST_HH__

#include "DIET_data.h"
#include "ServiceTable.hh"
 
typedef struct fast_param_t{
  diet_profile_t* initprofilePtr;
  ServiceTable* SRVT;	
};

class Cori_Fast{
  
public:
  Cori_Fast();
int 
get_Information(diet_est_tag_t type_Info,       
		estVector_t* info,
		const diet_profile_t* const initprofilePtr,
		ServiceTable* SRVT);
int
diet_service_table_lookup_by_profile(const diet_profile_t* const profile,
				     ServiceTable* SRVT );
};

#endif //_CORI_FAST_HH__
