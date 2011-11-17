/**
 * @file server.cc
 *
 * @brief  DIET all data transfers example: a server for transfering all data types
 *
 * @author  - Benjamin Depardon (Benjamin.Depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

#include "DIET_server.h"
#include "DIET_Dagda.h"
#include "progs.hh"

static const unsigned int STRING_SIZE = 27;
static const char *STRING_TEST = "abcdefghijklmnopqrstuvwxyz";

/* This server offers all additions of two scalars:                  */
/*   - CADD = sum of two chars                                       */
/*   - BADD = sum of two bytes                                       */
/*   - IADD = sum of two ints                                        */
/*   - LADD = sum of two longs                                       */
/*   - FADD = sum of two floats                                      */
/*   - DADD = sum of two doubles                                     */
static const unsigned int NB_SRV_SCALAR = 6;
static const char *SRV_SCALAR[NB_SRV_SCALAR] =
{"CADD", "BADD", "IADD", "LADD", "FADD", "DADD"};

/* This server offers all additions of two vectors of scalars:       */
/*   - CVADD = sum of two chars                                      */
/*   - BVADD = sum of two bytes                                      */
/*   - IVADD = sum of two ints                                       */
/*   - LVADD = sum of two longs                                      */
/*   - FVADD = sum of two floats                                     */
/*   - DVADD = sum of two doubles                                    */
static const unsigned int NB_SRV_VECTOR = 6;
static const char *SRV_VECTOR[NB_SRV_VECTOR] =
{"CVADD", "BVADD", "IVADD", "LVADD", "FVADD", "DVADD"};

/* This server offers all additions of two matrices of scalars:      */
/*   - CMADD = sum of two chars                                      */
/*   - BMADD = sum of two bytes                                      */
/*   - IMADD = sum of two ints                                       */
/*   - LMADD = sum of two longs                                      */
/*   - FMADD = sum of two floats                                     */
/*   - DMADD = sum of two doubles                                    */
static const unsigned int NB_SRV_MATRIX = 6;
static const char *SRV_MATRIX[NB_SRV_MATRIX] =
{"CMADD", "BMADD", "IMADD", "LMADD", "FMADD", "DMADD"};


/* This server offers a service to print strings */
static const unsigned int NB_SRV_STRING = 1;
static const char *SRV_STRING[NB_SRV_STRING] =
{"SPRINT"};

/* This server offers a service to print paramstrings */
static const unsigned int NB_SRV_PSTRING = 1;
static const char *SRV_PSTRING[NB_SRV_PSTRING] =
{"PSPRINT"};


/* This server offers a service to transfer files:
 * Copies the IN file to the OUT file, INOUT is unchanged
 */
static const unsigned int NB_SRV_FILE = 1;
static const char *SRV_FILE[NB_SRV_FILE] =
{"FTRANSFER"};

/* This server offers addition of 2 longs stored in a container: */
/*   - LCADD = sum of two longs                                           */
static const unsigned int NB_SRV_CONTAINER = 1;
static const char *SRV_CONTAINER[NB_SRV_CONTAINER] =
{"LCADD"};


/*
 * Scalar solve function
 */
