/****************************************************************************/
/* Batch submission:                                                        */
/* Calling the PasTiX sparse linear solver (VecPar 08)                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Yves CANIOU (ycaniou@ens-lyon.fr)                                   */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/05/11 16:25:22  ycaniou
 * Added an almost finished prototype as an example of using Cori_batch to
 *   automatically tune a script (moldability) for batch submissions
 *
 ****************************************************************************/

/* Description of this code:
   Realize the resolution of PasTiX problems by calling PasTiX application
   
   - It tunes the script that lunches the application: makes the right number
   of processors depending on the max idle proc available
     -> Use CoRI for batch systems
   - The scheduling is made by selecting the site that has the maximum number
   of idle processors
     -> Use Plug-in schedulers
   
   It has been used for VecPar 08
*/

#include <string.h>
#include <unistd.h>              /* for getcwd() POSIX.1-2001 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libgen.h>              /* basename()   */
#include <fcntl.h>               /* for O_RDONLY */
#include <errno.h>               /* for errno    */
#include <assert.h>              /* for assert   */

#include "DIET_server.h"
/* #include "DIET_mutex.h" */
#include <sys/stat.h>

#define MAX_NBDIGITS 10     /* Number of digit read in PasTiX perf pref file */
#define MAX_FILEDIGITS 32   /* Size (digit) read in PasTiX perf pref file    */
                            /*  -> 3*MAX_NBDIGITS+2 because n;n;n            */
#define PasTiXPredPerfName "FIXME!"

#if defined YC_DEBUG || defined DEBUG_YC
#define DEBUG
#endif
/* #define DEBUG */

/****************************************************************************
 * UTILITY FUNCTION
 ****************************************************************************/

void
copyFile( char * inputName, char * outputName )
{
  int inputFile, outputFile ;
  char line[512] ;
  int bytes ;

  if((inputFile = open(inputName, O_RDONLY)) == -1) {
    perror("open") ;
    exit(-1) ;
  }
  if((outputFile = open(outputName, O_WRONLY | O_CREAT)) == -1) {
    perror("open") ;
    exit(-1) ;
  }
  while((bytes = read(inputFile, line, sizeof(line))) > 0)
    write(outputFile, line, bytes) ;

  close(inputFile) ;
  close(outputFile) ;
}

int
readn(int fd, char * buffer, int n)
{
  int nread;
  int nleft;
  char * ptr;

#if defined YC_DEBUG_
  TRACE_TEXT(TRACE_ALL_STEPS,"Enter readn() " << n << " octets to read\n") ;
#endif

  ptr = buffer ;
  nleft = n;
  while( nleft != 0 ) {
    if( (nread = read(fd, ptr, nleft)) < 0 ) {
      if( nread < 0 )       /* ERROR */
#if defined YC_DEBUG_
	switch(errno) {
	case EBADF:
	  ERROR("Descripteur de fichiers invalide ou pas ouvert en lecture",0) ;
	case EFAULT:
	  ERROR("Buffer pointe en dehors de l'espace d'adressage",0) ;
	case EINVAL:
	  ERROR("EINVAL",0) ;
	case EIO:
	  ERROR("EIO",0) ;
	default:
	  ERROR("Undefined",0) ;
	}
#endif
	return( 0 ) ;
    } else if( nread == 0 ) /*EOF*/
      break ;
    nleft -= nread;
    ptr += nread;
  }
  return( n-nleft ) ;
}

size_t
writen(int fd, const char * buffer, size_t n)
{
  size_t nleft;
  size_t nwritten;
  const char * ptr;
  
  ptr = buffer ;
  nleft = n ;
  while( nleft > 0 ) {
    if( (nwritten = write(fd, ptr, nleft)) <= 0 ) {
      if( errno == EINTR )
	nwritten = 0 ;	/* and call write() again */
      else
	return( 0 ) ;	/* error */
    }
    nleft -= nwritten ;
    ptr   += nwritten ;
  }
  return(n) ;
}

/****************************************************************************
 * PERFORMANCE FUNCTIONS
 ****************************************************************************/

