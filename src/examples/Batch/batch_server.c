/****************************************************************************/
/* DIET server for Batch submission                                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Yves CANIOU (ycaniou@ens-lyon.fr)                                   */
/* $LICENSE$                                                                */
/****************************************************************************/
/* 
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
  pb->nbprocs = 2 ;
  pb->nbprocess = pb->nbprocs ;
}


/****************************************************************************
 * SOLVE FUNCTION
 ****************************************************************************/

int solve_helloMPI(diet_profile_t *pb)
{
  size_t arg_size1  = 0 ;
  size_t arg_size2  = 0 ;
  char *prologue = NULL ;
  char *copying = NULL ;
  char *cmd = NULL ;
  char *epilogue = NULL ;
  char *script = NULL ;
  char *path1 = NULL ;
  char *path2 = NULL ;
  char *path_result = NULL ;
  double* ptr_nbreel = NULL ;
  int status = 0 ;
  int result ;
  struct stat buf;

  printf("Résolution de helloMPI !\n\n") ;

  /* IN args */  
  diet_file_get(diet_parameter(pb,0), NULL, &arg_size1, &path1);
  if ((status = stat(path1, &buf)))
    return status;
  if( !(buf.st_mode & S_IFREG) ) /* regular file */
    return 2;
  printf("Le nom du fichier est %s\n",path1) ;
  
  diet_scalar_get(diet_parameter(pb,1), &ptr_nbreel, NULL);
  diet_file_get(diet_parameter(pb,2), NULL, &arg_size2, &path2);
  if ((status = stat(path2, &buf)))
    return status;
  if( !(buf.st_mode & S_IFREG) ) /* regular file */
    return 2;
  printf("Le nom du fichier est %s\n",path2) ;

  /* OUT args */
  path_result = "JobMPI/result.txt" ;
  if( diet_file_desc_set(diet_parameter(pb,3), path_result) ) {
    printf("diet_file_desc_set error\n");
    return 1;
  }
  printf("Le nom du fichier est %s\n",path_result) ;

  /******************************************/
  /* Make the command to submit in a script */
  /******************************************/
  printf("Making script...\n\n") ;
  /* Some unecessary things, only for the example */
  prologue = (char*)malloc(300*sizeof(char)) ;  
  sprintf(prologue,
	  "echo \"Nom de la frontale: $DIET_NAME_FRONTALE\"\n"
	  "echo \"Reserved Nodes are:\"\n"
	  "echo $DIET_BATCH_NODESLIST\n"
	  "echo \"Number of nodes:  $DIET_BATCH_NBNODES\"\n"
	  "echo \"Nom du fichier batch qui les gère :"
	  " $DIET_BATCH_NODESFILE\"\n"
	  "\n") ;
  
  /* Data management: scp for file1, NFS for file2 */
  /* Note: one can do the NFS cp in a C thread (see batch_server_2) */
  /* mettre dans batch_server_2 un test sur la taille pour s'assurer 
     que le fichier est copié :

     while ( size != %d ) ; do
     sleep 1
     done

  */ 
  copying = (char*)malloc(400*sizeof(char)) ;  
  sprintf(copying,
	  "WORKING_DIRECTORY=/home/ycaniou/JobMPI/\n"
	  "# Copy the file on reserved nodes\n"
	  "for i in $DIET_BATCH_NODESLIST ; do\n"
	  "  scp $DIET_NAME_FRONTALE:%s $i:%s\n"
	  "done\n"
	  "input_file1=%s\n"
	  "\n"
	  "# Use NFS\n"
	  "ssh $DIET_NAME_FRONTALE \"cp %s $WORKING_DIRECTORY/\"\n"
	  "input_file2=%s\n"
	  "\n"
	  ,path1,path1,path1,path2,basename(path2)) ; 
  
  /* The MPI command itself */
  cmd = (char*)malloc(300*sizeof(char)) ;  
  sprintf(cmd,
	  "# Execution\n"
	  "cd $WORKING_DIRECTORY\n"
	  "mpirun.mpich_1_2 -np $DIET_USER_NBPROCS "
	  "-machinefile $DIET_BATCH_NODESFILE "
	  "helloMPI %.2f\n"
	  "\n",*ptr_nbreel) ;
  
  /* Put the Output file in the right place */
  /* Note: if output on NFS, with "ln -s" (see batch_server_2) 
     or by Diet (see batch_server_3) */
  epilogue = (char*)malloc(100*sizeof(char)) ;  
  sprintf(epilogue,
	  "# Get the result file\n"
	  "#FIXME\n"
	  "#scp $DIET_BATCH_STDOUT $DIET_NAME_FRONTALE:%s\n"
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
  result = diet_submit_batch(pb, script) ;
  if( result )
    printf("Error when submitting the script\n") ;

  status=120 ;
  printf("Waiting %d seconds\n",status) ;
  sleep(status) ;
  
  /* Free memory */
  free(prologue) ;
  free(copying) ;
  free(cmd) ;
  free(epilogue) ;
  free(script) ;

  /* Don't free path1, path2 and path_result */
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
  profile = diet_profile_desc_alloc("helloMPI",2,2,3);

  /* Set profile parameters : this job is submitted by a batch system */
  diet_profile_desc_set_batch(profile) ;

  diet_generic_desc_set(diet_param_desc(profile,0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,3), DIET_FILE, DIET_CHAR);
  /* All done */

  /* Add service to the service table */
  if( diet_service_table_add(profile, NULL, solve_helloMPI) ) return 1 ;
  
  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);

  /* Print the table to check */
  diet_print_service_table();

  /* Launch the server */
  res = diet_SeD(argv[1], argc, argv);

  return res;
}
