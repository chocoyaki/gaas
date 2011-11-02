/**
* @file Slurm_BatchSystem.cc
* 
* @brief  Batch System Management with performance prediction: Slurm 
* 
* @author - Benjamin Depardon (Benjamin.Depardon@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 ****************************************************************************/

#include "debug.hh"
#include "Slurm_BatchSystem.hh"
#include <limits.h>

const char * Slurm_BatchSystem::statusNames[] = {
  "FAILED", // not OK: there is no error state in Slurm, either the job is
  "CANCELLED", //         running, or it has exited
  "COMPLETED", //         So, how to decide if all is ok? Parse the error file?
  "RUNNING", // Job is Running
  "PENDING", // Job is Held
  "PENDING", // Job is Queued, eligible to run or be routed
  "SUSPENDED", // ?? Job is Suspended
  "TIMEOUT"  // ?? Job is waiting for its requested execution time to be reached
  //   or job specified a stage-in request which failed for some reason
};

Slurm_BatchSystem::Slurm_BatchSystem(int ID, const char * batchname) {
  if (pathToNFS == NULL) {
    ERROR_EXIT("Slurm needs a path to a NFS directory to store its script");
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS, "Nom NFS: " << getNFSPath() << "\n");
#endif

  batch_ID = ID;
  batchName = batchname;

  shell    = BatchSystem::emptyString;
  prefixe  = "#!/bin/sh";
  // the -V option declares that all environment variables in the qsub
  // command's environment are to be exported to the batch job
  // the -J option is to set the name of the job (usefull to debug)
  postfixe = strdup(
    (std::string("\n#SBATCH -J DIET_SeD \n DIET_HOST_FILE=") +
     std::string(pathToNFS) +
     std::string("/slurm-${SLURM_JOB_ID}.hosts") +
     std::string(" \n srun hostname -s > $DIET_HOST_FILE \n if [ -z \"$SLURM_NPROCS\" ]; then \n if [ -z \"$SLURM_NTASKS_PER_NODE\" ]; then \n SLURM_NTASKS_PER_NODE = 1 \n fi \n SLURM_NPROCS=$(($SLURM_JOB_NUM_NODES * $SLURM_NTASKS_PER_NODE)) \n fi")).c_str());
  nodesNumber       = "#SBATCH --nodes=";
  serial            = "#SBATCH --nodes = 1";
  coresNumber = BatchSystem::emptyString;
  walltime          = "\n#SBATCH --time=";
  submittingQueue   = "\n#SBATCH -p ";
  minimumMemoryUsed = "\n#SBATCH --mem=";

  /* TODO: When we use some ID for DIET client, change there! */
  //mail      = "#Slurm -m a\n#Slurm -M ";  // -m, send mail when:
  // a: job is aborted by batch system
  // b: job begins execution
  // e: job ends execution
  // n: do not send mail
  // -M, recipient list
  mail      = "\n#SBATCH --mail-type = ALL --mail-user=";
  account   = "\n#SBATCH -A ";
  setSTDOUT = "\n#SBATCH -o ";
  setSTDIN  = "\n#SBATCH -i ";
  setSTDERR = "\n#SBATCH -e ";

  submitCommand = "sbatch ";
  killCommand   = "scancel ";
  wait4Command  = "scontrol show job ";  // nom de la commande qui va prendre l'id du job pour en r�cup�rer les informations
  waitFilter    = "grep JobState | cut -d\"=\" -f2 | cut -d\" \" -f1";  // commande pour r�cup�rer l'�tat  du job dont l'info sort de wait4Command.
  // Qui doit �tre l'un des statusNames
  exitCode      = "0";

  // nothing to do to retrieve the ID of the submission...
  // but we need to add something, let's say 'uniq' :-)
  jid_extract_patterns = "cut -d\" \" -f4";

  /* Information for META_VARIABLES */
  batchJobID     = "$SLURM_JOB_ID";
  nodeFileName   = strdup(
    (std::string(pathToNFS) +
     std::string("/slurm-${SLURM_JOB_ID}.hosts")).c_str());
  nodeIdentities = "$SLURM_JOB_NODELIST";
}

Slurm_BatchSystem::~Slurm_BatchSystem() {
}

/*********************** Job Managing ******************************/

