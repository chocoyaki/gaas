/****************************************************************************/
/* Elagi part used to glue SeD and Batch schedulers (from files elbase.[ch] */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Jim Hayes                                                           */
/*    - Modified for Diet, Yves Caniou                                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.15  2008/01/01 19:43:49  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 * Revision 1.14  2007/07/04 15:55:02  ycaniou
 * Cosmetic change
 *
 * Revision 1.13  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.12  2007/01/24 20:37:28  ycaniou
 * Commentaries Doxygen compliant
 *
 * Revision 1.11  2007/01/08 13:43:23  ycaniou
 * Changelog now incorporates batch stuff for diet 2.3
 * Missing ')' in .tex
 * Minor change in batch.c
 *
 * Revision 1.10  2006/11/28 20:40:31  ycaniou
 * Only headers
 *
 * Revision 1.9  2006/11/27 08:13:59  ycaniou
 * Added missing fields Id and Log in headers
 ****************************************************************************/

/* TODO: look at functions ELBASE_Kill and ELBASE_Poll to kill a process
** on a sequential machine, or to know its status */

#ifdef __cplusplus
extern "C" {
#endif


#include <oshseed.h>
#include <stdio.h>     /* file functions */
#include <ctype.h>     /* isspace */
#include <signal.h>    /* kill */
#include <stdlib.h>    /* free getenv strtol */
#include <string.h>    /* string functions */
#define ASFILE_SHORT_NAMES
#include <fileseed.h>
#include <ipseed.h>
#define ASPROP_SHORT_NAMES
#include <propertyseed.h>
#define ASSTR_SHORT_NAMES
#include <strseed.h>
#include "batch.h"
#include <pthread.h>

/* The following include is generated from batch_configure.h.in
   (see CMakelist.txt */
#include "batch_configure.h"

#define TEMP_PREFIX "/tmp/"

#ifdef DEBUG_YC
#include <sys/time.h>
#endif

typedef enum {
  NO_ESCAPE, GRAM_ESCAPE, PERL_ESCAPE, RASH_ESCAPE
} EscapeTypes;
static const char *computeCommands[ELBASE_COMPUTE_SERVICE_COUNT] = {
  "perl", "globusrun", "grid-rpc", "rash.pl", "ssh"
};
static char *computeCommandsSet[ELBASE_COMPUTE_SERVICE_COUNT] = {
  NULL, NULL, NULL, NULL, NULL
};
static const char *computeCommandsDefaults[ELBASE_COMPUTE_SERVICE_COUNT] = {
  "perl", "globusrun", "grid-rpc", "rash.pl", "ssh"
};
static const char *storageCommands[ELBASE_STORAGE_SERVICE_COUNT] = {
  "cp", "ftp", "globus-url-copy", "scp", "sftp", "Scp"
};

/** In order to return the ID of the batch, given the name */
static const char 
*availableBatchScheduler[ELBASE_SCHEDULER_SERVICE_COUNT] = 
  {"cmdshell","condor","dqs","loadleveler","lsf","pbs","sge",
   "oar", "shellscript" } ;

  /**
   * If @param s is non-null, appends 
   *   @param prefix to 
   *   @param toWhat followed 
   *   by \c s, then
   *   @param suffix . \c prefix and \c suffix may each be NULL. 
   * Appropriate characters
   * within \c s are escaped as indicated by @param escape .
   */
static void
AppendIfThere(char ** toWhat,
              const char * prefix,
              const char * s,
              const char * suffix,
              EscapeTypes escape) {
  char *c;
  char *escaped;
  unsigned offset;
  if(s == NULL)
    return;
  *toWhat = StrAppend(*toWhat, prefix, NULL);
  escaped = strdup(s);
  if(escape == GRAM_ESCAPE)
    StrReplaceAll(&escaped, "\"", "\"\"");
  else if(escape == PERL_ESCAPE) {
    StrReplaceAll(&escaped, "\\", "\\\\");
    StrReplaceAll(&escaped, "\"", "\\\"");
    StrReplaceAll(&escaped, "@", "\\@");
    for(c = strchr(escaped, '$'); c != NULL; c = strchr(c + 1, '$')) {
      if(*(c + 1) == '$' || *(c + 1) == '{' || *(c + 1) == '?')
        c++; /* Intentional variable reference */
      else if(c > escaped && *(c - 1) == '\\')
        ; /* empty */ /* Already escaped */
      else {
        offset = c - escaped;
        StrReplace(&escaped, offset, 0, "\\");
        c = escaped + offset + 2;
      }
    }
  }
  else if(escape == RASH_ESCAPE)
    StrReplaceAll(&escaped, "\"", "\\\"");
  *toWhat = StrAppend(*toWhat, escaped, suffix, NULL);
  free(escaped);
}


  /**
   * If @param s is non-null, appends @param prefix to @param toWhat 
   * followed by the elements of \c s separated by @param separator
   * then \c suffix.  
   * \c prefix, \c separator and @param suffix may each be NULL. If 
   * @param equalsSub is non-NULL, elements of \c s are
   * assumed to have the form name=value, and each equal sign is replaced by
   * \c equalSub . 
   * Appropriate characters within \c s are escaped as specified
   * by @param escape .
   */
static void
AppendListIfThere(char **toWhat,
                  const char *prefix,
                  const char **s,
                  const char *separator,
                  const char *suffix,
                  const char *equalsSub,
                  EscapeTypes escape) {
  char *c;
  char *noEqual;
  char *toAppend;
  unsigned i;
  if(s == NULL || s[0] == NULL)
    return;
  toAppend = NULL;
  for(i = 0; s[i] != NULL; i++) {
    if(equalsSub == NULL || strchr(s[i], '=') == NULL)
      AppendIfThere(&toAppend, prefix, s[i], NULL, escape);
    else {
      noEqual = strdup(s[i]);
      *(c = strchr(noEqual, '=')) = '\0';
      AppendIfThere(&toAppend, prefix, noEqual, NULL, NO_ESCAPE);
      AppendIfThere(&toAppend, equalsSub, c + 1, NULL, escape);
      free(noEqual);
    }
    prefix = separator;
  }
  *toWhat = StrAppend(*toWhat, toAppend, suffix, NULL);
  free(toAppend);
}


  /**
   * Returns in an allocated string a /bin/sh invocation of @param sftp 
   * that asks @param server to perform @param command on @param path1 and, 
   * if it is not NULL, @param path2 .
   */
static char *
FtpScript(int sftp,
          const char *server,
          const char *command,
          const char *path1,
          const char *path2) {

  char *host;
  const char *login;
  char *password;
  char *port;
  char *result;
  char *serverCopy;
  char *userName;

  if((userName = getenv("USER")) == NULL &&
     (userName = getenv("USERNAME")) == NULL)
    userName = "anonymous";
  /*
   * TODO: How do we get the (s)ftp call to return a non-zero exit code if the
   * command fails?
   */
  /* Parse the [login[:password]@]host[:port] server value. */
  serverCopy = strdup(server);
  if((host = strchr(serverCopy, '@')) == NULL) {
    login = sftp ? userName : "anonymous";
    host = serverCopy;
  }
  else {
    login = serverCopy;
    *(host++) = '\0';
  }
  if((password = strchr(login, ':')) == NULL) {
    char *localhost = ASIP_MyMachineName();
    password = StrAppend(NULL, userName, "@", localhost, NULL);
    free(localhost);
  }
  else {
    *(password++) = '\0';
    password = strdup(password);
  }
  if((port = strchr(host, ':')) == NULL)
    port = "21";
  else
    *(port++) = '\0';

  if(sftp)
    result = StrAppend
      (NULL, storageCommands[ELBASE_SFTP], " ", login, "@", host,
       " > /dev/null << ENDFTP\n", NULL);
  else
    result = StrAppend
      (NULL, storageCommands[ELBASE_FTP], " -n > /dev/null << ENDFTP\n", 
             "open ", host, " ", port, "\n",
             "user ", login, " ", password, "\n", NULL);
  result = StrAppend(result, command, " ", path1, NULL);
  if(path2 != NULL)
    result = StrAppend(result, " ", path2, NULL);
  result = StrAppend(result, "\n", "quit\nENDFTP", NULL);

  free(password);
  free(serverCopy);
  return result;
}


  /** Returns true iff temp files created by this package should be deleted. */
static int
RemoveTempFiles(void) {
  return getenv("ELAGI_NO_RM") == NULL && getenv("ELBASE_NO_RM") == NULL;
}


  /** Returns in an allocated string an invocation of the SSH server 
      @param server . 
   */
static char *
SshInvocation(const char *server) {
  char *c;
  unsigned i;
  char *result = NULL;
  char **servers = StrSplit(server, ";");
  for(i = 0; servers[i] != NULL; i++) {
    result = StrAppend(result, "ssh ", NULL);
    if((c = strchr(servers[i], ':')) != NULL) {
      result = StrAppend(result, "-p ", c + 1, " ", NULL);
      *c = '\0';
    }
    result = StrAppend(result, servers[i], " ", NULL);
  }
  StrArrayFree(servers);
  return result;
}

                
int
ELBASE_CopyFile(ELBASE_StorageServiceTypes sourceService,
                const char *sourceServer,
                const char *sourcePath,
                ELBASE_StorageServiceTypes destService,
                const char *destServer,
                const char *destPath,
                ELBASE_Process *process) {
  int result;
  char *script = ELBASE_ScriptForCopy
    (sourceService,sourceServer,sourcePath,destService,destServer,destPath);
  if(script == NULL)
    return 0;
  result = ELBASE_SpawnScript(script, process);
  free(script);
  return result;
}


int
ELBASE_DeleteFile(ELBASE_StorageServiceTypes service,
                  const char *server,
                  const char *path,
                  ELBASE_Process *process) {
  int result;
  char *script = ELBASE_ScriptForDelete(service, server, path);
  if(script == NULL)
    return 0;
  result = ELBASE_SpawnScript(script, process);
  free(script);
  return result;
}

  /* YC */
  /* TODO: ELBASE_Process is defined by a void*, like ASEXEC_ProcessId
  ** then, how can we cast it as a ProcessInfo? */
  /* FYC */
int
ELBASE_Kill(ELBASE_Process process) {
  ProcessInfo p = (ProcessInfo)process;
#ifndef WIN32
  if(p->gramProcess)
    return kill((pid_t)p->pid, SIGINT) == 0;
  else
#endif
    return ProcessStop(p->pid);
}


int
ELBASE_Poll(ELBASE_Process process,
            int waitForTermination,
            int *exitStatus) {
  int result = ProcessStopped
    (((ProcessInfo)process)->pid, waitForTermination, exitStatus);
  if(result)
    free(process);
  return result;
}


char *
ELBASE_ScriptForCopy(ELBASE_StorageServiceTypes sourceService,
                    const char *sourceServer,
                    const char *sourcePath,
                    ELBASE_StorageServiceTypes destService,
                    const char *destServer,
                    const char *destPath) {

  ELBASE_StorageServiceTypes remoteService;
  char *result;
  char *tempPath;
  char *toScript;

  if(sourceServer == NULL || *sourceServer == '\0')
    sourceService = ELBASE_CP;
  if(destServer == NULL || *destServer == '\0')
    destService = ELBASE_CP;

  if(sourceService == ELBASE_CP || destService == ELBASE_CP ||
     (destService == sourceService &&
      (destService == ELBASE_GASS || destService == ELBASE_SCP))) {
    /* Local->remote, remote->local or supported remote->remote copy. */
    remoteService = sourceService != ELBASE_CP ? sourceService : destService;
    if(remoteService == ELBASE_CP)
      result = StrAppend
        (NULL,storageCommands[ELBASE_CP]," ",sourcePath," ",destPath,NULL);
    else if(remoteService == ELBASE_FTP || remoteService == ELBASE_SFTP)
      result = FtpScript
        (remoteService == ELBASE_SFTP,
         sourceService != ELBASE_CP ? sourceServer : destServer,
         sourceService != ELBASE_CP ? "get" : "put", sourcePath, destPath);
    else if(remoteService == ELBASE_GASS) {
      char cwd[256];
      /*
       * Unlike other services, globus-url-copy doesn't support relative paths.
       * We provide partial support by prefixing local paths with the cwd.
       */
      getcwd(cwd, sizeof(cwd));
      strcat(cwd, "/");
      result = StrAppend(NULL, storageCommands[ELBASE_GASS], " ", NULL);
      if(sourceService == ELBASE_CP)
        result = StrAppend
          (result, "file://", *sourcePath == '/' ? "" : cwd, sourcePath, NULL);
      else
        result = StrAppend(result, sourceServer, sourcePath, NULL);
      if(destService == ELBASE_CP)
        result = StrAppend
          (result, " file://", *destPath == '/' ? "" : cwd, destPath, NULL);
      else
        result = StrAppend(result, " ", destServer, destPath, NULL);
    }
    else if(remoteService == ELBASE_SCP) {
      if((sourceService != ELBASE_SCP ||
          (strchr(sourceServer, ';') == NULL &&
           strchr(sourceServer, ':') == NULL)) &&
         (destService != ELBASE_SCP ||
          (strchr(destServer, ';') == NULL &&
           strchr(destServer, ':') == NULL))) {
        /* No hops or alternate ports; normal scp */
        result = StrAppend(NULL, storageCommands[ELBASE_SCP], " ", NULL);
        if(sourceService == ELBASE_SCP)
          result = StrAppend(result, sourceServer, ":", NULL);
        result = StrAppend(result, sourcePath, " ", NULL);
        if(destService == ELBASE_SCP)
          result = StrAppend(result, destServer, ":", NULL);
        result = StrAppend(result, destPath, NULL);
      }
      else {
        /*
         * To support hops and alternate ports, we use ssh instead of scp,
         * using perl to do the file i/o so env references will work.
         */
        result = sourceService==ELBASE_SCP ? SshInvocation(sourceServer) : NULL;
        result = StrAppend(result, "perl << 'ENDCOPY' | ", NULL);
        if(destService == ELBASE_SCP) {
          char *destInvocation = SshInvocation(destServer);
          result = StrAppend(result, destInvocation, NULL);
          free(destInvocation);
        }
        result = StrAppend(result, "perl\n",
          "$s = 'open(O, \">", destPath, "\") or ",
                "print(STDERR \"open ", destPath, " failed\\n\") && exit 1;",
                "syswrite(O, $_, length $_) while $_ = <DATA>;' . ",
                "\"\\n__END__\\n\";\n",
          "open(I,\"", sourcePath, "\") or ",
            "$s = 'print(STDERR \"open ' . \"", sourcePath, "\" . ",
                         "' failed\\n\") && exit 1';\n",
          "syswrite(STDOUT, $s, length $s);\n",
          "syswrite(STDOUT, $_, length $_) while $_ = <I>;\n",
          "ENDCOPY\n",
        NULL);
        StrReplaceAll(&result, "${", "$ENV{");
      }
    }
    else if(remoteService == ELBASE_SRB)
      result = StrAppend
        (NULL, "Sinit\n", sourceService == ELBASE_CP ? "Sput" : "Sget", " -f ",
         sourcePath, " ", destPath, "\n", "Sexit", NULL);
  }
  else {
    /* Unsupported remote->remote copy.  Go through a local temp file. */
    tempPath = UniqueFilePath("elagi_temp");
    result = ELBASE_ScriptForCopy
      (sourceService, sourceServer, sourcePath, ELBASE_CP, NULL, tempPath);
    toScript = ELBASE_ScriptForCopy
      (ELBASE_CP, NULL, tempPath, destService, destServer, destPath);
    result = StrAppend(result, "\n", toScript, "\nrm -f ", tempPath, NULL);
    free(tempPath);
    free(toScript);
  }
  return result;

}


char *
ELBASE_ScriptForDelete(ELBASE_StorageServiceTypes service,
                      const char *server,
                      const char *path) {

  char *result;

  if(server == NULL || *server == '\0')
    service = ELBASE_CP;
  if(service == ELBASE_CP || service == ELBASE_SCP) {
    const char *rmArgs[] = {"-f", NULL, NULL};
    rmArgs[1] = path;
    result = ELBASE_ScriptForSubmit
      (service == ELBASE_CP ? ELBASE_FORK : ELBASE_SSH, server, ELBASE_SHELL,
       NULL, "/bin/rm", NULL, rmArgs, NULL, NULL, NULL, NULL);
  }
  else if(service == ELBASE_FTP)
    result = FtpScript(0, server, "delete", path, NULL);
  else if(service == ELBASE_SFTP)
    result = FtpScript(1, server, "rm", path, NULL);
  else if(service == ELBASE_SRB)
    result = StrAppend(NULL, "Sinit\n", "Srm ", path, "\n", "Sexit", NULL);
  else
    return NULL; /* Service does not support file deletion. */
  return result;

}

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
                      const char **environment) {

  /* Scheduler attributes only available internally to avoid conflicts. */
  #define ELBASE_ARGS    "arguments"
  #define ELBASE_ENV     "environment"
  #define ELBASE_PROGRAM "executable"
  #define ELBASE_SH      "shell"
  #define ELBASE_OTHERS  "insert_unrecognized_attributes_here"
  #define ELBASE_WD      "directory"

  /* All known attributes, both external (batch.h) and internal (above). */
  const char *KNOWN_ATTRIBUTES[] = {
    ELBASE_ACCOUNT, ELBASE_ARGS, ELBASE_EMAIL, ELBASE_ENV, ELBASE_MEMORY,
    ELBASE_NODECOPY, ELBASE_NODES, ELBASE_NODESPAWN, ELBASE_NODETYPE,
    ELBASE_NOTIFY, ELBASE_PROCESSES, ELBASE_PROGRAM, ELBASE_QUEUE, ELBASE_SH,
    ELBASE_STDERR, ELBASE_STDIN, ELBASE_STDOUT, ELBASE_TIME, ELBASE_WD,
    NULL
  };

  /*
   * The arrays below associate formats with the recognized attributes for each
   * scheduling mechanism.  A "%s" marks the spot in the format where the
   * attribute value is placed.  The ELBASE_OTHERS format, used with
   * unrecognized, service-specific attributes, includes two "%s"s--the first
   * for the name, the second for the value.  (Every service supported to date
   * places the name before the value.)  When constructing a submission script,
   * the formats for included attributes are incorporated into the script in
   * the order that they are listed in the appropriate format array; formats
   * for unspecified attributes are ignored.  Text that must always be included
   * in submission scripts is associated with the pseudo-attribute "".
   */

  /* GRAM supports all but EMAIL NODETYPE NOTIFY SH */
  /*
   * NOTE: Annoyingly, globusrun does not propogate the exit code of the
   * executable to the user; it almost always returns 0.  To recover the exit
   * code when possible, we use /bin/sh to run the executable, then print the
   * process exit code so that we can capture it on the local side.  Ick!
   * This has the nice side affect of circumventing Globus' insistence on a
   * full path for the executable; /bin/sh searches the user's PATH.
   */
  static const char *GRAM_ATTR_FORMATS[] = {
    "",               "&",
    ELBASE_ACCOUNT,   "(project=\"%s\")",
    ELBASE_ENV,       "(environment=%s)",
    ELBASE_MEMORY,    "(min_memory=\"%s\")",
    ELBASE_NODES,     "(host_count=\"%s\")",
    ELBASE_PROCESSES, "(count=\"%s\")",
    ELBASE_QUEUE,     "(queue=\"%s\")",
    ELBASE_TIME,      "(max_wall_time=\"%s\")",
    ELBASE_OTHERS,    "(%s=\"%s\")",
    "",               "(executable=/bin/sh)(arguments=\"-c\" \"",
    ELBASE_WD,        "cd %s && ",
    ELBASE_PROGRAM,   "%s",
    ELBASE_ARGS,      " %s",
    ELBASE_STDIN,     " < %s",
    ELBASE_STDOUT,    " > %s",
    ELBASE_STDERR,    " 2> %s",
    "",  "; echo _EXITCODE $? \")",
    NULL, NULL
  };

  /* NETSOLVE only supports program and args */
  static const char *NETSOLVE_ATTR_FORMATS[] = {
    ELBASE_PROGRAM, "%s",
    ELBASE_ARGS,    " %s",
    NULL, NULL
  };

  /* RASH only supports standard shell attributes */
  static const char *RASH_ATTR_FORMATS[] = {
    ELBASE_ENV,     "%s",
    ELBASE_WD,      "cd %s;",
    ELBASE_STDERR,  "redirect 2 \"%s\";",
    ELBASE_STDIN,   "redirect 0 \"%s\";",
    ELBASE_STDOUT,  "redirect 1 \"%s\";",
    ELBASE_PROGRAM, "%s",
    ELBASE_ARGS,    " %s",
    NULL, NULL
  };

  /* SHELL only supports standard shell attributes */
  static const char *SHELL_ATTR_FORMATS[] = {
    ELBASE_ENV,    "%s\n",
    ELBASE_WD,     "chdir(\"%s\") or exit 2;\n",
    ELBASE_STDERR, "open(STDERR, \">%s\") or exit 3;\n",
    ELBASE_STDIN,  "open(STDIN, \"<%s\") or exit 4;\n",
    ELBASE_STDOUT, "open(STDOUT, \">%s\") or exit 5;\n",
    ELBASE_PROGRAM, "exec(\"%s\"",
    ELBASE_ARGS,    "%s",
    "",             ") or exit 6;",
    NULL, NULL
  };

  /*
   * Formats common to those batch schedulers where we have to spread the job
   * to allocated nodes ourselves.
   */

/* Le 12.04.06 */
/*     ELBASE_ENV,       "%s\n", \ */
/*     ELBASE_WD,        "cd %s\n", \ */
/*     /\* YC *\/ */
/*     ELBASE_IS_NFS,    "NFS=%s\n", \ */
/*     ELBASE_IS_MPI,    "MPI=%s\n", \ */
/*     "",               "case $NFS in\n", \ */
/*     "",               "  Nfs)\n", \ */
/*     "",               "    case $MPI in\n", \ */
/*     "",               "      None)\n", \ */
/*     /\* TODO: call the same executable on each machine the same */
/*     **       way Jim does in Elagi *\/ */
/*     "",               "      ;;\n", \ */
/*     "",               "      Mpich_1_2)\n", \ */
/*     ELBASE_BATCH_NBPROCESSES, "        mpirun.mpich.1.2 -np %s ", \ */
/*     ELBASE_BATCH_NODESFILE, "-machinefile %s ", \ */
/*     ELBASE_PROGRAM,         "%s", \ */
/*     ELBASE_ARGS,            " %s", \ */
/*     ELBASE_STDIN,           " < %s\n", \ */
/*     "",               "      ;;\n", \ */
/*     "",               "      *)\n", \ */
/*     "",               "        echo \"TODO\" ; exit 1\n",\  */
/*     /\* TODO: LAM, PVM, etc. *\/ */
/*     "",               "      ;;\n", \ */
/*     "",               "    esac\n", \ */
/*     "",               "  Rename)\n", \ */
/*     "",               "    case $MPI in\n", \ */
/*     "",               "      None)\n", \ */
/*     /\* Then copy script with a different name on the N reserved machines */
/*     **   and call it ELBASE_PROCESSES/N times, */
/*     **   with an arg so that it calls the service program *\/ */
/*     /\* FYC *\/ */
/*     "",               "        if test $# -eq 0; then\n", \ */
/*     "",               "          L=`" getHosts " | sort | uniq`\n", \ */
/*     "",               "          for H in $L; do\n", \ */
/*     ELBASE_NODECOPY,  "            %s $0 $H:" BATCH_PATH "-$H || exit 7\n", \ */
/*     "",               "          done\n", \ */
/*     ELBASE_PROCESSES, "          P=%s\n", \ */
/*     "",               "          while test $P -gt 0; do\n", \ */
/*     "",               "            for H in $L; do\n", \ */
/*     ELBASE_NODESPAWN, "              %s $H /bin/sh " BATCH_PATH "-$H child_process &\n", \ */
/*     "",               "              P=`expr $P - 1`\n", \ */
/*     "",               "              if test $P -eq 0; then break; fi\n", \ */
/*     "",               "            done\n", \ */
/*     "",               "          done\n", \ */
/*     "",               "          sleep 15\n", \ */
/*     "",               "          for H in $L; do\n", \ */
/*     ELBASE_NODESPAWN, "            %s $H /bin/rm -f " BATCH_PATH "-$H\n", \ */
/*     "",               "          done\n", \ */
/*     "",               "          wait\n", \ */
/*     "",               "          exit 0\n", \ */
/*     "",               "        fi\n", \ */
/*     ELBASE_PROGRAM,   "%s", \ */
/*     ELBASE_ARGS,      " %s", \ */
/*     ELBASE_STDIN,     " < %s", \ */
/*     /\* YC *\/ */
/*     "",               "      ;;\n", \ */
/*     "",               "      *)\n", \ */
/*     "",               "        echo \"Not possible\" ; exit 1\n",\  */
/*     "",               "      ;;\n", \ */
/*     "",               "    esac\n", \ */
/*     "",               "    ;;\n", \ */
/*       "",               "  Copy)\n", \ */
/*     "",               "    case $MPI in\n", \ */
/*     "",               "      None)\n", \ */
/*     "",               "        echo \"TODO\" ; exit 1\n",\  */
/*     /\* TODO: call code on the all N machines, ELBASE_PROCESSES times *\/ */
/*     "",               "      ;;\n", \ */
/*     "",               "      Mpich_1_2)\n", \ */
/*     "",               "        echo \"TODO\" ; exit 1\n",\  */
/*     /\* TODO: call mpirun on all N machines with np>n *\/ */
/*     ELBASE_BATCH_NBPROCESSES, "        mpirun.mpich.1.2 -np %s ", \ */
/*     ELBASE_BATCH_NODESFILE, "-machinefile %s ", \ */
/*     ELBASE_PROGRAM,         "%s", \ */
/*     ELBASE_ARGS,            " %s", \ */
/*     ELBASE_STDIN,           " < %s", \ */
/*     "",               "      ;;\n", \ */
/*     "",               "      *)\n", \ */
/*     "",               "        echo \"TODO\" ; exit 1\n", \  */
/*     /\* TODO: LAM, PVM, etc. *\/ */
/*     "",               "      ;;\n", \ */
/*     "",               "    esac\n", \ */
/*     "",               "  ;;\n", \ */
/*     "",               "esac\n" */


/*     "",               "  for H in $L; do\n", \ */
/*       ELBASE_NODECOPY,  "    %s $0 $H:" BATCH_PATH "-$H || exit 7\n", \ */
/*     "",               "  done\n", \ */
/*     ELBASE_PROCESSES, "  P=%s\n", \ */
/*     "",               "  while test $P -gt 0; do\n", \ */
/*     "",               "    for H in $L; do\n", \ */
/*       ELBASE_NODESPAWN, "      %s $H /bin/sh " BATCH_PATH "-$H child_process &\n", \ */
/*     "",               "      P=`expr $P - 1`\n", \ */
/*     "",               "      if test $P -eq 0; then break; fi\n", \ */
/*     "",               "    done\n", \ */
/*     "",               "  done\n", \ */
/*     "",               "  sleep 15\n", \ */
/*     "",               "  for H in $L; do\n", \ */
/*       ELBASE_NODESPAWN, "    %s $H /bin/rm -f " BATCH_PATH "-$H\n", \ */
/*     "",               "  done\n", \ */
/*     "",               "  wait\n", \ */
/*     "",               "  exit 0\n", \ */
/*     "",               "fi\n", \ */
/*     ELBASE_PROGRAM,   "%s", \ */
/*     ELBASE_ARGS,      " %s", \ */
/*     ELBASE_STDIN,     " < %s" */


#define COMMON_ATTR_FORMATS(getHosts) \
  "",    "DIET_BATCH_NODESLIST=`" getHosts " | sort | uniq`\n", \
  "",    "DIET_BATCH_NBNODES=`echo $DIET_BATCH_NODESLIST | wc -l`\n"

#define COMMON_FILE_FORMATS(getHosts) \
  "",    "DIET_BATCH_NODESFILE=" getHosts "\n"

  /* CONDOR supports all but ACCOUNT, NODETYPE PROCESSES QUEUE SH TIME */
  static const char *CONDOR_ATTR_FORMATS[] = {
    "",             "universe=vanilla\n",
    ELBASE_ARGS,    "arguments=%s\n",
    ELBASE_EMAIL,   "notify_user=%s\n",
    ELBASE_ENV,     "environtment=%s\n",
    ELBASE_MEMORY,  "image_size=%s000\n",
    ELBASE_NODES,   "machine_count=%s\n",
    ELBASE_NOTIFY,  "notify=%s\n",
    ELBASE_PROGRAM, "executable=%s\n",
    ELBASE_STDERR,  "error=%s\n",
    ELBASE_STDIN,   "input=%s\n",
    ELBASE_STDOUT,  "output=%s\n",
    ELBASE_WD,      "initialdir=%s\n",
    "",             "getenv=true\n",
    ELBASE_OTHERS,  "%s=%s\n",
    "",             "queue",
    NULL, NULL
  };

  /* DQS supports all but NODETYPE PROCESSES TIME */
  static const char *DQS_ATTR_FORMATS[] = {
    "", "#!/bin/sh\n",
    ELBASE_ACCOUNT, "#$ -A %s\n",
    ELBASE_EMAIL,   "#$ -M %s\n",
    ELBASE_NODES,   "#$ -l qty.eq.%s",
    ELBASE_MEMORY,  ".and.mem.ge.%s",
    "",             "\n",
    ELBASE_NOTIFY,  "#$ -m %s\n",
    ELBASE_QUEUE,   "#$ -q %s\n",
    ELBASE_SH,      "#$ -S %s\n",
    ELBASE_STDERR,  "#$ -e %s\n",
    ELBASE_STDOUT,  "#$ -o %s\n",
    ELBASE_OTHERS,  "#$ %s %s\n",
    COMMON_ATTR_FORMATS("cat $HOST_FILE"),
    COMMON_FILE_FORMATS("$HOST_FILE"),
    NULL, NULL
  };

  /* LOADLEVELER supports all but NODETYPE */
  static const char *LOADLEVELER_ATTR_FORMATS[] = {
    "",               "#!/bin/sh\n",
    ELBASE_ACCOUNT,   "#@ account_no=%s\n",
    ELBASE_EMAIL,     "#@ notify_user=%s\n",
    ELBASE_ENV,       "#@ environment=%s\n",
    ELBASE_MEMORY,    "#@ image_size=%s000\n",
    ELBASE_NODES,     "#@ node=%s\n",
    ELBASE_NOTIFY,    "#@ notification=%s\n",
    ELBASE_PROCESSES, "#@ total_tasks=%s\n",
    ELBASE_QUEUE,     "#@ class=%s\n",
    ELBASE_SH,        "#@ shell=%s\n",
    ELBASE_STDERR,    "#@ error=%s\n",
    ELBASE_STDIN,     "#@ input=%s\n",
    ELBASE_STDOUT,    "#@ output=%s\n",
    ELBASE_TIME,      "#@ wall_clock_limit=%s\n",
    ELBASE_WD,        "#@ initialdir=%s\n",
    "",               "#@ job_type=parallel\n",
    "",               "#@ environment=COPY_ALL\n",
    ELBASE_OTHERS,    "#@ %s=%s\n",
    "",               "#@ queue\n",
    ELBASE_PROGRAM,   "poe %s",
    ELBASE_ARGS,      " %s",
    NULL, NULL
  };

  /* LSF supports all but MEMORY NODETYPE NOTIFY */
  static const char *LSF_ATTR_FORMATS[] = {
    "",             "#!/bin/sh\n",
    ELBASE_ACCOUNT, "#BSUB -P %s\n",
    ELBASE_EMAIL,   "#BSUB -u %s\n",
    ELBASE_NODES,   "#BSUB -n %s\n",
    ELBASE_QUEUE,   "#BSUB -q %s\n",
    ELBASE_SH,      "#BSUB -L %s\n",
    ELBASE_STDERR,  "#BSUB -e %s\n",
    ELBASE_STDOUT,  "#BSUB -o %s\n",
    ELBASE_TIME,    "#BSUB -W %s\n",
    ELBASE_OTHERS,  "#BSUB %s %s\n",
    COMMON_ATTR_FORMATS("for x in $LSB_HOSTS; do echo $x; done"),
    /* The following must be tested */
    COMMON_FILE_FORMATS("$LSB_HOSTS"),
    NULL, NULL
  };

  /* PBS supports all */
  /*
   * TODO The use and interpretation of PBS's "-l nodes=%d" and "-l ncpus=%d"
   * options are apparently installation-specific.  Some sites use one, some
   * the other, at some sites the two are incompatible, and I even found one
   * site that invented its own "-l vcpus=%s" option.  For now, these formats
   * reflect the SDSC installation, but this will bite us eventually.
   */
  static const char *PBS_ATTR_FORMATS[] = {
    "",             "#!/bin/sh\n",
    ELBASE_ACCOUNT, "#PBS -A %s\n",
    ELBASE_EMAIL,   "#PBS -M %s\n",
    ELBASE_MEMORY,  "#PBS -l mem=%smb\n",
    ELBASE_NODES,   "#PBS -l nodes=%s",
    ELBASE_NODETYPE, ":%s",
    "",              "\n",
    ELBASE_NOTIFY,  "#PBS -m %s\n",
    ELBASE_QUEUE,   "#PBS -q %s\n",
    ELBASE_SH,      "#PBS -S %s\n",
    ELBASE_STDERR,  "#PBS -e %s\n",
    ELBASE_STDOUT,  "#PBS -o %s\n",
    ELBASE_TIME,    "#PBS -l walltime=%s\n",
    ELBASE_OTHERS,  "#PBS %s %s\n",
    COMMON_ATTR_FORMATS("cat $PBS_NODEFILE"),
    COMMON_FILE_FORMATS("$PBS_NODEFILE"),
    NULL, NULL
  };

  /* SGE supports all but NODETYPE */
  /*
   * NOTE: Although SGE has something called queues, they are single-host
   * abstract entities used only for configuration.  SGE "parallel environments"
   * are analogous to other batch services' queues.
   */
  static const char *SGE_ATTR_FORMATS[] = {
    "",             "#!/bin/sh\n",
    ELBASE_ACCOUNT, "#$ -A %s\n",
    ELBASE_EMAIL,   "#$ -M %s\n",
    ELBASE_MEMORY,  "#$ -l mem_total=%sM\n",
    ELBASE_NOTIFY,  "#$ -m %s\n",
    ELBASE_QUEUE,   "#$ -pe %s",
    ELBASE_NODES,   " %s",
    "",             "\n",
    ELBASE_SH,      "#$ -S %s\n",
    ELBASE_STDERR,  "#$ -e %s\n",
    ELBASE_STDOUT,  "#$ -o %s\n",
    ELBASE_TIME,    "#$ -l h_rt=%s\n",
    ELBASE_OTHERS,  "#$ %s %s\n",
    COMMON_ATTR_FORMATS("awk ' {print $1;}' $PE_HOSTFILE"),
    /* The following must be tested */
    COMMON_FILE_FORMATS("awk ' {print $1;}' $PE_HOSTFILE"),
    NULL, NULL
  };

  /* YC
    OAR supports a SQL query option with -p
    More options with v2.0:
    --notify, --stdout, stderr, etc.
  */
  static const char *OAR_ATTR_FORMATS[] = {
    "",             "#!/bin/sh\n",
    ELBASE_NODES,   "#OAR -l nodes=%s\n",
    /* OAR behaves with SQL scripts to reserve specials nodes */
    /* but the following line is not good enough: too less nodes */
    /*    ELBASE_NODETYPE,"#OAR -p \"hostname='%s'\"", */
    ELBASE_QUEUE,   "#OAR -q %s\n",
    ELBASE_TIME,    "#OAR -l walltime=%s\n",
    ELBASE_OTHERS,  "#OAR %s %s\n",
    "",              "\n",
    COMMON_ATTR_FORMATS("cat $OAR_NODEFILE"),
    COMMON_FILE_FORMATS("$OAR_NODEFILE"),
    NULL, NULL
  };

  static const char *SHELLSCRIPT_ATTR_FORMATS[] = {
    "",             "#!/bin/sh\n",
    NULL, NULL
  };
  /* FYC */

  static const char *JID_EXTRACT_PATTERNS[ELBASE_SCHEDULER_SERVICE_COUNT] = {
    "",
    "/cluster (\\d+)/",
    "/job (\\d+)/",
    "/\"([^\"]+). has been submitted/",
    "/<(\\d+)/",
    "/^(\\d+)/m",
    "/job (\\d+)/"
    /* YC */
    /* Result of a oar request : IdJob = 108426 */
    ,"/IdJob = (\\d+)/", "/(\\d+)/"
    /* FYC */
  };
  static const char *KILL_COMMANDS[ELBASE_SCHEDULER_SERVICE_COUNT] = {
    "kill(9, $jid)", "`condor_rm $jid`", "`qdel $jid`", "`llcancel $jid`",
    "`bkill $jid`", "`qdel $jid`", "`qdel $jid`"
    /* YC */
    ,"oardel $jid", "kill(9, $jid)"
    /* FYC */
  };
  static const char *SUBMIT_COMMANDS[ELBASE_SCHEDULER_SERVICE_COUNT] = {
    "",
    "condor_submit " BATCH_PATH,
    "qsub " BATCH_PATH,
    "llsubmit " BATCH_PATH,
    "bsub < " BATCH_PATH,
    "qsub " BATCH_PATH,
    "qsub " BATCH_PATH
    /* YC */
    /* In order to be sure that OAR take PWD and not /bin/pwd: for Grenoble */
    ,"cd ; oarsub " BATCH_PATH, 
    "/bin/bash " BATCH_PATH
    /* FYC */
  };
  /*
   * NOTE: We can get a job exit code for some batch schedulers (LoadLeveler,
   * at least), but it's not clear that it's meaningful, especially if we're
   * running multiple processes.  For the time being, we return 0.
   */
  static const char *EXIT_CODE_EXPRS[ELBASE_SCHEDULER_SERVICE_COUNT] = {
    "$? >> 8",
    "0",
    "0",
    "0",
    "0",
    "0",
    "0"
    /* YC */
    ,"0", "$? >> 8",
    /* FYC */
  };

  static const char *WAIT_EXPRS[ELBASE_SCHEDULER_SERVICE_COUNT] = {
    "waitpid($jid, &WNOHANG) == 0",
    "`condor_q` =~ /^$jid\\D/m",
    "`qstat $jid 2>&1` =~ /-----/",
    "`llq $jid` =~ /[1-9] job step/",
    "`bjobs $jid` =~ /PEND|RUN|SUSP/",
    "`qstat $jid 2>&1` =~ /-----/",
    "`qstat 2>&1` =~ /^ *$jid /m"
    /* YC */
    ,"`oarstat 2>&1` =~ /^ *$jid /m"
    ,"waitpid($jid, &WNOHANG) == 0",
    /* FYC */
  };

  const char **attr;
  char *c;
  const char **formats;
  Property *p;
  const char *pollFreq = scheduler == ELBASE_SHELL ? "1" : "30";
  PropertyList pl;
  char *result;
  char *submit;
  /* YC */
  char *executableCommand = strdup(executablePath) ;
  /* FYC */

  if(server == NULL || *server == '\0')
    service = ELBASE_FORK;
  if(workingDirectory != NULL && strcmp(workingDirectory, ".") == 0)
    workingDirectory = NULL;

  formats =
    service == ELBASE_GRAM ? GRAM_ATTR_FORMATS :
    service == ELBASE_NETSOLVE ? NETSOLVE_ATTR_FORMATS :
    service == ELBASE_RASH ? RASH_ATTR_FORMATS :
    scheduler == ELBASE_SHELL ? SHELL_ATTR_FORMATS :
    scheduler == ELBASE_CONDOR ? CONDOR_ATTR_FORMATS :
    scheduler == ELBASE_DQS ? DQS_ATTR_FORMATS :
    scheduler == ELBASE_LOADLEVELER ? LOADLEVELER_ATTR_FORMATS :
    scheduler == ELBASE_LSF ? LSF_ATTR_FORMATS :
    scheduler == ELBASE_PBS ? PBS_ATTR_FORMATS :
    scheduler == ELBASE_SGE ? SGE_ATTR_FORMATS :
    /* YC */
    scheduler == ELBASE_OAR ? OAR_ATTR_FORMATS : 
    scheduler == ELBASE_SHELLSCRIPT ? SHELLSCRIPT_ATTR_FORMATS : 
    /* FYC */
    SHELL_ATTR_FORMATS;

  /* Get initial properties and set others based on the other fn params. */
  pl = schedulerAttributes == NULL ? PropertyListNew() :
       EnvironToPropertyList(schedulerAttributes);
  
  if(arguments != NULL && *arguments != NULL) {
    char *args = NULL;
    if(service == ELBASE_GRAM)
      AppendListIfThere
	(&args, "\"\"", arguments, "\"\" \"\"", "\"\"", NULL, GRAM_ESCAPE);
    else if(service == ELBASE_NETSOLVE)
      AppendListIfThere(&args, "\"", arguments, "\" \"", "\"", NULL,NO_ESCAPE);
    else if(service == ELBASE_RASH)
      AppendListIfThere
	(&args, "\"", arguments, "\" \"", "\"", NULL, RASH_ESCAPE);
    else if(scheduler == ELBASE_CONDOR)
      AppendListIfThere(&args, "", arguments, " ", "", NULL, NO_ESCAPE);
    else if(scheduler == ELBASE_SHELL)
      AppendListIfThere
	(&args, ", \"", arguments, "\", \"", "\"", NULL, PERL_ESCAPE);
    else
      AppendListIfThere(&args, "\"", arguments, "\" \"", "\"", NULL,NO_ESCAPE);
    SetProperty(&pl, ELBASE_ARGS, args);
    free(args);
  }

  /*  DIET case: environment always NULL for the moment! */
  if(environment != NULL && *environment != NULL) {
    char *env = NULL;
    unsigned i;
    if(service == ELBASE_GRAM)
      AppendListIfThere
	(&env, "(", environment, "\")(", "\")", " \"", GRAM_ESCAPE);
    else if(service == ELBASE_RASH)
      AppendListIfThere
	(&env, "set ", environment, "\"; set ", "\";", " \"", RASH_ESCAPE);
    else if(scheduler == ELBASE_CONDOR)
      AppendListIfThere(&env, "", environment, ";", "", NULL, NO_ESCAPE);
    else if(scheduler == ELBASE_LOADLEVELER)
      AppendListIfThere(&env, "", environment, "\";", "\"", "=\"", NO_ESCAPE);
    else if(scheduler == ELBASE_SHELL)
      AppendListIfThere
	(&env, "$ENV{\"", environment, "\";\n$ENV{\"", "\";\n", "\"}=\"",
	 PERL_ESCAPE);
    else {
      AppendListIfThere
	(&env, "", environment, "\"\n", "\"\n", "=\"", NO_ESCAPE);
      for(i = 0; environment[i] != NULL; i++) {
	env = StrAppend(env, "export ", environment[i], "\n", NULL);
	*strrchr(env, '=') = '\0';
      }
    }
    SetProperty(&pl, ELBASE_ENV, env);
    free(env);
  } 

  if(FindPropertyByName(pl, ELBASE_NODECOPY) == NULL)
    SetProperty(&pl, ELBASE_NODECOPY, "scp");
  if(FindPropertyByName(pl, ELBASE_NODES) == NULL)
    SetProperty(&pl, ELBASE_NODES, "1");
  if(FindPropertyByName(pl, ELBASE_NODESPAWN) == NULL)
    SetProperty(&pl, ELBASE_NODESPAWN, "ssh");
  if(FindPropertyByName(pl, ELBASE_NOTIFY) == NULL)
    SetProperty(&pl, ELBASE_NOTIFY, "never");
  if(service != ELBASE_GRAM) {
    if(scheduler == ELBASE_DQS ||
       scheduler == ELBASE_PBS ||
       scheduler == ELBASE_SGE
       /* TODO: when OAR v2.0 ; || scheduler == ELBASE_OAR */
       ) {
      char *notify = strdup(FindPropertyValueByName(pl, ELBASE_NOTIFY));
      notify[1] = '\0';
      SetProperty(&pl, ELBASE_NOTIFY, notify);
      free(notify);
    }
    else if(scheduler == ELBASE_LSF) {
      char *notify = FindPropertyValueByName(pl, ELBASE_NOTIFY);
      if(strcmp(notify, "complete") == 0 || strcmp(notify, "always") == 0)
        SetProperty(&pl, "-N", "");
    }
  }
  if(FindPropertyByName(pl, ELBASE_PROCESSES) == NULL)
    SetProperty
      (&pl, ELBASE_PROCESSES, FindPropertyValueByName(pl, ELBASE_NODES));
  /* Only Condor appends relative exec paths to submission dir; others use wd */
  if(scheduler == ELBASE_CONDOR &&
     *executableCommand != '/' &&
     workingDirectory != NULL) {
    char *fullPath =
      StrAppend(NULL, workingDirectory, "/", executableCommand, NULL);
    SetProperty(&pl, ELBASE_PROGRAM, fullPath);
    free(fullPath);
  } else SetProperty(&pl, ELBASE_PROGRAM, executableCommand);
  
  SetProperty(&pl, ELBASE_SH, "/bin/sh");
  if(stderrPath != NULL)
    SetProperty(&pl, ELBASE_STDERR, stderrPath);
  if(stdinPath != NULL)
    SetProperty(&pl, ELBASE_STDIN, stdinPath);
  if(stdoutPath != NULL)
    SetProperty(&pl, ELBASE_STDOUT, stdoutPath);
  if(service != ELBASE_GRAM &&
     (scheduler == ELBASE_LOADLEVELER || scheduler == ELBASE_LSF ||
      scheduler == ELBASE_PBS || scheduler == ELBASE_SGE
      /* YC */
      || scheduler == ELBASE_OAR
      /* FYC */
      ) &&
     FindPropertyByName(pl, ELBASE_TIME) != NULL
     ) {
    /* Translate minutes into the preferred format. */
    char formatted[20];
    long minutes = strtol(FindPropertyValueByName(pl, ELBASE_TIME), NULL, 10);
    sprintf(formatted, "%02ld:%02ld%s",
            minutes / 60, minutes % 60, scheduler == ELBASE_LSF ? "" : ":00");
    SetProperty(&pl, ELBASE_TIME, formatted);
  }
  if(workingDirectory != NULL)
    SetProperty(&pl, ELBASE_WD, workingDirectory);
  if(scheduler == ELBASE_SGE && FindPropertyByName(pl, ELBASE_QUEUE) == NULL)
    SetProperty(&pl, ELBASE_QUEUE, "*");

  submit = NULL;
  /* Add the attrs given in parameter */ 
  for(attr = formats; *attr != NULL; attr += 2) {
    const char *format = *(attr + 1);
    if(**attr == '\0')
      /* Invariant text */
      submit = StrAppend(submit, format, NULL);
    else if((c = FindPropertyValueByName(pl, *attr)) != NULL) {
      char *toAdd = strdup(format);
      StrReplace(&toAdd, strstr(toAdd, "%s") - toAdd, 2, c);
      submit = StrAppend(submit, toAdd, NULL);
      free(toAdd);
    }
    else if(strcmp(*attr, ELBASE_OTHERS) == 0) {
      /* Plug name and value of each unrecognized attr in turn into format. */
      char *toAdd;
      ForEachProperty(pl, p) {
        if(StrArrayFind(KNOWN_ATTRIBUTES, PropertyName(*p), NULL))
          continue;
        toAdd = strdup(format);
        StrReplace(&toAdd, strstr(toAdd, "%s") - toAdd, 2, PropertyName(*p));
        StrReplace(&toAdd, strstr(toAdd, "%s") - toAdd, 2, PropertyValue(*p));
        submit = StrAppend(submit, toAdd, NULL);
        free(toAdd);
      }
    }
  }
  PropertyListFree(&pl);

  if(service == ELBASE_FORK && scheduler == ELBASE_SHELL) {
    while((c = strstr(submit, "${")) != NULL && strchr(c + 2, '}') != NULL)
      StrReplace(&submit, c + 1 - submit, 0, "ENV");
  }
  else if(service == ELBASE_FORK || service == ELBASE_SSH) {
    /*
     * A note on killing ssh processes.  In response to an ssh client request,
     * sshd on the remote machine forks a copy of itself that spawns a copy of
     * the user's shell that spawns a copy of the application--three processes.
     * When the client is killed, only the sshd copy goes away.  The other two
     * remain active unless the application writes to stdout/err, in which case
     * it gets a SIGPIPE.  (The sshd copy captures the output and sends it over
     * the socket to the client.)  There seems to be no way to force ssh to
     * kill the other two remote processes in reponse to client termination.
     * We tried running ssh with -t -t, forcing pseudo-tty allocation and
     * ultimately sending a SIGHUP to the application when the client died.
     * Although this worked, it caused all sorts of other havoc and so had to
     * be abandoned.  Since we're already using a perl script to run remote ssh
     * processes, killing the process is no problem; detecting when to do so is
     * hard.  The one observable difference on the remote side when the client
     * dies is that the parent of the shell process (the grandparent of the
     * perl process) changes from the now-defunct sshd copy to init (process
     * 1). Unfortunately, Unix defines no way for a process to get its
     * grandparent's pid.  We wound up using ps to retrieve this info, but this
     * approach may fail to kill the application on some systems due to the
     * highly variant output of ps.
     */
    char *withMonitor = NULL ;
    if(scheduler == ELBASE_SHELL) {
      withMonitor = StrAppend(NULL,
        "use POSIX ':sys_wait_h';\n",
        "if(($jid = fork) == 0) {\n",
        submit,
        "\n}\n",
        "&Done($jid) if $jid < 0;",
        NULL
      );
    } else {
      if( scheduler == ELBASE_SHELLSCRIPT )
	withMonitor = StrAppend(NULL,
				"use POSIX ':sys_wait_h';\n",
				NULL ) ;
      if( workingDirectory != NULL )
        withMonitor = StrAppend(NULL,
				"chdir(\"", workingDirectory, "\");\n",
				"$ENV{PWD}=\"", workingDirectory, "\";\n",
				NULL
				);
      withMonitor = StrAppend(withMonitor,
			      "# Write the batch submission script.\n",
			      /* YC */
			      "open(BSS, \">" BATCH_PATH "\");\n",
			      /* FYC */
			      "print BSS <<ENDBATCH;\n",
			      NULL
			      );
      AppendIfThere(&withMonitor, NULL, submit, NULL, PERL_ESCAPE);

      /* Add the command to the batch script */
      AppendIfThere(&withMonitor, NULL, executableCommand, NULL, PERL_ESCAPE) ;

      /* TODO: Add here the storage of DIET_ID and batch_id in a file */
      withMonitor = StrAppend(withMonitor,
			      "\nENDBATCH\n",
			      "close BSS;\n",
			      /* YC */
			      /* OAR needs the script to be executable */
			      "eval{ chmod(0777,\""
			      BATCH_PATH "\");};\n"
			      /*FYC*/
        "# Submit the job and get the id (jid).\n",
        "$submitOut = `", SUBMIT_COMMANDS[scheduler], " 2>&1`;\n",
        "print(STDERR $submitOut) and &Done($? >> 8) if $? != 0;\n",
        "$submitOut =~ ", JID_EXTRACT_PATTERNS[scheduler], ";\n",
        "print(STDERR $submitOut) and &Done(33) if !defined $1;\n",
        "$jid = $1;",
        NULL
      );
    }
    free(submit);
    submit = withMonitor;
    submit = StrAppend(submit, "\nsub Done {\n", NULL);
    if(scheduler != ELBASE_SHELL)
      submit = StrAppend(submit,
        RemoveTempFiles() ? "  " : "  # ", "unlink(\"" BATCH_PATH "\");\n",
        NULL);
    submit = StrAppend(submit,
      "  exit($_[0]) if defined $_[0];\n",
      "  ", KILL_COMMANDS[scheduler], " if defined $jid;\n",
      "  exit(44);\n",
      "};\n",
      "sub GetGrandparent {\n",
      "  my $ppid = getppid();\n",
      "  `ps -o pid,ppid -p $ppid 2>&1` =~ /$ppid\\s+(\\d+)/;\n",
      "  return defined $1 ? $1 : 0;\n",
      "};\n",
      "$gppid = &GetGrandparent();\n",
    /* Catch SIGINT for a local kill, SIGPIPE if the ssh client is killed. */
      "$SIG{HUP} = $SIG{INT} = $SIG{PIPE} = $SIG{TERM} = sub { &Done(); };\n",
    /*
     * Perl weirdisms!  For reasons we can't determine, using a while loop
     * (while(test) { sleep 1; }) doesn't set up $1 properly if the test
     * contains a pattern match (at least for Condor), whereas the short form
     * (sleep 1 while test;) works.  Also, on at least one machine (SDSC Blue
     * Horizon), we have seem zombie processes from backtick commands, although
     * we cannot now reproduce the problem.
     */
      "sleep(", pollFreq, ") while &GetGrandparent() == $gppid and (",
        WAIT_EXPRS[scheduler], ");\n",
      "$exitCode = ", EXIT_CODE_EXPRS[scheduler], ";\n",
      "&Done(&GetGrandparent() == $gppid ? $exitCode : undef);",
      NULL);
    while((c = strstr(submit, "${")) != NULL && strchr(c + 2, '}') != NULL)
      StrReplace(&submit, c + 1 - submit, 0, "ENV");
  }
  if(service == ELBASE_FORK || service == ELBASE_SSH) {
    char *targetPath = UniqueFilePath(TEMP_PREFIX "elagi_perl");
    char *withWrapper;
    withWrapper = StrAppend
      (NULL, "cat << 'ENDPERL' > ", targetPath, " || exit $?\n",
       /* Comment the following to keep perl script in /tmp of frontale node */
       RemoveTempFiles() ? "" : "# ", "unlink($0);\n",
       submit,
       "\nENDPERL", NULL);
    free(submit);
    submit = withWrapper;
    if(service == ELBASE_FORK)
      result = StrAppend(NULL,
        submit, "\n",
        "exec ", computeCommands[ELBASE_FORK], " ", targetPath, "\n",
        NULL);
    else {
      char *scpScript = ELBASE_ScriptForCopy
        (ELBASE_CP, "", targetPath, ELBASE_SCP, server, targetPath);
      char *sshCommand = SshInvocation(server);
      sshCommand = StrAppend(sshCommand, "perl ", targetPath, NULL);
      result = StrAppend(NULL,
        submit, "\n",
        scpScript, "\n",
        "x=$?\n",
	RemoveTempFiles() ? "" : 
	"#", "/bin/rm -f ", targetPath, "\n",
	"if test $x -ne 0; then exit $x; fi\n",
        "exec ", sshCommand,
        NULL);

      free(sshCommand);
      free(scpScript);
    }
    free(targetPath);
  }
  else if(service == ELBASE_GRAM) {
    result = StrAppend
      (NULL, computeCommands[ELBASE_GRAM], " -o -r ", server,
             scheduler == ELBASE_CONDOR ?      "/jobmanager-condor" :
             scheduler == ELBASE_DQS ?         "/jobmanager-dqs" :
             scheduler == ELBASE_LOADLEVELER ? "/jobmanager-loadleveler" :
             scheduler == ELBASE_LSF ?         "/jobmanager-lsf" :
             scheduler == ELBASE_PBS ?         "/jobmanager-pbs" :
             scheduler == ELBASE_SGE ?         "/jobmanager-grd" : "",
             " '", submit, "'", NULL);
    result = StrAppend(result, " | perl -e 'while(<>) {$_ =~ s/_EXITCODE (\\d+)\\n//; print; exit($1) if defined $1;}'", NULL);
  }
  else if(service == ELBASE_NETSOLVE)
    result = StrAppend
      (NULL, computeCommands[ELBASE_NETSOLVE], " ", submit, NULL);
  else if(service == ELBASE_RASH)
    result = StrAppend
      (NULL,computeCommands[ELBASE_RASH]," ",server," '", submit,"'",NULL);
  else
    result = strdup(submit);

  free(submit);
  return result;

}


