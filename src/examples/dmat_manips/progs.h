/****************************************************************************/
/* dmat_manips example: matrix manipulations programs                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2003/04/10 13:28:15  pcombes
 * Apply new Coding Standards.
 *
 * Revision 1.5  2003/02/07 17:03:05  pcombes
 * Apply Coding Standards.
 *
 * Revision 1.4  2003/01/23 19:13:45  pcombes
 * Update to API 0.6.4
 *
 * Revision 1.3  2002/12/03 19:05:12  pcombes
 * Clean CVS logs in file.
 * Separate BLAS and SCALAPACK examples.
 *
 * Revision 1.2  2002/11/15 17:15:32  pcombes
 * FAST integration complete ...
 *
 * Revision 1.1  2002/11/05 17:23:36  pcombes
 * FAST support: convertors implemented and compatible to --without-fast
 *               configure option, but still not tested with FAST !
 ****************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define print_matrix(mat, m, n, rm)        \
  {                                        \
    size_t i, j;                           \
    printf("%s (%s-major) = \n", #mat,     \
           (rm) ? "row" : "column");       \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
        if (rm)                            \
	  printf("%3f ", (mat)[j + i*(n)]);\
        else                               \
	  printf("%3f ", (mat)[i + j*(m)]);\
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

/*
 * Transpose a matrix (column-major <=> rm == 0)
 */

inline int
T(int m, int n, double* A, int rm)
{
  size_t i, j;
  double* tmp = NULL;

  tmp = malloc(m*n*sizeof(double));
  memcpy(tmp, A, m*n*sizeof(double));

  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      if (rm)
	A[i*m + j] = tmp[j*n + i];
      else
	A[j*n + i] = tmp[i*m + j];
    }
  }

  free(tmp);
  return 0;
}

/*
 * Sum 2 column-major matrices (modulo tA and tB):
 * if tA == 'T', then A is row-major ...
 */

inline int
MatSUM(char tA, char tB, int m, int n, double* A, double* B, double* C)
{
  size_t i, j;
  
  if (tA == 'T') {
    if (tB == 'T') {
      for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	  C[j*m + i] = A[i*n + j] + B[i*n + j];
	}
      }
    } else {
      for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	  C[j*m + i] = A[i*n + j] + B[j*m + i];
	}
      }
    }
    
  } else {
    if (tB == 'T') {
      for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	  C[j*m + i] = A[j*m + i] + B[i*n + j];
	}
      }
    } else {
      for (i = 0; i < m; i++) {
	for (j = 0; j < n; j++) {
	  C[j*m + i] = A[j*m + i] + B[j*m + i];
	}
      }
    }
  }
  
  return 0;
}


/*
 * Multiply 2 column-major matrices (modulo tA and tB):
 * if tA == 'T', then A is row-major ...
 */

inline int
MatPROD(char tA, char tB,
	int mA, int nA, double* A,
	int nB, double* B, double* C)
{
  size_t i, j, k;
  int mB = nA;
  
  if (tA == 'T') {
    if (tB == 'T') {
      for (i = 0; i < mA; i++) {
	for (j = 0; j < nB; j++) {
	  C[j*mA + i] = 0;
	  for (k = 0; k < nA; k++) {
	    C[j*mA + i] += A[i*nA + k] * B[k*nB + j];
	  }
	}
      }
    } else {
      for (i = 0; i < mA; i++) {
	for (j = 0; j < nB; j++) {
	  C[j*mA + i] = 0;
	  for (k = 0; k < nA; k++) {
	    C[j*mA + i] += A[i*nA + k] * B[j*mB + k];
	  }
	}
      }
    }
    
  } else {
    if (tB == 'T') {
      for (i = 0; i < mA; i++) {
	for (j = 0; j < nB; j++) {
	  C[j*mA + i] = 0;
	  for (k = 0; k < nA; k++) {
	    C[j*mA + i] += A[k*mA + i] * B[k*nB + j];
	  }
	}
      }
    } else {
      for (i = 0; i < mA; i++) {
	for (j = 0; j < nB; j++) {
	  C[j*mA + i] = 0;
	  for (k = 0; k < nA; k++) {
	    C[j*mA + i] += A[k*mA + i] * B[j*mB + k];
	  }
	}
      }
    }
  }
  
  return 0;
}

