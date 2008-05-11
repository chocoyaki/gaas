/****************************************************************************/
/* DIET client for Batch submission:                                        */
/* Calling the PasTiX sparse linear solver (VecPar 08)                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Yves CANIOU (ycaniou@ens-lyon.fr)                                   */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log: *
 ****************************************************************************/

/* Description of this code:
   It reads in a file different arguments that are used as parameter in a
   given number of submissions.
   These are made in async. way.
   It has been used for VecPar 08

   Notes: 
   - args for the DIET submission are in file because at the date of
   writing this code, there are still interoperability problems with
   AIX platforms, and this is designed to work for a multiple
   architectures platform.
   - a burst client should be employed
*/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"

#include <sys/time.h>

#define DEBUG

/* argv[1]: client config file path */

int
main(int argc, char* argv[])
{
  int i ;
#ifndef DEBUG
  int reqID ;
#endif
  size_t file_size ;
  char * path = NULL;
  char * tmpFileName = NULL ;
  diet_profile_t ** arrayOfProfile = NULL;
  int numberOfSubmissions ;
  char * matrixName = NULL ;
  double fillingCoefficient = 0.4 ;
  char ** arrayOfResultFileNames ;
  diet_reqID_t * arrayOfReqID ;
  
  struct timeval tv ;
  struct timezone tz ;

  if( argc != 3 ) {
    fprintf(stderr, "Usage: %s <file.cfg> <file1>\n",
	    argv[0]);
    return 1;
  }

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  /* Set the name of the DIET service to call */
  path = "PasTiX" ;

  /* Read the number of submissions in the file accessed through argv[2]
     and reserve memory for profiles */
  numberOfSubmissions = 1 ;
  arrayOfProfile = (diet_profile_t**)malloc(numberOfSubmissions*
					    sizeof(diet_profile_t*)) ;

  arrayOfResultFileNames = (char**)
    malloc(numberOfSubmissions*sizeof(char*)) ;
  arrayOfReqID = (diet_reqID_t*)
    malloc(numberOfSubmissions*sizeof(diet_reqID_t)) ;
  
  for( i=0 ; i < numberOfSubmissions ; i++ ) {

    /* Build the profile: file (IN), file (OUT) */
    arrayOfProfile[i] = diet_profile_alloc(path, 0, 0, 1);

    /* Read the name of the matrice and its parameter */
    matrixName = "AuPif" ;
    fillingCoefficient = 0.4 ;

#ifdef DEBUG
    printf("Debug mode: use %s as input file...\n",argv[2]) ;
    tmpFileName = strdup(argv[2]) ;

    /* Set DIET args */
    if( diet_file_set(diet_parameter(arrayOfProfile[i],0), DIET_VOLATILE,
		      tmpFileName)) {
      printf("file1: diet_file_set error\n");
      return 1;
    }
    if( diet_file_set(diet_parameter(arrayOfProfile[i],1), DIET_VOLATILE,
		      NULL)) {
      printf("result_file: diet_file_set error\n");
      return 1;
    }
    
    /* Make the async calls */
    gettimeofday(&tv, &tz);
    printf("Debug mode: submission: %ld:%ld\n\n",
	   tv.tv_sec,tv.tv_usec) ;

    if( diet_call(arrayOfProfile[i]) ) {
      printf("Error in diet_call()\n") ;
      exit(0) ;
    }
    
    printf("Submission %d terminated!\n",i) ;
    diet_file_get(diet_parameter(arrayOfProfile[i],1), NULL, &file_size,
		  &(arrayOfResultFileNames[i]));
    if( arrayOfResultFileNames[i] &&
	(*arrayOfResultFileNames[i] != '\0')) {
      printf("Location of returned file is %s, its size is %d.\n",
	     arrayOfResultFileNames[i], (int) file_size);
    }
  }
  gettimeofday(&tv, &tz);
  printf("Finishing the whole set of experiment at %ld:%ld\n\n",tv.tv_sec,
	 tv.tv_usec) ;
#else
    /* Create a temporary file and write input args into the file */
    tmpFileName = strdup("FixME!") ;


    /* Set DIET args */
    if (diet_file_set(diet_parameter(arrayOfProfile[i],0), DIET_VOLATILE,
		      tmpFileName)) {
      printf("file1: diet_file_set error\n");
      return 1;
    }
    if (diet_file_set(diet_parameter(arrayOfProfile[i],1), DIET_VOLATILE,
		      NULL)) {
      printf("result_file: diet_file_set error\n");
      return 1;
    }
    
    /* Make the async calls */
    gettimeofday(&tv, &tz);
    printf("Beginning the whole set experiment submission: %ld:%ld\n\n",
	   tv.tv_sec,tv.tv_usec) ;

    if( diet_call_async(arrayOfProfile[i], &(arrayOfReqID[i]) ) )
      printf("Error in diet_async_call() #%d\n", i) ;
    else {
      printf("Request ID value %d submitted\n", i) ;
      if( arrayOfReqID[i] < 0 )
	printf("Error in request value\n") ;
    }
  }
  
  /* Wait for DIET answers */
  for( i=0 ; i < numberOfSubmissions ; i++ ) {
    diet_wait_any( &reqID ) ;
    printf("Submission %d terminated!\n",reqID) ;
    diet_file_get(diet_parameter(arrayOfProfile[reqID],1), NULL, &file_size,
		  &(arrayOfResultFileNames[reqID]));
    if( arrayOfResultFileNames[reqID] &&
	(*arrayOfResultFileNames[reqID] != '\0')) {
      printf("Location of returned file is %s, its size is %d.\n",
	     arrayOfResultFileNames[reqID], (int) file_size);
    }
  }
  
  gettimeofday(&tv, &tz);
  printf("Finishing the whole set of experiment at %ld:%ld\n\n",tv.tv_sec,
	 tv.tv_usec) ;
#endif
  /* Release memory and space storage */
  
  /* Delete memory */
  for( i=0 ; i < numberOfSubmissions ; i++ ) {
    diet_profile_free( arrayOfProfile[i] ) ;
  }
  
  diet_finalize();

  return 0;
}

