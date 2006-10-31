/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* Collecting Ressource Information with FAST                               */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2006/10/31 23:18:33  ecaron
 * Correct wrong header
 *
 ****************************************************************************/

#ifndef _CORI_FAST_HH__
#define _CORI_FAST_HH__

#include "DIET_data.h"
#include "ServiceTable.hh"

typedef struct commTime_t{
  char* host1;
  char* host2;
  unsigned long size;
  bool to;
};

typedef struct fast_param_t{
  diet_profile_t* initprofilePtr;
  ServiceTable* SRVT;	
};

class Cori_Fast{
  
public:
  Cori_Fast();

int 
get_Information(int type_Info,       
		estVector_t* info,
		const void * data);

int
start();
 
private:
int
diet_service_table_lookup_by_profile(const diet_profile_t* const profile,
				     ServiceTable* SRVT );
};

#endif //_CORI_FAST_HH__
