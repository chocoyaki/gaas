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
 * Revision 1.7  2011/01/20 17:38:50  bdepardo
 * Added #ifndef _DMAT_JXTA_PROGS_H_
 * #define _DMAT_JXTA_PROGS_H_
 *
 * Revision 1.6  2006/11/28 13:51:29  ctedesch
 * old logs in header
 *
 * Revision 1.5  2006/11/28 13:42:21  ctedesch
 * Add header
 *
 * Revision 1.4
 * date: 2006/11/28 13:40:11  ctedesch
 * Add header
 *
 * Revision 1.3
 * date: 2006/11/28 13:35:52  ctedesch
 * Add header
 * 
 * Revision 1.2
 * date: 2006/11/28 13:29:27  ctedesch
 * Header
 *
 * revision 1.1
 * date: 2004/06/11 15:45:39  ctedesch
 * add DIET/JXTA
 ****************************************************************************/


/****************************************************************************/
#ifndef _DMAT_JXTA_PROGS_H_
#define _DMAT_JXTA_PROGS_H_

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

  tmp = (double*)malloc(m*n*sizeof(double));
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

#endif // _DMAT_JXTA_PROGS_H_