int
ELBASE_SetComputeCommand(ELBASE_ComputeServiceTypes service,
                         const char *command) {
  if(service >= ELBASE_COMPUTE_SERVICE_COUNT)
    return 0;
  if(computeCommandsSet[service] != NULL)
    free(computeCommandsSet[service]);
  if(command == NULL) {
    computeCommandsSet[service] = NULL;
    computeCommands[service] = computeCommandsDefaults[service];
  }
  else
    computeCommands[service] = computeCommandsSet[service] = strdup(command);
  return 1;
}


int
ELBASE_SpawnScript(const char *script,
                   ELBASE_Process *process) {

  char **argv;
  char *c;
  FILE *f;
  ASEXEC_ProcessId pid;
  char *quote;
  int result;
  char *tempPath;
  char *toExec = strdup(script);

  for(c = toExec + strlen(toExec) - 1; c >= toExec && isspace((int)*c); c--)
    ; /* empty */
  *(c + 1) = '\0';
  
  if(strchr(toExec, '\n') != NULL || strchr(toExec, '|') != NULL) {
    tempPath = UniqueFilePath("elagi_sh");
    if((f = fopen(tempPath, "w")) == NULL) {
      free(tempPath);
      free(toExec);
      return 0;
    }
    fprintf(f, "%s%s\n%s\n",
	    RemoveTempFiles() ? "" : 
	    "# ", "/bin/rm -f $0", toExec);
    fclose(f);
    free(toExec);
    toExec = StrAppend(NULL, "/bin/sh ", tempPath, NULL);
    free(tempPath);
  }
  for(c = toExec, quote = NULL; *c != '\0'; c++) {
    if(quote == NULL) {
      if(*c == '"' || *c == '\'')
        quote = c;
      else if(isspace((int)*c))
        *c = '\001';
    }
    else if(*c == *quote) {
      *c = *quote = '\001';
      quote = NULL;
    }
  }
  argv = StrSplit(toExec, "\001");
  result =
    ProcessStart(argv[0], (const char **)argv, process == NULL ? NULL : &pid);
  StrArrayFree(argv);
  free((char *)toExec);
  if(result && process != NULL) {
    ProcessInfo pi = (ProcessInfo)malloc(sizeof(struct _ProcessInfoStruct));
    pi->gramProcess = computeCommands[ELBASE_GRAM] != NULL &&
                      strstr(script, computeCommands[ELBASE_GRAM]) != NULL;
    pi->pid = pid;
    *process = pi;
  }
  return result;
}


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
              ELBASE_Process *process) {
  int result;

  char *script = ELBASE_ScriptForSubmit
    (service, server, scheduler, schedulerAttributes, executablePath,
     workingDirectory, arguments, stdinPath, stdoutPath, stderrPath,
     environment);
  if(script == NULL)
    return 0;

  result = ELBASE_SpawnScript(script, process);
  free(script);
  return result;
}

