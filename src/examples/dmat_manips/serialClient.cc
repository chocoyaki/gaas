/****************************************************************************/
/* dmat_manips example: a DIET client for transpose, MatSUM and MatPROD     */
/*   problems (MatSUM is expanded to SqMatSUM and SqMatSUM_bis) using the   */
/*   asynchronous API, but simulating the synchronous one.                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Christope PERA            - LIP ENS-Lyon (France)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.2  2003/06/30 11:15:12  cpera
 * Fix bugs in ReaderWriter and new internal debug macros.
 *
 * Revision 1.1  2003/06/16 17:12:49  pcombes
 * Move the examples using the asynchronous API into this directory.
 *
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "Global_macros.hh"
#include "DIET_client.h"

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


void
displayArg(FILE* f, const diet_data_desc_t* arg)
{
  switch((int) arg->generic.type) {
  case DIET_SCALAR: fprintf(f, "scalar");                break;
  case DIET_VECTOR: fprintf(f, "vector (%ld)",
			    (long)arg->specific.vect.size);    break;
  case DIET_MATRIX: fprintf(f, "matrix (%ldx%ld)",
			    (long)arg->specific.mat.nb_r,
			    (long)arg->specific.mat.nb_c);   break;
  case DIET_STRING: fprintf(f, "string (%ld)",
			    (long)arg->specific.str.length); break;
  case DIET_FILE:   fprintf(f, "file (%ld)",
			    (long)arg->specific.file.size);  break;
  }
  if ((arg->generic.type != DIET_STRING)
      && (arg->generic.type != DIET_FILE)) {
    fprintf(f, " of ");
    switch ((int) arg->generic.base_type) {
    case DIET_CHAR:     fprintf(f, "char");           break;
    case DIET_BYTE:     fprintf(f, "byte");           break;
    case DIET_INT:      fprintf(f, "int");            break;
    case DIET_LONGINT:  fprintf(f, "long int");       break;
    case DIET_FLOAT:    fprintf(f, "float");          break;
    case DIET_DOUBLE:   fprintf(f, "double");         break;
    case DIET_SCOMPLEX: fprintf(f, "float complex");  break;
    case DIET_DCOMPLEX: fprintf(f, "double complex"); break;
    }
  }
  fprintf(f, "id=|%s|", arg->id);
}
  
#define NB_PB 5
static const char* PB[NB_PB] =
  {"T", "MatPROD", "MatSUM", "SqMatSUM", "SqMatSUM_opt"};


void
displayProfile(const diet_profile_t* profile, const char* path)
{
  int i = 0;
  FILE* f = stdout;
  fprintf(f, " - Service %s", path);
  for (i = 0; i <= profile->last_out; i++) {
    fprintf(f, "\n     %s ",
	    (i <= profile->last_in) ? "IN   "
	    : (i <= profile->last_inout) ? "INOUT"
	    : "OUT  ");
    displayArg(f, &(profile->parameters[i].desc));
  }
  fprintf(f, "\n");
}
  
/* argv[1]: client config file path
   argv[2]: one of the strings above */
void
usage(char* cmd)
{
   fprintf(stderr, "Usage: %s [--repeat <n>] <file.cfg> [%s|%s|%s|%s|%s]\n",
	   cmd, PB[0], PB[1], PB[2], PB[3], PB[4]);
  fprintf(stderr, "    ex: %s client.cfg T\n", cmd);
  fprintf(stderr, "        %s --repeat 1000 client.cfg MatSUM\n", cmd);
  exit(1);
}