int
solve_ADD(diet_profile_t *pb) {
  int res = 0;

  std::cout << "Solve ADD ..." << std::endl;

  if (*(pb->pb_name) == 'F' || *(pb->pb_name) == 'D') {
    switch ((diet_base_type_t) (diet_parameter(pb,
                                               1))->desc.generic.base_type) {
    case DIET_FLOAT: {
      float *f1 = NULL;
      float *f2 = NULL;
      float *f3 = NULL;
      diet_scalar_get(diet_parameter(pb, 0), &f1, NULL);
      diet_scalar_get(diet_parameter(pb, 1), &f2, NULL);
      diet_scalar_get(diet_parameter(pb, 2), &f3, NULL);
      std::cout << "f1=" << *(float *) f1 << ", f2=" << *(float *) f2 << " -> ";
      *(float *) f3 = *(float *) f1 + *(float *) f2;
      *(float *) f2 = *(float *) f3;
      std::cout << "f2=" << *(float *) f2 << ", f3=" << *(float *) f3 <<
      std::endl;
      diet_scalar_desc_set(diet_parameter(pb, 1), f2);
      diet_scalar_desc_set(diet_parameter(pb, 2), f3);
      break;
    }
    case DIET_DOUBLE: {
      double *d1 = NULL;
      double *d2 = NULL;
      double *d3 = NULL;
      diet_scalar_get(diet_parameter(pb, 0), &d1, NULL);
      diet_scalar_get(diet_parameter(pb, 1), &d2, NULL);
      diet_scalar_get(diet_parameter(pb, 2), &d3, NULL);
      std::cout << "d1=" << *(double *) d1 << ", d2=" << *(double *) d2 <<
      " -> ";
      *(double *) d3 = *(double *) d1 + *(double *) d2;
      *(double *) d2 = *(double *) d3;
      std::cout << "d2=" << *(double *) d2 << ", d3=" << *(double *) d3 <<
      std::endl;
      diet_scalar_desc_set(diet_parameter(pb, 1), d2);
      diet_scalar_desc_set(diet_parameter(pb, 2), d3);
      break;
    }
    default:
      res = 1;
    } // switch
  } else {
    /* For integers, we can use the longest type to store values
       of smaller types. */
    long *l1 = NULL;
    long *l2 = NULL;
    long *l3 = NULL;
    diet_scalar_get(diet_parameter(pb, 0), &l1, NULL);
    diet_scalar_get(diet_parameter(pb, 1), &l2, NULL);
    diet_scalar_get(diet_parameter(pb, 2), &l3, NULL);
    switch ((diet_base_type_t) (diet_parameter(pb,
                                               1))->desc.generic.base_type) {
    case DIET_CHAR:
      std::cout << "l1=" << *(char *) l1 << ", l2=" << *(char *) l2 << " -> ";
      *(char *) l3 = *(char *) l1 + *(char *) l2;
      *(char *) l2 = *(char *) l3;
      std::cout << "l2=" << *(char *) l2 << ", l3=" << *(char *) l3 <<
      std::endl;
      break;
    case DIET_SHORT:
      std::cout << "l1=" << *(short *) l1 << ", l2=" << *(short *) l2 << " -> ";
      *(short *) l3 = *(short *) l1 + *(short *) l2;
      *(short *) l2 = *(short *) l3;
      std::cout << "l2=" << *(short *) l2 << ", l3=" << *(short *) l3 <<
      std::endl;
      break;
    case DIET_INT:
      std::cout << "l1=" << *(int *) l1 << ", l2=" << *(int *) l2 << " -> ";
      *(int *) l3 = *(int *) l1 + *(int *) l2;
      *(int *) l2 = *(int *) l3;
      std::cout << "l2=" << *(int *) l2 << ", l3=" << *(int *) l3 << std::endl;
      break;
    case DIET_LONGINT:
      std::cout << "l1=" << *(long *) l1 << ", l2=" << *(long *) l2 << " -> ";
      *(long *) l3 = *(long *) l1 + *(long *) l2;
      *(long *) l2 = *(long *) l3;
      std::cout << "l2=" << *(long *) l2 << ", l3=" << *(long *) l3 <<
      std::endl;
      break;
    default:
      res = 1;
    } // switch
    diet_scalar_desc_set(diet_parameter(pb, 1), l2);
    diet_scalar_desc_set(diet_parameter(pb, 2), l3);
  }

  diet_free_data(diet_parameter(pb, 0));

  std::cout << "Solve ADD ... done" << std::endl;
  return res;
} // solve_ADD


/*
 * Vector solve function
 */
