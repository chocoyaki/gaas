/****************************************************************************/
/* Elagi part used to glue SeD and Batch schedulers (from files elbase.[ch] */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Jim Hayes                                                           */
/*    - Modified for Diet, Yves Caniou                                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

#ifndef BATCH_H
#define BATCH_H

#include "DIET_server.h" /* for definition of DIET_MPI things */

/* Supported file copy infrastructures. */
typedef enum {
  ELBASE_CP, ELBASE_FTP, ELBASE_GASS, ELBASE_SCP, ELBASE_SFTP, ELBASE_SRB,
  ELBASE_STORAGE_SERVICE_COUNT
} ELBASE_StorageServiceTypes;

/* Supported compute infrastructures. */
typedef enum {
  ELBASE_FORK, ELBASE_GRAM, ELBASE_NETSOLVE, ELBASE_RASH, ELBASE_SSH,
  ELBASE_COMPUTE_SERVICE_COUNT
} ELBASE_ComputeServiceTypes;

/* Supported batch scheduling infrastructures. */
typedef enum {
  ELBASE_SHELL, ELBASE_CONDOR, ELBASE_DQS, ELBASE_LOADLEVELER, ELBASE_LSF,
  ELBASE_PBS, ELBASE_SGE,
  /* YC */
  ELBASE_OAR,
  /* FYC */
  ELBASE_SCHEDULER_SERVICE_COUNT
} ELBASE_SchedulerServiceTypes;

/* A spawned process. */
typedef void *ELBASE_Process;

/* Recognized elements of ELBASE_Submit's #schedulerAttributes# parameter. */
/* Account to charge */
#define ELBASE_ACCOUNT   "project"
/* email to notify about status */
#define ELBASE_EMAIL     "email"
/* Minimum required megabytes */
#define ELBASE_MEMORY    "min_memory"
/* Program to copy files between batch nodes */
#define ELBASE_NODECOPY  "node_copy"
/* Type of node */
#define ELBASE_NODES     "host_count"
/* Program to spawn programs between batch nodes */
#define ELBASE_NODESPAWN "node_spawn"
/* Type of node */
#define ELBASE_NODETYPE  "node_type"
/* When to send email--always|complete|error|never */
#define ELBASE_NOTIFY    "notify"
/* # of copies to run */
#define ELBASE_PROCESSES "count"
/* Name of batch queue */
#define ELBASE_QUEUE     "queue"
/* Job stderr path */
#define ELBASE_STDERR    "stderr"
/* Job stdin path */
#define ELBASE_STDIN     "stdin"
/* Job stdout path */
#define ELBASE_STDOUT    "stdout"
/* Maximum wall clock minutes */
#define ELBASE_TIME      "max_wall_time"
/* Batch file that contains the identities of reserved nodes */
#define DIET_BATCH_NODESFILE "$DIET_BATCH_NODESFILE"

/*
 * Equivalent to calling ELBASE_SpawnScript with a script constructed by
 * calling ELBASE_ScriptForCopy with all the same parameters.
 */
int
ELBASE_CopyFile(ELBASE_StorageServiceTypes sourceService,
                const char *sourceServer,
                const char *sourcePath,
                ELBASE_StorageServiceTypes destService,
                const char *destServer,
                const char *destPath,
                ELBASE_Process *process);


/*
 * Equivalent to calling ELBASE_SpawnScript with a script constructed by
 * calling ELBASE_ScriptForDelete with all the same parameters.
 */
int
ELBASE_DeleteFile(ELBASE_StorageServiceTypes service,
                  const char *server,
                  const char *path,
                  ELBASE_Process *process);


/*
 * Kills the process #process#, which must have been returned from a successful
 * call to a function in this package.  Returns 1 if successful, else 0.
 */
int
ELBASE_Kill(ELBASE_Process process);


/*
 * Returns 1 if #process#, which must have been returned from a successful call
 * to a function in this package, has terminated, else 0.  The function blocks
 * until the process terminates if #waitForTermination# is non-zero.  The
 * process' exit status is copied into #exitStatus# if it is non-NULL.  NOTE:
 * A successful call to this procedure frees any resources consumed by
 * #process# and invalidates its value, so don't try to use it afterwards.
 */
