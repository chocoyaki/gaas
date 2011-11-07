/**
* @file parallel_server.c
* 
* @brief   DIET server for parallel submission on a non NFS cluster   
* 
* @author   Yves Caniou (Yves.Caniou@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libgen.h> /* basename() */
#include <errno.h>

#include "DIET_server.h"
#include <sys/stat.h>

#include <sys/time.h> /* For gettimeofday() */
#include <string.h> /* For strlen */

ssize_t writen(int fd, const void *vptr, size_t n);

int
makeMachineFile(int * file_descriptor_ptr, char ** filename_ptr)
{
  /* In this example, we use static names */
  char * machines_identity = "gdsdmi15\ngdsdmi16\ngdsdmi17";
  int sl = strlen(machines_identity);

  *file_descriptor_ptr = mkstemp(*filename_ptr);
  if (*file_descriptor_ptr == -1) return -1;

  if (writen(*file_descriptor_ptr, machines_identity, sl) != sl)
    return 1;

  return 0;
}

/****************************************************************************
 * PREDICTION PERFORMANCE FUNCTION
 ****************************************************************************/

void make_perf(diet_profile_t *pb)
{
  /* TODO: an API in order to provide the SeD with communication means
  ** with batch scheduler for prediction performances  */

  /* For the moment, give arbitrary values */
  pb->nbprocs = 2;
  pb->nbprocess = pb->nbprocs;
}


/****************************************************************************
 * SOLVE FUNCTION
 ****************************************************************************/