void
make_perfPred( diet_profile_t * pb)
{
  int nbnodes ;
  char * current_directory ;
  char * path_result ;
  char * path_input ;
  int file_descriptor_input ;
  int file_descriptor_result ;
  char * command ;
#ifndef DEBUG
  char integers_string[MAX_FILEDIGITS] ; /* Will contain result file: n;n;n */
  char * ptr_coma ;
  char * tmpPtr ;
  int i, nbread ;
#endif
 
  /* At 2008.05.05 DIET API, we need to use our own vector here */
  estVector_t perfBatchVal = diet_new_estVect() ;


  /**************** Create file to pass args to the script ***************/
  current_directory = (char*)malloc(sizeof(char)*100) ;
  /* Note: we cannot use get_current_dir_name() which is a GNU extension */
  if( getcwd( current_directory, 99 ) == NULL ) {
    printf("Current directory is too long for the size of reserved memory!\n"
	   "Change it...\n\n") ;
    exit( 0 ) ;
  }
  current_directory[99]='\0';
  
  path_input = (char*)malloc(sizeof(char)*(strlen(current_directory)+30)) ;
  sprintf(path_input,"%s/pasTiX_predInputs.XXXXXX",current_directory) ;
  file_descriptor_input = mkstemp( path_input ) ;
  if( file_descriptor_input == -1 ) {
    perror("mkstemp") ;
  }
  /* Create file to get results from the script */
  path_result = (char*)malloc(sizeof(char)*(strlen(current_directory)+30)) ;
  sprintf(path_result,"%s/pasTiX_predResults.XXXXXX",current_directory) ;
  file_descriptor_result = mkstemp( path_result ) ;
  if( file_descriptor_result == -1 ) {
    perror("mkstemp for") ;
  }

  /****************** Get information and put it in the file **************/
  /* Set Collector and get NB_FREE_RESOURCES_IN_DEFAULT_QUEUE in vector */
  diet_estimate_cori(perfBatchVal,
		     EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE,
		     EST_COLL_BATCH, pb);
  /* Access NB_FREE_RESOURCES_IN_DEFAULT_QUEUE value */
  nbnodes = (int)
    diet_est_get_system(perfBatchVal,
			EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE, 0);
#ifdef DEBUG
  printf("There is %d idle processor in the default queue\n", nbnodes) ;
  printf("Set arbitrary value for nbprocs, walltime, etc.\n") ;
  pb->nbprocs = 2 ;
  pb->nbprocess = 2 ;
  pb->walltime = 125 ;
#else

  /* If 0 idle resources, what do we do? */
  if( nbnodes == 0 )
    printf("Fixme: there is no idle resource at the moment\n") ;
    
  /* Put the value in input file */
  sprintf(integers_string,"%d", nbnodes) ;
  if( writen(file_descriptor_input, integers_string, strlen(integers_string))
      != strlen(integers_string) ) {
    printf("Cannot write the input PasTiX file on the filesystem") ;
    exit(-1) ;
  }

  /************************** Call script *****************************/
  command = (char*)malloc(sizeof(char)*(strlen(PasTiXPredPerfName) +
					strlen(path_input) +
					strlen(path_result))) ;
  sprintf(command,"%s %s %s", PasTiXPredPerfName, path_input, path_result) ;
  system(command) ; /* Exec script and return */

  
  /* Read results: nbnodes, nbprocess on each node and walltime */
  for( i = 0 ; i<=MAX_NBDIGITS ; i++ )
    integers_string[i] = '\0' ;
  if( (nbread=readn(file_descriptor_result,integers_string,
		    MAX_FILEDIGITS)) == 0 ) {
    printf("Error reading PasTiX pref. pred. file") ;
    exit(-1) ;
  }
  /* Just in case */
  if( integers_string[nbread-1] == '\n' )
    integers_string[nbread-1] = '\0' ;

  /* Set parameters in the profile */
  pb->nbprocs = strtol(integers_string,&ptr_coma,10) ;
  assert(*ptr_coma=';') ;
  tmpPtr=ptr_coma ;
    
  pb->nbprocess = strtol(tmpPtr,&ptr_coma,10) ;
  assert(*ptr_coma=';') ;
  tmpPtr=ptr_coma ;
    
  pb->walltime = strtol(tmpPtr,&ptr_coma,10) ;
#endif
  /* Close and unlink temp files */
  if( close(file_descriptor_input) != 0 ) {
    printf("Couln't close input file") ;
  }
  if( close(file_descriptor_result) != 0 ) {
    printf("Couln't close result file") ;
  }

  /* Free memory */
#ifndef DEBUG
  free( command ) ;
#endif
  diet_destroy_estVect( perfBatchVal ) ;
}

void
performance_PasTiX_paral( diet_profile_t * pb, estVector_t perfValues )
{
  /* Get NB_FREE_RESOURCES_IN_DEFAULT_QUEUE for scheduling issue       */
  /* but we do not have to work on it. Only storage for scheduling     */
  diet_estimate_cori(perfValues, EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE,
		     EST_COLL_BATCH, pb);
#ifdef DEBUG
  printf("There are %d idle resources in default queue\n",
	 (int)diet_est_get_system(perfValues,
			     EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE, 0)
	 ) ;
#endif
}

/****************************************************************************
 * SOLVE FUNCTION
 ****************************************************************************/

