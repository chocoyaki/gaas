/**
 * @file SGE_BatchSystem.cc
 *
 * @brief  Batch System Management with performance prediction: SGE
 *
 * @author  Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "debug.hh"
#include "SGE_BatchSystem.hh"

const char *const SGE_BatchSystem::statusNames[] = {
  "E", // Error
  "d", // canceled = deleted
  "-", // The job does not appear in qstat if it has ended
  "r", // running
  "qw", //
  "pending",
  "preempted"
};

SGE_BatchSystem::SGE_BatchSystem(int ID, const char *batchname) {
  /* FIXME: Dont know if this is true -> Check it! */

  if (pathToNFS == NULL) {
    ERROR_EXIT("SGE needs a path to a NFS directory to store its script");
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS, "Nom NFS: " << getNFSPath() << "\n");
#endif

  batch_ID = ID;
  batchName = batchname;

  shell = "-S ";
  prefixe = "#!/bin/sh";
  postfixe = BatchSystem::emptyString;

  serial = BatchSystem::emptyString;
  nodesNumber = "\n#$ -pe ";
  coresNumber = BatchSystem::emptyString;

  walltime = "\n#$ -l h_rt=";
  submittingQueue = "\n#$ -q ";
  minimumMemoryUsed = "\n#$ -l mem_free=";

  mail = "\n#$ -M ";
  account = "\n#$ -A ";
  setSTDOUT = "\n#$ -o ";
  setSTDIN = BatchSystem::emptyString;
  setSTDERR = "\n#$ -e ";

  submitCommand = "qsub ";
  killCommand = "qdel ";
  /* SGE is a bit weird:
     - Si le grep du job sur "qstat -u "*"" ne donne rien, on n'a pas encore
     l'output dans le fichier
     - l'output dans le fichier ne semble apparaître que quand qstat -j dit
     que le job n'est plus dans le système
   */
  wait4Command = "qstat -j ";
  waitFilter =
    "grep step | cut --delimiter = 1 --field = 3 | cut --delimiter=\",\" --field = 1";
  exitCode = "0";

  jid_extract_patterns = "cut --delimiter=\" -f 2 | cut --delimiter=. -f 2";

  /* Information for META_VARIABLES */
  batchJobID = "$JOB_ID";
  nodeFileName = "$TMPDIR/machines";
  nodeIdentities = "cat $TMPDIR/machines";
}

SGE_BatchSystem::~SGE_BatchSystem() {
}

/*********************** Job Managing ******************************/

BatchSystem::batchJobState
SGE_BatchSystem::askBatchJobStatus(int batchJobID) {
  char *filename;
  int file_descriptor;
  char *chaine;
  int i = 0;
  int nbread;
  batchJobState status;

  /* If job has completed, not ask batch system */
  status = getRecordedBatchJobStatus(batchJobID);
  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR)) {
    return status;
  }
  /* create a temporary file to get results and batch job ID */
  filename = (char *) malloc(sizeof(char) * strlen(pathToTmp) + 30);
  sprintf(filename, "%sDIET_batch_finish.XXXXXX", pathToTmp);
  file_descriptor = mkstemp(filename);
  if (file_descriptor == -1) {
    ERROR("Cannot create batch I/O redirection file", NB_STATUS);
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS, "Fichier_finish: " << filename << "\n");
#endif

  /* Ask batch system the job status */
  chaine = (char *) malloc(sizeof(char) * (strlen(wait4Command) * 2
                                           + NBDIGITS_MAX_BATCH_JOB_ID * 2
                                           + strlen(waitFilter) * 2
                                           + strlen(filename) * 2
                                           + 85 + 1));
  /* See EOF to get an example of what we parse */
  // ugly trick to use a SGE which does not keep the status of the batch once finished
  sprintf(
    chaine,
    "TMP_VAL=`%s %d 2>/dev/null | %s`;if [ \"$TMP_VAL\" == \"\" ];then echo E > %s;else %s %d | %s > %s;fi",
    wait4Command, batchJobID, waitFilter,
    filename,
    wait4Command, batchJobID, waitFilter, filename);
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS, "Execute:\n" << chaine << "\n");
#endif
  if (system(chaine) != 0) {
    ERROR("Cannot submit script", NB_STATUS);
  }

  /* Get job status */
  for (int i = 0; i <= NBDIGITS_MAX_BATCH_JOB_ID; i++)
    chaine[i] = '\0';
  if ((nbread = readn(file_descriptor, chaine, NBDIGITS_MAX_JOB_STATUS))
      == 0) {
    ERROR("Error with I/O file. Cannot read the batch status", NB_STATUS);
  }
  /* Adjust what have been read */
  if (chaine[nbread - 1] == '\n') {
    chaine[nbread - 1] = '\0';
  }
  /* Compare to chaine+1 because of a space as a first char */
  while ((i < NB_STATUS) &&
         (strcmp(chaine + 1, SGE_BatchSystem::statusNames[i]) != 0)) {
    i++;
  }

  if (i == NB_STATUS) {
    ERROR("Cannot get batch job " << batchJobID << " status: " << chaine,
          NB_STATUS);
  }
  /* Remove temporary file by closing it */
