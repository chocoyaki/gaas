/****************************************************************************/
/* dmat_manips example: a DIET client for transpose, MatSUM and MatPROD     */
/*   problems (MatSUM is expanded to SqMatSUM and SqMatSUM_bis).            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2004/10/08 12:19:59  hdail
 * Correcting warnings with mismatched types (size_t and int) on 64-bit.
 *
 * Revision 1.7  2004/09/14 12:46:37  hdail
 * Commented out unused variables so that compile succeeds with DIET configure
 * option "--enable-maintainer-mode".
 *
 * Revision 1.6  2004/03/02 16:37:10  mcolin
 * make a uniform storage form for all matrices
 *
 * Revision 1.5  2004/03/02 14:19:10  bdelfabr
 * *** empty log message ***
 *
 * Revision 1.4  2004/02/27 10:37:19  bdelfabr
 * example for data persistency : a sequence of three operations is computed
 * MatPROD, MatSUM, tA
 * a file ID.MA.... is creted that contains all the persistent data identifiers.
 * To run this examples an other time, remove this file or modify the diet_use_data method to
 * use the good id.
 *
 * Revision 1.2  2003/10/14 20:24:32  bdelfabr
 * example with persistence using MatPROD MatSUM and T
 * C=A*B D=C+E A=tA
 * clientWithID ../cfgs/client.cfg *+T
 *
 * Revision 1.1  2003/10/10 15:29:04  bdelfabr
 * data Persistence example : invoke MatPPROD first then T - to be completed
 *
 * Revision 1.20  2003/09/26 14:07:01  pcombes
 * Get the examples out of the automake chain, to make Makefiles more readable.
 * The generated before.mk includes the right Makefile.inc.
 *
 * Revision 1.19  2003/08/01 19:35:11  pcombes
 * Fix bugs on memcpy (misused) and DIET_STAT_FILE_NAME.
 *
 * Revision 1.18  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.17  2003/07/09 17:11:43  pcombes
 * Add --pause option to longer each step of the loop of submissions.
 *
 * Revision 1.15  2003/02/07 17:05:23  pcombes
 * Add SqMatSUM_opt with the new convertor API.
 * Use diet_free_data to properly free user's data.
 *
 * Revision 1.14  2003/01/27 17:55:49  pcombes
 * Bug fix on OUT matrix: C was not initialized.
 *
 * Revision 1.11  2002/12/24 08:25:38  lbertsch
 * Added a way to execute n tests by a command line argument :
 * Usage is : client --repeat <n> <cfg file> <op>
 *
 * Revision 1.8  2002/11/07 18:42:42  pcombes
 * Add includes and configured Makefile variables to install directory.
 * Update dgemm to the implementation that is hardcoded in FAST.
 *
 * Revision 1.7  2002/09/17 15:23:18  pcombes
 * Bug fixes on inout arguments and examples
 * Add support for omniORB 4.0.0
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
#define print_matrix(mat, m, n, rm)        \
  if (print_condition) {                   \
    size_t i, j;                           \
    printf("%s (%s-major) = \n", #mat,     \
           (rm) ? "row" : "column");       \
    printf(" %zd %zd \n",m,n);               \
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


#define NB_PB 5
static const char* PB[NB_PB] =
  {"*+T", "MatPROD", "MatSUM", "SqMatSUM", "SqMatSUM_opt"};


/* argv[1]: client config file path
   argv[2]: one of the strings above */

void
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s %s <file.cfg> [%s|%s|%s|%s|%s]\n", cmd,
	  "[--repeat <n>] [--pause <n µs>]", PB[0], PB[1], PB[2], PB[3], PB[4]);
  fprintf(stderr, "    ex: %s client.cfg T\n", cmd);
  fprintf(stderr, "        %s --repeat 1000 client.cfg MatSUM\n", cmd);
  fprintf(stderr, "        %s --repeat 1000 --pause 1000 %s\n",
	  cmd, "client.cfg MatSUM (1ms between two steps of the loop)");
  exit(1);
}

