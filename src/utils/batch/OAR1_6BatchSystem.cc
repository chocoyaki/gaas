/**
 * @file OAR1_6BatchSystem.cc
 *
 * @brief  Batch System Management with performance prediction: OAR v1.6
 *
 * @author Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <fcntl.h>       // for O_RDONLY
#ifndef __WIN32__
#include <unistd.h>
#else
#include <Winsock2.h>
#include <windows.h>
#endif

#include "debug.hh"
#include "OAR1_6BatchSystem.hh"
#include "configuration.hh"

const char *const OAR1_6BatchSystem::statusNames[] = {
  "Error",
  "Error",
  "Terminated",
  "Running",
  "Waiting",
  "Launching"
};

OAR1_6BatchSystem::OAR1_6BatchSystem(int ID, const char *batchname) {
  if (pathToNFS == NULL) {
    ERROR_EXIT("OAR1.6 needs a path to a NFS directory to store its script");
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS, "Nom NFS: " << getNFSPath() << "\n");
#endif

  batch_ID = ID;
  batchName = batchname;
  /* Dirty Trick for OAR1.6 to get information on default queue */
  std::string tmpString;
  if (!CONFIG_STRING(diet::INTERNOARQUEUENAME, tmpString)) {
    ERROR_EXIT(
      "We need to know the internal queue name to be able to gather information with Cori");
  } else {
    internQueueName = strdup(tmpString.c_str());
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS, "Nom queue interne: " << internQueueName
                                                    << "\n");
#endif

  shell = BatchSystem::emptyString;
  prefixe = "#!/bin/sh\n";
  postfixe = BatchSystem::emptyString;

  nodesNumber = "#OAR -l nodes=";
  serial = "#OAR -l nodes = 1";
  coresNumber = BatchSystem::emptyString;
  walltime = "\n#OAR -l walltime=";
  submittingQueue = "\n#OAR -q ";
  minimumMemoryUsed = BatchSystem::emptyString;

  mail = BatchSystem::emptyString;
  account = BatchSystem::emptyString;
  setSTDOUT = BatchSystem::emptyString;
  setSTDIN = BatchSystem::emptyString;
  setSTDERR = BatchSystem::emptyString;

  /* cd, to be sure that OAR takes PWD and not /bin/pwd: for Grenoble */
  submitCommand = "cd .; oarsub ";
  killCommand = "oardel ";
  wait4Command = "oarstat -j ";
  waitFilter =
    "grep state | cut --delimiter== -f 2 | cut --delimiter=\" \" -f 2";
  exitCode = "0";

  jid_extract_patterns = "grep \"IdJob =\" | cut --delimiter== -f 2";

  /* OAR behaves with SQL scripts to reserve specials nodes */
  /* but the following line is not good enough: too less nodes */
  /*    ELBASE_NODETYPE, "#OAR -p \"hostname='%s'\"", */

  /* Information for META_VARIABLES */
  batchJobID = "$OAR_JOBID";
  nodeFileName = "$OAR_NODEFILE";
  nodeIdentities = "cat $OAR_NODEFILE";
}

OAR1_6BatchSystem::~OAR1_6BatchSystem() {
}

/*********************** Job Managing ******************************/

BatchSystem::batchJobState
OAR1_6BatchSystem::askBatchJobStatus(int batchJobID) {
  char *filename;
  int file_descriptor;
  char *chaine;
  int i = 0;
  int nbread;
  batchJobState status;

  /* If job has completed, not ask batch system */
  status = getRecordedBatchJobStatus(batchJobID);
  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR_JOB)) {
    return status;
  }
  /* create a temporary file to get results and batch job ID */
  filename = createUniqueTemporaryTmpFile("DIET_batch_finish");
  file_descriptor = open(filename, O_RDONLY);
  if (file_descriptor == -1) {
    ERROR_DEBUG("Cannot open file", UNDETERMINED);
  }

  /* Ask batch system the job status */
  chaine = (char *) malloc(sizeof(char) * (strlen(wait4Command)
                                           + NBDIGITS_MAX_BATCH_JOB_ID
                                           + strlen(waitFilter)
                                           + strlen(filename)
                                           + 7 + 1));
  sprintf(chaine, "%s %d | %s > %s",
          wait4Command, batchJobID, waitFilter, filename);
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS, "Execute:" << "\n" << chaine << "\n");
#endif
  if (system(chaine) != 0) {
    ERROR_DEBUG("Cannot submit script", NB_STATUS);
  }
  /* Get job status */
  for (int i = 0; i <= NBDIGITS_MAX_BATCH_JOB_ID; i++)
    chaine[i] = '\0';
  if ((nbread = readn(file_descriptor, chaine, NBDIGITS_MAX_JOB_STATUS))
      == 0) {
    ERROR_DEBUG("Error with I/O file. Cannot read the batch status", NB_STATUS);
  }
  /* Adjust what have been read */
  if (chaine[nbread - 1] == '\n') {
    chaine[nbread - 1] = '\0';
  }
  while ((i < NB_STATUS) &&
         (strcmp(chaine, OAR1_6BatchSystem::statusNames[i]) != 0)) {
    i++;
  }

  if (i == NB_STATUS) {
    ERROR_DEBUG("Cannot get batch job " << batchJobID << " status: " << chaine,
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
OAR1_6BatchSystem::isBatchJobCompleted(int batchJobID) {
  batchJobState status = getRecordedBatchJobStatus(batchJobID);

  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR_JOB)) {
    return 1;
  }
  status = askBatchJobStatus(batchJobID);
  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR_JOB)) {
    return 1;
  } else if (status == NB_STATUS) {
    return -1;
  }
  return 0;
} // isBatchJobCompleted