int
main(int argc, char* argv[])
{
  size_t i, m, n;
  size_t n_loops = 1;
  char* path = NULL;
  diet_profile_t* profile = NULL;
  double mat1[9] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0};
  double mat2[9] = {10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0};
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  diet_matrix_order_t oA, oB, oC;

  int   pb[NB_PB] = {0, 0, 0, 0, 0};
  DIET_DEBUG(TEXT_OUTPUT(("DEBUT du client asynchrone avec des appels en serie.")))

  srand(time(NULL));

  for (i = 1; (int) i < argc - 2; i++) {
    if (strcmp("--repeat", argv[i]) == 0) {
      int i_n_loops = atoi(argv[i + 1]);
      if (i_n_loops < 0) {
	fprintf(stderr, "Argument to option %s must be >= 0.\n", argv[i]);
	usage(argv[0]);
      }
      n_loops = (size_t) i_n_loops;
      i++;
      memcpy(argv + i - 2, argv + i, (argc - i)*sizeof(char*));
      i -= 2;
      argc -= 2;
    } else {
      fprintf(stderr, "Unrecognized option %s\n", argv[i]);
      usage(argv[0]);
    }
  }

  if (argc - i != 2) {
    usage(argv[0]);
  }
  path = argv[argc - 1];
  
  A = mat1;
  B = mat2;

  m = 3;
  n = 2;

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  for (i = 0; i < NB_PB; i++) {
    if ((pb[i] = !strcmp(path, PB[i]))) break;
  }
  // Square matrix problems:
  if (pb[3] || pb[4])
    n = m;

  for (i = 0; i < n_loops; i++) {
    oA = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oB = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oC = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    
    if (pb[0]) {
      profile = diet_profile_alloc(path, -1, 0, 0);
      diet_matrix_set(diet_parameter(profile,0),
		      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
    } 
    else if (pb[1] || pb[2] || pb[3]) {  
      profile = diet_profile_alloc(path, 1, 1, 2);
      diet_matrix_set(diet_parameter(profile,0),
		      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
      if (pb[1]) {
	diet_matrix_set(diet_parameter(profile,1),
			B, DIET_VOLATILE, DIET_DOUBLE, n, m, oB);
	print_matrix(B, n, m, (oB == DIET_ROW_MAJOR));
	diet_matrix_set(diet_parameter(profile,2),
			NULL, DIET_VOLATILE, DIET_DOUBLE, m, m, oC);
      } 
      else {
	diet_matrix_set(diet_parameter(profile,1),
			B, DIET_VOLATILE, DIET_DOUBLE, m, n, oB);
	print_matrix(B, m, n, (oB == DIET_ROW_MAJOR));
	diet_matrix_set(diet_parameter(profile,2),
			NULL, DIET_VOLATILE, DIET_DOUBLE, m, n, oC);
      }
    } 
    else if (pb[4]) {
      profile = diet_profile_alloc(path, 0, 1, 1);
      diet_matrix_set(diet_parameter(profile,0),
		      A, DIET_VOLATILE, DIET_DOUBLE, m, m, oA);
      print_matrix(A, m, m, (oA == DIET_ROW_MAJOR));
      diet_matrix_set(diet_parameter(profile,1),
		      B, DIET_VOLATILE, DIET_DOUBLE, m, m, oB);
      print_matrix(B, m, m, (oB == DIET_ROW_MAJOR));
      
    } 
    else {
      fprintf(stderr, "Unknown problem: %s !\n", path);
      return 1;
    }
    /*displayProfile(profile, path);*/
    diet_reqID_t rst;
    diet_call_async(profile, &rst);
    printf("valeur de retour de diet_call_async = -%d- \n", rst);
    if (rst > 0){
      printf("debut du diet_waitfor ...\n");
      diet_wait(rst);
      printf("fin du diet_waitfor ...\n");
      if (pb[0]) {
	diet_matrix_get(diet_parameter(profile,0), NULL, NULL, &m, &n, &oA);
        print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
      } 
      else if (pb[4]) {
	diet_matrix_get(diet_parameter(profile,0), NULL, NULL, &m, &n, &oB);
	print_matrix(B, m, n, (oB == DIET_ROW_MAJOR));
      } 
      else {
	diet_matrix_get(diet_parameter(profile,2), &C, NULL, &m, &n, &oC);
	print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
	diet_free_data(diet_parameter(profile,2));
      }
    }
    else {
      printf("error in diet_call_async ...\n");
    }
    diet_profile_free(profile);
  }
  diet_finalize();
  DIET_DEBUG(TEXT_OUTPUT(("FIN du client asynchrone avec des appels en serie.")))
  return 0;
}