/* YC */
/* Return the ID of the Batch scheduler */
ELBASE_SchedulerServiceTypes 
ELBASE_GiveBatchID(const char *batchName)
{
  int i=0 ;
  
  while( (i<ELBASE_SCHEDULER_SERVICE_COUNT)
	 && (strncmp(batchName,availableBatchScheduler[i],2)!=0) )
    /* for the moment, only a check on the 2 first chars is needed */
    i++ ;
  if( i == ELBASE_SCHEDULER_SERVICE_COUNT )
    return -1 ;
  else
    return (ELBASE_SchedulerServiceTypes)i ;
}

int 
ELBASE_ExistBatchScheduler(const char *batchName,
			   ELBASE_SchedulerServiceTypes *batchID)
{
   *batchID = ELBASE_GiveBatchID(batchName) ;
   if ((int)batchID >= 1) return 1 ;
   else
     return 0 ; 
}

const char*
ELBASE_GiveBatchName(ELBASE_SchedulerServiceTypes ID)
{
  if( (ID < 0) || (ID > ELBASE_SCHEDULER_SERVICE_COUNT) )
    return "" ;
  else
    return availableBatchScheduler[ID] ;
  /* FIXME: strdup? */
}
int
ELBASE_IsEqual(ELBASE_SchedulerServiceTypes id1, 
	       ELBASE_SchedulerServiceTypes id2)
{
  return (id1 == id2) ;
}
/* FYC */

  
#ifdef __cplusplus
}
#endif

