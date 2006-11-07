/****************************************************************************/
/* Workflow example : a server computing the successor of an integer        */
/* In  : x1                                                                 */
/* Out : x4                                                                 */
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
mul_succ(diet_profile_t* pb)
{
  int * in = NULL;
  int * out1 = NULL;
  int * out2 = NULL;
  int * out3 = NULL;
  int * out4 = NULL;

  fprintf(stderr, "SUCC(X4) SOLVING\n");
  
  diet_scalar_get(diet_parameter(pb,0), &in, NULL);
  diet_scalar_get(diet_parameter(pb,1), &out1, NULL);
  diet_scalar_get(diet_parameter(pb,2), &out2, NULL);
  diet_scalar_get(diet_parameter(pb,3), &out3, NULL);
  diet_scalar_get(diet_parameter(pb,4), &out4, NULL);

  fprintf(stderr, "in = %d\n", *(int*)in);

  *out1 = *in + 1;
  *out2 = *in + 1;
  *out3 = *in + 1;
  *out4 = *in + 1;

  diet_scalar_desc_set(diet_parameter(pb,1), out1);
  diet_scalar_desc_set(diet_parameter(pb,2), out2);
  diet_scalar_desc_set(diet_parameter(pb,3), out3);
  diet_scalar_desc_set(diet_parameter(pb,4), out4);

  return 0;
}

int main(int argc, char * argv[]) {
  int res;
  diet_profile_desc_t* profile = NULL;

  diet_service_table_init(1);
  profile = diet_profile_desc_alloc("mul_succ", 0, 0, 4);

  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,3), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,4), DIET_SCALAR, DIET_INT);
  if (diet_service_table_add(profile, NULL, mul_succ)) return 1;

  diet_profile_desc_free(profile);

  diet_print_service_table();

  res = diet_SeD(argv[1], argc, argv);

  return 0;
}
