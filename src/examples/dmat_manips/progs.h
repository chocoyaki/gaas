/****************************************************************************/
/* $Id$ */
/* dmat_manips example: matrix manipulations programs                       */
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
 * Revision 1.2  2002/11/15 17:15:32  pcombes
 * FAST integration complete ...
 *
 * Revision 1.1  2002/11/05 17:23:36  pcombes
 * FAST support: convertors implemented and compatible to --without-fast
 *               configure option, but still not tested with FAST !
 *
 ****************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define print_matrix(mat, m, n)            \
  {                                        \
    size_t i, j;                           \
    printf("%s = \n", #mat);               \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
	printf("%3f ", (mat)[i + j*(m)]);  \
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

/*
 * Transpose matrix
 */

inline int
T(int m, int n, double *A)
{
  size_t i, j;
  double *tmp;

  tmp = malloc(m*n*sizeof(double));
  memcpy(tmp, A, m*n*sizeof(double));

  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      A[j*n + i] = tmp[i*m + j];
    }
  }

  free(tmp);
  return 0;
}

/*
 * Sum 2 matrices
 */

inline int
MatSUM(char tA, char tB, int m, int n, double *A, double *B, double *C)
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
 * Mult 2 matrices
 */

inline int
MatPROD(char tA, char tB,
	int mA, int nA, double *A,
	int nB, double *B, double *C)
{
  size_t i, j, k;
  int mB = nA;
  
  if (tA == 'T') {
    if (tB == 'T') {
      for (i = 0; i < mA; i++) {
	for (j = 0; j < nB; j++) {
	  C[j*mA + i] = 0;
	  for (k = 0; k < nA; k++) {
	    C[j*mA + i] += A[i*nA + k] + B[k*nB + j];
	  }
	}
      }
    } else {
      for (i = 0; i < mA; i++) {
	for (j = 0; j < nB; j++) {
	  C[j*mA + i] = 0;
	  for (k = 0; k < nA; k++) {
	    C[j*mA + i] += A[i*nA + k] + B[j*mB + k];
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
	    C[j*mA + i] += A[k*mA + i] + B[k*nB + j];
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

