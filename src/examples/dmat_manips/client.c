/****************************************************************************/
/* $Id$ */
/* dmat_manips example: a DIET client for transpose, MatSUM and MatPROD     */
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
 * Revision 1.3  2002/05/24 19:36:53  pcombes
 * Add BLAS/dgemm example (implied bug fixes)
 *
 * Revision 1.2  2002/05/17 20:35:18  pcombes
 * Version alpha without FAST
 *
 ****************************************************************************/


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET.h"
#include "slimfast_api_local.h"


#define print_matrix(mat, m, n)            \
  {                                        \
    size_t i, j;                           \
    printf("%s = \n", #mat);               \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
	printf("%3f ", (mat)[i*(n) + j]);  \
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

enum {IN, INOUT, OUT};

#define init_data_seq(data_seq, pb, type)                           \
  {                                                                 \
    int i;                                                          \
    (data_seq).length=((type) == IN)                                \
                       ? (pb).last_in + 1                           \
                       : ((type) == INOUT)                          \
                         ? (pb).last_inout - (pb).last_in           \
                         : (pb).last_out - (pb).last_inout;         \
    if ((data_seq).length > 0)                                      \
      (data_seq).seq=malloc((data_seq).length*sizeof(diet_data_t)); \
    else                                                            \
      (data_seq).seq = NULL;                                        \
    for (i = 0; i < (data_seq).length; i++) {                       \
      int j = ((type) == IN) ? i : ((type) == INOUT)                \
                                   ? (pb).last_in + 1 + i           \
                                   : (pb).last_inout + 1 + i;       \
      (data_seq).seq[i].desc    = (pb).param_desc[j];               \
      (data_seq).seq[i].desc.id = j + 1;                            \
    }                                                               \
  }


/* argv[1]: client config file path
   argv[2]: trans, rsum, or rprod   */

int
main(int argc, char **argv)
{
  char *path;
  sf_pb_desc_t pb;
  diet_data_seq_t in_data, inout_data, out_data;
 
  double mat1[6] = {1.0,2.0,3.0,4.0,5.0,6.0};
  double mat2[6] = {7.0,8.0,9.0,10.0,11.0,12.0};
  double *A, *B, *C;

  char *configfile[3] = {"", "MASTER", "nowhere.com"};
  char *PB[3] = {"T", "MatSUM", "MatPROD"};
  size_t max_pb_length = 8;
  

  if (argc != 3) {
    fprintf(stderr, "Usage: client <file> [T|MatSUM|MatPROD]\n");
    return 1;
  }
  path = argv[2];
  
  A = mat1;
  B = mat2;

  // Filling pb desc and data

  if (!strcmp(path, PB[0])) {
    
    pb.path       = strdup(path);
    pb.last_in    = -1;
    pb.last_inout = 0;
    pb.last_out   = 0;
    pb.code_desc  = NULL;
    pb.param_desc = malloc((pb.last_out + 1) * sizeof(sf_data_desc_t)); 
    pb.param_desc[0].id            = 0;
    pb.param_desc[0].type          = sf_type_cons_mat;
    pb.param_desc[0].base_type     = sf_type_base_double;
    pb.param_desc[0].ctn.mat.nb_l  = 3;
    pb.param_desc[0].ctn.mat.nb_c  = 2;
    pb.param_desc[0].ctn.mat.trans = 0;
    
    // in_data.length = 0
    init_data_seq(in_data, pb, IN);

    // inout_data.length = 1
    init_data_seq(inout_data, pb, INOUT);
    inout_data.seq[0].value = A;
    print_matrix(A,
		 inout_data.seq[0].desc.ctn.mat.nb_l,
		 inout_data.seq[0].desc.ctn.mat.nb_c);

    // out_data.length = 0
    init_data_seq(out_data, pb, OUT);

  } else if (!strcmp(path, PB[1])) {
    size_t i;
    
    pb.path       = strdup(path);
    pb.last_in    = 1;
    pb.last_inout = 1;
    pb.last_out   = 2;
    pb.param_desc = malloc((pb.last_out + 1) * sizeof(sf_data_desc_t)); 
    for (i = 0; i < 3; i++) {
      pb.param_desc[i].id            = 0;
      pb.param_desc[i].type          = sf_type_cons_mat;
      pb.param_desc[i].base_type     = sf_type_base_double;
      pb.param_desc[i].ctn.mat.nb_l  = 3;
      pb.param_desc[i].ctn.mat.nb_c  = 2;
      pb.param_desc[i].ctn.mat.trans = 0;
    }

    // in_data.length = 2;
    init_data_seq(in_data, pb, IN);
    in_data.seq[0].value = A;
    print_matrix(A,
		 in_data.seq[0].desc.ctn.mat.nb_l,
		 in_data.seq[0].desc.ctn.mat.nb_c);
    in_data.seq[1].value   = B;
    print_matrix(B,
		 in_data.seq[1].desc.ctn.mat.nb_l,
		 in_data.seq[1].desc.ctn.mat.nb_c);

    // inout_data.length = 0;
    init_data_seq(inout_data, pb, INOUT);
    
    // out_data.length = 1;
    init_data_seq(out_data, pb, OUT);

  } else {
    size_t i;

    pb.path       = strdup(path); // ! this can be unknown problem
    pb.last_in    = 1;
    pb.last_inout = 1;
    pb.last_out   = 2;
    pb.param_desc = malloc((pb.last_out + 1) * sizeof(sf_data_desc_t)); 
    for (i = 0; i < 3; i++) {
      pb.param_desc[i].id            = 0;
      pb.param_desc[i].type          = sf_type_cons_mat;
      pb.param_desc[i].base_type     = sf_type_base_double;
      pb.param_desc[i].ctn.mat.trans = 0;
    }
    pb.param_desc[0].ctn.mat.nb_l  = 3;
    pb.param_desc[0].ctn.mat.nb_c  = 2;
    pb.param_desc[1].ctn.mat.nb_l  = 2;
    pb.param_desc[1].ctn.mat.nb_c  = 3;
    pb.param_desc[2].ctn.mat.nb_l  = 3;
    pb.param_desc[2].ctn.mat.nb_c  = 3;

    // in_data.length = 2;
    init_data_seq(in_data, pb, IN);
    in_data.seq[0].value   = A;
    print_matrix(A,
		 in_data.seq[0].desc.ctn.mat.nb_l,
		 in_data.seq[0].desc.ctn.mat.nb_c);
    in_data.seq[1].value   = B;
    print_matrix(B,
		 in_data.seq[1].desc.ctn.mat.nb_l,
		 in_data.seq[1].desc.ctn.mat.nb_c);

    // inout_data.length = 0;
    init_data_seq(inout_data, pb, INOUT);
    
    // out_data.length = 1;
    init_data_seq(out_data, pb, OUT);
    
  }
    
  if (! DIET_client(3, configfile, &pb, &in_data, &inout_data, &out_data)) {
    if (!strcmp(path, PB[0])) {
      print_matrix(A,
		   inout_data.seq[0].desc.ctn.mat.nb_l,
		   inout_data.seq[0].desc.ctn.mat.nb_c);
    } else {
      C = out_data.seq[0].value;
      print_matrix(C,
		   out_data.seq[0].desc.ctn.mat.nb_l,
		   out_data.seq[0].desc.ctn.mat.nb_c);
    }
  }
  
  free(pb.path);
  free(pb.param_desc);  
  if (!strcmp(path, PB[0])) {
    free(inout_data.seq);
  } else {
    free(in_data.seq);
    free(out_data.seq);
  }

  return 0;
}