int
solve_PasTiX(diet_profile_t * pb)
{
  size_t arg_size1  = 0 ;
  char * prologue = NULL ;
  char * copying = NULL ;
  char * cmd = NULL ;
  char * epilogue = NULL ;
  char * script = NULL ;
  char * path1 = NULL ;
  char * path_result = NULL ;
  char * current_directory = NULL ;
#ifndef DEBUG
  char * local_output_filename = NULL ;
  double * ptr_nbreel = NULL ;
  char * outputName_1, * outputName_2 ;
  char * path2 = NULL ;
#endif
  int file_descriptor ;
  int status = 0 ;
  int result ;
  struct stat buf;

  printf("Resolving batch service PasTiX!\n\n") ;

  /* IN args */  
  diet_file_get(diet_parameter(pb,0), NULL, &arg_size1, &path1);
  if ((status = stat(path1, &buf)))
    return status;
  if( !(buf.st_mode & S_IFREG) ) /* regular file */
    return 2;
  printf("Name of the input file: %s\n",path1) ;
  
  /* OUT args, MUST NOT BE CONSTANT STRINGS */
  current_directory = (char*)malloc(sizeof(char)*100) ;
  if( getcwd( current_directory, 99 ) == NULL ) {
    printf("Current directory is too long for the size of reserved memory!\n"
	   "Change it...\n\n") ;
    exit( 0 ) ;
  }
  current_directory[99]='\0';
  
  path_result = (char*)malloc(sizeof(char)*(strlen(current_directory)+30)) ;
  sprintf(path_result,"%s/DIET_Result.XXXXXX",current_directory) ;
  file_descriptor = mkstemp( path_result ) ;
  if( file_descriptor == -1 ) {
    perror("mkstemp") ;
  }
  if( diet_file_desc_set(diet_parameter(pb,1), path_result) ) {
    printf("diet_file_desc_set() error\n");
    return 1;
  }
  printf("Name of result file: %s\n",path_result) ;

#ifndef DEBUG
  /* FIXME: Dirty trick for Loadlever: put files in current rep */
  outputName_1 = (char*)calloc(sizeof(char),
			       (strlen(current_directory)+strlen(path1))) ;
  if( outputName_1 == NULL )
    perror("malloc outputName_1") ;
  sprintf(outputName_1,"%s/%s", current_directory,basename(path1)) ;
  copyFile(path1,outputName_1) ;
  outputName_2 = (char*)calloc(sizeof(char),
			       (strlen(current_directory)+strlen(path2))) ;
  if( outputName_2 == NULL )
    perror("malloc outputName_2") ;
  sprintf(outputName_2,"%s/%s", current_directory,basename(path2)) ;
  copyFile(path2,outputName_2) ;
  
  /********************************************/
  /* Put the command to submit into a script */
  /********************************************/
  /* Some unecessary things, only for the example */
  prologue = (char*)malloc(5000*sizeof(char)) ; /* TODO: Reduce size */
  if( prologue == NULL ) {
    fprintf(stderr,"Memory allocation problem.. not solving the service\n\n") ;
    return 2 ;
  }
  sprintf(prologue,
	  "batchID=$DIET_BATCHNAME\n"
	  "echo \"Name of the frontale station: $DIET_NAME_FRONTALE\"\n"
	  "case \"$batchID\" in\n"
	  "  loadleveler) echo DIET_BATCH_NODESLIST, DIET_BATCH_NODESFILE \
	                  cannot be used in this case ;;\n"
	  "  *) echo \"Number of nodes:  $DIET_BATCH_NBNODES\"\n"
	  "     echo \"Reserved Nodes are:\"\n"
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
  copying = (char*)calloc(6000,sizeof(char)) ;  /* TODO: Reduce size */
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
	  path1,basename(path1),basename(path1),path2,basename(path2)) ;
  sprintf(copying+strlen(copying),
	  "  loadleveler) echo Execute on Loadleveler\n"
	  "               # Machine //, all files accessible from all nodes\n"
	  "               # we can use mcp..\n"
	  "               WORKING_DIRECTORY=/users/cri/diet/YC/JobMPI/\n"
	  "               input_file1=%s\n"
	  "               input_file2=%s ;;\n"
	  "  *) echo NOT TESTED! ;;\n"
	  "esac\n",
	  outputName_1, outputName_2) ;
  	  /*	  path1,path2) ;  */
  
  /* The proceeding of the command */
  local_output_filename = (char*)malloc(sizeof(char*)*
					(strlen(path_result)+10)) ;
  sprintf(local_output_filename,"%s_local",path_result) ; /*"/tmp/result_local.txt" ;*/
  cmd = (char*)calloc(5000,sizeof(char)) ;  /* TODO: Reduce size */
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
	             $input_file2 $local_output_filename ;;\n",
	  local_output_filename, *ptr_nbreel) ;
  sprintf(cmd+strlen(cmd),
	  "  loadleveler) cd $WORKING_DIRECTORY\n"
	  "          local_output_filename=%s\n"
	  "          # Test if job is serial or parallel (usage of poe)\n"
	  "          #if [ %d -eq 1 ] ; then\n"
	  "            ./concatenation.sh $input_file1 %.2f \
	               $input_file2 $local_output_filename\n"
	  "          #else\n"
	  "          #  poe -np $DIET_USER_NBPROCS \
	               -machinefile $DIET_BATCH_NODESFILE \
	               concatenation $input_file1 %.2f \
	               $input_file2 $local_output_filename\n"
	  "          #fi\n"
	  "esac\n"
	  "\n",
	  local_output_filename, 1, *ptr_nbreel,
	  *ptr_nbreel
	  ) ;
  
  /* Put the Output file in the right place */
  /* Note: if output on NFS, with "ln -s" (see batch_server_2) 
     or by Diet (see batch_server_3) */
  epilogue = (char*)malloc(1000*sizeof(char)) ;  /* was 200 */
  if( epilogue == NULL ) {
    fprintf(stderr,"Memory allocation problem.. not solving the service\n\n") ;
    return 2 ;
  }
  sprintf(epilogue,
	  "# Get the result file\n"
	  "case $batchID in\n"
	  "  oar1.6) scp $local_output_filename $DIET_NAME_FRONTALE:%s ;;\n"
	  "  loadleveler) cp $local_output_filename %s ;;\n"
	  "esac\n",
	  path_result,
	  path_result) ;
  
  /* Make Diet submit */
  script = (char*)malloc( (strlen(prologue)  
			   + strlen(copying)  
			   + strlen(cmd) 
			   + strlen(epilogue)
			   + 1 ) * sizeof(char) ) ;
  if( script == NULL ) {
    fprintf(stderr,"Memory allocation problem.. not solving the service\n\n") ;
    return 2 ;
  }
  sprintf(script,"%s%s%s%s",prologue,copying,cmd,epilogue) ;
