/**
* @file scalars_server.c
* 
* @brief  DIET scalars example: a server for additions of all types of scalars.   
* 
* @author  - Philippe COMBES (Philippe.Combes@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.1  2010/08/06 14:25:30  glemahec
 * Cmake corrections + uuid lib module + fPIC error control
 *
 * Revision 1.4  2010/03/23 12:44:19  glemahec
 * Correction des exemples pour DAGDA
 *
 * Revision 1.3  2006/06/30 15:26:03  ycaniou
 * C++ commentaries -> C commentaries to remove compilation warnings
 *
 * Revision 1.2  2003/09/27 07:53:46  pcombes
 * Fix bugs on floating types
 *
 * Revision 1.1  2003/09/26 14:07:33  pcombes
 * Add an example devoted to the management of scalars (and their encoding length).
 ****************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DIET_server.h"

/* This server offers all additions of two scalars:                  */
/*   - CADD = sum of two chars                                       */
/*   - BADD = sum of two bytes                                       */
/*   - IADD = sum of two ints                                        */
/*   - LADD = sum of two longs                                       */
/*   - FADD = sum of two floats                                      */
/*   - DADD = sum of two doubles                                     */
#define NB_SRV 6
static const char* SRV[NB_SRV] =
  {"CADD", "BADD", "IADD", "LADD", "FADD", "DADD"};


/*
 * SOLVE FUNCTION
 */

int
solve_ADD(diet_profile_t* pb)
{
  int res = 0;

  printf("Solve ADD ...\n");

  if (*(pb->pb_name) == 'F' || *(pb->pb_name) == 'D') {
    switch ((diet_base_type_t)(diet_parameter(pb, 1))->desc.generic.base_type) {
    case DIET_FLOAT: {
      float*  f1 = NULL;
      float*  f2 = NULL;
      float*  f3 = NULL;
      diet_scalar_get(diet_parameter(pb, 0), &f1, NULL);
      diet_scalar_get(diet_parameter(pb, 1), &f2, NULL);
      diet_scalar_get(diet_parameter(pb, 2), &f3, NULL);
      printf("f1=%g, f2=%g -> ", *(float*)f1, *(float*)f2);
      *(float*)f3 = *(float*)f1 + *(float*)f2;
      *(float*)f2 = *(float*)f3;
      printf("f2=%g, f3=%g\n", *(float*)f2, *(float*)f3);
      diet_scalar_desc_set(diet_parameter(pb, 1), f2);
      diet_scalar_desc_set(diet_parameter(pb, 2), f3);
      break;
    }
    case DIET_DOUBLE: {
      double* d1 = NULL;
      double* d2 = NULL;
      double* d3 = NULL;
      diet_scalar_get(diet_parameter(pb, 0), &d1, NULL);
      diet_scalar_get(diet_parameter(pb, 1), &d2, NULL);
      diet_scalar_get(diet_parameter(pb, 2), &d3, NULL);
      printf("d1=%lg, d2=%lg -> ", *(double*)d1, *(double*)d2);
      *(double*)d3 = *(double*)d1 + *(double*)d2;
      *(double*)d2 = *(double*)d3;
      printf("d2=%lg, d3=%lg\n", *(double*)d2, *(double*)d3);
      diet_scalar_desc_set(diet_parameter(pb, 1), d2);
      diet_scalar_desc_set(diet_parameter(pb, 2), d3);
      break;
    }
    default:
      res = 1;
    }
  } else {
    /* For integers, we can use the longest type to store values 
       of smaller types. */
    long* l1 = NULL;
    long* l2 = NULL;
    long* l3 = NULL;
    diet_scalar_get(diet_parameter(pb, 0), &l1, NULL);
    diet_scalar_get(diet_parameter(pb, 1), &l2, NULL);
    diet_scalar_get(diet_parameter(pb, 2), &l3, NULL);
    switch ((diet_base_type_t)(diet_parameter(pb, 1))->desc.generic.base_type) {
    case DIET_CHAR:
      printf("l1 = 0x%hhX, l2 = 0x%hhX -> ", *(char*)l1, *(char*)l2);
      *(char*)l3 = *(char*)l1 + *(char*)l2;
      *(char*)l2 = *(char*)l3;
      printf("l2 = 0x%hhX, l3 = 0x%hhX\n", *(char*)l2, *(char*)l3);
      break;
    case DIET_SHORT:
      printf("l1 = 0x%hX, l2 = 0x%hX -> ", *(short*)l1, *(short*)l2);
      *(short*)l3 = *(short*)l1 + *(short*)l2;
      *(short*)l2 = *(short*)l3;
      printf("l2 = 0x%hX, l3 = 0x%hX\n", *(short*)l2, *(short*)l3);
      break;
    case DIET_INT:
      printf("l1 = 0x%X, l2 = 0x%X -> ", *(int*)l1, *(int*)l2);
      *(int*)l3 = *(int*)l1 + *(int*)l2;
      *(int*)l2 = *(int*)l3;
      printf("l2 = 0x%X, l3 = 0x%X\n", *(int*)l2, *(int*)l3);
      break;
    case DIET_LONGINT:
      printf("l1 = 0x%lX, l2 = 0x%lX -> ", *(long*)l1, *(long*)l2);
      *(long*)l3 = *(long*)l1 + *(long*)l2;
      *(long*)l2 = *(long*)l3;
      printf("l2 = 0x%lX, l3 = 0x%lX\n", *(long*)l2, *(long*)l3);
      break;
    default:
      res = 1;
    }
    diet_scalar_desc_set(diet_parameter(pb, 1), l2);
    diet_scalar_desc_set(diet_parameter(pb, 2), l3);
  }

  // diet_free_data(diet_parameter(pb, 0));

  printf("Solve ADD ... done\n");
  return res;
}

int
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg>\n", cmd);
  return 1;
}


/*
 * MAIN
 */

int
main(int argc, char* argv[])
{
  size_t i;
  int res;

  diet_profile_desc_t* profile = NULL;

  if (argc < 2) {
    return usage(argv[0]);
  }
  diet_service_table_init(NB_SRV);

  profile = diet_profile_desc_alloc("CADD", 0, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile, 1),
                        DIET_SCALAR, DIET_CHAR);
  diet_profile_desc_free(profile);

  printf("GLOP\n");

  for (i = 0; i < NB_SRV; i++) {
    profile = diet_profile_desc_alloc(SRV[i], 0, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0),
                          DIET_SCALAR, (diet_base_type_t)i);
    diet_generic_desc_set(diet_param_desc(profile, 1),
                          DIET_SCALAR, (diet_base_type_t)i);
    diet_generic_desc_set(diet_param_desc(profile, 2),
                          DIET_SCALAR, (diet_base_type_t)i);

    if (diet_service_table_add(profile, NULL, solve_ADD))
      return 1;
    diet_profile_desc_free(profile);
  }

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
}
