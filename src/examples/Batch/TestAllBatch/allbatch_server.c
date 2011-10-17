/****************************************************************************/
/* General batch submission: serial service on a parallel machine           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Yves CANIOU (ycaniou@ens-lyon.fr)                                   */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2011/01/23 19:20:00  bdepardo
 * Fixed memory and resources leaks, variables scopes, unread variables
 *
 * Revision 1.1  2010/08/06 14:25:28  glemahec
 * Cmake corrections + uuid lib module + fPIC error control
 *
 * Revision 1.6  2010/06/15 08:10:53  amuresan
 * Added cloud configuration files to src/examples/cfgs/
 *
 * Revision 1.5  2009/11/27 03:24:30  ycaniou
 * Add user_command possibility before the end of Batch prologue (only
 * to be used for batch dependent code!)
 * Memory leak/segfault--
 * New easy Batch basic example
 * Management of OAR2_X Batch scheduler
 *
 * Revision 1.4  2008/05/11 16:19:50  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.3  2008/04/19 09:16:46  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Rewrite (and addition) of the propagation of the response concerning:
 *   job parallel_flag and serverType (batch or serial for the moment)
 * Complete debug info with batch stuff
 *
 * Revision 1.2  2008/01/01 18:53:07  ycaniou
 * Update batch examples (Lammps still a work in project)
 *
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libgen.h> /* basename() */
#include <fcntl.h>       /* for O_RDONLY */

#include "DIET_server.h"
/* #include "DIET_mutex.h" */
#include <sys/stat.h>

/****************************************************************************
 * UTILITY FUNCTION
 ****************************************************************************/

void
copyFile(char * inputName, char * outputName)
{
  int inputFile, outputFile;
  char line[512];
  int bytes;

  if ((inputFile = open(inputName, O_RDONLY)) == -1) {
    perror("open");
    exit(-1);
  }
  if ((outputFile = open(outputName, O_WRONLY | O_CREAT, S_IRUSR|S_IWUSR)) == -1) {
    perror("open");
    exit(-1);
  }
  while((bytes = read(inputFile, line, sizeof(line))) > 0)
    write(outputFile, line, bytes);

  close(inputFile);
  close(outputFile);
}

/****************************************************************************
 * PERFORMANCE FUNCTION
 ****************************************************************************/

void make_perf(diet_profile_t * pb)
{

#ifdef YC_DEBUG
  printf("Requested job is ");
  if (pb->parallel_flag == 1)
    printf("sequential\n\n");
  else if (pb->parallel_flag == 2)
    printf("parallel\n\n");
  else
    printf("ARGHHH\n\n");
#endif

  /* TODO: an API in order to provide the SeD with communication means
  ** with batch scheduler for prediction performances  */
  
  /* For the moment, give arbitrary values */

  if (pb->parallel_flag == 1) { /* Job requested is sequential */
    pb->walltime = 125; /* in seconds */
    pb->nbprocs = 1;
    pb->nbprocess = pb->nbprocs;
  } else {
    pb->walltime = 125; /* in seconds */
    pb->nbprocs = 2;
    pb->nbprocess = pb->nbprocs;
  }
}


/****************************************************************************
 * SOLVE FUNCTION
 ****************************************************************************/