BatchSystem::batchJobState
Slurm_BatchSystem::askBatchJobStatus(int batchJobID) {
  char * filename;
  int file_descriptor;
  char * chaine;
  int i = 0;
  int nbread;
  batchJobState status;

  /* If job has completed, not ask batch system */
  status = getRecordedBatchJobStatus(batchJobID);
  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR)) {
    return status;
  }
  /* create a temporary file to get results and batch job ID */
  filename = createUniqueTemporaryTmpFile("DIET_batch_finish");
  file_descriptor = open(filename, O_RDONLY);
  if (file_descriptor == -1) {
    ERROR("Cannot open file", UNDETERMINED);
  }

  /*** Ask batch system the job status ***/
  chaine = (char*)malloc(sizeof(char)*(strlen(wait4Command) * 2
                                       + NBDIGITS_MAX_BATCH_JOB_ID * 2
                                       + strlen(waitFilter) * 2
                                       + strlen(filename) * 3
                                       + 200 + 1));
  //cout << "First chaine : " << chaine << "\n";
  /* See EOF to get an example of what we parse */
  // ugly trick to use a Slurm which does not keep the status of the batch once finished
  sprintf(chaine, "TMP_VAL=`%s %d 2>/dev/null | %s`;if [ \"$TMP_VAL\" == \"\" ];then echo FAILED > %s;else %s %d | %s > %s;fi",
          wait4Command, batchJobID, waitFilter,
          filename,
          wait4Command, batchJobID, waitFilter, filename, filename);
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS, "Execute:\n" << chaine << "\n");
#endif
  if (system(chaine) != 0) {
    ERROR("Cannot submit script", NB_STATUS);
  }
  /* Get job status */
  for (int i = 0; i <= NBDIGITS_MAX_BATCH_JOB_ID; i++) {
    chaine[i] = '\0';
  }

  if ((nbread = readn(file_descriptor, chaine, NBDIGITS_MAX_JOB_STATUS)) == 0) {
    ERROR("Error with I/O file. Cannot read the batch status", NB_STATUS);
  }

  if (nbread == 0) {
    /* we consider that like OK */
    i = TERMINATED;
  } else {
    /* Adjust what have been read */
    if (chaine[nbread-1] == '\n') {
      chaine[nbread-1] = '\0';
    }
    while ((i<NB_STATUS) &&
           (strcmp(chaine, Slurm_BatchSystem::statusNames[i])!=0)) {
      i++;
    }
  }
  if (i == NB_STATUS) {
    ERROR("Cannot get batch job " << batchJobID << " status: " << chaine, NB_STATUS);
  }
  /* Remove temporary file by closing it */
#if REMOVE_BATCH_TEMPORARY_FILE
  unlink(filename);
#endif
  if (close(file_descriptor) != 0) {
    WARNING("Couln't remove I/O redirection file");
  }
  updateBatchJobStatus(batchJobID,(batchJobState)i);
  free(chaine);
  free(filename);
  return (batchJobState)i;
}

int
Slurm_BatchSystem::isBatchJobCompleted(int batchJobID) {
  int status = getRecordedBatchJobStatus(batchJobID);

  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR)) {
    return 1;
  }
  status = askBatchJobStatus(batchJobID);
  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR)) {
    return 1;
  } else if (status == NB_STATUS) {
    return -1;
  }
  return 0;
}

/********** Batch static information accessing Functions **********/

int
Slurm_BatchSystem::getNbResources() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return getNbTotResources();
}

int
Slurm_BatchSystem::getNbTotResources() {
  return launchCommandAndGetInt("sinfo -h -o \"%20F\" | cut -d\"/\" -f 4",
                                 "DIET_getNbResources");
}

/**
 * @todo DL: should be implemented correctly. I don't know what the unit is, so... INT_MAX is returned.
 *
 */
int
Slurm_BatchSystem::getMaxWalltime() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return INT_MAX;
}

int
Slurm_BatchSystem::getMaxProcs() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return getNbResources();
}

/********** Batch dynamic information accessing Functions *********/

int
Slurm_BatchSystem::getNbTotFreeResources() {
  return launchCommandAndGetInt("sinfo -h -o \"%20F\" | cut -d\"/\" -f 2",
                                 "DIET_getNbResources");
}

int
Slurm_BatchSystem::getNbFreeResources() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return getNbResources();
}

/*************************** Performance Prediction *************************/


/************************* Examples of commands ****************************/
/*
  [eddy@breeze ~]$ qstat -f 70.breeze.ics.hawaii.edu
  Job Id: 70.breeze.ics.hawaii.edu
  Job_Name = Anacle_8
  Job_Owner = davidls@breeze.ics.hawaii.edu
  resources_used.cput = 43:38:36
  resources_used.mem = 54412kb
  resources_used.vmem = 152520kb
  resources_used.walltime = 24:06:40
  job_state = R
  queue = default
  server = breeze.ics.hawaii.edu
  Checkpoint = u
  ctime = Fri Aug 15 17:26:21 2008
  Error_Path = breeze.ics.hawaii.edu:/home/davidls/anacle/scripts/Anacle_8.e
  70
  exec_host = compute-0-0.local/1+compute-0-0.local/0
  Hold_Types = n
  Join_Path = n
  Keep_Files = n
  Mail_Points = a
  mtime = Fri Aug 15 17:26:22 2008
  Output_Path = breeze.ics.hawaii.edu:/home/davidls/anacle/scripts/Anacle_8.
  o70
  Priority = 0
  qtime = Fri Aug 15 17:26:21 2008
  Rerunable = True
  Resource_List.nodect = 1
  Resource_List.nodes = 1:ppn = 2
  session_id = 19620
  etime = Fri Aug 15 17:26:21 2008

*/
