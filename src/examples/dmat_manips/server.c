/****************************************************************************/
/* $Id$ */
/* dmat_manips example: a DIET server for transpose, RSUM a and RPROD       */
/*   problems                                                               */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.5  2002/08/30 16:50:16  pcombes
 * This version works as well as the alpha version from the user point of view,
 * but the API is now the one imposed by the latest specifications (GridRPC API
 * in its sequential part, config file for all parts of the platform, agent
 * algorithm, etc.)
 *  - Reduce marshalling by using CORBA types internally
 *  - Creation of a class ServiceTable that is to be replaced
 *    by an LDAP DB for the MA
 *  - No copy for client/SeD data transfers
 *  - ...
 *
 * Revision 1.4  2002/08/09 14:30:33  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 * Revision 1.3  2002/05/24 19:36:53  pcombes
 * Add BLAS/dgemm example (implied bug fixes)
 *
 * Revision 1.2  2002/05/17 20:35:18  pcombes
 * Version alpha without FAST
 *
 ****************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DIET_server.h"
#include "slimfast_api_local.h"


/*
 * SOLVE FUNCTIONS
 */


int
solve_T(diet_data_seq_t *in, diet_data_seq_t *inout, diet_data_seq_t *out)
{
  size_t i, j, m, n;
  double *A, *tmp;

  printf("Solve T ...");

  m = inout->seq[0].desc.specific.mat.nb_r;
  n = inout->seq[0].desc.specific.mat.nb_c;
  A = (double *) inout->seq[0].value;
  
  tmp = malloc(m*n*sizeof(double));
  memcpy(tmp, A, m*n*sizeof(double));
  
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      A[j*m + i] = tmp[i*n + j];
    }
  }
  
  inout->seq[0].desc.specific.mat.nb_r = n;
  inout->seq[0].desc.specific.mat.nb_c = m;
  //  out->length = 0;

  free(tmp);
  
  printf(" done\n");
  return 0;
}

int
solve_MatSUM(diet_data_seq_t *in, diet_data_seq_t *inout, diet_data_seq_t *out)
{
  size_t i, j, m, n;
  double *A, *B, *C;
  
  printf("Solve MatSUM ...");

  //  out->length = 1;
  out->seq->desc = in->seq[0].desc;
  out->seq->desc.specific.mat.istrans = 0;
  
  m = in->seq[0].desc.specific.mat.nb_r;
  n = in->seq[0].desc.specific.mat.nb_c;
  if ((m != in->seq[1].desc.specific.mat.nb_r)
      || (n != in->seq[1].desc.specific.mat.nb_c)) {
    fprintf(stderr, "MatSUM error: mA=%d, nA=%d ; mB=%d, nB=%d\n",
	    m, n, in->seq[1].desc.specific.mat.nb_r,
	    in->seq[1].desc.specific.mat.nb_c);
    return 1;
  }
  A = (double *) in->seq[0].value;
  B = (double *) in->seq[1].value;
  C = (double *) out->seq[0].value;

  if (in->seq[0].desc.specific.mat.istrans) {
    if (in->seq[1].desc.specific.mat.istrans) {
      for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	  C[i*n + j] = A[j*m + i] + B[j*m + i];
	}
      }
    } else {
      for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	  C[i*n + j] = A[j*m + i] + B[i*n + j];
	}
      }
    }
  } else {
    if (in->seq[1].desc.specific.mat.istrans) {
      for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	  C[i*n + j] = A[i*n + j] + B[j*m + i];
	}
      }
    } else {
      for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	  C[i*n + j] = A[i*n + j] + B[i*n + j];
	}
      }
    }
  }
  
  printf(" done\n");
  return 0;
}


int
solve_MatPROD(diet_data_seq_t *in, diet_data_seq_t *inout, diet_data_seq_t *out)
{
  size_t i, j, k, mA, nA, mB, nB;
  double *A, *B, *C;
  
  printf("Solve MatPROD ...");

  mA = in->seq[0].desc.specific.mat.nb_r;
  nA = in->seq[0].desc.specific.mat.nb_c;
  mB = in->seq[1].desc.specific.mat.nb_r;
  nB = in->seq[1].desc.specific.mat.nb_c;
  if (nA != mB) {
    fprintf(stderr, "MatPROD error: mA=%d, nA=%d ; mB=%d, nB=%d\n", mA, nA, mB, nB);
    return 1;
  }

  //  out->length = 1;
  out->seq->desc = in->seq[0].desc;
  out->seq->desc.specific.mat.nb_c  = nB;
  out->seq->desc.specific.mat.istrans = 0;
  
  A = (double *) in->seq[0].value;
  B = (double *) in->seq[1].value;
  C = (double *) out->seq[0].value;
  
  for (i = 0; i < mA; i++) {
    for (j = 0; j < nB; j++) {
      C[i*nB + j] = 0;
      for (k = 0; k < nA; k++) {
	C[i*nB + j] += A[i*nA + k] * B[k*nB + j];
      }
    }
  }
  
  //  out->seq->value = C;
  //  out->seq->to_be_freed = 1;
  
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
  char *SRV[3]    = {"T", "MatSUM", "MatPROD"};
  int services[3];

  diet_profile_desc_t *profile;
  

  if (argc < 3) {
    fprintf(stderr, "Usage: SeD <file> [T][MatSUM][MatPROD]\n");
    return 1;
  }  
  
  for (i = 0; i < 3; i++) {
    services[i] = 0;
  }

  for (i = 2; i < argc; i++) {
    char *path = argv[i];
    if (!strcmp(SRV[0], path))
      services[0] = 1;
    else if (!strcmp(SRV[1], path))
      services[1] = 1;
    else if (!strcmp(SRV[2], path))
      services[2] = 1;
    else {
      fprintf(stderr, "Usage: SeD <file> [T][MatSUM][MatPROD]\n");
      exit(1);
    }
  }
  

  /* Brutal construction of fake service tables:  */
  /* This fake server can solve 3 pbs:            */
  /*   - T = matrix translation                   */
  /*   - MatSUM = matrix addition                 */
  /*   - MatPROD = matrix product                 */

  diet_service_table_init(3);
  
  if (services[0]) {
    profile = profile_desc_alloc(-1, 0, 0);
    generic_desc_set(&(profile->param_desc[0]), DIET_MATRIX, DIET_DOUBLE);
    diet_service_table_add(SRV[0], profile, solve_T);
    profile_desc_free(profile);
  }
  
  if (services[1] || services[2]) {
    profile = profile_desc_alloc(1, 1, 2);
    generic_desc_set(&(profile->param_desc[0]), DIET_MATRIX, DIET_DOUBLE);
    generic_desc_set(&(profile->param_desc[1]), DIET_MATRIX, DIET_DOUBLE);
    generic_desc_set(&(profile->param_desc[2]), DIET_MATRIX, DIET_DOUBLE);
    if (services[1])
      diet_service_table_add(SRV[1], profile, solve_MatSUM);
    if (services[2])
      diet_service_table_add(SRV[2], profile, solve_MatPROD);
    profile_desc_free(profile);
  }

  print_table();
  res = DIET_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}


