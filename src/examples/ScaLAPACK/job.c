/**
 * @file job.c
 *
 * @brief   Tools for job management on SCALAPACK servers (source code)
 *
 * @author  Bert VAN HEUKELOM  (Bert.Van-Heukelom@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include "job.h"


job *
job_construct(int nbProcs) {
  job *j;
  j = malloc(sizeof(job));

  j->procs = calloc(nbProcs, sizeof(int));
  j->commAll = malloc(sizeof(MPI_Comm));
  j->comm1x1 = malloc(sizeof(MPI_Comm));
  j->commWorkers = malloc(sizeof(MPI_Comm));
  j->intercomm = malloc(sizeof(MPI_Comm));
  j->procCount = -1;
  j->row = -1;
  j->col = -1;
  j->colCount = -1;
  j->rowCount = -1;
  j->sedId = -1;
  j->contextAll = -1;
  j->context1x1 = -1;
  j->contextWorkers = -1;
  return j;
} /* job_construct */


void
job_destruct(job *j) {
  free(j->commAll);
  free(j->commWorkers);
  free(j->comm1x1);
  free(j->intercomm);
  free(j->procs);
  free(j);
  j = 0;
}
