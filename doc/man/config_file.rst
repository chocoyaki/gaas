DIET CONFIGURATION FILE
=======================

Every DIET entity requires a configuration file.

Please note that:

* comments start with ’#’ and finish at the end of the current line,

* meaningful lines have the format: keyword = value, following the format
  of configuration files for omniORB 4,

* for options that accept 0 or 1, 0 means no and 1 means yes, and

* keywords are case sensitive.

Depending on the type of DIET element, different kinds of  keyword  could
be  found.  Here is a list of the possible keywords for a DIET Agent
configuration file:


**traceLevel**
  Integer value corresponding to the traceLevel for the DIET agent:

  + 0: DIET do not print anything.

  + 1: DIET prints only warnings and errors on the standard error output.

  + 2: [default] DIET prints information on the main steps of a call.

  + 5: DIET prints information on all internal steps too.

  + 10: DIET prints all the communication structures too.

  + >10: (traceLevel - 10) is given to the ORB to print CORBA messages too.


 0 Diet do not print anything, – 1 Diet prints only warnings and errors on the standard error output, – 2 [default] Diet prints information on the main steps of a call, – 5 Diet prints information on all internal steps too, – 10 Diet prints all the communication structures too, – > 10 (traceLevel - 10) is given to the ORB to print CORBA messages too.

**agentType**
  three possible values

  + DIET MASTER AGENT (or MA) for a Master Agent

  + DIET LOCAL AGENT (or LA) for a Local Agent

  + DIET_MA_DAG for an MA DAG Agent

**dietPort**
  Integer setting the listening port of the agent. If left empty, the ORB will get
  an open port from the system (if default 2809 is busy).

**dietHostName**
  String setting the listening interface of the agent. If left empty, the ORB 
  will use the system hostname (the first one if several are available).

**name**
  String identifying the element. Clients and children nodes (LAs and SeDs) must 
  point to the same CORBA Naming Service hosting the MA.

**parentName**
  String identifying the parent agent.

  [Remark: Only DIET Local Agents could use the parentName keyword]

**fastUse**
  Boolean enabling/disabling FAST module.
  If set to 0, all LDAP and NWS parameters are ignored, and all requests to FAST 
  are disabled (when Diet is compiled with FAST).
  This is useful while testing a DIET platform without having to deploy an LDAP 
  base nor an NWS platform.

  [Remark: DIET must be compiled with FAST]

**ldapUse**
  Boolean enabling/disabling LDAP support.

  [Remark: DIET must be compiled with FAST]

**ldapBase**
  String representing the LDAP base storing FAST-known services address 
  in the form host:port .

  [Remark: DIET must be compiled with FAST]

**ldapMask**
  String specifying the mask registered in the LDAP base.

  [Remark: DIET must be compiled with FAST]

**nwsUse**
  Boolean enabling/disabling NWS support.

  [Remark: DIET must be compiled with FAST]

**nwsNameserver**
  String representing the NWS naming service address in the form host:port .

  [Remark: DIET must be compiled with FAST]

**nwsForecaster**
  String representing the NWS forecast module used by FAST.

  [Remark: DIET must be compiled with FAST]

**useLogService**
  Boolean enabling/disabling the LogService for monitoring purposes.

**lsOutbuffersize**
  Integer setting outgoing messages buffer size.

**lsFlushinterval**
  Integer setting the flush interval for the outgoing messages buffer.

**neighbours**
  String listing MA that must be conntacted to build a federation. It is 
  formatted as a white-space separated list of addresses in the form host:port.

  [Remark: DIET must be compiled with the Multi-MA option]

**minimumNeighbours**
  Integer setting the minimum connected neighbours. If the agent has less 
  connected neighbours, it will try establishing new connections.
  
  [Remark: DIET must be compiled with the Multi-MA option]

**maximumNeighbours**
  Integer setting the maximum connected neighbours. Further, the agent will 
  refuse newer connections.

  [Remark: DIET must be compiled with the Multi-MA option]