int
solve_VADD(diet_profile_t *pb) {
  int res = 0;
  size_t nA, nB, nC;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  std::cout << "Solve VADD ..." << std::endl;

  /* Get data */
  switch ((diet_base_type_t) (diet_parameter(pb, 1))->desc.generic.base_type) {
  case DIET_FLOAT: {
    diet_vector_get(diet_parameter(pb, 0), (float *) &A, NULL, &nA);
    diet_vector_get(diet_parameter(pb, 1), (float *) &B, NULL, &nB);
    break;
  }
  case DIET_DOUBLE: {
    diet_vector_get(diet_parameter(pb, 0), (double *) &A, NULL, &nA);
    diet_vector_get(diet_parameter(pb, 1), (double *) &B, NULL, &nB);
    break;
  }
  default:
    diet_vector_get(diet_parameter(pb, 0), (long *) &A, NULL, &nA);
    diet_vector_get(diet_parameter(pb, 1), (long *) &B, NULL, &nB);
  } // switch

  /* Compute sum */
  switch ((diet_base_type_t) (diet_parameter(pb, 1))->desc.generic.base_type) {
  case DIET_DOUBLE: {
    diet_vector_get(diet_parameter(pb, 2), (double *) &C, NULL, &nC);
    C = calloc(nC, sizeof((double *) &C));
    res = VecSUM(nA, (double *) B, (double *) A, (double *) C);
    diet_vector_set(diet_parameter(pb, 1), (double *) B, DIET_VOLATILE,
                    DIET_DOUBLE, nB);
    diet_vector_set(diet_parameter(pb, 2), (double *) C, DIET_VOLATILE,
                    DIET_DOUBLE, nC);
    break;
  }
  case DIET_FLOAT: {
    diet_vector_get(diet_parameter(pb, 2), (float *) &C, NULL, &nC);
    C = calloc(nC, sizeof((float *) &C));
    res = VecSUM(nA, (float *) B, (float *) A, (float *) C);
    diet_vector_set(diet_parameter(pb, 1), (float *) B, DIET_VOLATILE,
                    DIET_FLOAT, nB);
    diet_vector_set(diet_parameter(pb, 2), (float *) C, DIET_VOLATILE,
                    DIET_FLOAT, nC);
    break;
  }
  default:
    diet_vector_get(diet_parameter(pb, 2), (long *) &C, NULL, &nC);
    C = calloc(nC, sizeof((long *) &C));
    res = VecSUM(nA, (long *) B, (long *) A, (long *) C);
    diet_vector_set(diet_parameter(pb, 1), (long *) B, DIET_VOLATILE,
                    (diet_base_type_t) (diet_parameter(pb,
                                                       1))->desc.generic.
                    base_type,
                    nB);
    diet_vector_set(diet_parameter(pb, 2), (long *) C, DIET_VOLATILE,
                    (diet_base_type_t) (diet_parameter(pb,
                                                       1))->desc.generic.
                    base_type,
                    nC);
  } // switch

  diet_free_data(diet_parameter(pb, 0));

  std::cout << "Solve VADD ... done" << std::endl;
  return res;
} // solve_VADD



/*
 * Matrix solve function
 */
int
solve_MADD(diet_profile_t *pb) {
  int res = 0;
  size_t mA, nA, mB, nB, mC, nC;
  char tA, tB;
  diet_matrix_order_t oA, oB, oC;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  std::cout << "Solve MADD ..." << std::endl;

  switch ((diet_base_type_t) (diet_parameter(pb, 1))->desc.generic.base_type) {
  case DIET_FLOAT: {
    diet_matrix_get(diet_parameter(pb, 0), (float *) &A, NULL, &mA, &nA, &oA);
    diet_matrix_get(diet_parameter(pb, 1), (float *) &B, NULL, &mB, &nB, &oB);
    break;
  }
  case DIET_DOUBLE: {
    diet_matrix_get(diet_parameter(pb, 0), (double *) &A, NULL, &mA, &nA, &oA);
    diet_matrix_get(diet_parameter(pb, 1), (double *) &B, NULL, &mB, &nB, &oB);
    break;
  }
  default:
    diet_matrix_get(diet_parameter(pb, 0), (long *) &A, NULL, &mA, &nA, &oA);
    diet_matrix_get(diet_parameter(pb, 1), (long *) &B, NULL, &mB, &nB, &oB);
  } // switch

  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if ((mA != mB) || (nA != nB)) {
    std::cerr << "MatSUM error: mA=" << mA << ", nA=" << nA << "; mB="
              << mB << ", nB=" << nB << std::endl;
    return 1;
  }

  switch ((diet_base_type_t) (diet_parameter(pb, 1))->desc.generic.base_type) {
  case DIET_DOUBLE: {
    diet_matrix_get(diet_parameter(pb, 2), (double *) &C, NULL, &mC, &nC, &oC);
    C = calloc(mC * nC, sizeof((double *) &C));
    if (oC == DIET_ROW_MAJOR) {
      tA = (tA == 'T') ? 'N' : 'T';
      tB = (tB == 'T') ? 'N' : 'T';
      res = MatSUM(tB, tA, nA, mA, (double *) B, (double *) A, (double *) C);
    } else {
      res = MatSUM(tA, tB, mA, nA, (double *) A, (double *) B, (double *) C);
    }
    diet_matrix_set(diet_parameter(pb, 1), (double *) B, DIET_VOLATILE,
                    DIET_DOUBLE, mB, nB, oB);
    diet_matrix_set(diet_parameter(pb, 2), (double *) C, DIET_VOLATILE,
                    DIET_DOUBLE, mC, nC, oC);
    break;
  }
  case DIET_FLOAT: {
    diet_matrix_get(diet_parameter(pb, 2), (float *) &C, NULL, &mC, &nC, &oC);
    C = calloc(mC * nC, sizeof((float *) &C));
    if (oC == DIET_ROW_MAJOR) {
      tA = (tA == 'T') ? 'N' : 'T';
      tB = (tB == 'T') ? 'N' : 'T';
      res = MatSUM(tB, tA, nA, mA, (float *) B, (float *) A, (float *) C);
    } else {
      res = MatSUM(tA, tB, mA, nA, (float *) A, (float *) B, (float *) C);
    }
    diet_matrix_set(diet_parameter(pb, 1), (float *) B, DIET_VOLATILE,
                    DIET_FLOAT, mB, nB, oB);
    diet_matrix_set(diet_parameter(pb, 2), (float *) C, DIET_VOLATILE,
                    DIET_FLOAT, mC, nC, oC);
    break;
  }
  default:
    diet_matrix_get(diet_parameter(pb, 2), (long *) &C, NULL, &mC, &nC, &oC);
    C = calloc(mC * nC, sizeof((long *) &C));
    if (oC == DIET_ROW_MAJOR) {
      tA = (tA == 'T') ? 'N' : 'T';
      tB = (tB == 'T') ? 'N' : 'T';
      res = MatSUM(tB, tA, nA, mA, (long *) B, (long *) A, (long *) C);
    } else {
      res = MatSUM(tA, tB, mA, nA, (long *) A, (long *) B, (long *) C);
    }
    diet_matrix_set(diet_parameter(pb, 1), (long *) B, DIET_VOLATILE,
                    (diet_base_type_t) (diet_parameter(pb,
                                                       1))->desc.generic.
                    base_type,
                    mB, nB, oB);
    diet_matrix_set(diet_parameter(pb, 2), (long *) C, DIET_VOLATILE,
                    (diet_base_type_t) (diet_parameter(pb,
                                                       1))->desc.generic.
                    base_type,
                    mC, nC, oC);
  } // switch

  diet_free_data(diet_parameter(pb, 0));

  std::cout << "Solve MADD ... done" << std::endl;
  return res;
} // solve_MADD




