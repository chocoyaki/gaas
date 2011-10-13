/****************************************************************************/
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.10  2011/01/23 19:20:00  bdepardo
 * Fixed memory and resources leaks, variables scopes, unread variables
 *
 * Revision 1.9  2010/03/05 15:52:08  ycaniou
 * Ordered things in CMakelist.txt and update Display (Batch, build_version...)
 * Fix version guess of compiler (was gcc only)
 * Use option to avoid %zd warning
 * Undo previous cast of size_t into int
 *
 * Revision 1.8  2010/03/05 04:11:05  ycaniou
 * Remove warning when printing size_t
 *
 * Revision 1.7  2006/06/30 15:26:02  ycaniou
 * C++ commentaries -> C commentaries to remove compilation warnings
 *
 * Revision 1.6  2006/01/13 10:41:53  mjan
 * Updating DIET for next JuxMem (0.2)
 *
 * Revision 1.5  2005/06/02 08:05:40  mjan
 * Updated client for looping requests.
 *
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "DIET_client.h"


#define print_condition 1
#define print_matrix(mat, m, n, rm)             \
  if (print_condition) {                        \
    size_t i, j;                                \
    printf("%s (%s-major) = \n", #mat,          \
           (rm) ? "row" : "column");            \
    printf(" %zd %zd \n",m,n);                  \
    for (i = 0; i < (m); i++) {                 \
      for (j = 0; j < (n); j++) {               \
        if (rm)                                 \
          printf("%3f ", (mat)[j + i*(n)]);     \
        else                                    \
          printf("%3f ", (mat)[i + j*(m)]);     \
      }                                         \
      printf("\n");                             \
    }                                           \
    printf("\n");                               \
  }

void
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s %s <file.cfg>\n", cmd, "[<n>] [<n µs>]");
  fprintf(stderr, "        %s 1000 1000 %s\n", cmd, "client.cfg (1ms between two steps of the loop (1000 iterations))");
  exit(1);
}

int
main(int argc, char* argv[])
{
  int i, pause = 0;
  int n_loops = 1;
  char* path = NULL;
  char* path2 = NULL;
  size_t mA, nA, nB, mB; /* use size_t for 32 / 64 portability */
  diet_profile_t* profile = NULL;
  diet_profile_t* profile2 = NULL;
  double mat1[15] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0};
  double mat2[18] = {10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,40.0,41.0,42.0};
  double mat3[30] = {19.0,20.0,21.0,22.0,23.0,24.0,25.0,26.0,27.0,28.0,29.0,30.0,31.0,32.0,33.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,40.0,41.0,42.0,50.0,51.0,52.0,53.0,54.0};  

  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  double *D = NULL;
  double *E = NULL;
  diet_matrix_order_t oA, oB, oC,oD,oE;

  struct timeval tv_pause;
  
  A = mat1;
  B = mat2;
  E = mat3;

  mA = 5;
  nA = 3;
  mB= 3;
  nB= 6;

  oA = DIET_ROW_MAJOR ;
  oB = DIET_ROW_MAJOR ;
  oC = DIET_ROW_MAJOR ;
  oD = DIET_ROW_MAJOR;
  oE = DIET_ROW_MAJOR;

  if (argc != 4) {
    usage(argv[0]);
  }
  
  n_loops = atoi(argv[1]);
  pause = atoi(argv[2]);

  if (diet_initialize(argv[3], argc, argv)) { 
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  path = (char*) malloc(sizeof(char) * 10);
  path2 = (char*) malloc(sizeof(char) * 10);

  strcpy(path,"MatPROD");
  profile = diet_profile_alloc(path, 1, 1, 2);
  diet_matrix_set(diet_parameter(profile,0),
                  A, DIET_PERSISTENT, DIET_DOUBLE, mA, nA, oA);
  print_matrix(A, mA, nA, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,1),
                  B, DIET_PERSISTENT, DIET_DOUBLE, mB, nB, oB);
  print_matrix(B, mB, nB, (oB == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,2),
                  NULL, DIET_PERSISTENT_RETURN, DIET_DOUBLE, mA, nB, oC);

  strcpy(path2,"MatSUM");
  profile2 = diet_profile_alloc(path2, 1, 1, 2);
  diet_matrix_set(diet_parameter(profile2,0),
                  NULL, DIET_PERSISTENT, DIET_DOUBLE, mA, nB, oC);
  
  diet_matrix_set(diet_parameter(profile2,1),
                  E, DIET_PERSISTENT, DIET_DOUBLE, mA, nB, oE);
  print_matrix(E, mA, nB, (oE == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile2,2),
                  NULL, DIET_PERSISTENT_RETURN, DIET_DOUBLE, mA, nB, oD);
  
  for (i = 0; i < n_loops; i++) {
    if (!diet_call(profile)) {
      diet_matrix_get(diet_parameter(profile,2),&C, NULL, &mA, &nB, &oC);
      print_matrix(C, mA, nB, (oC == DIET_ROW_MAJOR));
    }

    /** For giving the JuxMem ID of mat C to profile 2 */
    if (i == 0) {
      diet_use_data(diet_parameter(profile2,0), profile->parameters[2].desc.id);
    }
    
    if (pause != 0) {
      tv_pause.tv_sec = 0;
      tv_pause.tv_usec = pause;
      select(0, NULL, NULL, NULL, &tv_pause);
    }

    if (!diet_call(profile2)) {
      diet_matrix_get(diet_parameter(profile2,2), &D, NULL, &mA, &nB, &oD);
      print_matrix(D, mA, nB, (oD == DIET_ROW_MAJOR));
    }
  }
  
  diet_profile_free(profile);
  diet_profile_free(profile2);
  free(path);
  free(path2);
  diet_finalize();
  
  return 0;
}
