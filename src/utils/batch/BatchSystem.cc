/**
 * @file BatchSystem.cc
 *
 * @brief  Batch System Management with performance prediction
 *
 * @author - Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#include <cstring>
#include <cstdlib>
#include <sys/types.h>   // for chmod()
#include <sys/stat.h>    // for S_IWUSR, S_IXUSR, chmod()
#include <cerrno>       // for errno
#ifndef __WIN32__
#include <unistd.h>
#else
#include <Winsock2.h>
#include <windows.h>
#endif
#include <fcntl.h>       // for O_RDONLY

#include "debug.hh"      // ERROR
#include "configuration.hh"
#include "BatchSystem.hh"
#include "BatchCreator.hh" // LOADLVELER, OAR1.6, etc. definition
#ifdef __WIN32__
#include "mkstemp.hh"
#endif
using namespace std;
#define WAITING_BATCH_JOB_COMPLETION 30 // test all W_B_J_C sec if job completed

const char *BatchSystem::emptyString = "";

/****************************************************************************/
/* Public methods                                                           */
/****************************************************************************/

BatchSystem::BatchSystem() {
  std::string tmpString;

  batchJobQueue = NULL;

  /* We are still on the frontal, whose name must be given to the service
  ** in case of data transfer or fault tolerance mechanism */
  if (gethostname(frontalName, 256)) {
    ERROR_EXIT("could not get hostname");
  }
  frontalName[255] = '\0';  // If truncated, ensure null termination

  /* Continue to parse the SeD configuration file */
  /* Search for batch queues */
  if (!CONFIG_STRING(diet::BATCHQUEUE, tmpString)) {
    batchQueueName = emptyString;
  } else {
    batchQueueName = strdup(tmpString.c_str());
  }

  /*  "__\0" -> 3 char, strlen = 2 and [1] should be '\' */
  if (!CONFIG_STRING(diet::PATHTONFS, tmpString)) {
    pathToNFS = NULL;
  } else if (tmpString[tmpString.size() - 1] == '/') {
    pathToNFS = strdup(tmpString.c_str());
  } else {
    tmpString += "/";
    pathToNFS = strdup(tmpString.c_str());
  }
  errorIfPathNotValid(pathToNFS);

  if (!CONFIG_STRING(diet::PATHTOTMP, tmpString)) {
    // ERROR_EXIT("Please set a correct path to a tmp directory");
    WARNING("Assume /tmp/ as temporary file directory!");
    pathToTmp = strdup("/tmp/");
  } else if (tmpString[tmpString.size() - 1] == '/') {
    pathToTmp = strdup(tmpString.c_str());
  } else {
    tmpString += "/";
    pathToTmp = strdup(tmpString.c_str());
  }
  errorIfPathNotValid(pathToTmp);
}

BatchSystem::~BatchSystem() {
}

int
BatchSystem::getBatchID() {
  return (int) batch_ID;
}

const char *
BatchSystem::getBatchQueueName() {
  return batchQueueName;
}

const char *
BatchSystem::getBatchName() {
  return batchName;
}

const char *
BatchSystem::getNFSPath() {
  return pathToNFS;
}

const char *
BatchSystem::getTmpPath() {
  return pathToTmp;
}

/************************ Submitting Funtions *******************/

int
BatchSystem::wait4DietJobCompletion(diet_profile_t *profile) {
  // Build bach script that checks for batch job completion all WAIT_TIME

  // system(script) so that we wait for the script completion
  return -1;
}

int
BatchSystem::checkIfDietJobCompleted(diet_profile_t *profile) {
  // Build a bash script
  // System(script) and get if batch job has completed
  return -1;
}