/*
 * String solve function
 */
int
solve_SPRINT(diet_profile_t *pb) {
  int res = 0;
  char *s1 = NULL;
  char *s2 = NULL;
  char *s3 = NULL;

  std::cout << "Solve SPRINT ..." << std::endl;

  /* Get data */
  diet_string_get(diet_parameter(pb, 0), &s1, NULL);
  diet_string_get(diet_parameter(pb, 1), &s2, NULL);
  diet_string_get(diet_parameter(pb, 2), &s3, NULL);

  s3 = strdup(STRING_TEST);

  /* print */
  std::cout << "s1: " << s1 << std::endl;
  std::cout << "s2: " << s2 << std::endl;
  for (unsigned int i = 0; i < strlen(s2); ++i)
    s2[i] = 'c';
  std::cout << "s2: " << s2 << std::endl;
  std::cout << "s3: " << s3 << std::endl;
  diet_string_set(diet_parameter(pb, 1), s2, DIET_VOLATILE);
  diet_string_set(diet_parameter(pb, 2), s3, DIET_VOLATILE);

  diet_free_data(diet_parameter(pb, 0));

  std::cout << "Solve SPRINT ... done" << std::endl;
  return res;
} // solve_SPRINT

/*
 * Paramstring solve function
 */
int
solve_PSPRINT(diet_profile_t *pb) {
  int res = 0;
  char *s1 = NULL;
  char *s2 = NULL;
  std::cout << "Solve PSPRINT ..." << std::endl;

  /* Get data */
  diet_paramstring_get(diet_parameter(pb, 0), &s1, NULL);
  diet_paramstring_get(diet_parameter(pb, 1), &s2, NULL);


  /* print */
  std::cout << "s1: " << s1 << std::endl;
  std::cout << "s2: " << s2 << std::endl;
  for (unsigned int i = 0; i < strlen(s2); ++i)
    s2[i] = 'c';
  std::cout << "s2: " << s2 << std::endl;
  diet_paramstring_set(diet_parameter(pb, 1), s2, DIET_VOLATILE);

  diet_free_data(diet_parameter(pb, 0));

  std::cout << "Solve PSPRINT ... done" << std::endl;
  return res;
} // solve_PSPRINT



/*
 * File solve function
 */
