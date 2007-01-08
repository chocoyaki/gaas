/****************************************************************************/
/* DIET server for sequential submission for lammps                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Yves CANIOU (ycaniou@ens-lyon.fr)                                   */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id: 
 * $Log: 
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h> /* basename() */

#include "DIET_server.h"

#include <sys/stat.h>
#include <sys/time.h> /* For gettimeofday() */
#include <string.h> /* For strlen */

/* For fork() and wait() */
#include <sys/types.h>
#include <sys/wait.h> 

#define MAX_STRING_LENGTH 100

/****************************************************************************
 * SOLVE FUNCTION
 ****************************************************************************/

int solve_lammps(diet_profile_t *pb)
{
  size_t arg_size1  = 0 ;
  char chaine[ MAX_STRING_LENGTH ] ;
  char * path1 = NULL ;
  char * path_result = NULL ;
  int status = 0 ;
  char * lammpsName ;
  struct stat buf;
  struct timeval uur70 ;
  char ** options ;
  
  printf("Resolving sequential service 'lammps'!\n\n") ;
  /* Lammps binary dependant of installation. We suppose no renaming but
     an environment variable set within ~/.bash_profile for example */
  lammpsName = getenv("LAMMPS_NAME") ;
  if( lammpsName == NULL )
    lammpsName = "lammps_diet" ;
  
  /* IN args */
  diet_file_get(diet_parameter(pb,0), NULL, &arg_size1, &path1);
  if ((status = stat(path1, &buf)))
    return status;
  if( !(buf.st_mode & S_IFREG) ) /* regular file */
    return 2;
  printf("Name of the input file: %s\n",path1) ;
  
  /* OUT args */
  if( gettimeofday( &uur70, NULL ) == 0 ) {
    sprintf(&chaine,"/tmp/lammps_%d.txt", uur70.tv_sec) ;
    path_result = strdup(chaine) ; 
  } else {
    perror("Cannot access time on server. Use a static name\n") ;
    path_result = strdup("/tmp/lammps.txt") ; /*MUST NOT BE A CONSTANT STRING*/
  }
  if( diet_file_desc_set(diet_parameter(pb,1), path_result) ) {
      printf("diet_file_desc_set() error\n");
      return 1;
    }
  printf("Name of result file: %s\n",path_result) ;
	    
  /* Call lammps */
  if( (pid=fork()) == 0 ) { /* son */
    options = (char**)malloc(3*sizeof(char*)) ;
    options[1]=(char*)malloc((strlen(path1)+2)*sizeof(char)) ;
    sprintf(option[1],"< %s",path1) ;
    options[2]=(char*)malloc((strlen(path_result)+2)*sizeof(char)) ;
    sprintf(options[2],"> %s",path_result) ;
    options[3]=NULL ;

    /* Submission, that never returns */    
    if( execve(lammpsName, options, NULL)==-1 ) {
      perror("Error executing Lammps\n") ;
    }
  } else /* father */
    wait() ;
  
  /* Free memory */
        
  /* Don't free path1, and path_result since not duplicated by CORBA */
  return 0 ;
}

/****************************************************************************
 * MAIN
 ****************************************************************************/

int
main(int argc, char* argv[])
{
  int res = 0;
  int nb_max_services=1 ;
  diet_profile_desc_t* profile = NULL ;
  
  
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }  

  /* Initialize table with maximum services */
  diet_service_table_init(nb_max_services);

  /* Allocate batch profile (IN, INOUT, OUT) */
  profile = diet_profile_desc_alloc("concatenation",0,0,1);

  /* Set profile parameters:
     this job is submitted by a sequential system..
     Note that this is default, then the line is unecessary
     diet_profile_desc_set_sequential(profile) ; */

  diet_generic_desc_set(diet_param_desc(profile,0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_FILE, DIET_CHAR);
  /* All done */

  /* Add service to the service table */
  if( diet_service_table_add(profile, NULL, solve_concatenation) ) return 1 ;
  
  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);

  /* Print the table to check */
  diet_print_service_table();

  /* Launch the server */
  res = diet_SeD(argv[1], argc, argv);

  return res;
}
