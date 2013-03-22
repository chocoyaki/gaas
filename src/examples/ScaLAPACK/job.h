/**
 * @file job.h
 *
 * @brief   Tools for job management on SCALAPACK servers (source code)
 *
 * @author  Bert VAN HEUKELOM (Bert.Van-Heukelom@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */



#ifndef _JOB_H_
#define _JOB_H_

typedef struct {
  int *procs;
  int procCount;
  int row;
  int col;
  int blocksize;
  int colCount;
  int rowCount;
  int contextAll;
  int context1x1;
  int contextWorkers;
  int sedId;

  int id;

  MPI_Comm *commAll;
  MPI_Comm *commWorkers;
  MPI_Comm *comm1x1;
  MPI_Comm *intercomm;
} job_t;


job_t *
job_construct(int nbProcs);

void
job_destruct(job_t *j);

#Endif  // _JOB_H_