int solve_concatenation_seq(diet_profile_t *pb)
{
  size_t arg_size1  = 0;
  size_t arg_size2  = 0;
  char * prologue = NULL;
  char * copying = NULL;
  char * cmd = NULL;
  char * epilogue = NULL;
  char * script = NULL;
  char * path1 = NULL;
  char * path2 = NULL;
  char * path_result = NULL;
  char * local_output_filename = NULL;
  char * current_directory = NULL;
  double * ptr_nbreel = NULL;
  char * outputName_1, * outputName_2;
  int file_descriptor;
  int status = 0;
  int result;
  struct stat buf;

  printf("Resolving batch service 'concatenation'!\n\n");

  /* IN args */  
  diet_file_get(diet_parameter(pb, 0), NULL, &arg_size1, &path1);
  if ((status = stat(path1, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG)) /* regular file */
    return 2;
  printf("Name of the first file: %s\n", path1);
  
  diet_scalar_get(diet_parameter(pb, 1), &ptr_nbreel, NULL);

  diet_file_get(diet_parameter(pb, 2), NULL, &arg_size2, &path2);
  if ((status = stat(path2, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG)) /* regular file */
    return 2;
  printf("Name of the second file: %s\n", path2);

  /* OUT args, MUST NOT BE CONSTANT STRINGS */
  current_directory = (char*)malloc(sizeof(char)*100);
  getcwd(current_directory, 100);

  path_result = (char*)malloc(sizeof(char)*(strlen(current_directory)+30));
  sprintf(path_result, "%s/DIET_Result.XXXXXX", current_directory);
  file_descriptor = mkstemp(path_result);
  if (file_descriptor == -1) {
    perror("mkstemp");
  }
  close(file_descriptor);
  if (diet_file_desc_set(diet_parameter(pb, 3), path_result)) {
    printf("diet_file_desc_set() error\n");
    return 1;
  }
  printf("Name of result file: %s\n", path_result);

  /* FIXME: Dirty trick for Loadlever: put files in current rep */
  outputName_1 = (char*)calloc(sizeof(char),
                               (strlen(current_directory)+strlen(path1)));
  if (outputName_1 == NULL)
    perror("malloc outputName_1");
  sprintf(outputName_1, "%s/%s", current_directory, basename(path1));
  copyFile(path1, outputName_1);
  outputName_2 = (char*)calloc(sizeof(char),
                               (strlen(current_directory)+strlen(path2)));
  if (outputName_2 == NULL)
    perror("malloc outputName_2");
  sprintf(outputName_2, "%s/%s", current_directory, basename(path2));
  copyFile(path2, outputName_2);
  
  /********************************************/
  /* Put the command to submit into a script */
  /********************************************/
  /* Some unecessary things, only for the example */
  prologue = (char*)malloc(5000*sizeof(char)); /* TODO: Reduce size */
  if (prologue == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    return 2;
  }
  sprintf(prologue,
          "batchID=$DIET_BATCHNAME\n"
          "echo \"Name of the frontale station: $DIET_NAME_FRONTALE\"\n"
          "case \"$batchID\" in\n"
          "  loadleveler) echo DIET_BATCH_NODESLIST, DIET_BATCH_NODESFILE \
                          cannot be used in this case;;\n"
          "  *) echo \"Number of nodes:  $DIET_BATCH_NBNODES\"\n"
          "     echo \"Reserved Nodes are:\"\n"
          "     echo $DIET_BATCH_NODESLIST\n"
          "     echo Name of the batch file containing their identity: \
                $DIET_BATCH_NODESFILE;;\n"
          "esac\n"
          "\n");

  /* Data management: scp for file1, NFS for file2 */
  /* Note for advanced usage:  
     one can do the NFS cp in a C thread (see batch_server_2)
     Don't forget to put in batch_server_2 a test on size to be sure that file
     is copied:

     while (size != %d); do
     sleep 1
     done

  */ 
  copying = (char*)calloc(6000, sizeof(char));  /* TODO: Reduce size */
  if (copying == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    return 2;
  }
  sprintf(copying,
          "case $batchID in\n"
          "  oar1.6) echo Execute on OAR1.6\n"
          "          WORKING_DIRECTORY=/home/ycaniou/JobMPI/\n"
          "          # Copy the file on reserved nodes\n"
          "          for i in $DIET_BATCH_NODESLIST; do\n"
          "            scp $DIET_NAME_FRONTALE:%s $i:/tmp/%s_local\n"
          "          done\n"
          "          input_file1=/tmp/%s_local\n\n"
          "          # Use NFS (we are not on the frontale anymore!)\n"
          "          ssh $DIET_NAME_FRONTALE \"cp %s $WORKING_DIRECTORY/\"\n"
          "          input_file2=$WORKING_DIRECTORY/%s;;\n",
          path1, basename(path1), basename(path1), path2, basename(path2));
  sprintf(copying+strlen(copying),
          "  loadleveler) echo Execute on Loadleveler\n"
          "               # Machine //, all files accessible from all nodes\n"
          "               # we can use mcp..\n"
          "               WORKING_DIRECTORY=/users/cri/diet/YC/JobMPI/\n"
          "               input_file1=%s\n"
          "               input_file2=%s;;\n"
          "  *) echo NOT TESTED!;;\n"
          "esac\n",
          outputName_1, outputName_2);
  /*      path1, path2);  */
  
  /* The proceeding of the command */
  local_output_filename = (char*)malloc(sizeof(char*)*
                                        (strlen(path_result)+10));
  sprintf(local_output_filename, "%s_local", path_result); /*"/tmp/result_local.txt";*/
  cmd = (char*)calloc(5000, sizeof(char));  /* TODO: Reduce size */
  if (cmd == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    free(copying);
    free(local_output_filename);
    return 2;
  }
  sprintf(cmd,
          "# Execution\n"
          "case $batchID in\n"
          "  oar1.6) cd $WORKING_DIRECTORY\n"
          "          local_output_filename=%s\n"
          "          mpirun.mpich_1_2 -np $DIET_USER_NBPROCS \
                     -machinefile $DIET_BATCH_NODESFILE \
                     concatenation $input_file1 %.2f \
                     $input_file2 $local_output_filename;;\n",
          local_output_filename, *ptr_nbreel);
  sprintf(cmd+strlen(cmd),
          "  loadleveler) cd $WORKING_DIRECTORY\n"
          "          local_output_filename=%s\n"
          "          # Test if job is serial or parallel (usage of poe)\n"
          "          #if [ %d -eq 1 ]; then\n"
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
);
  
  /* Put the Output file in the right place */
  /* Note: if output on NFS, with "ln -s" (see batch_server_2) 
     or by Diet (see batch_server_3) */
  epilogue = (char*)malloc(1000*sizeof(char));  /* was 200 */
  if (epilogue == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    free(copying);
    free(cmd);
    free(local_output_filename);
    return 2;
  }
  sprintf(epilogue,
          "# Get the result file\n"
          "case $batchID in\n"
          "  oar1.6) scp $local_output_filename $DIET_NAME_FRONTALE:%s;;\n"
          "  loadleveler) cp $local_output_filename %s;;\n"
          "esac\n",
          path_result,
          path_result);
  
  /* Make Diet submit */
  script = (char*)malloc((strlen(prologue)  
                           + strlen(copying)  
                           + strlen(cmd) 
                           + strlen(epilogue)
                           + 1) * sizeof(char));
  if (script == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    free(copying);
    free(cmd);
    free(epilogue);
    free(local_output_filename);
    return 2;
  }
  sprintf(script, "%s%s%s%s", prologue, copying, cmd, epilogue);


  /* Call performance prediction or not, but fields are to be fullfilled */
  make_perf(pb);
  
  /* Submission */
  result = diet_submit_parallel(pb, NULL, script);

  /* Free memory */
  free(prologue);
  free(copying);
  free(cmd);
  free(epilogue);
  free(script);
  free(local_output_filename);

  /* Don't free path1, path2 and path_result */
  if (result == -1)
    printf("Error when submitting the script\n");
  return result;
}

/* Same code as before, but not the same pred perf */
int solve_concatenation_parallel(diet_profile_t *pb)
{
  size_t arg_size1  = 0;
  size_t arg_size2  = 0;
  char * prologue = NULL;
  char * copying = NULL;
  char * cmd = NULL;
  char * epilogue = NULL;
  char * script = NULL;
  char * path1 = NULL;
  char * path2 = NULL;
  char * path_result = NULL;
  char * local_output_filename = NULL;
  char * current_directory = NULL;
  double * ptr_nbreel = NULL;
  char * outputName_1, * outputName_2;
  int file_descriptor;
  int status = 0;
  int result;
  struct stat buf;

  printf("Resolving batch service 'concatenation'!\n\n");

  /* IN args */  
  diet_file_get(diet_parameter(pb, 0), NULL, &arg_size1, &path1);
  if ((status = stat(path1, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG)) /* regular file */
    return 2;
  printf("Name of the first file: %s\n", path1);
  
  diet_scalar_get(diet_parameter(pb, 1), &ptr_nbreel, NULL);

  diet_file_get(diet_parameter(pb, 2), NULL, &arg_size2, &path2);
  if ((status = stat(path2, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG)) /* regular file */
    return 2;
  printf("Name of the second file: %s\n", path2);

  /* OUT args, MUST NOT BE CONSTANT STRINGS */
  current_directory = (char*)malloc(sizeof(char)*100);
  getcwd(current_directory, 100);

  path_result = (char*)malloc(sizeof(char)*(strlen(current_directory)+30));
  sprintf(path_result, "%s/DIET_Result.XXXXXX", current_directory);
  file_descriptor = mkstemp(path_result);
  if (file_descriptor == -1) {
    perror("mkstemp");
  }
  close(file_descriptor);
  if (diet_file_desc_set(diet_parameter(pb, 3), path_result)) {
    printf("diet_file_desc_set() error\n");
    return 1;
  }
  printf("Name of result file: %s\n", path_result);

  /* FIXME: Dirty trick for Loadlever: put files in current rep */
  outputName_1 = (char*)calloc(sizeof(char),
                               (strlen(current_directory)+strlen(path1)));
  if (outputName_1 == NULL)
    perror("malloc outputName_1");
  sprintf(outputName_1, "%s/%s", current_directory, basename(path1));
  copyFile(path1, outputName_1);
  outputName_2 = (char*)calloc(sizeof(char),
                               (strlen(current_directory)+strlen(path2)));
  if (outputName_2 == NULL)
    perror("malloc outputName_2");
  sprintf(outputName_2, "%s/%s", current_directory, basename(path2));
  copyFile(path2, outputName_2);
  
  /********************************************/
  /* Put the command to submit into a script */
  /********************************************/
  /* Some unecessary things, only for the example */
  prologue = (char*)malloc(5000*sizeof(char)); /* TODO: Reduce size */
  if (prologue == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    return 2;
  }
  sprintf(prologue,
          "batchID=$DIET_BATCHNAME\n"
          "echo \"Name of the frontale station: $DIET_NAME_FRONTALE\"\n"
          "case \"$batchID\" in\n"
          "  loadleveler) echo DIET_BATCH_NODESLIST, DIET_BATCH_NODESFILE \
                          cannot be used in this case;;\n"
          "  *) echo \"Number of nodes:  $DIET_BATCH_NBNODES\"\n"
          "     echo \"Reserved Nodes are:\"\n"
          "     echo $DIET_BATCH_NODESLIST\n"
          "     echo Name of the batch file containing their identity: \
                $DIET_BATCH_NODESFILE;;\n"
          "esac\n"
          "\n");

  /* Data management: scp for file1, NFS for file2 */
  /* Note for advanced usage:  
     one can do the NFS cp in a C thread (see batch_server_2)
     Don't forget to put in batch_server_2 a test on size to be sure that file
     is copied:

     while (size != %d); do
     sleep 1
     done

  */ 
  copying = (char*)calloc(6000, sizeof(char));  /* TODO: Reduce size */
  if (copying == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    return 2;
  }
  sprintf(copying,
          "case $batchID in\n"
          "  oar1.6) echo Execute on OAR1.6\n"
          "          WORKING_DIRECTORY=/home/ycaniou/JobMPI/\n"
          "          # Copy the file on reserved nodes\n"
          "          for i in $DIET_BATCH_NODESLIST; do\n"
          "            scp $DIET_NAME_FRONTALE:%s $i:/tmp/%s_local\n"
          "          done\n"
          "          input_file1=/tmp/%s_local\n\n"
          "          # Use NFS (we are not on the frontale anymore!)\n"
          "          ssh $DIET_NAME_FRONTALE \"cp %s $WORKING_DIRECTORY/\"\n"
          "          input_file2=$WORKING_DIRECTORY/%s;;\n",
          path1, basename(path1), basename(path1), path2, basename(path2));
  sprintf(copying+strlen(copying),
          "  loadleveler) echo Execute on Loadleveler\n"
          "               # Machine //, all files accessible from all nodes\n"
          "               # we can use mcp..\n"
          "               WORKING_DIRECTORY=/users/cri/diet/YC/JobMPI/\n"
          "               input_file1=%s\n"
          "               input_file2=%s;;\n"
          "  *) echo NOT TESTED!;;\n"
          "esac\n",
          outputName_1, outputName_2);
  /*      path1, path2);  */
  
  /* The proceeding of the command */
  local_output_filename = (char*)malloc(sizeof(char*)*
                                        (strlen(path_result)+10));
  sprintf(local_output_filename, "%s_local", path_result); /*"/tmp/result_local.txt";*/
  cmd = (char*)calloc(5000, sizeof(char));  /* TODO: Reduce size */
  if (cmd == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    free(copying);
    free(local_output_filename);
    return 2;
  }
  sprintf(cmd,
          "# Execution\n"
          "case $batchID in\n"
          "  oar1.6) cd $WORKING_DIRECTORY\n"
          "          local_output_filename=%s\n"
          "          mpirun.mpich_1_2 -np $DIET_USER_NBPROCS \
                     -machinefile $DIET_BATCH_NODESFILE \
                     concatenation $input_file1 %.2f \
                     $input_file2 $local_output_filename;;\n",
          local_output_filename, *ptr_nbreel);
  sprintf(cmd+strlen(cmd),
          "  loadleveler) cd $WORKING_DIRECTORY\n"
          "          local_output_filename=%s\n"
          "          # Test if job is serial or parallel (usage of poe)\n"
          "          #if [ %d -eq 1 ]; then\n"
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
);
  
  /* Put the Output file in the right place */
  /* Note: if output on NFS, with "ln -s" (see batch_server_2) 
     or by Diet (see batch_server_3) */
  epilogue = (char*)malloc(1000*sizeof(char));  /* was 200 */
  if (epilogue == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    free(copying);
    free(cmd);
    free(local_output_filename);
    return 2;
  }
  sprintf(epilogue,
          "# Get the result file\n"
          "case $batchID in\n"
          "  oar1.6) scp $local_output_filename $DIET_NAME_FRONTALE:%s;;\n"
          "  loadleveler) cp $local_output_filename %s;;\n"
          "esac\n",
          path_result,
          path_result);
  
  /* Make Diet submit */
  script = (char*)malloc((strlen(prologue)  
                           + strlen(copying)  
                           + strlen(cmd) 
                           + strlen(epilogue)
                           + 1) * sizeof(char));
  if (script == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    free(copying);
    free(cmd);
    free(epilogue);
    free(local_output_filename);
    return 2;
  }
  sprintf(script, "%s%s%s%s", prologue, copying, cmd, epilogue);


  /* Call performance prediction or not, but fields are to be fullfilled */
  make_perf(pb);
  
  /* Submission */
  result = diet_submit_parallel(pb, NULL, script);

  /* Free memory */
  free(prologue);
  free(copying);
  free(cmd);
  free(epilogue);
  free(script);
  free(local_output_filename);

  /* Don't free path1, path2 and path_result */
  if (result == -1)
    printf("Error when submitting the script\n");
  return result;
}

/****************************************************************************
 * MAIN
 ****************************************************************************/

int
main(int argc, char* argv[])
{
  int res = 0;
  int nb_max_services = 2;
  diet_profile_desc_t* profile_fct1_seq = NULL;
  diet_profile_desc_t* profile_fct1_parallel = NULL;

  /* TODO: implement these for futher tests
     diet_profile_desc_t* profile_fct1_seq = NULL;
     diet_profile_desc_t* profile_fct2_parallel = NULL;
  */
  
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }  

  /* Initialize state of SeD: batch or not */
  printf("Server -> %d\n",(int)BATCH);
  diet_set_server_status(BATCH);

  /* Initialize table with maximum services */
  diet_service_table_init(nb_max_services);

  /******************************** For fct1 *********************************/
  /* Allocate profiles (IN, INOUT, OUT) */
  profile_fct1_seq = diet_profile_desc_alloc("concatenation", 2, 2, 3);
  profile_fct1_parallel = diet_profile_desc_alloc("concatenation", 2, 2, 3);

  /* Set profile parameters:
     Define if the code is parallel or serial */
  diet_profile_desc_set_sequential(profile_fct1_seq);
  diet_profile_desc_set_parallel(profile_fct1_parallel);
    
  diet_generic_desc_set(diet_param_desc(profile_fct1_seq, 0), DIET_FILE,
                        DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile_fct1_seq, 1), DIET_SCALAR,
                        DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile_fct1_seq, 2), DIET_FILE,
                        DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile_fct1_seq, 3), DIET_FILE,
                        DIET_CHAR);

  diet_generic_desc_set(diet_param_desc(profile_fct1_parallel, 0), DIET_FILE,
                        DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile_fct1_parallel, 1), DIET_SCALAR,
                        DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile_fct1_parallel, 2), DIET_FILE,
                        DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile_fct1_parallel, 3), DIET_FILE,
                        DIET_CHAR);
  /* All done */

  /* Add service to the service table */
  if (diet_service_table_add(profile_fct1_seq, NULL, solve_concatenation_seq)) 
    return 1;
  if (diet_service_table_add(profile_fct1_parallel, NULL,
                             solve_concatenation_parallel))
    return 1;
  
  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile_fct1_seq);
  diet_profile_desc_free(profile_fct1_parallel);

  /* Print the table to check */
  diet_print_service_table();

  /* Launch the server */
  res = diet_SeD(argv[1], argc, argv);

  return res;
}
