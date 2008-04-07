/****************************************************************************/
/* Batch System Management with performance prediction: Loadleveler         */
/*                                                                          */
/* Author(s):                                                               */
/*    - Yves Caniou (yves.caniou@ens-lyon.fr)                               */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2008/04/07 13:11:44  ycaniou
 * Correct "deprecated conversion from string constant to 'char*'" warnings
 * First attempt to code functions to dynamicaly get batch information
 * 	(e.g.,  getNbMaxResources(), etc.)
 *
 * Revision 1.1  2008/01/01 19:43:49  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 *
 ****************************************************************************/

#include "debug.hh"
#include "Loadleveler_BatchSystem.hh"

const char * Loadleveler_BatchSystem::statusNames[] = {
  "Error", // not OK: sucks, because error or terminated
  "Error", // not OK  it seems that LL reports no info with llq
  "Error", // not OK  so, how to decide if all is ok? Parse the error file?
  "running",
  "waiting",
  "pending",
  "preempted"
} ;

Loadleveler_BatchSystem::Loadleveler_BatchSystem(int ID, const char * batchname)
{
  if( pathToNFS == NULL ) {
    ERROR_EXIT("LL needs a path to a NFS directory to store its script\n") ;
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS,"Nom NFS: " << getNFSPath() << "\n") ;
#endif

  batch_ID = ID ;
  batchName = batchname ;
  
  shell = "#@ shell=" ;
  prefixe = "#@ environment = COPY_ALL;" ; // "#!/bin/sh\n\n"

  /* first line is mandatory to end LL batch directives, 
     second is to get a file containing the nodes ID. Hope it is unique
  */
  postfixe = "#@ queue\n";
  /* "export MP_SAVEHOSTFILE=$LOADL_STEP_INITDIR/machine_$LOADL_STEP_ID" ; */

  /* #@ network.MPI  indicates which switch adapter is used, whether
  or not the adapter is shared, and which communications library is
  used. It is standard to use the switch adapter (css0), in a
  dedicated mode (not_shared) and with the User Space library (US).
  That kinf of info must rely in the SeD.cfg, because not all LL
  environment possess a switch!  
  #@ network.MPI = css0,not_shared,US\n
  */
  /* FIXME: fix this part!

  unsigned int * LL_switched = (unsigned int *) 
  Parsers::Results::getParamValue(Parsers::Results::SWITCH) ;
  if( *LL_switched == 1 )
  nodesNumber = "#@ network.MPI = css0,not_shared,US\n#@ job_type=parallel\n#@ node=" ;
  else
  */
  nodesNumber = "#@ job_type = parallel\n#@ node =" ;
  serial = "#@ job_type = serial" ;
  walltime = "#@ wall_clock_limit =" ;
  submittingQueue = "#@ Class = " ;
  minimumMemoryUsed = BatchSystem::emptyString ;
  
  /* TODO: When we use some ID for DIET client, change there! */
  mail = "#@ notification = never\n#@ notify_user =" ;
  account = "#@ account_no =" ;
  setSTDOUT = "#@ output =" ;
  setSTDIN = "#@ input =" ; /* Not used for the moment */
  setSTDERR = "#@ error =" ;

  submitCommand = "llsubmit " ;
  killCommand = "llcancel " ;
  wait4Command = "llq -j " ;
  waitFilter = "grep step | cut --delimiter=1 --field=3 | cut --delimiter=\",\" --field=1" ;
  exitCode = "0" ;
  
  jid_extract_patterns = "cut --delimiter=\\\" -f 2 | cut --delimiter=. -f 2" ;

  /* Information for META_VARIABLES */
  batchJobID = "$LOADL_STEP_ID" ;
  nodeFileName = "$MP_SAVEHOSTFILE" ;
  //  nodeIdentities = "cat $MP_SAVEHOSTFILE" ;  
}

Loadleveler_BatchSystem::~Loadleveler_BatchSystem()
{
}

/*********************** Job Managing ******************************/

