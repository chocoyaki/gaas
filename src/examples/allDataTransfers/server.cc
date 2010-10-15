/****************************************************************************/
/* DIET all data transfers example: a server for transfering all data types */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin Depardon (Benjamin.Depardon@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/10/15 06:08:08  bdepardo
 * Client/Server to test all kinds of data transfers.
 * Each profile has 1 IN, 1 INOUT and 1 OUT all of the same type.
 * Currently are implemented:
 * - base types:
 *   DIET_CHAR, DIET_SHORT, DIET_INT, DIET_LONGINT, DIET_FLOAT and DIET_DOUBLE
 * - data types:
 *   DIET_SCALAR, DIET_VECTOR and DIET_MATRIX
 *
 ****************************************************************************/


#include <iostream>
#include <cstring>

#include "DIET_server.h"
#include "progs.hh"

/* This server offers all additions of two scalars:                  */
/*   - CADD = sum of two chars                                       */
/*   - BADD = sum of two bytes                                       */
/*   - IADD = sum of two ints                                        */
/*   - LADD = sum of two longs                                       */
/*   - FADD = sum of two floats                                      */
/*   - DADD = sum of two doubles                                     */
#define NB_SRV_SCALAR 6
static const char* SRV_SCALAR[NB_SRV_SCALAR] =
  {"CADD", "BADD", "IADD", "LADD", "FADD", "DADD"};

/* This server offers all additions of two vectors of scalars:       */
/*   - CVADD = sum of two chars                                      */
/*   - BVADD = sum of two bytes                                      */
/*   - IVADD = sum of two ints                                       */
/*   - LVADD = sum of two longs                                      */
/*   - FVADD = sum of two floats                                     */
/*   - DVADD = sum of two doubles                                    */
#define NB_SRV_VECTOR 6
static const char* SRV_VECTOR[NB_SRV_VECTOR] =
  {"CVADD", "BVADD", "IVADD", "LVADD", "FVADD", "DVADD"};

/* This server offers all additions of two matrices of scalars:      */
/*   - CMADD = sum of two chars                                      */
/*   - BMADD = sum of two bytes                                      */
/*   - IMADD = sum of two ints                                       */
/*   - LMADD = sum of two longs                                      */
/*   - FMADD = sum of two floats                                     */
/*   - DMADD = sum of two doubles                                    */
#define NB_SRV_MATRIX 6
static const char* SRV_MATRIX[NB_SRV_MATRIX] =
  {"CMADD", "BMADD", "IMADD", "LMADD", "FMADD", "DMADD"};


/*
 * Scalar solve function
 */
int
solve_ADD(diet_profile_t* pb) {
  int res = 0;

  std::cout << "Solve ADD ..." << std::endl;

  if (*(pb->pb_name) == 'F' || *(pb->pb_name) == 'D') {
    switch((diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type) {
    case DIET_FLOAT: {
      float*  f1 = NULL;
      float*  f2 = NULL;
      float*  f3 = NULL;
      diet_scalar_get(diet_parameter(pb,0), &f1, NULL);
      diet_scalar_get(diet_parameter(pb,1), &f2, NULL);
      diet_scalar_get(diet_parameter(pb,2), &f3, NULL);
      std::cout << "f1=" << *(float*)f1 << ", f2=" << *(float*)f2 << " -> ";
      *(float*)f3 = *(float*)f1 + *(float*)f2;
      *(float*)f2 = *(float*)f3;
      std::cout << "f2=" << *(float*)f2 << ", f3=" << *(float*)f3 << std::endl;
      diet_scalar_desc_set(diet_parameter(pb,1), f2);
      diet_scalar_desc_set(diet_parameter(pb,2), f3);
      break;
    }
    case DIET_DOUBLE: {
      double* d1 = NULL;
      double* d2 = NULL;
      double* d3 = NULL;
      diet_scalar_get(diet_parameter(pb,0), &d1, NULL);
      diet_scalar_get(diet_parameter(pb,1), &d2, NULL);
      diet_scalar_get(diet_parameter(pb,2), &d3, NULL);
      std::cout << "d1=" << *(double*)d1 << ", d2=" << *(double*)d2 << " -> ";
      *(double*)d3 = *(double*)d1 + *(double*)d2;
      *(double*)d2 = *(double*)d3;
      std::cout << "d2=" << *(double*)d2 << ", d3=" << *(double*)d3 << std::endl;
      diet_scalar_desc_set(diet_parameter(pb,1), d2);
      diet_scalar_desc_set(diet_parameter(pb,2), d3);
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
    diet_scalar_get(diet_parameter(pb,0), &l1, NULL);
    diet_scalar_get(diet_parameter(pb,1), &l2, NULL);
    diet_scalar_get(diet_parameter(pb,2), &l3, NULL);
    switch((diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type) {
    case DIET_CHAR:
      std::cout << "l1=" << *(char*)l1 << ", l2=" << *(char*)l2 << " -> ";
      *(char*)l3 = *(char*)l1 + *(char*)l2;
      *(char*)l2 = *(char*)l3;
      std::cout << "l2=" << *(char*)l2 << ", l3=" << *(char*)l3 << std::endl;
      break;
    case DIET_SHORT:
      std::cout << "l1=" << *(short*)l1 << ", l2=" << *(short*)l2 << " -> ";
      *(short*)l3 = *(short*)l1 + *(short*)l2;
      *(short*)l2 = *(short*)l3;
      std::cout << "l2=" << *(short*)l2 << ", l3=" << *(short*)l3 << std::endl;
      break;
    case DIET_INT:
      std::cout << "l1=" << *(int*)l1 << ", l2=" << *(int*)l2 << " -> ";
      *(int*)l3 = *(int*)l1 + *(int*)l2;
      *(int*)l2 = *(int*)l3;
      std::cout << "l2=" << *(int*)l2 << ", l3=" << *(int*)l3 << std::endl;
      break;
    case DIET_LONGINT:
      std::cout << "l1=" << *(long*)l1 << ", l2=" << *(long*)l2 << " -> ";
      *(long*)l3 = *(long*)l1 + *(long*)l2;
      *(long*)l2 = *(long*)l3;
      std::cout << "l2=" << *(long*)l2 << ", l3=" << *(long*)l3 << std::endl;
      break;
    default:
      res = 1;
    }
    diet_scalar_desc_set(diet_parameter(pb,1), l2);
    diet_scalar_desc_set(diet_parameter(pb,2), l3);
  }

  diet_free_data(diet_parameter(pb,0));

  std::cout << "Solve ADD ... done" << std::endl;
  return res;
}


/*
 * Vector solve function
 */
int
solve_VADD(diet_profile_t* pb) {
  int res = 0;
  size_t nA, nB, nC;
  void* A = NULL;
  void* B = NULL;
  void* C = NULL;

  std::cout << "Solve VADD ..." << std::endl;

  /* Get data */
  switch((diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type) {
  case DIET_FLOAT: {
    diet_vector_get(diet_parameter(pb, 0), (float *)&A, NULL, &nA);
    diet_vector_get(diet_parameter(pb, 1), (float *)&B, NULL, &nB);
    break;
  }
  case DIET_DOUBLE: {
    diet_vector_get(diet_parameter(pb, 0), (double *)&A, NULL, &nA);
    diet_vector_get(diet_parameter(pb, 1), (double *)&B, NULL, &nB);      
    break;
  }
  default:
    diet_vector_get(diet_parameter(pb, 0), (long *)&A, NULL, &nA);
    diet_vector_get(diet_parameter(pb, 1), (long *)&B, NULL, &nB);
  }

  /* Compute sum */
  switch((diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type) {
  case DIET_DOUBLE: {
    diet_vector_get(diet_parameter(pb, 2), (double *)&C, NULL, &nC);
    C = calloc(nC, sizeof((double *) &C));
    res = VecSUM(nA, (double *)B, (double *)A, (double *)C);
    diet_vector_set(diet_parameter(pb, 1), (double *) B, DIET_VOLATILE,
		    DIET_DOUBLE, nB);
    diet_vector_set(diet_parameter(pb, 2), (double *) C, DIET_VOLATILE,
		    DIET_DOUBLE, nC);
    break;
  }
  case DIET_FLOAT: {
    diet_vector_get(diet_parameter(pb, 2), (float *)&C, NULL, &nC);
    C = calloc(nC, sizeof((float *) &C));
    res = VecSUM(nA, (float *)B, (float *)A, (float *)C);
    diet_vector_set(diet_parameter(pb, 1), (float *) B, DIET_VOLATILE,
		    DIET_FLOAT, nB);
    diet_vector_set(diet_parameter(pb, 2), (float *) C, DIET_VOLATILE,
		    DIET_FLOAT, nC);
    break;
  }
  default:
    diet_vector_get(diet_parameter(pb, 2), (long *)&C, NULL, &nC);
    C = calloc(nC, sizeof((long *) &C));
    res = VecSUM(nA, (long *)B, (long *)A, (long *)C);
    diet_vector_set(diet_parameter(pb, 1), (long *) B, DIET_VOLATILE,
		    (diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type,
		    nB);
    diet_vector_set(diet_parameter(pb, 2), (long *) C, DIET_VOLATILE,
		    (diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type,
		    nC);
  }

  diet_free_data(diet_parameter(pb,0));

  std::cout << "Solve VADD ... done" << std::endl;
  return res;
}
 


/*
 * Matrix solve function
 */
int
solve_MADD(diet_profile_t* pb) {
  int res = 0;
  size_t mA, nA, mB, nB, mC, nC;
  char tA, tB;
  diet_matrix_order_t oA, oB, oC;
  void* A = NULL;
  void* B = NULL;
  void* C = NULL;

  std::cout << "Solve MADD ..." << std::endl;

  switch((diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type) {
  case DIET_FLOAT: {
    diet_matrix_get(diet_parameter(pb, 0), (float *)&A, NULL, &mA, &nA, &oA);
    diet_matrix_get(diet_parameter(pb, 1), (float *)&B, NULL, &mB, &nB, &oB);
    break;
  }
  case DIET_DOUBLE: {
    diet_matrix_get(diet_parameter(pb, 0), (double *)&A, NULL, &mA, &nA, &oA);
    diet_matrix_get(diet_parameter(pb, 1), (double *)&B, NULL, &mB, &nB, &oB);      
    break;
  }
  default:
    diet_matrix_get(diet_parameter(pb, 0), (long *)&A, NULL, &mA, &nA, &oA);
    diet_matrix_get(diet_parameter(pb, 1), (long *)&B, NULL, &mB, &nB, &oB);
  }

  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if ((mA != mB) || (nA != nB)) {
    std::cerr << "MatSUM error: mA=" << mA << ", nA=" << nA << "; mB="
	      << mB << ", nB=" << nB << std::endl;
    return 1;
  }
  
  switch((diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type) {
  case DIET_DOUBLE: {
    diet_matrix_get(diet_parameter(pb, 2), (double *)&C, NULL, &mC, &nC, &oC);
    C = calloc(mC*nC, sizeof((double *) &C));
    if (oC == DIET_ROW_MAJOR) {
      tA = (tA == 'T') ? 'N' : 'T';
      tB = (tB == 'T') ? 'N' : 'T';
      res = MatSUM(tB, tA, nA, mA, (double *)B, (double *)A, (double *)C);
    } else {
      res = MatSUM(tA, tB, mA, nA, (double *)A, (double *)B, (double *)C);
    }
    diet_matrix_set(diet_parameter(pb, 1), (double *) B, DIET_VOLATILE,
		    DIET_DOUBLE, mB, nB, oB);
    diet_matrix_set(diet_parameter(pb, 2), (double *) C, DIET_VOLATILE,
		    DIET_DOUBLE, mC, nC, oC);
    break;
  }
  case DIET_FLOAT: {
    diet_matrix_get(diet_parameter(pb, 2), (float *)&C, NULL, &mC, &nC, &oC);
    C = calloc(mC*nC, sizeof((float *) &C));
    if (oC == DIET_ROW_MAJOR) {
      tA = (tA == 'T') ? 'N' : 'T';
      tB = (tB == 'T') ? 'N' : 'T';
      res = MatSUM(tB, tA, nA, mA, (float *)B, (float *)A, (float *)C);
    } else {
      res = MatSUM(tA, tB, mA, nA, (float *)A, (float *)B, (float *)C);
    }
    diet_matrix_set(diet_parameter(pb, 1), (float *) B, DIET_VOLATILE,
		    DIET_FLOAT, mB, nB, oB);
    diet_matrix_set(diet_parameter(pb, 2), (float *) C, DIET_VOLATILE,
		    DIET_FLOAT, mC, nC, oC);
    break;
  }
  default:
    diet_matrix_get(diet_parameter(pb, 2), (long *)&C, NULL, &mC, &nC, &oC);
    C = calloc(mC*nC, sizeof((long *) &C));
    if (oC == DIET_ROW_MAJOR) {
      tA = (tA == 'T') ? 'N' : 'T';
      tB = (tB == 'T') ? 'N' : 'T';
      res = MatSUM(tB, tA, nA, mA, (long *)B, (long *)A, (long *)C);
    } else {
      res = MatSUM(tA, tB, mA, nA, (long *)A, (long *)B, (long *)C);
    }
    diet_matrix_set(diet_parameter(pb, 1), (long *) B, DIET_VOLATILE,
		    (diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type,
		    mB, nB, oB);
    diet_matrix_set(diet_parameter(pb, 2), (long *) C, DIET_VOLATILE,
		    (diet_base_type_t)(diet_parameter(pb,1))->desc.generic.base_type,
		    mC, nC, oC);
  }

  diet_free_data(diet_parameter(pb,0));

  std::cout << "Solve MADD ... done" << std::endl;
  return res;
}





int
usage(char* cmd) {
  std::cerr << "Usage: " << cmd << " <file.cfg>" << std::endl;
  return 1;
}


/*
 * MAIN
 */

int
main(int argc, char* argv[]) {
  size_t i;
  int res;

  diet_profile_desc_t* profile = NULL;

  if (argc < 2) {
    return usage(argv[0]);
  }
  diet_service_table_init(NB_SRV_SCALAR
			  + NB_SRV_VECTOR);


  /**
   * Scalar types
   */
  for (i = 0; i < NB_SRV_SCALAR; i++) {
    profile = diet_profile_desc_alloc(SRV_SCALAR[i], 0, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile,0),
			  DIET_SCALAR, (diet_base_type_t)i);
    diet_generic_desc_set(diet_param_desc(profile,1),
			  DIET_SCALAR, (diet_base_type_t)i);
    diet_generic_desc_set(diet_param_desc(profile,2),
			  DIET_SCALAR, (diet_base_type_t)i);

    if (diet_service_table_add(profile, NULL, solve_ADD))
      return 1;
    diet_profile_desc_free(profile);
  }

  /**
   * Vector types
   */
  for (i = 0; i < NB_SRV_VECTOR; i++) {
    profile = diet_profile_desc_alloc(SRV_VECTOR[i], 0, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile,0),
			  DIET_VECTOR, (diet_base_type_t)i);
    diet_generic_desc_set(diet_param_desc(profile,1), 
			  DIET_VECTOR, (diet_base_type_t)i);
    diet_generic_desc_set(diet_param_desc(profile,2), 
			  DIET_VECTOR, (diet_base_type_t)i);
 
    if (diet_service_table_add(profile, NULL, solve_VADD))
      return 1;
    diet_profile_desc_free(profile);
  }



  /**
   * Matrix types
   */
  for (i = 0; i < NB_SRV_MATRIX; i++) {
    profile = diet_profile_desc_alloc(SRV_MATRIX[i], 0, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile,0),
			  DIET_MATRIX, (diet_base_type_t)i);
    diet_generic_desc_set(diet_param_desc(profile,1), 
			  DIET_MATRIX, (diet_base_type_t)i);
    diet_generic_desc_set(diet_param_desc(profile,2), 
			  DIET_MATRIX, (diet_base_type_t)i);
 
    if (diet_service_table_add(profile, NULL, solve_MADD))
      return 1;
    diet_profile_desc_free(profile);
  }


  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
}
