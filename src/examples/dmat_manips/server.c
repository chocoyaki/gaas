/****************************************************************************/
/* $Id$                */
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
 * Revision 1.2  2002/05/17 20:35:18  pcombes
 * Version alpha without FAST
 * */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DIET.h"
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

  m = inout->seq[0].desc.ctn.mat.nb_l;
  n = inout->seq[0].desc.ctn.mat.nb_c;
  
  A = (double *) inout->seq[0].value;
  
  tmp = malloc(m*n*sizeof(double));
  memcpy(tmp, A, m*n*sizeof(double));
  
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      A[j*m + i] = tmp[i*n + j];
    }
  }
  
  inout->seq[0].desc.ctn.mat.nb_l = n;
  inout->seq[0].desc.ctn.mat.nb_c = m;
  out->length = 0;

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

  out->length = 1;
  out->seq->desc = in->seq[0].desc;
  out->seq->desc.ctn.mat.trans = 0;
  
  m = in->seq[0].desc.ctn.mat.nb_l;
  n = in->seq[0].desc.ctn.mat.nb_c;
  if ((m != in->seq[1].desc.ctn.mat.nb_l) || (n != in->seq[1].desc.ctn.mat.nb_c)) {
    fprintf(stderr, "MatSUM error: mA=%d, nA=%d ; mB=%d, nB=%d\n",
	    m, n, in->seq[1].desc.ctn.mat.nb_l, in->seq[1].desc.ctn.mat.nb_c);
    return 1;
  }
  A = (double *) in->seq[0].value;
  B = (double *) in->seq[1].value;

  /* Result matrix allocation */
  C = malloc(m*n*sizeof(double));
  
  if (in->seq[0].desc.ctn.mat.trans) {
    if (in->seq[1].desc.ctn.mat.trans) {
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
    if (in->seq[1].desc.ctn.mat.trans) {
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
  
  out->seq->value = C;
  out->seq->to_be_freed = 1;

  printf(" done\n");
  return 0;
}


int
solve_MatPROD(diet_data_seq_t *in, diet_data_seq_t *inout, diet_data_seq_t *out)
{
  size_t i, j, k, mA, nA, mB, nB;
  double *A, *B, *C;
  
  printf("Solve MatPROD ...");

  mA = in->seq[0].desc.ctn.mat.nb_l;
  nA = in->seq[0].desc.ctn.mat.nb_c;
  mB = in->seq[1].desc.ctn.mat.nb_l;
  nB = in->seq[1].desc.ctn.mat.nb_c;
  if (nA != mB) {
    fprintf(stderr, "MatPROD error: mA=%d, nA=%d ; mB=%d, nB=%d\n", mA, nA, mB, nB);
    return 1;
  }

  out->length = 1;
  out->seq->desc = in->seq[0].desc;
  out->seq->desc.ctn.mat.nb_c  = nB;
  out->seq->desc.ctn.mat.trans = 0;
  
  A = (double *) in->seq[0].value;
  B = (double *) in->seq[1].value;

  /* Result matrix allocation */
  C = malloc(mA*nB*sizeof(double));
  
  for (i = 0; i < mA; i++) {
    for (j = 0; j < nB; j++) {
      C[i*nB + j] = 0;
      for (k = 0; k < nA; k++) {
	C[i*nB + j] += A[i*nA + k] * B[k*nB + j];
      }
    }
  }
  
  out->seq->value = C;
  out->seq->to_be_freed = 1;
  
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
  int nb_insts;                
  sf_inst_desc_t *instTable; /* Instances */
  int nb_pbs, pb_no;
  sf_pb_desc_t  *pbTable;    /* Problemes */
  solve_function_t *solveTable;

  char *configfile[3] = {"", "nowhere.com", "MASTER"};
  char *PB[3]    = {"T", "MatSUM", "MatPROD"};
  int max_pb_length = 8;
  int solved_pbs[3];


  if (argc < 3) {
    fprintf(stderr, "Usage: SeD <file> [T][MatSUM][MatPROD]\n");
    return 1;
  }  
  
  for (i = 0; i < 3; i++) {
    solved_pbs[i] = 0;
  }

  for (i = 2; i < argc; i++) {
    char *path = argv[i];
    if (!strcmp(PB[0], path))
      solved_pbs[0] = 1;
    else if (!strcmp(PB[1], path))
      solved_pbs[1] = 1;
    else if (!strcmp(PB[2], path))
      solved_pbs[2] = 1;
    else {
      fprintf(stderr, "Usage: SeD <file> [T][MatSUM][MatPROD]\n");
      exit(1);
    }
  }
  nb_pbs = 0;
  for (i = 0; i < 3; i++) {
    if (solved_pbs[i])
      nb_pbs++;
  }
  

  /* Brutal construction of fake pb/inst tables:  */
  /* This fake server can solve 3 pbs:            */
  /*   - T = matrix translation                   */
  /*   - MatSUM = matrix addition                 */
  /*   - MatPROD = matrix product                 */
  /* For now, instTable and pbTable are similar.  */
  /* This should change when slim is in function. */

  pbTable = malloc(nb_pbs*sizeof(sf_pb_desc_t));
  nb_insts = nb_pbs;
  instTable = malloc(nb_insts*sizeof(sf_inst_desc_t));
  solveTable = malloc(nb_insts*sizeof(solve_function_t));
  
  pb_no = 0;
  
  if (solved_pbs[0]) {
    
    pbTable[pb_no].path = malloc(max_pb_length*sizeof(char));
    strcpy(pbTable[pb_no].path, PB[0]);
    pbTable[pb_no].last_in    = -1;
    pbTable[pb_no].last_inout = 0;
    pbTable[pb_no].last_out   = 0;
    pbTable[pb_no].param_desc = malloc(sizeof(sf_data_desc_t));
    pbTable[pb_no].code_desc  = NULL;
    (pbTable[pb_no].param_desc[0]).id = 0;
    (pbTable[pb_no].param_desc[0]).type = sf_type_cons_mat;
    (pbTable[pb_no].param_desc[0]).base_type = sf_type_base_double;
    (pbTable[pb_no].param_desc[0]).ctn.mat.nb_c  = 0;
    (pbTable[pb_no].param_desc[0]).ctn.mat.nb_l  = 0;
    (pbTable[pb_no].param_desc[0]).ctn.mat.trans = 0;

    instTable[pb_no].path       = strdup(pbTable[pb_no].path);
    instTable[pb_no].last_in    = pbTable[pb_no].last_in;
    instTable[pb_no].last_inout = pbTable[pb_no].last_inout;
    instTable[pb_no].last_out   = pbTable[pb_no].last_out;
    instTable[pb_no].param_desc = pbTable[pb_no].param_desc;

    solveTable[pb_no] = solve_T;

    pb_no++;
  }
  
  if (solved_pbs[1]) {
    
    pbTable[pb_no].path = malloc(max_pb_length*sizeof(char));
    strcpy(pbTable[pb_no].path, PB[1]);
    pbTable[pb_no].last_in    = 1;
    pbTable[pb_no].last_inout = 1;
    pbTable[pb_no].last_out   = 2;
    pbTable[pb_no].code_desc  = NULL;
    pbTable[pb_no].param_desc = malloc(3*sizeof(sf_data_desc_t));
    (pbTable[pb_no].param_desc[0]).id = 0;
    (pbTable[pb_no].param_desc[0]).type = sf_type_cons_mat;
    (pbTable[pb_no].param_desc[0]).base_type = sf_type_base_double;
    (pbTable[pb_no].param_desc[0]).ctn.mat.nb_l  = 0;
    (pbTable[pb_no].param_desc[0]).ctn.mat.nb_c  = 0;
    (pbTable[pb_no].param_desc[0]).ctn.mat.trans = 0;
    (pbTable[pb_no].param_desc[1]).id = 0;
    (pbTable[pb_no].param_desc[1]).type = sf_type_cons_mat;
    (pbTable[pb_no].param_desc[1]).base_type = sf_type_base_double;
    (pbTable[pb_no].param_desc[1]).ctn.mat.nb_l  = 0;
    (pbTable[pb_no].param_desc[1]).ctn.mat.nb_c  = 0;
    (pbTable[pb_no].param_desc[1]).ctn.mat.trans = 0;
    (pbTable[pb_no].param_desc[2]).id = 0;
    (pbTable[pb_no].param_desc[2]).type = sf_type_cons_mat;
    (pbTable[pb_no].param_desc[2]).base_type = sf_type_base_double;
    (pbTable[pb_no].param_desc[2]).ctn.mat.nb_l  = 0;
    (pbTable[pb_no].param_desc[2]).ctn.mat.nb_c  = 0;
    (pbTable[pb_no].param_desc[2]).ctn.mat.trans = 0;
  
    instTable[pb_no].path       = strdup(pbTable[pb_no].path);
    instTable[pb_no].last_in    = pbTable[pb_no].last_in;
    instTable[pb_no].last_inout = pbTable[pb_no].last_inout;
    instTable[pb_no].last_out   = pbTable[pb_no].last_out;
    instTable[pb_no].param_desc = pbTable[pb_no].param_desc;

    solveTable[pb_no] = (solve_function_t) solve_MatSUM;

    pb_no++;
  }
  
  if (solved_pbs[2]) {
    
    pbTable[pb_no].path = malloc(max_pb_length*sizeof(char));
    strcpy(pbTable[pb_no].path, PB[2]);
    pbTable[pb_no].last_in    = 1;
    pbTable[pb_no].last_inout = 1;
    pbTable[pb_no].last_out   = 2;
    pbTable[pb_no].code_desc  = NULL;
    pbTable[pb_no].param_desc = malloc(3*sizeof(sf_data_desc_t));
    (pbTable[pb_no].param_desc[0]).id = 0;
    (pbTable[pb_no].param_desc[0]).type = sf_type_cons_mat;
    (pbTable[pb_no].param_desc[0]).base_type = sf_type_base_double;
    (pbTable[pb_no].param_desc[0]).ctn.mat.nb_l  = 0;
    (pbTable[pb_no].param_desc[0]).ctn.mat.nb_c  = 0;
    (pbTable[pb_no].param_desc[0]).ctn.mat.trans = 0;
    (pbTable[pb_no].param_desc[1]).id = 0;
    (pbTable[pb_no].param_desc[1]).type = sf_type_cons_mat;
    (pbTable[pb_no].param_desc[1]).base_type = sf_type_base_double;
    (pbTable[pb_no].param_desc[1]).ctn.mat.nb_l  = 0;
    (pbTable[pb_no].param_desc[1]).ctn.mat.nb_c  = 0;
    (pbTable[pb_no].param_desc[1]).ctn.mat.trans = 0;
    (pbTable[pb_no].param_desc[2]).id = 0;
    (pbTable[pb_no].param_desc[2]).type = sf_type_cons_mat;
    (pbTable[pb_no].param_desc[2]).base_type = sf_type_base_double;
    (pbTable[pb_no].param_desc[2]).ctn.mat.nb_l  = 0;
    (pbTable[pb_no].param_desc[2]).ctn.mat.nb_c  = 0;
    (pbTable[pb_no].param_desc[2]).ctn.mat.trans = 0;

    instTable[pb_no].path       = strdup(pbTable[pb_no].path);
    instTable[pb_no].last_in    = pbTable[pb_no].last_in;
    instTable[pb_no].last_inout = pbTable[pb_no].last_inout;
    instTable[pb_no].last_out   = pbTable[pb_no].last_out;
    instTable[pb_no].param_desc = pbTable[pb_no].param_desc;
  
    solveTable[pb_no] = (solve_function_t) solve_MatPROD;

    pb_no++;
  }
  
  if (pb_no != nb_pbs) {  
    fprintf(stderr, "Program Error in server: pb_no=%d, nb_pbs=%d\n", pb_no, nb_pbs);
    exit(1);
  }

  return DIET_SeD(3, configfile, nb_pbs, pbTable, nb_insts, instTable, solveTable);

}