int
solve_FTRANSFER(diet_profile_t *pb) {
  size_t arg_size = 0;
  char *path1 = NULL;
  char *path2 = NULL;
  char *path3 = NULL;
  int status = 0;
  struct stat buf;

  /* IN file */
  std::cout << "Solve size ";
  diet_file_get(diet_parameter(pb, 0), NULL, &arg_size, &path1);
  std::cout << "on " << path1 << " (" << arg_size << ")" << std::endl;
  if ((status = stat(path1, &buf))) {
    return status;
  }
  /* Regular file */
  if (!(buf.st_mode & S_IFREG)) {
    return 2;
  }

  /*  INOUT file */
  diet_file_get(diet_parameter(pb, 1), NULL, &arg_size, &path2);
  std::cout << " and on " << path2 << " (" << arg_size << ")" << std::endl;
  if ((status = stat(path2, &buf))) {
    return status;
  }
  if (!(buf.st_mode & S_IFREG)) {
    return 2;
  }
  if (diet_file_set(diet_parameter(pb, 1), path2, DIET_VOLATILE)) {
    std::cerr << "diet_file_desc_set error on INOUT file" << std::endl;
    return 1;
  }

  /* Send IN file as OUT file */
  path3 = strdup(path1);
  if (diet_file_set(diet_parameter(pb, 2), path3, DIET_VOLATILE)) {
    std::cerr << "diet_file_desc_set error on OUT file" << std::endl;
    return 1;
  }

  std::cout << "Returned file: " << path3 << std::endl;


  diet_free_data(diet_parameter(pb, 0));

  return 0;
} // solve_FTRANSFER


/*
 * Container solve function
 */
int
solve_CONTAINER(diet_profile_t *pb) {
  int res = 0;
  char *ID_long1;
  char *ID_long2;
  long *l1, *l2, *l3;
  diet_container_t content1, content2;

  /* no need to call dagda_get_container for root container as it is
   * downloaded automatically by DIET */
  std::cout << "Get container 1 element list" << std::endl;
  dagda_get_container_elements((*diet_parameter(pb, 0)).desc.id, &content1);
  std::cout << "Container 1 contains " << content1.size << " elements." <<
  std::endl;

  std::cout << "Get container 2 element list" << std::endl;
  dagda_get_container_elements((*diet_parameter(pb, 1)).desc.id, &content2);
  std::cout << "Container 2 contains " << content2.size << " elements." <<
  std::endl;

  std::cout << "Init OUTPUT container" << std::endl;
  dagda_init_container(diet_parameter(pb, 2));

  std::cout << "Get container 1 elements" << std::endl;
  dagda_get_scalar(content1.elt_ids[0], &l1, NULL);
  dagda_get_scalar(content1.elt_ids[1], &l2, NULL);
  l3 = new long;
  *l3 = *l1 + *l2;
  std::cout << "l1 = " << *l1 << ", l2 = " << *l2
            << ", l3 = " << *l3 << std::endl;
  dagda_put_scalar(l3, DIET_LONGINT, DIET_PERSISTENT, &ID_long1);

  std::cout << "Put 1st element of OUTPUT container" << std::endl;
  dagda_add_container_element((*diet_parameter(pb, 2)).desc.id, ID_long1, 0);


  std::cout << "Get container 2 elements" << std::endl;
  dagda_get_scalar(content2.elt_ids[0], &l1, NULL);
  dagda_get_scalar(content2.elt_ids[1], &l2, NULL);
  l3 = new long;
  *l3 = *l1 + *l2;
  std::cout << "l1 = " << *l1 << ", l2 = " << *l2
            << ", l3 = " << *l3 << std::endl;
  dagda_put_scalar(l3, DIET_LONGINT, DIET_PERSISTENT, &ID_long2);

  std::cout << "Put 2nd element of OUTPUT container" << std::endl;
  dagda_add_container_element((*diet_parameter(pb, 2)).desc.id, ID_long2, 1);
  dagda_add_container_element((*diet_parameter(pb, 1)).desc.id, ID_long2, 0);

  diet_free_data(diet_parameter(pb, 0));

  free(content1.elt_ids);
  free(content2.elt_ids);
  return res;
} // solve_CONTAINER



int
usage(char *cmd) {
  std::cerr << "Usage: " << cmd << " <file.cfg>" << std::endl;
  return 1;
}


/*
 * MAIN
 */