#if REMOVE_BATCH_TEMPORARY_FILE
  unlink(filename);
#endif
  if (close(file_descriptor) != 0) {
    WARNING("Couln't remove I/O redirection file");
  }
  updateBatchJobStatus(batchJobID, (batchJobState) i);
  free(chaine);
  free(filename);
  return (batchJobState) i;
} // askBatchJobStatus

int
SGE_BatchSystem::isBatchJobCompleted(int batchJobID) {
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
} // isBatchJobCompleted

/********** Batch static information accessing Functions **********/

int
SGE_BatchSystem::getNbTotResources() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return 16;
}

int
SGE_BatchSystem::getNbResources() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return 16;
}

int
SGE_BatchSystem::getMaxWalltime() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return 500;
}

int
SGE_BatchSystem::getMaxProcs() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return getNbResources();
}

/********** Batch dynamic information accessing Functions *********/

int
SGE_BatchSystem::getNbTotFreeResources() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return getNbResources();
}

int
SGE_BatchSystem::getNbFreeResources() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return getNbResources();
}

/*************************** Performance Prediction *************************/


/******************** Example of results */
/*
   Doc :
   http://ait.web.psi.ch/services/linux/hpc/merlin3/sge/admin/sge_jobs.html
   http://ait.web.psi.ch/services/linux/hpc/merlin3/sge/user/
   http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qstat.html

   p2chpd-cluster:~>qstat -j 20118
   ==============================================================
   job_number:                 20118
   submission_time:            Tue Feb 16 15:03:48 2010
   owner:                      ycaniou
   uid:                        7005
   group:                      users
   gid:                        100
   sge_o_home:                 /home_nfs/ycaniou
   sge_o_log_name:             ycaniou
   sge_o_path:                 .:..:/usr/local/bin:/home/ycaniou/Bin/bin:/home/ycaniou/Bin:/usr/local/jre1.6.0_06/bin:/opt/vltmpi/OPENIB/mpi/bin:/opt/xcsm/bin:/usr/local/n1ge61/bin/lx24-amd64:/softs/pgi_7.1.2/linux86-64/7.1-2/bin:/softs/openmpi/OPENIB/mpi.gcc.rsh//bin:/softs/openmpi/OPENIB/mpi.gcc.rsh/mpe2-1/bin:/softs/intel/cce/10.0.023/bin:/softs/intel/fce/10.0.023/bin:/opt/csm/bin:/usr/local/bin:/bin:/usr/bin:/softs/comsol34/bin:/softs/Fluent.Inc/bin:/softs/Fluent.Inc/bin:/softs/maple11/bin:/softs/matlabR2007b/bin:/usr/sbin:/softs/tecplot_360/bin:/sbin:/usr/sbin:/home_nfs/ycaniou/bin
   sge_o_shell:                /bin/bash
   sge_o_workdir:              /home_nfs/ycaniou
   sge_o_host:                 p2chpd-cluster
   account:                    sge
   mail_list:                  ycaniou@p2chpd-cluster.univ-lyon1.fr
   notify:                     FALSE
   job_name:                   script.sh
   jobshare:                   0
   env_list:
   script_file:                script.sh
   scheduling info:            queue instance "batch-v20z@node001.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node005.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node006.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node007.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node008.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node009.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node010.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node011.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node012.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node013.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node014.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node015.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node016.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node017.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node018.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node019.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node020.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node021.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node022.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node023.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node024.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node025.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node026.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node027.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node028.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node002.calcul" dropped because it is temporarily not available
   queue instance "interactive-v40z@nodesa001.calcul" dropped because it is temporarily not available
   queue instance "interactive-v40z@nodesa002.calcul" dropped because it is temporarily not available
   queue instance "batch-v20z@node003.calcul" dropped because it is disabled
   queue instance "batch-v20z@node004.calcul" dropped because it is disabled
   queue instance "admin@p2chpd-cluster" dropped because it is full
   queue instance "batch-x3550@node046.calcul" dropped because it is full
   queue instance "batch-x3550@node041.calcul" dropped because it is full
   queue instance "batch-x3550@node045.calcul" dropped because it is full
   queue instance "batch-x3550@node033.calcul" dropped because it is full
   queue instance "batch-x3550@node042.calcul" dropped because it is full
   queue instance "batch-x3550@node044.calcul" dropped because it is full
   queue instance "batch-x3550@node038.calcul" dropped because it is full
   queue instance "batch-x3550@node040.calcul" dropped because it is full
   queue instance "batch-x3550@node037.calcul" dropped because it is full
   queue instance "batch-x3550@node036.calcul" dropped because it is full
   queue instance "batch-x3550@node032.calcul" dropped because it is full
   queue instance "batch-x3550@node039.calcul" dropped because it is full
   queue instance "batch-x3550@node031.calcul" dropped because it is full
   queue instance "batch-x3550@node043.calcul" dropped because it is full
 */
