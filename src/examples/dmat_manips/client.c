/****************************************************************************/
/* $Id$                */
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
 * Revision 1.2  2002/05/17 20:35:18  pcombes
 * Version alpha without FAST
 * */


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


/* argv[1]: client config file path
   argv[2]: trans, rsum, or rprod   */

int
main(int argc, char **argv)
{
  char *path;
  sf_pb_desc_t pb;
  sf_data_desc_t *tmp;
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
  
  pb.path = malloc(max_pb_length*sizeof(char));     /* problem name */
  pb.param_desc = malloc(3*sizeof(sf_data_desc_t)); 

  A = mat1;
  B = mat2;

  // Filling pb desc and data

  if (!strcmp(path, PB[0])) {
    
    strcpy(pb.path, path);
    pb.last_in    = -1;
    pb.last_inout = 0;
    pb.last_out   = 0;
    tmp = &(pb.param_desc[0]);
    tmp->id = 0;
    tmp->type = sf_type_cons_mat;
    tmp->base_type = sf_type_base_double;
    tmp->ctn.mat.nb_l  = 3;
    tmp->ctn.mat.nb_c  = 2;
    tmp->ctn.mat.trans = 0;

    in_data.length = 0;

    inout_data.length = 1;
    inout_data.seq = malloc(sizeof(diet_data_t));
    inout_data.seq[0].value = A;
    tmp = &(inout_data.seq[0].desc);
    tmp->id = 1;
    tmp->type = sf_type_cons_mat;
    tmp->base_type = sf_type_base_double;
    tmp->ctn.mat.nb_l  = 3;
    tmp->ctn.mat.nb_c  = 2;
    tmp->ctn.mat.trans = 0;
    print_matrix(A,
		 inout_data.seq[0].desc.ctn.mat.nb_l,
		 inout_data.seq[0].desc.ctn.mat.nb_c);

    out_data.length = 0;

  } else if (!strcmp(path, PB[1])) {
    size_t i;
    
    strcpy(pb.path, path);
    pb.last_in    = 1;
    pb.last_inout = 1;
    pb.last_out   = 2;
    for (i = 0; i < 3; i++) {
      tmp = &(pb.param_desc[i]);
      tmp->id = 0;
      tmp->type = sf_type_cons_mat;
      tmp->base_type = sf_type_base_double;
      tmp->ctn.mat.nb_l  = 3;
      tmp->ctn.mat.nb_c  = 2;
      tmp->ctn.mat.trans = 0;
    }

    in_data.length = 2;
    in_data.seq = malloc(2*sizeof(diet_data_t));
    in_data.seq[0].value = A;
    tmp = &(in_data.seq[0].desc);
    tmp->id = 1;
    tmp->type = sf_type_cons_mat;
    tmp->base_type = sf_type_base_double;
    tmp->ctn.mat.nb_l  = 3;
    tmp->ctn.mat.nb_c  = 2;
    tmp->ctn.mat.trans = 0;
    print_matrix(A,
		 in_data.seq[0].desc.ctn.mat.nb_l,
		 in_data.seq[0].desc.ctn.mat.nb_c);
    in_data.seq[1].value = B;
    tmp = &(in_data.seq[1].desc);
    tmp->id = 2;
    tmp->type = sf_type_cons_mat;
    tmp->base_type = sf_type_base_double;
    tmp->ctn.mat.nb_l  = 3;
    tmp->ctn.mat.nb_c  = 2;
    tmp->ctn.mat.trans = 0;
    print_matrix(B,
		 in_data.seq[1].desc.ctn.mat.nb_l,
		 in_data.seq[1].desc.ctn.mat.nb_c);

    inout_data.length = 0;
    
    out_data.length = 1;
    out_data.seq = malloc(sizeof(diet_data_t));
    out_data.seq[0].value = C;
    tmp = &(out_data.seq[0].desc);
    tmp->id = 1;
    tmp->type = sf_type_cons_mat;
    tmp->base_type = sf_type_base_double;
    tmp->ctn.mat.nb_l  = 3;
    tmp->ctn.mat.nb_c  = 2;
    tmp->ctn.mat.trans = 0;
    
  } else {
    size_t i;

    if (!strcmp(path, PB[2])) {
      strcpy(pb.path, path);
    } else {
      // this is unknown problem
      strncpy(pb.path, path, max_pb_length - 1);
      pb.path[max_pb_length] = '\0';
    }
    pb.last_in    = 1;
    pb.last_inout = 1;
    pb.last_out   = 2;
    for (i = 0; i < 3; i++) {
      tmp = &(pb.param_desc[i]);
      tmp->id = 0;
      tmp->type = sf_type_cons_mat;
      tmp->base_type = sf_type_base_double;
      tmp->ctn.mat.trans = 0;
    }
    pb.param_desc[0].ctn.mat.nb_l  = 3;
    pb.param_desc[0].ctn.mat.nb_c  = 2;
    pb.param_desc[1].ctn.mat.nb_l  = 2;
    pb.param_desc[1].ctn.mat.nb_c  = 3;
    pb.param_desc[2].ctn.mat.nb_l  = 3;
    pb.param_desc[2].ctn.mat.nb_c  = 3;

    in_data.length = 2;
    in_data.seq = malloc(2*sizeof(diet_data_t));
    in_data.seq[0].value = A;
    tmp = &(in_data.seq[0].desc);
    tmp->id = 1;
    tmp->type = sf_type_cons_mat;
    tmp->base_type = sf_type_base_double;
    tmp->ctn.mat.nb_l  = 3;
    tmp->ctn.mat.nb_c  = 2;
    tmp->ctn.mat.trans = 0;
    print_matrix(A,
		 in_data.seq[0].desc.ctn.mat.nb_l,
		 in_data.seq[0].desc.ctn.mat.nb_c);
    in_data.seq[1].value = B;
    tmp = &(in_data.seq[1].desc);
    tmp->id = 2;
    tmp->type = sf_type_cons_mat;
    tmp->base_type = sf_type_base_double;
    tmp->ctn.mat.nb_l  = 2;
    tmp->ctn.mat.nb_c  = 3;
    tmp->ctn.mat.trans = 0;
    print_matrix(B,
		 in_data.seq[1].desc.ctn.mat.nb_l,
		 in_data.seq[1].desc.ctn.mat.nb_c);
    
    inout_data.length = 0;
    
    out_data.length = 0;
    out_data.seq = malloc(sizeof(diet_data_t));
    out_data.seq[0].value = C;
    tmp = &(out_data.seq[0].desc);
    tmp->id = 1;
    tmp->type = sf_type_cons_mat;
    tmp->base_type = sf_type_base_double;
    tmp->ctn.mat.nb_l  = 3;
    tmp->ctn.mat.nb_c  = 3;
    tmp->ctn.mat.trans = 0;

  }
    
  if (! DIET_client(3, configfile, &pb, &in_data, &inout_data, &out_data)) {
    if (!strcmp(path, PB[0])) {
      print_matrix(A,
		   inout_data.seq[0].desc.ctn.mat.nb_l,
		   inout_data.seq[0].desc.ctn.mat.nb_c);
      print_matrix(mat1, 2, 3);
    } else {
      C = out_data.seq[0].value;
      print_matrix(C,
		   out_data.seq[0].desc.ctn.mat.nb_l,
		   out_data.seq[0].desc.ctn.mat.nb_c);
    }
  }
  
  if (!strcmp(path, PB[0])) {
    free(inout_data.seq);
  } else {
    free(in_data.seq);
    free(out_data.seq);
  }
  free(pb.param_desc);     

  return 0;
}

