/****************************************************************************/
/* BLAS/dgemm server : a DIET client for dgemm using threads                */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (Benjamin.Depardon@ens-lyon.fr                    */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/01/22 10:36:48  bdepardo
 * Matrices initialization outside the loop in order to obtain a better
 * throughput.
 *
 * Revision 1.1  2009/10/16 08:07:34  bdepardo
 * Threaded version of the client
 *
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include <pthread.h>


#include "DIET_client.h"


#define print_matrix(mat, m, n, rm)		\
  {						\
    size_t i, j;				\
    printf("%s (%s-major) = \n", #mat,		\
           (rm) ? "row" : "column");		\
    for (i = 0; i < (m); i++) {			\
      for (j = 0; j < (n); j++) {		\
        if (rm)					\
          printf("%3f ", (mat)[j + i*(n)]);	\
        else					\
          printf("%3f ", (mat)[i + j*(m)]);	\
      }						\
      printf("\n");				\
    }						\
    printf("\n");				\
  }

/* argv[1]: client config file path
 * argv[2]: dgemm, MatSUM, or MatPROD */


static size_t m, n, k;
static double alpha, beta;
static unsigned int nb = 0;


void *
call(void* par) {
  diet_profile_t* profile;
  char* path = "dgemm";
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  diet_matrix_order_t oA, oB, oC;
  size_t i, j;
  unsigned int id = nb;
  ++ nb;


  oA = DIET_ROW_MAJOR;
  oB = DIET_COL_MAJOR;
  oC = DIET_ROW_MAJOR;

  /* Fill A, B and C randomly ... */
  A = (double*) calloc(m*k, sizeof(double));
  B = (double*) calloc(k*n, sizeof(double));
  C = (double*) calloc(m*n, sizeof(double));
  for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
  for (i = 0; i < k * n; i++)     B[i] = 1.0 + j++;
  for (i = 0; i < m * n; i++)     C[i] = 1.0 + j++;

  profile = diet_profile_alloc(path, 3, 4, 4);
  
  diet_scalar_set(diet_parameter(profile,0), &alpha,
                  DIET_VOLATILE, DIET_DOUBLE);
  diet_matrix_set(diet_parameter(profile,1), A,
                  DIET_VOLATILE, DIET_DOUBLE, m, k, oA);
  diet_matrix_set(diet_parameter(profile,2), B,
                  DIET_VOLATILE, DIET_DOUBLE, k, n, oB);
  diet_scalar_set(diet_parameter(profile,3), &beta,
                  DIET_VOLATILE, DIET_DOUBLE);
  diet_matrix_set(diet_parameter(profile,4), C,
                  DIET_VOLATILE, DIET_DOUBLE, m, n, oC);

  for (;;) {
    
    printf("Calling DGEMM (%u)\n", id);
    
    /*    print_matrix(A, m, k, (oA == DIET_ROW_MAJOR));
	  print_matrix(B, k, n, (oB == DIET_ROW_MAJOR));
	  print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
    */
    if (!diet_call(profile)) {
      //print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
    }    
  }

  diet_profile_free(profile);
  free(A);
  free(B);
  free(C);  
}


int
main(int argc, char* argv[])
{
  unsigned int nbThreads, i;

  srand(time(NULL));
  
  if (argc != 8) {
    fprintf(stderr, "Usage: %s <file.cfg> m n k alpha beta nbThreads\n", argv[0]);
    return 1;
  }

  /* Parsing and preparation of m, n, j, A, B, C, alpha and beta */
  m     = (size_t) atoi(argv[2]);
  n     = (size_t) atoi(argv[3]);
  k     = (size_t) atoi(argv[4]);
  alpha = strtod(argv[5], NULL);
  beta  = strtod(argv[6], NULL);

  nbThreads = atoi(argv[7]);
  printf("nb threads %u\n", nbThreads);

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  } 

  pthread_t * thread = (pthread_t*) malloc(nbThreads * sizeof(pthread_t));
  for (i = 0; i < nbThreads; ++ i) {
    pthread_create(&thread[i], NULL, call, NULL);
    //     omni_thread::create(call, (void *) &l1, omni_thread::PRIORITY_NORMAL);
  }

  for (i = 0; i < nbThreads; ++ i)
    pthread_join(thread[i], NULL);

    
  diet_finalize();

  return 0;
}