#endif

  /* Call performance prediction or not, but fields are to be fullfilled */
  make_perfPred(pb) ;

#ifdef DEBUG
  /* Dont do the submission */
  printf("Debug mode: submission not done..\n") ;
  result = 0 ;
#else
  /* Submission */
  result = diet_submit_parallel(pb, script) ;
#endif

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
  int nb_max_services = 1 ;
  char ** arrayOfServiceNames ;
  diet_profile_desc_t * profile_PasTiX_paral = NULL ;
  diet_aggregator_desc_t * agg = NULL ;
  

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }  

  /* Initialize table with maximum services */
  diet_service_table_init(nb_max_services);
  
  /* Create the array containing the service names */
  arrayOfServiceNames = (char**)malloc(sizeof(char*)*nb_max_services) ;
  
  /* Initialize state of SeD: batch or not */
  printf("Server -> %d\n",(int)BATCH) ;
  diet_set_server_status( BATCH ) ;

  /******************************** For fct1 *********************************/
  /* Allocate profiles (IN, INOUT, OUT) */
  arrayOfServiceNames[0] = "PasTiX" ;
  profile_PasTiX_paral = diet_profile_desc_alloc(arrayOfServiceNames[0],0,0,1);

  /* Set profile parameters:
     Define if the code is parallel or serial */
  diet_profile_desc_set_parallel(profile_PasTiX_paral) ;
  
  diet_generic_desc_set(diet_param_desc(profile_PasTiX_paral,0), DIET_FILE,
			DIET_CHAR); /* IN */
  diet_generic_desc_set(diet_param_desc(profile_PasTiX_paral,1), DIET_FILE,
			DIET_CHAR); /* OUT */

  /* Set Collector */
  diet_estimate_cori_add_collector( EST_COLL_BATCH, profile_PasTiX_paral ) ;
  /* Set PasTiX scheduling policy: choose the site where max memory reported */
  diet_service_use_perfmetric( performance_PasTiX_paral ) ;
  agg = diet_profile_desc_aggregator( profile_PasTiX_paral );
  diet_aggregator_set_type( agg, DIET_AGG_PRIORITY ) ;
  diet_aggregator_priority_max( agg,
				EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE );

  /* All done */

  /* Add service to the service table */
  if( diet_service_table_add(profile_PasTiX_paral, NULL,
			     solve_PasTiX) )
    return 1 ;
  
  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile_PasTiX_paral);

  /* Print the table to check */
  diet_print_service_table();

  /* Launch the server */
  res = diet_SeD(argv[1], argc, argv);

  return res;
}
