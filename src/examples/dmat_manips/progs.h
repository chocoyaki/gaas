/**
 * @file progs.h
 *
 * @brief  dmat_manips example: matrix manipulations programs
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _PROGS_H_
#define _PROGS_H_

#ifndef __WIN32__
#include <unistd.h>
#else
#include <Winsock2.h>
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef __WIN32__
#define INLINE __inline
#else
#define INLINE inline
#endif

#define print_matrix(mat, m, n, rm) {           \
    size_t i, j;                                \
    printf("%s (%s-major) = \n", # mat,          \
           (rm) ? "row" : "column");            \
    for (i = 0; i < (m); i++) {                 \
      for (j = 0; j < (n); j++) {               \
        if (rm) {                               \
          printf("%3f ", (mat)[j + i * (n)]);     \
        } else {                                \
          printf("%3f ", (mat)[i + j * (m)]);     \
        }                                     \
      }                                         \
      printf("\n");                             \
    }                                           \
    printf("\n");                               \
}

/*
 * Transpose a matrix (column-major <=> rm == 0)
 */

INLINE int
T(int m, int n, double *A, int rm) {
  size_t i, j;
  double *tmp = NULL;

  tmp = (double *)malloc(m * n * sizeof(tmp));
  memcpy(tmp, A, m * n * sizeof(tmp));

  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      if (rm) {
        A[i * m + j] = tmp[j * n + i];
      } else {
        A[j * n + i] = tmp[i * m + j];
      }
    }
  }

  free(tmp);
  return 0;
} /* T */

/*
 * Sum 2 column-major matrices (modulo tA and tB):
 * if tA == 'T', then A is row-major ...
 */

INLINE int
MatSUM(char tA, char tB, int m, int n, double *A, double *B, double *C) {
  size_t i, j;

  if (tA == 'T') {
    if (tB == 'T') {
      for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
          C[j * m + i] = A[i * n + j] + B[i * n + j];
        }
      }
    } else {
      for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
          C[j * m + i] = A[i * n + j] + B[j * m + i];
        }
      }
    }
  } else {
    if (tB == 'T') {
      for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
          C[j * m + i] = A[j * m + i] + B[i * n + j];
        }
      }
    } else {
      for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
          C[j * m + i] = A[j * m + i] + B[j * m + i];
        }
      }
    }
  }

  return 0;
} /* MatSUM */


/*
 * Multiply 2 column-major matrices (modulo tA and tB):
 * if tA == 'T', then A is row-major ...
 */

INLINE int
MatPROD(char tA, char tB,
        int mA, int nA, double *A,
        int nB, double *B, double *C) {
  size_t i, j, k;
  int mB = nA;

  if (tA == 'T') {
    if (tB == 'T') {
      for (i = 0; i < mA; i++) {
        for (j = 0; j < nB; j++) {
          C[j * mA + i] = 0;
          for (k = 0; k < nA; k++) {
            C[j * mA + i] += A[i * nA + k] * B[k * nB + j];
          }
        }
      }
    } else {
      for (i = 0; i < mA; i++) {
        for (j = 0; j < nB; j++) {
          C[j * mA + i] = 0;
          for (k = 0; k < nA; k++) {
            C[j * mA + i] += A[i * nA + k] * B[j * mB + k];
          }
        }
      }
    }
  } else {
    if (tB == 'T') {
      for (i = 0; i < mA; i++) {
        for (j = 0; j < nB; j++) {
          C[j * mA + i] = 0;
          for (k = 0; k < nA; k++) {
            C[j * mA + i] += A[k * mA + i] * B[k * nB + j];
          }
        }
      }
    } else {
      for (i = 0; i < mA; i++) {
        for (j = 0; j < nB; j++) {
          C[j * mA + i] = 0;
          for (k = 0; k < nA; k++) {
            C[j * mA + i] += A[k * mA + i] * B[j * mB + k];
          }
        }
      }
    }
  }

  return 0;
} /* MatPROD */

#endif  /* _PROGS_H_ */
