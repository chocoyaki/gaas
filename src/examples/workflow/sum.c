/****************************************************************************/
/* Workflow example : a server computing the addition of two integers       */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $@Id$
 * $@Log$
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
sum(diet_profile_t* pb)
{
  fprintf(stderr, "SUM SOLVING\n");
  
  int * ix = NULL;
  int * jx = NULL;
  int * kx = NULL;

  diet_scalar_get(diet_parameter(pb,0), &ix, NULL);
  diet_scalar_get(diet_parameter(pb,1), &jx, NULL);
  diet_scalar_get(diet_parameter(pb,2), &kx, NULL);
  fprintf(stderr, "i = %d, j =%d\n", *(int*)ix, *(int*)jx);
  *(int*)kx = *(int*)ix + *(int*)jx;
  fprintf(stderr, "k = i + j = %d\n", *(int*)kx);
  diet_scalar_desc_set(diet_parameter(pb,2), kx);
  return 0;
}

int main(int argc, char * argv[]) {
  int res;
  diet_profile_desc_t* profile = NULL;

  diet_service_table_init(1);
  profile = diet_profile_desc_alloc("sum", 1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_INT);
  if (diet_service_table_add(profile, NULL, sum)) return 1;

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return 0;
}