BatchSystem::batchJobState
Loadleveler_BatchSystem::askBatchJobStatus(int batchJobID)
{
  char * filename ;
  int file_descriptor ;
  char * chaine ;
  int i=0 ;
  int nbread ;
  batchJobState status ;
  
  /* If job has completed, not ask batch system */
  status = getRecordedBatchJobStatus( batchJobID ) ;
  if( (status == TERMINATED) || (status == CANCELED) || (status == ERROR) )
    return status ;
  /* create a temporary file to get results and batch job ID */
  filename = (char*)malloc(sizeof(char)*strlen(pathToTmp) + 30 ) ;
  sprintf(filename,"%sDIET_batch_finish.XXXXXX", pathToTmp) ;
  file_descriptor = mkstemp( filename ) ;
  if( file_descriptor == -1 ) {
    ERROR("Cannot create batch I/O redirection file", NB_STATUS) ;
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS,"Fichier_finish: " << filename << "\n") ;
#endif

  /*** Ask batch system the job status ***/      
  chaine = (char*)malloc(sizeof(char)*(strlen(wait4Command)
				       + NBDIGITS_MAX_BATCH_ID
				       + strlen(waitFilter)
				       + strlen(filename)
				       + 7 + 1) ) ;
  /* See EOF to get an example of what we parse */
  sprintf(chaine,"%s %d | %s > %s",
	  wait4Command,batchJobID,waitFilter,filename) ;
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS,"Execute:\n " << chaine << "\n") ;
#endif
  if( system(chaine) != 0 ) {
    ERROR("Cannot submit script", NB_STATUS) ;
  }

  /* Get job status */  
  for( int i = 0 ; i<=NBDIGITS_MAX_BATCH_ID ; i++ )
    chaine[i] = '\0' ;

  nbread=readn(file_descriptor,chaine,NBDIGITS_MAX_JOB_STATUS) ;
  /* When job is finished, no information is reported by Loadleveler
     -> nbread=0
     TODO: if error?
  */

  if( nbread == 0 )
    /* we consider that like OK */
    i=TERMINATED ;
  else {
    /* Adjust what have been read */
    if( chaine[nbread-1] == '\n' )
      chaine[nbread-1] = '\0' ;
    /* Compare to chaine+1 because of a space as a first char */
    while( (i<NB_STATUS) && 
	   (strcmp(chaine+1,Loadleveler_BatchSystem::statusNames[i])!=0) ) {
      i++ ;
    }
  }
    
  if( i==NB_STATUS ) {
    ERROR("Cannot get batch job " << batchJobID << " status: " << chaine
	  << "\n",NB_STATUS) ;
  }
  /* Remove temporary file by closing it */
#if REMOVE_BATCH_TEMPORARY_FILE
  unlink( filename ) ;
#endif
  if( close(file_descriptor) != 0 ) {
    WARNING("Couln't remove I/O redirection file") ;
  }
  updateBatchJobStatus(batchJobID,(batchJobState)i) ;
  free(chaine) ;
  free(filename) ;
  return (batchJobState)i ;
}

int
Loadleveler_BatchSystem::isBatchJobCompleted(int batchJobID)
{
  int status = getRecordedBatchJobStatus(batchJobID) ;
  
  if( (status == TERMINATED) || (status == CANCELED) || (status == ERROR) )
    return 1 ;
  status = askBatchJobStatus(batchJobID) ;
  if( (status == TERMINATED) || (status == CANCELED) || (status == ERROR) )
    return 1 ;
  else if( status == NB_STATUS )
    return -1 ;
  return 0 ;
}

/*************************** Performance Prediction *************************/
int
Loadleveler_BatchSystem::getNbMaxResources()
{
  {
    ERROR("This funtion is not implemented yet", 1) ;
  }
}

int
Loadleveler_BatchSystem::getNbIdleResources()
{
  {
    ERROR("This funtion is not implemented yet", 1) ;
  }
}