/********** Batch static information accessing Functions **********/

int
OAR1_6BatchSystem::getNbTotResources() {
  return launchCommandAndGetInt("oarnodes | grep state | wc -l",
                                "DIET_getNbResources");
}

/* TODO: this function should be C++ written
   or, as OAR relies on Perl, use a Perl script which has to be
   deployed
 */
int
OAR1_6BatchSystem::getNbResources() { /* in the queue internQueueName */
  char chaine[500];

  if (internQueueName == NULL) {
    WARNING("No internal queue Name given: use total information" << "\n\n");
    return getNbTotResources();
  }

  sprintf(chaine, "oarnodes | grep %s | wc -l", internQueueName);
  return launchCommandAndGetInt(chaine,
                                "DIET_getNameResources");
} // getNbResources

char *
OAR1_6BatchSystem::getResourcesName() {
  char chaine[500];

  sprintf(chaine, "oarnodes | grep %s | cut --delimiter=\"=\" --fields = 6"
                  " | cut --delimiter=\",\" --fields = 1", internQueueName);
  return launchCommandAndGetResultFilename(chaine,
                                           "DIET_getNameResources");
}

int
OAR1_6BatchSystem::getMaxWalltime() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented" << "\n\n");
  return 500;
}

int
OAR1_6BatchSystem::getMaxProcs() {
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented" << "\n\n");
  return getNbResources();
}

/********** Batch dynamic information accessing Functions *********/

int
OAR1_6BatchSystem::getNbTotFreeResources() {
  /* Command could be
     "oarstat -a | grep Free | cut --delimiter=" " --fields = 4" */
  return launchCommandAndGetInt("oarnodes | grep state | grep free | wc -l",
                                "DIET_getFreeResources");
}

int
OAR1_6BatchSystem::getNbFreeResources() {
  char chaine[500], hostname[50];
  char *filename, *filenameToParse;
  FILE *file, *fileToParse;
  int nbfree = 0;
  int j, k;

  if (internQueueName == NULL) {
    WARNING("No internal queue Name given: use total information" << "\n\n");
    return getNbTotResources();
  }

  filename = getResourcesName();
  filenameToParse = launchCommandAndGetResultFilename("oarnodes",
                                                      "DIET_oarInfo");

  /* For each name (each line), get information about host status */
  /* TODO: Do it the C++ way! Better, use a parser! */
  file = fopen(filename, "r");
  if (file == NULL) {
    WARNING("GetNbRsource: Cannot open file " << filename << "\n\n");
    return getNbTotResources();
  }
  fileToParse = fopen(filenameToParse, "r");
  if (fileToParse == NULL) {
    WARNING("GetNbRsource: Cannot open file " << filenameToParse << "\n");
    if (fclose(file) != 0) {
      WARNING("Couln't close file");
    }
    return getNbTotResources();
  }

  do { /* Read each hostname */
    j = fscanf(file, "%49s", hostname);

    /* Search for hostname in fileToParse */
    do
      k = fscanf(fileToParse, "%499s", chaine);
    while ((k != EOF) && (strcmp(chaine, hostname) != 0));
    /* TODO: should end if k == EOF because ERROR! */
    /* We have reached the line ^hostname */
    /* Search for the status */
    do
      k = fscanf(fileToParse, "%499s", chaine);
    while ((k != EOF) && (strcmp(chaine, "state") != 0));

    /* Seek "=" and get status */
    k = fscanf(fileToParse, "%499s", chaine);
    k = fscanf(fileToParse, "%499s", chaine);

    if (strcmp(chaine, "free") == 0) {
      nbfree++;
    }
  } while ((j != EOF));

#if REMOVE_BATCH_TEMPORARY_FILE
  unlink(filename);
#endif

  if (fclose(file) != 0) {
    WARNING("Couln't close file");
  }
  if (fclose(fileToParse) != 0) {
    WARNING("Couln't close file");
  }

  free(filename);
  free(filenameToParse);

  return nbfree;
} // getNbFreeResources

/*************************** Performance Prediction *************************/
