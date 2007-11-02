/****************************************************************************/
/* General batch submission: serial service on a parallel machine           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Yves CANIOU (ycaniou@ens-lyon.fr)                                   */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2007/11/02 16:12:57  ycaniou
 * Ajout des fichiers exemple. Tests en cours.
 *
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libgen.h> /* basename() */

#include "DIET_server.h"
/* #include "DIET_mutex.h" */
#include <sys/stat.h>

/****************************************************************************
 * PERFORMANCE FUNCTION
 ****************************************************************************/

void make_perf(diet_profile_t *pb)
{
  /* TODO: an API in order to provide the SeD with communication means
  ** with batch scheduler for prediction performances  */
  
  /* For the moment, give arbitrary values */
  pb->walltime = 125 ;
  pb->nbprocs = 1 ;
  pb->nbprocess = pb->nbprocs ;
}


/****************************************************************************
 * SOLVE FUNCTION
 ****************************************************************************/

int solve_concatenation(diet_profile_t *pb)
{
  size_t arg_size1  = 0 ;
  size_t arg_size2  = 0 ;
  char * prologue = NULL ;
  char * copying = NULL ;
  char * cmd = NULL ;
  char * epilogue = NULL ;
  char * script = NULL ;
  char * path1 = NULL ;
  char * path2 = NULL ;
  char * path_result = NULL ;
  char * local_output_filename = NULL ;
  double * ptr_nbreel = NULL ;
  int status = 0 ;
  int result ;
  struct stat buf;

  printf("Resolving batch service 'concatenation'!\n\n") ;

  /* IN args */  
  diet_file_get(diet_parameter(pb,0), NULL, &arg_size1, &path1);
  if ((status = stat(path1, &buf)))
    return status;
  if( !(buf.st_mode & S_IFREG) ) /* regular file */
    return 2;
  printf("Name of the first file: %s\n",path1) ;
  
  diet_scalar_get(diet_parameter(pb,1), &ptr_nbreel, NULL);
  diet_file_get(diet_parameter(pb,2), NULL, &arg_size2, &path2);
  if ((status = stat(path2, &buf)))
    return status;
  if( !(buf.st_mode & S_IFREG) ) /* regular file */
    return 2;
  printf("Name of the second file: %s\n",path2) ;

  /* OUT args */
  /* Resulting file name should be temporary (see mkstemp()) 
     AND given to the parallel job, else overwritten */
  path_result = strdup("/tmp/result.txt") ; /* MUST NOT BE CONSTANT STRING */
  if( diet_file_desc_set(diet_parameter(pb,3), path_result) ) {
    printf("diet_file_desc_set() error\n");
    return 1;
  }
  printf("Name of result file: %s\n",path_result) ;

  /********************************************/
  /* Put the command to submit into a script */
  /********************************************/
  /* Some unecessary things, only for the example */
  prologue = (char*)malloc(500*sizeof(char)) ;
  if( prologue == NULL ) {
    fprintf(stderr,"Memory allocation problem.. not solving the service\n\n") ;
    return 2 ;
  }
  sprintf(prologue,
	  "batchID=$DIET_BATCHNAME\n"
	  "echo \"Name of the frontale station: $DIET_NAME_FRONTALE\"\n"
	  "echo \"Number of nodes:  $DIET_BATCH_NBNODES\"\n"
	  "case \"$batchID\" in\n"
	  "  loadleveler) echo DIET_BATCH_NODESLIST, DIET_BATCH_NODESFILE \
	                  cannot be used in this case ;;\n"
	  "  *) echo \"Reserved Nodes are:\"\n"
	  "     echo $DIET_BATCH_NODESLIST\n"
	  "     echo Name of the batch file containing their identity: \
	        $DIET_BATCH_NODESFILE;;\n"
	  "esac\n"
	  "\n") ;

  /* Data management: scp for file1, NFS for file2 */
  /* Note for advanced usage:  
     one can do the NFS cp in a C thread (see batch_server_2)
     Don't forget to put in batch_server_2 a test on size to be sure that file
     is copied:

     while ( size != %d ) ; do
     sleep 1
     done

  */ 
  copying = (char*)malloc(600*sizeof(char)) ;  
  if( copying == NULL ) {
    fprintf(stderr,"Memory allocation problem.. not solving the service\n\n") ;
    return 2 ;
  }
  sprintf(copying,
	  "case $batchID in\n"
	  "  oar1.6) echo Execute on OAR1.6\n"
	  "          WORKING_DIRECTORY=/home/ycaniou/JobMPI/\n"
	  "          # Copy the file on reserved nodes\n"
	  "          for i in $DIET_BATCH_NODESLIST ; do\n"
	  "            scp $DIET_NAME_FRONTALE:%s $i:/tmp/%s_local\n"
	  "          done\n"
	  "          input_file1=/tmp/%s_local\n\n"
	  "          # Use NFS (we are not on the frontale anymore!)\n"
	  "          ssh $DIET_NAME_FRONTALE \"cp %s $WORKING_DIRECTORY/\"\n"
	  "          input_file2=$WORKING_DIRECTORY/%s ;;\n",
	  "  loadleveler) echo Execute on Loadleveler\n"
	  "               # Machine //, all files accessible from all nodes\n"
	  "               WORKING_DIRECTORY=/users/cri/diet/YC/JobMPI/\n"
	  "               input_file1=%s\n"
	  "               input_file2=%s ;;\n",
	  "  *) echo NOT TESTED! ;;\n"
	  "esac\n",
	  path1,basename(path1),basename(path1),path2,basename(path2)
	  path1,path2
	  ) ; 
  
  /* The proceeding of the command */
  local_output_filename = "/tmp/result_local.txt" ;
  cmd = (char*)malloc(500*sizeof(char)) ;  
  if( cmd == NULL ) {
    fprintf(stderr,"Memory allocation problem.. not solving the service\n\n") ;
    return 2 ;
  }
  sprintf(cmd,
	  "# Execution\n"
	  "case $batchID in\n"
	  "  oar1.6) cd $WORKING_DIRECTORY\n"
	  "          local_output_filename=%s\n"
	  "          mpirun.mpich_1_2 -np $DIET_USER_NBPROCS \
	             -machinefile $DIET_BATCH_NODESFILE \
	             concatenation $input_file1 %.2f \
	             $input_file2 $local_output_filename ;;\n"
	  "  loadleveler) cd $WORKING_DIRECTORY\n"
	  "          local_output_filename=%s\n"
	  "          # Test if job is serial or parallel (usage of poe)\n"
	  "          if [ %d -eq 1 ] ; then\n"
	  "            ./concatenation $input_file1 %.2f \
	               $input_file2 $local_output_filename\n"
	  "          else\n"
	  "            poe -np $DIET_USER_NBPROCS \
	               -machinefile $DIET_BATCH_NODESFILE \
	               concatenation $input_file1 %.2f \
	               $input_file2 $local_output_filename\n"
	  "          fi\n"
	  "esac\n"
	  "\n",local_output_filename, *ptr_nbreel) ;
  
  /* Put the Output file in the right place */
  /* Note: if output on NFS, with "ln -s" (see batch_server_2) 
     or by Diet (see batch_server_3) */
  epilogue = (char*)malloc(200*sizeof(char)) ;  
  if( epilogue == NULL ) {
    fprintf(stderr,"Memory allocation problem.. not solving the service\n\n") ;
    return 2 ;
  }
  sprintf(epilogue,
	  "# Get the result file\n"
	  "scp $local_output_filename $DIET_NAME_FRONTALE:%s\n"
	  ,path_result) ;
  
  /* Make Diet submit */
  script = (char*)malloc( (strlen(prologue)  
			   + strlen(copying)  
			   + strlen(cmd) 
			   + strlen(epilogue)
			   + 1 ) * sizeof(char) ) ;
  sprintf(script,"%s%s%s%s",prologue,copying,cmd,epilogue) ;


  /* Call performance prediction or not, but fields are to be fullfilled */
  make_perf(pb) ;
  
  /* Submission */
  result = diet_submit_parallel(pb, script) ;

  /* Free memory */
  free(prologue) ;
  free(copying) ;
  free(cmd) ;
  free(epilogue) ;
  free(script) ;

  /* Don't free path1, path2 and path_result */
  if( result == -1 )
    printf("Error when submitting the script\n") ;
  return result ;
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

  /* Allocate profile (IN, INOUT, OUT) */
  profile = diet_profile_desc_alloc("concatenation",2,2,3);

  /* Set profile parameters:
     this job is submitted by a parallel/batch system */
  diet_profile_desc_set_parallel(profile) ;

  diet_generic_desc_set(diet_param_desc(profile,0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,3), DIET_FILE, DIET_CHAR);
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