int
main(int argc, char *argv[]) {
  size_t i;
  int res;

  diet_profile_desc_t *profile = NULL;

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
    diet_generic_desc_set(diet_param_desc(profile, 0),
                          DIET_SCALAR, (diet_base_type_t) i);
    diet_generic_desc_set(diet_param_desc(profile, 1),
                          DIET_SCALAR, (diet_base_type_t) i);
    diet_generic_desc_set(diet_param_desc(profile, 2),
                          DIET_SCALAR, (diet_base_type_t) i);

    if (diet_service_table_add(profile, NULL, solve_ADD)) {
      return 1;
    }
    diet_profile_desc_free(profile);
  }

  /**
   * Vector types
   */
  for (i = 0; i < NB_SRV_VECTOR; i++) {
    profile = diet_profile_desc_alloc(SRV_VECTOR[i], 0, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0),
                          DIET_VECTOR, (diet_base_type_t) i);
    diet_generic_desc_set(diet_param_desc(profile, 1),
                          DIET_VECTOR, (diet_base_type_t) i);
    diet_generic_desc_set(diet_param_desc(profile, 2),
                          DIET_VECTOR, (diet_base_type_t) i);

    if (diet_service_table_add(profile, NULL, solve_VADD)) {
      return 1;
    }
    diet_profile_desc_free(profile);
  }



  /**
   * Matrix types
   */
  for (i = 0; i < NB_SRV_MATRIX; i++) {
    profile = diet_profile_desc_alloc(SRV_MATRIX[i], 0, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0),
                          DIET_MATRIX, (diet_base_type_t) i);
    diet_generic_desc_set(diet_param_desc(profile, 1),
                          DIET_MATRIX, (diet_base_type_t) i);
    diet_generic_desc_set(diet_param_desc(profile, 2),
                          DIET_MATRIX, (diet_base_type_t) i);

    if (diet_service_table_add(profile, NULL, solve_MADD)) {
      return 1;
    }
    diet_profile_desc_free(profile);
  }


  /**
   * String types
   */
  for (i = 0; i < NB_SRV_STRING; i++) {
    profile = diet_profile_desc_alloc(SRV_STRING[i], 0, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0),
                          DIET_STRING, DIET_CHAR);
    diet_generic_desc_set(diet_param_desc(profile, 1),
                          DIET_STRING, DIET_CHAR);
    diet_generic_desc_set(diet_param_desc(profile, 2),
                          DIET_STRING, DIET_CHAR);

    if (diet_service_table_add(profile, NULL, solve_SPRINT)) {
      return 1;
    }
    diet_profile_desc_free(profile);
  }


  /**
   * Paramstring types
   */
  for (i = 0; i < NB_SRV_PSTRING; i++) {
    profile = diet_profile_desc_alloc(SRV_PSTRING[i], 0, 1, 1);
    diet_generic_desc_set(diet_param_desc(profile, 0),
                          DIET_PARAMSTRING, DIET_CHAR);
    diet_generic_desc_set(diet_param_desc(profile, 1),
                          DIET_PARAMSTRING, DIET_CHAR);

    if (diet_service_table_add(profile, NULL, solve_PSPRINT)) {
      return 1;
    }
    diet_profile_desc_free(profile);
  }


  /**
   * File types
   */
  for (i = 0; i < NB_SRV_FILE; i++) {
    profile = diet_profile_desc_alloc(SRV_FILE[i], 0, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0),
                          DIET_FILE, DIET_CHAR);
    diet_generic_desc_set(diet_param_desc(profile, 1),
                          DIET_FILE, DIET_CHAR);
    diet_generic_desc_set(diet_param_desc(profile, 2),
                          DIET_FILE, DIET_CHAR);

    if (diet_service_table_add(profile, NULL, solve_FTRANSFER)) {
      return 1;
    }
    diet_profile_desc_free(profile);
  }



  /**
   * Container types
   */
  for (i = 0; i < NB_SRV_CONTAINER; i++) {
    profile = diet_profile_desc_alloc(SRV_CONTAINER[i], 0, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0),
                          DIET_CONTAINER, DIET_CHAR);
    diet_generic_desc_set(diet_param_desc(profile, 1),
                          DIET_CONTAINER, DIET_CHAR);
    diet_generic_desc_set(diet_param_desc(profile, 2),
                          DIET_CONTAINER, DIET_CHAR);

    if (diet_service_table_add(profile, NULL, solve_CONTAINER)) {
      return 1;
    }
    diet_profile_desc_free(profile);
  }


  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
} // main