/************************* Examples of commands ****************************/
/*
bash-2.05b$ llsubmit ll_script.essai
llsubmit: The job "meso-d.16247" has been submitted.

bash-2.05b$ llq -j meso-d.16247
llq: There is currently no job status to report.

bash-2.05b$ llq -j meso-d.16250
Id                       Owner      Submitted   ST PRI Class        Running on
------------------------ ---------- ----------- -- --- ------------ -----------
meso-d.16250.0           diet        7/24 10:19 R  50  parall       meso-2

1 job step(s) in query, 0 waiting, 0 pending, 1 running, 0 held, 0 preempted

bash-2.05b$ llq -j meso-d.16250 -l
=============== Job Step meso-d.16250.0 ===============
        Job Step Id: meso-d.16250.0
           Job Name: meso-d.16250
          Step Name: 0
  Structure Version: 10
              Owner: diet
         Queue Date: Tue Jul 24 10:19:37 CDT 2007
             Status: Running
     Reservation ID:
  Requested Res. ID:
 Scheduling Cluster:
 Submitting Cluster:
    Sending Cluster:
  Requested Cluster:
     Schedd History:
   Outbound Schedds:
    Submitting User:
   Execution Factor: 1
      Dispatch Time: Tue Jul 24 10:19:38 CDT 2007
    Completion Date:
    Completion Code:
        Favored Job: No
      User Priority: 50
       user_sysprio: 0
      class_sysprio: 60
      group_sysprio: 0
    System Priority: -2856015
          q_sysprio: -2856015
 Previous q_sysprio: 0
      Notifications: Complete
 Virtual Image Size: 1 kb
         Large Page: N
     Checkpointable: no
    Ckpt Start Time:
Good Ckpt Time/Date:
   Ckpt Elapse Time: 0 seconds
Fail Ckpt Time/Date:
    Ckpt Accum Time: 0 seconds
    Checkpoint File:
   Ckpt Execute Dir:
  Restart From Ckpt: no
 Restart Same Nodes: no
            Restart: yes
        Preemptable: yes
 Preempt Wait Count: 0
     Hold Job Until:
               RSet: RSET_NONE
Mcm Affinity Options:
                Env:
                 In: /dev/null
                Out: output.txt
                Err: error.txt
Initial Working Dir: /users/cri/diet/YC
         Dependency:
          Resources:
          Step Type: General Parallel
         Node Usage: shared
    Submitting Host: meso-d
        Schedd Host: meso-d
      Job Queue Key:
        Notify User: diet@meso-d
              Shell: /bin/ksh
  LoadLeveler Group: cri
              Class: parall
    Ckpt Hard Limit: undefined
    Ckpt Soft Limit: undefined
     Cpu Hard Limit: undefined
     Cpu Soft Limit: undefined
    Data Hard Limit: undefined
    Data Soft Limit: undefined
    Core Hard Limit: undefined
    Core Soft Limit: undefined
    File Hard Limit: undefined
    File Soft Limit: undefined
   Stack Hard Limit: undefined
   Stack Soft Limit: undefined
     Rss Hard Limit: 7.764 gb (8336179200 bytes)
     Rss Soft Limit: 7.764 gb (8336179200 bytes)
Step Cpu Hard Limit: undefined
Step Cpu Soft Limit: undefined
Wall Clk Hard Limit: 00:01:00 (60 seconds)
Wall Clk Soft Limit: 00:01:00 (60 seconds)
            Comment:
            Account:
         Unix Group: cri
   NQS Submit Queue:
   NQS Query Queues:
Negotiator Messages:
      Bulk Transfer: No
Step Adapter Memory: 0 bytes
Adapter Requirement:
          Step Cpus: 0
Step Virtual Memory: 0.000 mb
   Step Real Memory: 0.000 mb
--------------------------------------------------------------------------------
Node
----

   Name            :
   Requirements    : (Arch == "R6000") && (OpSys == "AIX53")
   Preferences     :
   Node minimum    : 2
   Node maximum    : 2
   Node actual     : 2
   Allocated Hosts : meso-2::(NOT SPECIFIED,IP,-1,Shared,0M)
                   + meso-d::(NOT SPECIFIED,IP,-1,Shared,0M)

   Master Task
   -----------

      Executable   : /users/cri/diet/YC/ll_script.essai
      Exec Args    :
      Num Task Inst: 1
      Task Instance: meso-2:-1,

   Task
   ----

      Num Task Inst: 2
      Task Instance: meso-2:0:(NOT SPECIFIED,IP,-1,Shared,0M),
      Task Instance: meso-d:1:(NOT SPECIFIED,IP,-1,Shared,0M),

1 job step(s) in query, 0 waiting, 0 pending, 1 running, 0 held, 0 preempted
*/