int solve_lammps(diet_profile_t *pb)
{
  size_t arg_size1  = 0;
  char * prologue = NULL;
  char * copying = NULL;
  char * cmd = NULL;
  char * epilogue = NULL;
  char * script = NULL;
  char * path1 = NULL;
  char * path_result = NULL;
  char * local_output_filename = NULL;
  int status = 0;
  int result;
  char * machine_filename;
  int machine_file_descriptor;
  struct stat buf;
  char * lammpsName;
  struct timeval uur70;
  char chaine[50];

  printf("Resolving sequential service 'lammps'");
  /* Lammps binary dependant of installation. We suppose no renaming but
     an environment variable set within ~/.bash_profile for example */
  lammpsName = getenv("LAMMPS_NAME");
  if (lammpsName == NULL)
    lammpsName = "lammps_diet";
  printf("by calling '%s' binary\n\n", lammpsName);

  /* IN args */
  diet_file_get(diet_parameter(pb, 0), NULL, &arg_size1, &path1);
  if ((status = stat(path1, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG)) /* regular file */
    return 2;
  printf("Name of the input file: %s\n", path1);

  /* OUT args */
  if (gettimeofday(&uur70, NULL) == 0) {
    sprintf(&chaine, "/tmp/lammps_%d.txt", uur70.tv_sec);
    local_output_filename = strdup(chaine);
  } else {
    perror("Cannot access time on server. Use a static name\n");
    local_output_filename = strdup("/tmp/lammps.txt");
    /*MUST NOT BE A CONSTANT STRING*/
  }
  if (diet_file_desc_set(diet_parameter(pb, 1), local_output_filename)) {
    printf("diet_file_desc_set() error\n");
    return 1;
  }
  printf("Name of result file: %s\n", local_output_filename);

  /*******************************************/
  /* Put the command to submit into a script */
  /*******************************************/
  machine_filename = (char*)malloc(sizeof(char)*100);
  if (machine_filename == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    return 2;
  }
  /* Seems to only work on local filesystem (not on NFS) */
  sprintf(machine_filename, "/tmp/DIET_machineFile_XXXXXX");
  if (makeMachineFile(& machine_file_descriptor, & machine_filename) != 0) {
    fprintf(stderr,
            "Making machineFile problem.. not solving the service\n\n");
    free(machine_filename);
    return 2;
  }

  /* Some unecessary things, only for the example */
  prologue = (char*)malloc(200*sizeof(char));
  if (prologue == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(machine_filename);
    return 2;
  }
  sprintf(prologue,
          "echo \"Name of the frontale station: $DIET_NAME_FRONTALE\"\n\n");

  copying = (char*)malloc(600*sizeof(char));
  if (copying == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    free(machine_filename);
    return 2;
  }
  sprintf(copying,
          "WORKING_DIRECTORY=/home/ycaniou/JobMPI/ \n"
          "######################################### \n"
          "user_machinefile=%s \n"
          "######################################### \n"
          "# Use NFS, and suppose we 'cd' to WORKING_DIRECTORY after.. \n"
          "ssh $DIET_NAME_FRONTALE \"cp %s $WORKING_DIRECTORY/\"\n"
          "input_file=%s\n\n",
          machine_filename,
          path1, basename(path1));

  /* The MPI command itself */
  if (gettimeofday(&uur70, NULL) == 0) {
    sprintf(chaine, "/tmp/lammps_%d.txt", uur70.tv_sec);
    path_result = strdup(chaine);
  } else {
    perror("Cannot access time on server. Use a static name\n");
    path_result = strdup("/tmp/lammps.txt"); /*MUST NOT BE A CONSTANT STRING*/
  }
  printf("Name of result file: %s\n", path_result);

  cmd = (char*)malloc(500*sizeof(char));
  if (cmd == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    free(copying);
    free(machine_filename);
    free(path_result);
    return 2;
  }
  sprintf(cmd,
          "# Execution\n"
          "cd $WORKING_DIRECTORY \n"
          "local_output_filename=%s \n"
          "mpirun.mpich_1_2 -np $DIET_USER_NBPROCS "
          "-machinefile $user_machinefile "
          "%s -in $input_file1 -screen "
          "$local_output_filename \n"
          "\n",
          local_output_filename);

  /* Put the Output file in the right place */
  /* Note: if output on NFS, with "ln -s" (see batch_server_2)
     or by Diet (see batch_server_3) */
  epilogue = (char*)malloc(200*sizeof(char));
  if (epilogue == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    free(prologue);
    free(copying);
    free(cmd);
    free(machine_filename);
    free(path_result);
    return 2;
  }
  sprintf(epilogue,
          "# Get the result file\n"
          "scp $local_output_filename $DIET_NAME_FRONTALE:%s\n"
          , path_result);

  /* Make Diet submit */
  script = (char*)malloc((strlen(prologue)
                           + strlen(copying)
                           + strlen(cmd)
                           + strlen(epilogue)
                           + 1) * sizeof(char));
  sprintf(script, "%s%s%s%s", prologue, copying, cmd, epilogue);


  /* Call performance prediction or not, but fields are to be fullfilled */
  make_perf(pb);

  /* Submission */
  result = diet_submit_parallel(pb, script);
  if (result == 0)
    printf("Error when submitting the script\n");

  /* Free memory */
  free(prologue);
  free(copying);
  free(cmd);
  free(epilogue);
  free(script);
  while ((-1 == (status = close(machine_file_descriptor))) && (errno == EINTR))
   ;
  if (status == -1)
    fprintf(stderr, "Error closing machineFile\n");


  free(machine_filename);
  free(path_result);
  diet_free_data(diet_parameter(pb, 0));

  return 0;
}

/****************************************************************************
 * MAIN
 ****************************************************************************/

int
main(int argc, char* argv[])
{
  int res = 0;
  int nb_max_services = 1;
  diet_profile_desc_t* profile = NULL;


  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }

  /* Initialize table with maximum services */
  diet_service_table_init(nb_max_services);

  /* Allocate batch profile (IN, INOUT, OUT) */
  profile = diet_profile_desc_alloc("concatenation", 2, 2, 3);

  /* Set profile parameters : this job is submitted by a batch system */
  diet_profile_desc_set_parallel(profile);

  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 3), DIET_FILE, DIET_CHAR);
  /* All done */

  /* Add service to the service table */
  if (diet_service_table_add(profile, NULL, solve_lammps)) return 1;

  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);

  /* Print the table to check */
  diet_print_service_table();

  /* Launch the server */
  res = diet_SeD(argv[1], argc, argv);

  return res;
}

/****************************************************************************
 * Utilities: do not change!
 ****************************************************************************/
ssize_t /*Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
  size_t        nleft;
  ssize_t       nwritten;
  const char    *ptr;

  ptr = vptr;
  nleft = n;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0) {
      if (errno == EINTR)
        nwritten = 0;   /* and call write() again */
      else
        return(-1);     /* error */
    }
    nleft -= nwritten;
    ptr   += nwritten;
  }
  return(n);
}

