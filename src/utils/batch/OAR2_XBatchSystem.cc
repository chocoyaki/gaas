/****************************************************************************/
/* Batch System Management with performance prediction: OAR v2.x            */
/*                                                                          */
/* Author(s):                                                               */
/*    - Yves Caniou (yves.caniou@ens-lyon.fr)                               */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2011/02/24 16:52:40  bdepardo
 * Use new parser
 *
 * Revision 1.2  2010/03/31 19:37:55  bdepardo
 * Changed "\n" into std::endl
 *
 * Revision 1.1  2009/11/27 03:24:30  ycaniou
 * Add user_command possibility before the end of Batch prologue (only
 * to be used for batch dependent code!)
 * Memory leak/segfault--
 * New easy Batch basic example
 * Management of OAR2_X Batch scheduler
 *
 ****************************************************************************/

#include <fcntl.h>       // for O_RDONLY
#include <unistd.h>      // for read()

#include "debug.hh"
#include "OAR2_XBatchSystem.hh"

const char * const OAR2_XBatchSystem::statusNames[] = {
  "Error",
  "Error",
  "Terminated",
  "Running",
  "Waiting",
  "Launching",
};

OAR2_XBatchSystem::OAR2_XBatchSystem(int ID, const char * batchname)
{
  if (pathToNFS == NULL) {
    ERROR_EXIT("OAR2.X needs a path to a NFS directory to store its script");
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS, "Nom NFS: " << getNFSPath() << "\n");
#endif

  batch_ID = ID;
  batchName = batchname;

  shell = BatchSystem::emptyString;
  prefixe = "#!/bin/sh";
  postfixe = BatchSystem::emptyString;

  nodesNumber = "#OAR -l /nodes=";
  serial = "#OAR -l /nodes = 1";
  coresNumber = "/cores=";
  walltime = ", walltime=";
  submittingQueue = "\n#OAR -q ";
  minimumMemoryUsed = BatchSystem::emptyString;

  mail = " --notify mail:";
  account = BatchSystem::emptyString;
  setSTDOUT = " --stdout=";
  setSTDIN = BatchSystem::emptyString;
  setSTDERR = " --stderr=";

  /* cd, to be sure that OAR takes PWD and not /bin/pwd: for Grenoble */
  submitCommand = "oarsub -S ";
  killCommand = "oardel ";
  wait4Command = "oarstat -X -j";
  waitFilter = "grep state | cut -d \"<\" -f 2 | cut -d \">\" -f 2";
  exitCode = "0";

  jid_extract_patterns = "grep \"OAR_JOB_ID=\" | cut -d \"=\" -f 2";

  /* Information for META_VARIABLES */
  batchJobID = "$OAR_JOBID";
  nodeFileName = "$OAR_NODEFILE";
  nodeIdentities = "cat $OAR_NODEFILE";
}

OAR2_XBatchSystem::~OAR2_XBatchSystem()
{
}

/*********************** Job Managing ******************************/

BatchSystem::batchJobState
OAR2_XBatchSystem::askBatchJobStatus(int batchJobID)
{
  char * filename;
  int file_descriptor;
  char * chaine;
  int i = 0;
  int nbread;
  batchJobState status;

  /* If job has completed, not ask batch system */
  status = getRecordedBatchJobStatus(batchJobID);
  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR))
    return status;
  /* create a temporary file to get results and batch job ID */
  filename = createUniqueTemporaryTmpFile("DIET_batch_finish");
  file_descriptor = open(filename, O_RDONLY);
  if (file_descriptor == -1) {
    ERROR("Cannot open file", UNDETERMINED);
  }

  /* Ask batch system the job status */
  chaine = (char*)malloc(sizeof(char)*(strlen(wait4Command)
                                       + NBDIGITS_MAX_BATCH_JOB_ID
                                       + strlen(waitFilter)
                                       + strlen(filename)
                                       + 7 + 1));
  sprintf(chaine, "%s %d | %s > %s",
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
  if (chaine[nbread-1] == '\n')
    chaine[nbread-1] = '\0';
  while ((i<NB_STATUS) &&
         (strcmp(chaine, OAR2_XBatchSystem::statusNames[i])!=0)) {
    i++;
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
OAR2_XBatchSystem::isBatchJobCompleted(int batchJobID)
{
  batchJobState status = getRecordedBatchJobStatus(batchJobID);

  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR))
    return 1;
  status = askBatchJobStatus(batchJobID);
  if ((status == TERMINATED) || (status == CANCELED) || (status == ERROR))
    return 1;
  else if (status == NB_STATUS)
    return -1;
  return 0;
}

/********** Batch static information accessing Functions **********/

int
OAR2_XBatchSystem::getNbTotResources()
{
  return launchCommandAndGetInt("oarnodes -s | grep \":\" | wc -l",
                                 "DIET_getNbResources");
}

/* TODO: this function should be C++ written
   or, as OAR relies on Perl, use a Perl script which has to be
   deployed
*/
int
OAR2_XBatchSystem::getNbResources() /* in the queue interQueueName */
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return 500;
}

char *
OAR2_XBatchSystem::getResourcesName()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return NULL;
}

int
OAR2_XBatchSystem::getMaxWalltime()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return 500;
}

int
OAR2_XBatchSystem::getMaxProcs()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n");
  return getNbResources();
}

/********** Batch dynamic information accessing Functions *********/

int
OAR2_XBatchSystem::getNbTotFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not  implemented\n");
  return getNbResources();
}

int
OAR2_XBatchSystem::getNbFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not  implemented\n");
  return getNbResources();
}
