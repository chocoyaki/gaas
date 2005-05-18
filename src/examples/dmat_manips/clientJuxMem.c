/****************************************************************************/
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2005/05/18 14:18:10  mjan
 * Initial adding of JuxMem support inside DIET. dmat_manips examples tested without JuxMem and with JuxMem
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


#define NB_PB 2
static const char* PB[NB_PB] =
  {"MatPROD", "MatSUM"};


/* argv[1]: client config file path
   argv[2]: one of the strings above */

void
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg>\n", cmd);
  exit(1);
}

int
main(int argc, char* argv[])
{
  int i;
  size_t mA, nA, nB, mB; // use size_t for 32 / 64 portability
  char* path = NULL;
  diet_profile_t* profile = NULL;
  diet_profile_t* profile2 = NULL;
  double mat1[15] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0};
  double mat2[18] = {10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,40.0,41.0,42.0};
  double mat3[30] = {19.0,20.0,21.0,22.0,23.0,24.0,25.0,26.0,27.0,28.0,29.0,30.0,31.0,32.0,33.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,40.0,41.0,42.0,50.0,51.0,52.0,53.0,54.0};  

  double* A = NULL;
  double* B = NULL;
  double *D = NULL;
  double *E = NULL;
  diet_matrix_order_t oA, oB, oC,oD,oE;
  int   pb[NB_PB] = {0, 0};
  char car;

  // STATS
  char* STAT_FILE_NAME = NULL;
  FILE* STAT_FILE      = NULL;
  size_t nb_of_requests;
  time_t t1,t2;
  srand(time(NULL));

  if (argc != 2) {
    usage(argv[0]);
  }
  path = argv[1];
  
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
    //diet_matrix_get(diet_parameter(profile,2),&C, NULL, &mA, &nB, &oC);
    store_id(profile->parameters[2].desc.id,"matrice C de doubles");
    store_id(profile->parameters[1].desc.id,"matrice B de doubles");
    store_id(profile->parameters[0].desc.id,"matrice A de doubles");
    //print_matrix(C, mA, nB, (oC == DIET_ROW_MAJOR));
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
  
  time(&t2);
  printf("\n\n COMPUTATION TIME = %d \n\n", (int)(t2-t1));

  printf ("next....");
  scanf("%c",&car);
  printf(" \nRemoving all persistent data............" );

  diet_free_persistent_data(profile->parameters[0].desc.id);
  diet_free_persistent_data(profile->parameters[1].desc.id);
  diet_free_persistent_data(profile->parameters[2].desc.id);
  diet_free_persistent_data(profile2->parameters[1].desc.id);
  diet_free_persistent_data(profile2->parameters[2].desc.id);
  printf(" \n...................data removed\n\n");
  diet_profile_free(profile);
  diet_profile_free(profile2);
  diet_finalize();
  
  return 0;
}
