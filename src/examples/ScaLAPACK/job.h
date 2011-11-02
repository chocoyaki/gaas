/**
* @file job.h
* 
* @brief   Tools for job management on SCALAPACK servers (source code)   
* 
* @author  - Bert VAN HEUKELOM (Bert.Van-Heukelom@ens-lyon.fr) 
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.1.1.1  2003/04/10 13:21:39  pcombes
 * SCALAPCK is included in the autotools chain under "ScaLAPACK".
 *
 * Revision 1.1  2002/12/03 19:05:12  pcombes
 * Clean CVS logs in file.
 * Separate BLAS and SCALAPACK examples.
 ****************************************************************************/


#ifndef _JOB_H_
#define _JOB_H_

typedef struct {
  int* procs;
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

  MPI_Comm* commAll;
  MPI_Comm* commWorkers;
  MPI_Comm* comm1x1;
  MPI_Comm* intercomm;
} job_t;


job_t*
job_construct(int nbProcs);

void
job_destruct(job_t *j);

#Endif  // _JOB_H_
