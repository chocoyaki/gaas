/****************************************************************************/
/* $Id$ */
/* file_transfer example: server side                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.7  2003/01/17 18:05:37  pcombes
 * Update to API 0.6.3
 *
 * Revision 1.6  2002/12/12 18:17:05  pcombes
 * Small bug fixes on prints (special thanks to Jean-Yves)
 *
 * Revision 1.4  2002/10/25 11:29:21  pcombes
 * FAST support: convertors implemented and compatible to --without-fast
 *               configure option, but still not tested with FAST !
 *
 * Revision 1.3  2002/10/18 18:13:21  pcombes
 * Bug fixes for files in OUT parameters.
 *
 * Revision 1.2  2002/10/17 15:36:37  pcombes
 * Two files are transfered for size.
 *
 * Revision 1.1.1.1  2002/10/15 18:52:03  pcombes
 * Example of a file transfer: size of the file.
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

void displayArg(const diet_data_t *arg)
{
      
  switch((int) arg->desc.generic.type) {
  case DIET_SCALAR: fprintf(stdout, "scalar");                break;
  case DIET_VECTOR: fprintf(stdout, "vector (%d)",
			    arg->desc.specific.vect.size);    break;
  case DIET_MATRIX: fprintf(stdout, "matrix (%dx%d)",
			    arg->desc.specific.mat.nb_r,
			    arg->desc.specific.mat.nb_c);   break;
  case DIET_STRING: fprintf(stdout, "string (%d)",
			    arg->desc.specific.str.length); break;
  case DIET_FILE:   fprintf(stdout, "file (%d, %s)",
			    arg->desc.specific.file.size,
			    arg->desc.specific.file.path);  break;
  }
  if ((arg->desc.generic.type != DIET_STRING)
      && (arg->desc.generic.type != DIET_FILE)) {
    fprintf(stdout, " of ");
    switch ((int) arg->desc.generic.base_type) {
    case DIET_CHAR:     fprintf(stdout, "char");           break;
    case DIET_BYTE:     fprintf(stdout, "byte");           break;
    case DIET_INT:      fprintf(stdout, "int");            break;
    case DIET_LONGINT:  fprintf(stdout, "long int");       break;
    case DIET_FLOAT:    fprintf(stdout, "float");          break;
    case DIET_DOUBLE:   fprintf(stdout, "double");         break;
    case DIET_SCOMPLEX: fprintf(stdout, "float complex");  break;
    case DIET_DCOMPLEX: fprintf(stdout, "double complex"); break;
    }
  }
  fprintf(stdout, "\n");
}

/*
 * SOLVE FUNCTIONS
 */


int
solve_size(diet_profile_t *pb)
{
  size_t arg_size, comp_size;
  char *path;
  int status = 0;
  struct stat buf;

  fprintf(stderr, "Solve size ");
  
  diet_file_get(diet_parameter(pb,0), NULL, &arg_size, &path);
  fprintf(stderr, "on %s (%d) ", path, arg_size);
  if ((status = stat(path, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG))
    return 2;
  *(diet_value(size_t, diet_parameter(pb,2))) = (size_t) buf.st_size;
  
  diet_file_get(diet_parameter(pb,1), NULL, &arg_size, &path);
  fprintf(stderr, "and %s (%d) ...", path, arg_size);
  if ((status = stat(path, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG))
    return 2;
  /* This is equivalent to
     diet_scalar_desc_set(diet_parameter(pb,3), &buf.st_size); */
  *(diet_value(size_t, diet_parameter(pb,3))) = (size_t) buf.st_size;
  
  if (diet_file_desc_set(diet_parameter(pb,4), (rand() & 1) ? path : NULL)) {
    printf("diet_file_desc_set error\n");
    return 1;
  }
  printf(" done\n");
  return 0;
}


/*
 * MAIN
 */

int
main(int argc, char **argv)
{
  size_t i,j;
  int res;

  diet_profile_desc_t *profile;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }  

  /* This is for solve function (OUT parameter) */
  srand(time(NULL));


  diet_service_table_init(1);
  profile = diet_profile_desc_alloc(1, 1, 4);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,3), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,4), DIET_FILE, DIET_CHAR);
  diet_service_table_add("size", profile, NULL, solve_size);

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}


