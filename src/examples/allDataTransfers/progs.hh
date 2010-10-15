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


template <class T>
void
printMatrix(T *mat, size_t m, size_t n, bool rm) {
  size_t i, j;                           

  std:: cout << "Matrix is ";
  if (rm)
    std::cout << "row major" << std::endl;
  else 
    std::cout << "column major" << std::endl;

  for (i = 0; i < (m); i++) {            
    for (j = 0; j < (n); j++) {          
      if (rm)                            
	std::cout << mat[j + i*(n)] << " ";
      else                               
	std::cout << mat[i + j*(m)] << " ";
    }                                    
    std::cout << std::endl;
  }                                      
  std::cout << std::endl;
}


template <class T>
void
printVector(T *vec, size_t n) {
  size_t i;

  for (i = 0; i < n; i++)
    std::cout << vec[i] << " ";
  std::cout << std::endl;
}




/*
 * Sum 2 column-major matrices (modulo tA and tB):
 * if tA == 'T', then A is row-major ...
 */

template <class T>
int
MatSUM(char tA, char tB, size_t m, size_t n, T* A, T* B, T* C) {
  size_t i, j;
  
  if (tA == 'T') {
    if (tB == 'T') {
      for (i = 0; i < m; i++)
	for (j = 0; j < n; j++)
	  C[j*m + i] = A[i*n + j] + B[i*n + j];
    } else {
      for (i = 0; i < m; i++)
	for (j = 0; j < n; j++)
	  C[j*m + i] = A[i*n + j] + B[j*m + i];
    }
  } else {
    if (tB == 'T') {
      for (i = 0; i < m; i++)
	for (j = 0; j < n; j++)
	  C[j*m + i] = A[j*m + i] + B[i*n + j];
    } else {
      for (i = 0; i < m; i++)
	for (j = 0; j < n; j++)
	  C[j*m + i] = A[j*m + i] + B[j*m + i];
    }
  }
  
  return 0;
}



/*
 * Sum 2 vectors
 */

template <class T>
int
VecSUM(size_t n, T* A, T* B, T* C) {
  size_t i;
  
  for (i = 0; i < n ; i++)
    C[i] = A[i] + B[i];
  
  return 0;
}


  

