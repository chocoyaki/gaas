/****************************************************************************/
/* Workflow example : a server computing the addition of two integers       */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2006/11/07 12:44:48  aamar
 * *** empty log message ***
 *
 ****************************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "DIET_server.h"


int
mul_sum(diet_profile_t* pb)
{
  int  *i1  = NULL;
  int  *i2  = NULL;
  int  *i3  = NULL;
  int  *i4  = NULL;
  int  *out = NULL;

  fprintf(stderr, "SUM(X4) SOLVING\n");

  diet_scalar_get(diet_parameter(pb,0), &i1, NULL);
  diet_scalar_get(diet_parameter(pb,1), &i2, NULL);
  diet_scalar_get(diet_parameter(pb,2), &i3, NULL);
  diet_scalar_get(diet_parameter(pb,3), &i4, NULL);
  diet_scalar_get(diet_parameter(pb,4), &out, NULL);
  *(int*)out = *(int*)i1 + *(int*)i2 + *(int*)i3 + *(int*)i4 ;
  fprintf(stderr, "out = i1 + i2 + i3 + i4 = %d\n", *(int*)out);
  diet_scalar_desc_set(diet_parameter(pb,4), out);
  return 0;
}

int main(int argc, char * argv[]) {
  int res;
  diet_profile_desc_t* profile = NULL;

  diet_service_table_init(1);
  profile = diet_profile_desc_alloc("mul_sum", 3, 3, 4);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,3), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,4), DIET_SCALAR, DIET_INT);

  if (diet_service_table_add(profile, NULL, mul_sum)) return 1;

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return 0;
}
