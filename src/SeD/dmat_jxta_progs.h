/****************************************************************************/
/* dmat_manips example: matrix manipulations programs                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/*  This file is part of DIET 0.7_beta.                                     */
/*                                                                          */
/*  Copyright (C) 2000-2003 ENS Lyon, LIFC, INSA and INRIA,                 */
/*                          all rights reserved.                            */
/*                                                                          */
/*  Since DIET is open source, free software, you are free to use, modify,  */
/*  and distribute the DIET source code and object code produced from the   */
/*  source, as long as you include this copyright statement along with      */
/*  code built using DIET.                                                  */
/*                                                                          */
/*  Redistribution and use in source and binary forms, with or without      */
/*  modification, are permitted provided that the following conditions      */
/*  are met.                                                                */
/*                                                                          */
/*  Redistributions of source code must retain the copyright notice below   */
/*  this list of conditions and the following disclaimer. Redistributions   */
/*  in binary form must reproduce the copyright notice below, this list     */
/*  of conditions and the following disclaimer in the documentation         */
/*  and/or other materials provided with the distribution. Neither the      */
/*  name of ENS Lyon nor the names of its contributors (LIFC, INSA Lyon,    */
/*  INRIA) may be used to endorse or promote products derived from this     */
/*  software without specific prior written permission.                     */
/*                                                                          */
/*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     */
/*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       */
/*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       */
/*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          */
/*  REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,             */
/*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,    */
/*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES ;       */
/*  LOSS OF USE, DATA, OR PROFITS ; OR BUSINESS INTERRUPTION) HOWEVER       */
/*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT      */
/*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY   */
/*  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             */
/*  POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                          */
/****************************************************************************/


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