**updateLinkPeriod**
  Integer setting the period (in seconds) at which the agent will check its 
  neighbours status and will try establishing new connections if their numbers 
  is less than minimumNeighbours.

  [Remark: DIET must be compiled with the Multi-MA option]

**bindServicePort**
  Integer defining the port used by the MA to share its IOR.

  [Remark: Option used only by MAs]

**useConcJobLimit**
  Boolean enabling/disabling the SeD restriction about concurrent solves.
  This should be used in conjunction with maxConcJobs.

  [Remark: Option used only by SeDs]

**maxConcJobs**
  Integer setting the maximum number of jobs running at once.
  This should be used in conjunction with maxConcJobs.

  [Remark: Option used only by SeDs]

**locationID**
  String used for alternative transfer cost prediction in Dagda.

  [Remark: Option used only by SeDs]

**MADAGNAME**
  String corresponding to the name of the MADAG agent.

  [Remark: DIET must be compiled with the workflow option]

  [Remark: Option used only by clients]

**schedulerModule**
  Path to the sheduler library module containing the scheduler implementation.

  [Remark:  DIET  must  be compiled with the User Scheduling option]

  [Remark: Option used only by agents]

**moduleConfigFile**
  String corresponding to an optional configuration file for the module.

  [Remark: DIET must be compiled with the User Scheduling option]

  [Remark: Option used only by agents]

**batchName**
  String corresponding to the name of the queue where the job will be submitted.

  [Remark: DIET must be compiled with the Batch option]

  [Remark: Option used only by SeDs]

**pathToNFS**
  Path to the NFS directory where you have read/write permissions.

  [Remark: DIET must be compiled with the Batch option] 

  [Remark: Option used only by SeDs]

**pathToTmp**
  Path to the temporary directory where you have read/write permissions.

  [Remark: DIET must be compiled with the Batch option] 

  [Remark: Option used only by SeDs]

**internOARbatchQueueName**
  String only useful when using CORI batch features with OAR 1.6

  [Remark: DIET must be compiled with the Batch option]

  [Remark: Option used only by SeDs]

**initRequestID**
  Integer setting the MA Request ID starting value.

  [Remark: Option used only by MAs]

**ackFile**
  Path to a file that will be created when the element is ready to execute.

  [Remark: DIET must be compiled with the Acknowledge File option]

  [Remark: Option used only by Agents and SeDs]

**maxMsgSize**
  Integer setting the maximum size of CORBA messages sent by Dagda.
  By default, it's the same as the omniORB giopMaxMsgSize size.

**maxDiskSpace**
  Integer setting maximum disk space available to Dagda for storingt data.
  When set to 0, Dagda will ignore any disk quota. By default, it's the same 
  value as available disk space on the partition set by storageDirectory.
  
**maxMemSpace**
  Integer setting the maximum memory available to Dagda. When set to 0, Dagda 
  will ignore any memory usage limitation. By default, no limitations.

**cacheAlgorithm**
  String defining the cache replacement algorithm used when Dagda needs more
  memory for storing a piece of data. Possible values are: LRU, LFU, FIFO.
  By default, no cache replacement algorithm, Dagda never overwrite data.

**shareFiles**
  Boolean enabling/disabling Dagda file sharing with its children.
  Requires that the path is accessible by the children (ie: NFS partition shared
  by parent and children). By default, no file sharing.  

**dataBackupFile**
  Path to the backup file used by Dagda on user request (checkpointing).
  By default, no checkpointing is disabled.

  [Remark: Option used by Agents and ServerDaemon]

**restoreOnStart**
  Boolean defining if Dagda have to load the file set by dataBackupFile at 
  startup and restore all data recorded during the last checkpointing event.
  Disabled by default. 

  [Remark: Option used by agents and SeDs]

**storageDirectory**
  String defining the directory where Dagda will store data files. 
  By default /tmp is used.