int
main(int argc, char* argv[])
{
  //int i, mA, nA,nB,mB;  // j, k
  int i;
  size_t mA, nA, nB, mB; // use size_t for 32 / 64 portability
  int n_loops = 1;
  char* path = NULL;
  diet_profile_t* profile = NULL;
  diet_profile_t* profile1 = NULL;
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
  //double a=1.0;
  //double b=2.5;
  //double c=1.5;
  int   pb[NB_PB] = {0, 0, 0, 0, 0};
  char car;

  // STATS
  char* STAT_FILE_NAME = NULL;
  FILE* STAT_FILE      = NULL;
  //struct timeval tv, tv_pause;
  size_t nb_of_requests;
  int pause = 0;   // sec
  time_t t1,t2;
    //char *idC, *idD;
  srand(time(NULL));

  for (i = 1; i < argc - 2; i++) {
    if (strcmp("--repeat", argv[i]) == 0) {
      n_loops = atoi(argv[i + 1]);
      i++;
      memmove(argv + i - 2, argv + i, (argc - i)*sizeof(char*));
      i -= 2;
      argc -= 2;
    } else if (strcmp("--pause", argv[i]) == 0) {
      pause = atoi(argv[i + 1]);
      i++;
      memmove(argv + i - 2, argv + i, (argc - i)*sizeof(char*));
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
  
#undef print_condition
#define print_condition n_loops == 1

  A = mat1;
  B = mat2;
  E = mat3;

  mA = 5;
  nA = 3;
  mB= 3;
  nB= 6;
 
  if (diet_initialize(argv[1], argc, argv)) { 
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }
  time(&t1);
  for (i = 0; i < NB_PB; i++) {
    
    if ((pb[i] = !strcmp(path, PB[i]))) {
      printf("pb[%d] = %d",i,pb[i]);
      break;
    }
  }
  // Square matrix problems:
  

  if ((STAT_FILE_NAME = getenv("DIET_STAT_FILE_NAME")))
    STAT_FILE = fopen(STAT_FILE_NAME, "wc");
  nb_of_requests = 0;
  
  oA = DIET_ROW_MAJOR ;
  oB = DIET_ROW_MAJOR ;
  oC = DIET_ROW_MAJOR ;
  oD = DIET_ROW_MAJOR;
  oE = DIET_ROW_MAJOR;
  

  strcpy(path,"MatPROD");
  profile = diet_profile_alloc(path, 1, 1, 2);
  diet_matrix_set(diet_parameter(profile,0),
		  A, DIET_PERSISTENT, DIET_DOUBLE, mA, nA, oA);
  print_matrix(A, mA, nA, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,1),
		  B, DIET_PERSISTENT, DIET_DOUBLE, mB, nB, oB);
  print_matrix(B, mB, nB, (oB == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,2),
		  NULL, DIET_PERSISTENT, DIET_DOUBLE, mA, nB, oC);
  
  if (!diet_call(profile)) {
    diet_matrix_get(diet_parameter(profile,2),&C, NULL, &mA, &nB, &oC);
    store_id(profile->parameters[2].desc.id,"matrice C de doubles");
    store_id(profile->parameters[1].desc.id,"matrice B de doubles");
    store_id(profile->parameters[0].desc.id,"matrice A de doubles");
    print_matrix(C, mA, nB, (oC == DIET_ROW_MAJOR));
    // diet_profile_free(profile); 
    
  }
     
  printf ("next....");
  scanf("%c",&car);

  strcpy(path,"MatSUM");
  profile2 = diet_profile_alloc(path, 1, 1, 2);
  
  printf("second pb\n\n");
  diet_use_data(diet_parameter(profile2,0),profile->parameters[2].desc.id);
  diet_matrix_set(diet_parameter(profile2,1),
		  E, DIET_PERSISTENT, DIET_DOUBLE, mA, nB, oE);
   print_matrix(E, mA, nB, (oE == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile2,2),
		  NULL, DIET_PERSISTENT, DIET_DOUBLE, mA, nB, oD);
  
  if (!diet_call(profile2)) {
    diet_matrix_get(diet_parameter(profile2,2), &D, NULL, &mA, &nB, &oD);
   print_matrix(D, mA, nB, (oD == DIET_ROW_MAJOR));
   store_id(profile2->parameters[2].desc.id,"matrice D de doubles");
   store_id(profile2->parameters[1].desc.id,"matrice E de doubles");
   //  diet_profile_free(profile2);
  }
  
  printf ("next....");
  scanf("%c",&car);
  strcpy(path,"T");
  
  printf("third  pb  = T\n\n");
  strcpy(path,"T");
  profile1 = diet_profile_alloc(path, -1, 0, 0);
  
  diet_use_data(diet_parameter(profile1,0),profile->parameters[0].desc.id);
  if (!diet_call(profile1)) {
    diet_matrix_get(diet_parameter(profile1,0), NULL, NULL, &mA, &nA, &oA);
     print_matrix(A, mA, nA, (oA == DIET_ROW_MAJOR));
     //  diet_profile_free(profile1);
    }       
  


  time(&t2);
  printf("\n\n COMPUTATION TIME = %d \n\n", (int)(t2-t1));

  printf ("next....");
  scanf("%c",&car);
  printf(" \nRemoving all persistent data............" );
  //getchar();
  diet_free_persistent_data(profile->parameters[0].desc.id);
  diet_free_persistent_data(profile->parameters[1].desc.id);
  diet_free_persistent_data(profile->parameters[2].desc.id);
  diet_free_persistent_data(profile2->parameters[1].desc.id);
  diet_free_persistent_data(profile2->parameters[2].desc.id);
  printf(" \n...................data removed\n\n");
  diet_profile_free(profile);
  diet_profile_free(profile1);
  diet_profile_free(profile2);
  diet_finalize();
  
  return 0;
}