int
ELBASE_Poll(ELBASE_Process process,
            int waitForTermination,
            int *exitStatus);


/*
 * Returns in an allocated string a /bin/sh script to copy the file
 * #sourcePath# accessed via the #sourceService# server #sourceServer# to the
 * file #destPath# accessed via the #destService# server #destServer#.
 */
char *
ELBASE_ScriptForCopy(ELBASE_StorageServiceTypes sourceService,
                     const char *sourceServer,
                     const char *sourcePath,
                     ELBASE_StorageServiceTypes destService,
                     const char *destServer,
                     const char *destPath);


/*
 * Returns in an allocated string a /bin/sh script that asks the #service#
 * server #server# to delete the file #path#.
 */
char * 
ELBASE_ScriptForDelete(ELBASE_StorageServiceTypes service,
                       const char *server,
                       const char *path);


/*
 * Returns in an allocated string a /bin/sh script that asks the #service#
 * server #server# to execute or submit #executablePath# using #scheduler#.
 * #schedulerAttributes# is a NULL-terminated array of name=value elements
 * indicating parameters for batch submissions.  Execution is performed in
 * #workingDirectory#, #arguments# are passed to the execution, and #stdinPath#,
 * #stdoutPath#, and #stderrPath# redirect the standard files to file paths on
 * #server#.  #environment# is a NULL-terminated array of name=value elements
 * indicating local environment variable settings.  Any or all the last
 * seven parameters may be NULL.
 */
char *
ELBASE_ScriptForSubmit(ELBASE_ComputeServiceTypes service,
                       const char *server,
                       ELBASE_SchedulerServiceTypes scheduler,
                       const char **schedulerAttributes,
                       const char *executablePath,
                       const char *workingDirectory,
                       const char **arguments,
                       const char *stdinPath,
                       const char *stdoutPath,
                       const char *stderrPath,
                       const char **environment);


/*
 * Instructs the module to use #command# to launch #service# jobs, overriding
 * the default that relies on the user's PATH setting. Returns 1 on success,
 * else 0.
 */
int
ELBASE_SetComputeCommand(ELBASE_ComputeServiceTypes service, 
                         const char *command);


/*
 * Launches a process to run the /bin/sh script #script#.  Returns 1 if
 * successful, else 0.  If successful and #process# is non-NULL, sets it to a
 * process id that can be passed to ELBASE_Kill and ELBASE_Poll.
 */
int
ELBASE_SpawnScript(const char *script,
                   ELBASE_Process *process);


/*
 * Equivalent to calling ELBASE_SpawnScript with a script constructed by
 * calling ELBASE_ScriptForSubmit with all the same parameters.
 */
int
ELBASE_Submit(ELBASE_ComputeServiceTypes service,
              const char *server,
              ELBASE_SchedulerServiceTypes scheduler,
              const char **schedulerAttributes,
              const char *executablePath,
              const char *workingDirectory,
              const char **arguments,
              const char *stdinPath,
              const char *stdoutPath,
              const char *stderrPath,
              const char **environment,
              ELBASE_Process *process);
/* Convenience macro for spawning a non-batch process. */
#define ELBASE_Spawn(type,server,exec,wd,args,in,out,err,env,proc) \
  ELBASE_Submit(type,server,ELBASE_SHELL,0,exec,wd,args,in,out,err,env,proc)
/* Convenience macro for forking a local process. */
#define ELBASE_Fork(exec,wd,args,in,out,err,env,proc) \
  ELBASE_Spawn(ELBASE_FORK, "", exec, wd, args, in, out, err, env, proc)

/* YC */
/* Make the correspondance with outside */
ELBASE_SchedulerServiceTypes 
ELBASE_GiveBatchID(const char *batchName) ;

/* Return 1 and give batchID if batchName is defined */
int
ELBASE_ExistBatchScheduler(const char *batchName, 
			   ELBASE_SchedulerServiceTypes *batchID) ;
/* Return a copy of the name of the batch scheduler referenced by ID*/
const char*
ELBASE_GiveBatchName(ELBASE_SchedulerServiceTypes ID) ;
/* FYC */

#endif