int
BatchSystem::diet_submit_parallel(diet_profile_t *profile,
                                  const char *addon_prologue,
                                  const char *command) {
  int nbread;
  char *script = NULL;
  char small_chaine[NBDIGITS_MAX_BATCH_JOB_ID + 1];
  char *chaine;
  char *options;
  int file_descriptor;
  int file_descriptor_2;
  char *filename;
  char *filename_2;
  const char *loc_addon_prologue;

  /* Options, if available, are in the following order:
     nbnodes
     walltime
     queue
     minimumMemoryUsed
     mail
     stdin
     stdout
     stderr
   */
  options = (char *) calloc(9000, sizeof(char)); /* FIXME: Reduce size */
  if (options == NULL) {
   ERROR_DEBUG(
      "error allocating memory when building script (options)..." << endl <<
      "Service not launched", -1);
  }
  /* Convert the walltime in hh:mm:ss, standard for every batch */
  /* TODO:
     1) should be checked if possible, and even possible, with Cori!
     2) for LSF, hh:mm only
   */
  sprintf(small_chaine, "%02d:%02d:%02d",
          (int) (profile->walltime / 3600),
          (int) (profile->walltime % 3600) / 60,
          (int) (profile->walltime % 3600) % 60);

  if (profile->parallel_flag == 1) {
    sprintf(options,
            "%s%s%s",
            this->serial,
            this->walltime, small_chaine);
  } else {
    switch ((int) batch_ID) {
    case BatchCreator::SGE:
      sprintf(options,
              "%s $NSLOTS %s%s",
              this->nodesNumber,
              this->walltime, small_chaine);
      break;
    default:
      sprintf(options,
              "%s%d%s%s",
              this->nodesNumber, profile->nbprocs,
              this->walltime, small_chaine);
    } // switch
  }
  sprintf(options + strlen(options),
          "%s%s",
          submittingQueue, batchQueueName);


  /* TODO: the user will be able to set the shell, mail, stdin, etc. */
  // Shell semble ne pas marcher... :?
  // if (shell != emptyString) /* & if user hasnt specified one or set to user */
  // sprintf(options+strlen(options),
  // "%s bash\n",
  // shell);

  // if (setSTDERR != emptyString) /* FIXME: only for LL.. */
  // sprintf(options+strlen(options),
  // "%s$(job_name).err\n",
  // setSTDERR);
  // if (setSTDOUT != emptyString) /* FIXME: only for LL.. */
  // sprintf(options+strlen(options),
  // "%s$(job_name).out\n",
  // setSTDOUT);
  /*
     if (setSTDIN != emptyString) ** & if user has set one entry
     sprintf(options+strlen(options),
     "%s %s
   */

  /* Define addon_prologue for strlen */
  if (addon_prologue == NULL) {
    loc_addon_prologue = emptyString;
  } else {
    loc_addon_prologue = addon_prologue;
  }

  // Build Script, copy it on the NFS path and launch in a system()
  // and record the pid of the batch job
  script = (char *) malloc(sizeof(char) * (100
                                           + strlen(prefixe)
                                           + strlen(options)
                                           + strlen(loc_addon_prologue)
                                           + strlen(postfixe)
                                           + 1000
                                           + strlen(command)));
  if (script == NULL) {
   ERROR_DEBUG("error allocating memory when building script..." << endl <<
          "Service not launched", -1);
  }
  switch ((int) batch_ID) {
  case BatchCreator::LOADLEVELER:
    sprintf(script,
            "%s\n"
            "%s"
            "%s"
            "%s\n"
            "\n%s\n"
            , prefixe
            , options
            , loc_addon_prologue
            , postfixe
            , command
            );
    break;
  case BatchCreator::OAR1_6:
  case BatchCreator::OAR2_X:
  case BatchCreator::SLURM:
  case BatchCreator::SGE:
  case BatchCreator::PBS:
    sprintf(script,
            "%s\n" // prefixe
            "%s" // options
            "%s" // loc_addon_prologue
            "%s\n" // postfixe
            "DIET_BATCH_NODESFILE=%s\n" // nodeFileName
            "DIET_BATCH_NODESLIST=$(cat %s | sort | uniq)\n" // nodeFileName
            "DIET_BATCH_NBNODES=$(echo $DIET_BATCH_NODESLIST | wc -l)\n"
            "\n%s\n" // command
            , prefixe
            , options
            , loc_addon_prologue
            , postfixe
            , nodeFileName
            , nodeFileName
            , command
            );
    break;
  default:
   ERROR_DEBUG("BatchSystem not managed?", -1);
  } // switch
    /* Replace DIET meta-variable in SeD programmer's command */
  sprintf(small_chaine, "%d", profile->nbprocs);
  replaceAllOccurencesInString(&script, "$DIET_BATCH_NBNODES", small_chaine);
  sprintf(small_chaine, "%d", profile->nbprocess);
  replaceAllOccurencesInString(&script, "$DIET_USER_NBPROCS", small_chaine);
  replaceAllOccurencesInString(&script, "$DIET_NAME_FRONTALE", frontalName);
  replaceAllOccurencesInString(&script, "$DIET_BATCH_JOB_ID", batchJobID);
  replaceAllOccurencesInString(&script, "$DIET_BATCHNAME", batchName);

  /* Create temporary file to save the script, and submit it */
  filename = (char *) malloc(sizeof(char) * strlen(pathToNFS) + 30);
  sprintf(filename, "%sDIET_batch_script.XXXXXX", pathToNFS);

  file_descriptor = mkstemp(filename);
  if (file_descriptor == -1) {
   ERROR_DEBUG("Cannot create batch file", -1);
  }


#if defined YC_DEBUG
  TRACE_TEXT(TRACE_MAIN_STEPS, "Nom script: " << filename << endl);
#endif

  if (writen(file_descriptor, script, strlen(script)) != strlen(script)) {
   ERROR_DEBUG("Cannot write the batch script on the filesystem", -1);
  }
  /* Make the script runnable (OAR) */
  if (chmod(filename, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
    WARNING("Execution rights have not been set on the batch script."
            " This can lead to an error");
  }

  /***************** Submit script and store the batchID ********************/
  /* create a temporary file to get results and batch job ID */
  filename_2 = (char *) malloc(sizeof(char) * strlen(pathToTmp) + 30);
  sprintf(filename_2, "%sDIET_batch_launch.XXXXXX", pathToTmp);
  file_descriptor_2 = mkstemp(filename_2);
  if (file_descriptor_2 == -1) {
   ERROR_DEBUG("Cannot create batch I/O redirection file."
          " Verify that tmp path is ok", -1);
  }
  close(file_descriptor_2);


#if defined YC_DEBUG
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Fichier pour l'ID du job batch : " << filename_2 << endl);
#endif
  /* Submit and grep the jobID */
  chaine = (char *) malloc(sizeof(char) * (strlen(submitCommand)
                                           + strlen(filename)
                                           + strlen(jid_extract_patterns)
                                           + strlen(filename_2)
                                           + 7 + 1));
  sprintf(chaine, "%s %s | %s > %s",
          submitCommand, filename, jid_extract_patterns, filename_2);
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Submit avec la ligne :" << endl << chaine << endl << endl);
#endif

  if (system(chaine) == -1) {
   ERROR_DEBUG("Cannot submit script", -1);
  }
  file_descriptor_2 = open(filename_2, O_RDONLY);
  if (file_descriptor_2 == -1) {
   ERROR_DEBUG("Cannot open batch I/O redirection file", -1);
  }
  /* Get batch Job ID */
  for (int i = 0; i <= NBDIGITS_MAX_BATCH_JOB_ID; i++)
    small_chaine[i] = '\0';
  if ((nbread =
         readn(file_descriptor_2, small_chaine, NBDIGITS_MAX_BATCH_JOB_ID))
      == 0) {
   ERROR_DEBUG("Error during submission or with I/O file."
          " Cannot read the batch ID", -1);
  }
  /* Just in case */
  if (small_chaine[nbread - 1] == '\n') {
    small_chaine[nbread - 1] = '\0';
  }
  /* Store the JobID in correlation with DIET_taskID */
  if (storeBatchJobID(atoi(small_chaine), profile->dietReqID, filename) < 0) {
   ERROR_DEBUG("Not enough memory to store new batch information", -1);
  }

  /* Remove temporary files by closing them */
#if REMOVE_BATCH_TEMPORARY_FILE
  unlink(filename_2);
#endif
  if (close(file_descriptor) != 0) {
    WARNING("Couln't close batch script file");
  }
  if (close(file_descriptor_2) != 0) {
    WARNING("Couln't close I/O redirection file");
  }

  /* Free memory */
  free(options);
  free(script);
  free(chaine);
  free(filename_2);
  return 0;
} // diet_submit_parallel

int
BatchSystem::diet_submit_parallel(int batchJobID, diet_profile_t *profile,
                                  const char *command) {
  {
   ERROR_DEBUG("This function is not implemented yet", 1);
  }
}

/*********************** Job Managing ******************************/
int
BatchSystem::storeBatchJobID(int batchJobID, int dietReqID,
                             char *filename) {
  corresID *tmp;

  tmp = (corresID *) malloc(sizeof(corresID));
  if (tmp == NULL) {
    return(-1);
  }

  corresBatchReqID_mutex.lock();

  tmp->nextStruct = this->batchJobQueue;

  this->batchJobQueue = tmp;
  this->batchJobQueue->batchJobID = batchJobID;
  this->batchJobQueue->dietReqID = dietReqID;
  this->batchJobQueue->status = SUBMITTED;
  this->batchJobQueue->scriptFileName = filename;

  corresBatchReqID_mutex.unlock();

  return 0;
} // storeBatchJobID

int
BatchSystem::removeBatchJobID(int dietReqID) {
  corresID *index_1 = this->batchJobQueue;
  corresID *index_2;

  if (index_1 != NULL) {
    if (index_1->dietReqID != dietReqID) {
      index_2 = index_1->nextStruct;

      while ((index_2 != NULL) && (index_2->dietReqID != dietReqID)) {
        index_1 = index_2;
        index_2 = index_2->nextStruct;
      }
      if (index_2 == NULL) {
        return -1;
      }
      index_1 = index_2->nextStruct;
      if (index_2->scriptFileName != NULL) {
        unlink(index_2->scriptFileName);
        free(index_2->scriptFileName);
        index_2->scriptFileName = NULL;
      }
      free(index_2);
      return 1;
    } else {
      this->batchJobQueue = this->batchJobQueue->nextStruct;
      if (index_1->scriptFileName != NULL) {
        unlink(index_1->scriptFileName);
        free(index_1->scriptFileName);
        index_1->scriptFileName = NULL;
      }
      free(index_1);
      return 1;
    }
  }
  return -1;
} // removeBatchJobID

int
BatchSystem::getBatchJobID(int dietReqID) {
  corresID *index = this->batchJobQueue;

  while ((index != NULL) && (index->dietReqID != dietReqID)) {
    index = index->nextStruct;
  }

  if ((index == NULL)) {
    return -1;
  }

  return index->batchJobID;
} // getBatchJobID

int
BatchSystem::wait4BatchJobCompletion(int batchJobID) {
  int status;

  status = isBatchJobCompleted(batchJobID);

  while (status == 0) {
#ifndef __WIN32__
    sleep(WAITING_BATCH_JOB_COMPLETION);
#else
    Sleep(WAITING_BATCH_JOB_COMPLETION);
#endif
    status = isBatchJobCompleted(batchJobID);
  }
  return status;
} // wait4BatchJobCompletion

BatchSystem::batchJobState
BatchSystem::getRecordedBatchJobStatus(int batchJobID) {
  corresID *index = this->batchJobQueue;

  while ((index != NULL) && (index->batchJobID != batchJobID)) {
    index = index->nextStruct;
  }
  if ((index == NULL)) {
    return NB_STATUS;
  }
  return index->status;
} // getRecordedBatchJobStatus

int
BatchSystem::updateBatchJobStatus(int batchJobID, batchJobState job_status) {
  corresID *index = this->batchJobQueue;

  while ((index != NULL) && (index->batchJobID != batchJobID)) {
    index = index->nextStruct;
  }
  if ((index == NULL)) {
    return -1;
  }

  index->status = job_status;
  if (((job_status == TERMINATED)
       || (job_status == CANCELED)
       || (job_status == ERROR_JOB))
      && (index->scriptFileName != NULL)) {
    unlink(index->scriptFileName);
    free(index->scriptFileName);
    index->scriptFileName = NULL;
  }
  return 1;
} // updateBatchJobStatus

/****************** Performance Prediction Functions ***************/

int
BatchSystem::getSimulatedProcAndWalltime(int *nbprocPtr, int *walltimePtr,
                                         diet_profile_t *profilePtr) {
  {
   ERROR_DEBUG("This function is not implemented yet", 1);
  }
}

/****************** Utility function ********************/
int
BatchSystem::replaceAllOccurencesInString(char **input,
                                          const char *occurence,
                                          const char *by) {
  int found = 0;
  int lengthBy = strlen(by);
  int lengthOccurence = strlen(occurence);
  int lengthResult = strlen(*input);
  int lengthInput = strlen(*input);
  int tmpLength = 0;                           // size of what has been copied
  char *indexBegin = *input;
  char *indexEnd = *input;
  char *resultingString = NULL;
  char *tmpString = NULL;                 // index on where to write next time

#if defined YC_DEBUG_
  int passa = 0;
  TRACE_TEXT(TRACE_MAIN_STEPS, "Replace " << occurence << " by "
                                          << by << endl);
  cout << "Script:" << strlen(*input)
       << "--------------------------------------" << endl << *input
       << endl << "--------------------------------------------" << endl;
#endif

  resultingString = (char *) calloc(lengthResult, sizeof(char));
  tmpString = resultingString;

  while ((indexEnd != NULL) && (indexEnd < (*input) + lengthInput)) {
    indexEnd = strstr(indexBegin, occurence);
    if (indexEnd != NULL) {
      found = 1;
      /********* copy all before occurence in resultString ***********/
      /* check if string is long enough */
      if ((tmpLength + (int) (indexEnd - indexBegin) + lengthBy + 1)
          > lengthResult) {
#if defined YC_DEBUG_
        passa++;
        TRACE_TEXT(TRACE_MAIN_STEPS, "Pass " << passa
                                             << "Reallocate memory:"
                                             << tmpLength +
                   (int) (indexEnd - indexBegin) + lengthBy + 1
                                             << " > " << lengthResult << endl);
#endif
        tmpString = (char *) realloc(resultingString,
                                     tmpLength
                                     + (int) (indexEnd - indexBegin)
                                     + lengthBy
                                     + 1                        // \0 char
                                     );
        if (tmpString == NULL) {
          free(resultingString);
          return -1;
        }
        lengthResult = tmpLength + (int) (indexEnd - indexBegin) + lengthBy;
        resultingString = tmpString;       // address may have changed
        tmpString += tmpLength;            // points on next writable char
      }
      /* Copy */
#if defined YC_DEBUG_
      printf("Copy this number of bytes: %d\n", (int) (indexEnd - indexBegin));
#endif
      strncpy(tmpString, indexBegin, (int) (indexEnd - indexBegin));
      tmpString += (int) (indexEnd - indexBegin);
      strncpy(tmpString, by, lengthBy);
      tmpString += lengthBy;
      tmpLength += (int) (indexEnd - indexBegin) + lengthBy;
      indexEnd += lengthOccurence;         // pass occurence
      indexBegin = indexEnd;
    }
  }
  if (found == 1) {
    /***** Copy last part of input: (*input + strlen(*input) - indexBegin) */
    /* check if string is long enough */
    int lastchar = (int) ((*input + lengthInput) - indexBegin) + 1;
    if (tmpLength + lastchar > lengthResult) {   // or use strlen(indexBegin)
      tmpString = (char *) realloc(resultingString,
                                   tmpLength
                                   + lastchar // takes into account the \0
                                   );
      if (tmpString == NULL) {
        free(resultingString);
        return -1;
      }
      lengthResult = tmpLength + lastchar;
      resultingString = tmpString;       // address may have changed
      tmpString += tmpLength;            // points on next writable char
    }
    /* Copy */
    strcpy(tmpString, indexBegin);
    /* Replace string */
    free(*input);
    (*input) = resultingString;
  }
#if defined YC_DEBUG_
  cout << "Finished: script size" << strlen(resultingString)
       << " / " << lengthResult << endl << endl;
#endif
  return found;
} // replaceAllOccurencesInString

size_t
BatchSystem::writen(int fd, const char *buffer, size_t n) {
  size_t nleft;
  //ssize_t nwritten;
  size_t nwritten;
  const char *ptr;

  ptr = buffer;
  nleft = n;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0) {
      if (errno == EINTR) {
        nwritten = 0;  /* and call write() again */
      } else {
        return(0);   /* error */
      }
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return(n);
} // writen

int
BatchSystem::readn(int fd, char *buffer, int n) {
  int nread;
  int nleft;
  char *ptr;

#if defined YC_DEBUG_
  TRACE_TEXT(TRACE_ALL_STEPS,
             "Enter readn() " << n << " octets to read" << endl);
#endif

  ptr = buffer;
  nleft = n;
  while (nleft != 0) {
    if ((nread = read(fd, ptr, nleft)) < 0) {
      if (nread < 0)       // ERROR
#if defined YC_DEBUG_
      {switch (errno) {
       case EBADF:
        ERROR_DEBUG("Descripteur de fichiers invalide ou pas ouvert en lecture", 0);
       case EFAULT:
        ERROR_DEBUG("Buffer pointe en dehors de l'espace d'adressage", 0);
       case EINVAL:
        ERROR_DEBUG("EINVAL", 0);
       case EIO:
        ERROR_DEBUG("EIO", 0);
       default:
        ERROR_DEBUG("Undefined", 0);
       }                 // switch
      }
#endif // if defined YC_DEBUG_
      {return(0);
      }
    } else if (nread == 0) { /*EOF*/
      break;
    }
    nleft -= nread;
    ptr += nread;
  }
  return(n - nleft);
} // readn

void
BatchSystem::errorIfPathNotValid(const char *path) {
  struct stat buf;
  char chaine[100];

  if (stat(path, &buf) == -1) {
#ifndef __WIN32__
    snprintf(chaine, 99, "Cannot stat on file %s", path);
#else
    _snprintf(chaine, 99, "Cannot stat on file %s", path);
#endif
    perror(chaine);
    exit(1);
  }

  // TODO: check that if symlink, rights of the pointeur are ok
  // and if not, rights on the directory are ok
  if ((S_ISDIR(buf.st_mode) == 1) /* directory */
      // && ((buf.st_mode & S_IRWXU) == 1) /* can enter, read and write it */
      ) {
    return;
  }

#ifndef __WIN32__
  snprintf(chaine, 99, "file %s is not a directory", // , or rights problems",
#else
  _snprintf(chaine, 99, "file %s is not a directory", // , or rights problems",
#endif
           path);
  ERROR_EXIT(chaine);
} // errorIfPathNotValid

char *
BatchSystem::createUniqueTemporaryTmpFile(const char *pattern) {
  int file_descriptor;
  char *filename;

  filename = (char *) malloc(sizeof(char) * strlen(pathToTmp) + 30);
  sprintf(filename, "%s%s.XXXXXX", pathToTmp, pattern);
  file_descriptor = mkstemp(filename);
  if (file_descriptor == -1) {
   ERROR_DEBUG("Cannot create batch I/O redirection file."
          " Verify that tmp path is ok and that there is space left", 0);
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Fichier pour stocker info batch : " << filename << endl);
#endif
  close(file_descriptor);
  return filename;
} // createUniqueTemporaryTmpFile

char *
BatchSystem::createUniqueTemporaryNFSFile(const char *pattern) {
  int file_descriptor;
  char *filename;

  filename = (char *) malloc(sizeof(char) * strlen(pathToNFS) + 30);
  sprintf(filename, "%s%s.XXXXXX", pathToNFS, pattern);
  file_descriptor = mkstemp(filename);
  if (file_descriptor == -1) {
   ERROR_DEBUG("Cannot create batch I/O redirection file."
          " Verify that tmp path is ok and that there is space left", 0);
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Fichier pour stocker info batch : " << filename << endl);
#endif

  close(file_descriptor);
  return filename;
} // createUniqueTemporaryNFSFile

int
BatchSystem::readNumberInFile(const char *filename) {
  char small_chaine[10];  // This must be gt NBDIGITS_MAX_RESOURCES
  int file_descriptor;
  int nbread;

  file_descriptor = open(filename, O_RDONLY);
  if (file_descriptor == -1) {
   ERROR_DEBUG("Cannot open batch I/O redirection file", -1);
  }

  for (int i = 0; i <= NBDIGITS_MAX_RESOURCES; i++)
    small_chaine[i] = '\0';
  if ((nbread = readn(file_descriptor, small_chaine, NBDIGITS_MAX_RESOURCES))
      == 0) {
   ERROR_DEBUG("Error during submission or with I/O file."
          " Cannot read the batch ID", 0);
  }

  /* Just in case */
  if (small_chaine[nbread - 1] == '\n') {
    small_chaine[nbread - 1] = '\0';
  }

  if (close(file_descriptor) != 0) {
    WARNING("Couln't close batch script file");
  }

  return atoi(small_chaine);
} // readNumberInFile

int
BatchSystem::launchCommandAndGetInt(const char *submitCommand,
                                    const char *pattern) {
  char *filename, *chaine;
  int nbread;

  filename = createUniqueTemporaryTmpFile(pattern);

  chaine = (char *) malloc(sizeof(char) * (strlen(submitCommand)
                                           + strlen(filename)
                                           + 4));
  sprintf(chaine, "%s > %s", submitCommand, filename);
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_MAIN_STEPS, "Submit avec la ligne :" << endl << chaine
                                                        << endl << endl);
#endif

  if (system(chaine) == -1) {
   ERROR_DEBUG("Cannot submit script", 0);
  }

  nbread = readNumberInFile(filename);

#if REMOVE_BATCH_TEMPORARY_FILE
  unlink(filename);
#endif

  /* Free memory */
  free(chaine);
  free(filename);

  return nbread;
} // launchCommandAndGetInt

char *
BatchSystem::launchCommandAndGetResultFilename(const char *submitCommand,
                                               const char *pattern) {
  char *filename, *chaine;

  filename = createUniqueTemporaryTmpFile(pattern);

  chaine = (char *) malloc(sizeof(char) * (strlen(submitCommand)
                                           + strlen(filename)
                                           + 4));
  sprintf(chaine, "%s > %s", submitCommand, filename);
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_MAIN_STEPS, "Submit avec la ligne :" << endl
                                                        << chaine << endl <<
             endl);
#endif

  if (system(chaine) == -1) {
   ERROR_DEBUG("Cannot submit script", 0);
  }

  /* Free memory */
  free(chaine);

  return filename;
} // launchCommandAndGetResultFilename
