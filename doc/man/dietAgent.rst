=========
dietAgent
=========

:Author: david.loureiro@sysfera.com
:Date:   2010-09-07
:Copyright: DIET developers
:License: GPLv3+
:Version: 0.1
:Manual section: 1
:Manual group:

NAME
====

dietAgent - Main binary of the DIET Architecture for running DIET Agents 
(master and local agents)

SYNOPSYS
========

  dietAgent [config file] ...

DESCRIPTION
===========

The DietAgent is the main binary of the DIET distribution.
It is used for both master and local agents of a DIET hierarchy.

DIET PLATFORM
=============

A DIET platform is buildt upon Server Daemons (SeD). Requests are distributed 
amongst a hierarchy of agents. The scheduler can rely on resources availability 
informations collected from three different tools: NWS sensors which are placed 
on every node of the hierarchy, from the application-centric performance 
prediction tool FAST which relies on NWS informations or from CoRI Easy a 
module based on simple system calls and basic performance tests.

The different components of a DIET architecture are the following:

**Client**
  A client is an application which uses DIET to solve computational problems.
  Clients could be web pages, PSE scripts like Matlab or Scilab or native program.

**Master Agent (MA)** 
  A MA manages computation requests from clients. It chooses the best server 
  available to handle the request based on performance informations collected 
  from servers. Then, the reference of the chosen server is returned to the 
  client.

**Local Agent (LA)** 
  A LA transmits requests between MAs and servers. LAs store a list of services 
  available in their subtree. For each service, LAs store a list of children 
  (either agents or servers) providing the former. Depending on the underlying 
  network topology, a hierarchy of LAs may exists between the MA and the 
  appropriate servers, one of LAs tasks is to do a partial scheduling on its 
  subtree, effectively reducing its MA workload.

**Server Daemon (SeD)**
  A SeD encapsulate a computational resource. FOr instance, it can be locate on 
  the entry point of a parallel computer. SeD store a list of locally available 
  data, available computational solvers and performance-related information 
  (available memory amount or number or resources). During registration, SeD 
  declare to its parent agent (LA or MA) every computational problem it can solve.
  SeD can send performance and hardware informations by using the CoRI module or 
  performance predictions for some kinds of problems by using the FAST module.


CORBA USAGE FOR DIET
====================

DIET relies on the CORBA naming service for service discovery allowing every 
entity to interconnect. Reference to the omniORB naming service is written down 
in a CORBA configuration file whose path is given to omniORB through the 
environment variable OMNIORB_CONFIG.

The  lines  concerning  the name server in the omniORB configuration file
are built as follows:

::

  InitRef  =  NameService=corbaname::<hostname>:<port>

The  name  server port is the port given as an argument to the -start option of 
omniNames. You also need to update your LD LIBRARY PATH to  point
to <install dir>/lib. So your LD LIBRARY PATH environment variable should
now be :

  LD LIBRARY PATH=<omniORB home>/lib:<install dir>/lib.

  **NB1**:  In order to avoid name collision, every agent must be  assigned  a
  different name in the name server; since they don’t have any children, SeDs 
  do not need names assigned to them and they don’t register with the name server.

  **NB2**:  Each  Diet  hierarchy can use a different name server, or multiple
  hierarchies can share one name server (assuming all agents are assigned unique 
  names).  In  a multi-MA environment, in order for multiple hierarchies to be 
  able to cooperate it is necessary that they all share the same name server.

OPTIONS
=======

**config_file**
  Configuration file used by the agent to launch the DIET entity

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
be  found.  Here is a list of the possible keywords for a DIET Agent con‐
figuration file:

**traceLevel**
  Integer value corresponding to the traceLevel for the DIET agent:

  + 0: DIET prints only warnings and errors on the standard error output.

  + 1: [default] DIET prints information on the main steps of a call.

  + 5: DIET prints information on all internal steps too.

  + 10: DIET prints all the communication structures too.

  + >10: (traceLevel - 10) is given to the ORB to print CORBA messages too.

**agentType**
  two possible values

  + DIET MASTER AGENT (or MA) for a Master Agent

  + DIET LOCAL AGENT (or LA) for a Local Agent

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

  [Remark: DIET must be compiled with FAST ]

**ldapUse**
  Boolean enabling/disabling LDAP support.

  [Remark: DIET must be compiled with FAST ]

**ldapBase**
  String representing the LDAP base storing FAST-known services address 
  in the form host:port .

  [Remark: DIET must be compiled with FAST ]

**ldapMask**
String specifying the mask registered in the LDAP base.

[Remark: DIET must be compiled with FAST ]

**nwsUse**
  Boolean enabling/disabling NWS support.

  [Remark: DIET must be compiled with FAST ]

**nwsNameserver**
  String representing the NWS naming service address in the form host:port .

  [Remark: DIET must be compiled with FAST ]

**nwsForecaster**
  String representing the NWS forecast module used by FAST.

  [Remark: DIET must be compiled with FAST ]

**useAsyncAPI** (DEPRECATED)

**useLogService**
  Boolean enabling/disabling the LogService for monitoring purposes.

**lsOutbuffersize**
  Integer setting outgoing messages buffer size.

**lsFlushinterval**
  Integer setting the flush interval for the outgoing messages buffer.

**neighbours**
  String listing MA that must be conntacted to build a federation. It is 
  formatted as a white-space separated list of addresses in the form host:port.

  [Remark: DIET must be compiled with the Multi-MA option.  ]

**minimumNeighbours**
  Integer setting the minimum connected neighbours. If the agent has less 
  connected neighbours, it will try establishing new connections.
  
  [Remark: DIET must be compiled with the Multi-MA option.  ]

**maximumNeighbours**
  Integer setting the maximum connected neighbours. Further, the agent will 
  refuse newer connections.

  [Remark: DIET must be compiled with the Multi-MA option.  ]

**updateLinkPeriod**
  Integer setting the period (in seconds) at which the agent will check its 
  neighbours status and will try establishing new connections if their numbers 
  is less than minimumNeighbours.

  [Remark: DIET must be compiled with the Multi-MA option.  ]

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

  [Remark: DIET must be compiled with the Batch option ]
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


USE SPECIFIC SCHEDULING
=======================

Specific options setting scheduler policy used by the client whenever it submits 
a request:

* **BURST REQUEST**: round robin on the available SeD

* **BURST  LIMIT**:  only allow a certain number of request per SeD in
  parallel the limit can be set with ”void setAllowedReqPerSeD(un‐
  signed ix)”

  [Remark:  DIET  must  be compiled with the Custom Client Scheduling (CCS) option ]
  [Remark: Option used by clients]

**clientMaxNbSeD**: 
  Integer value representing the maximum number of SeD 
  the client should receive.

  [Remark: Option used by clients.]

ENVIRONMENT
===========

DIET needs some variables to be defined in order for the agent to be able to 
find all the mandatory library and the CORBA naming service.

**LD_LIBRARY_PATH**
  This environment variable must contain the path to the omniORB libraries

**OMNIORB CONFIG**
  Path to the CORBA configuration file where the reference to the omniORB 
  naming service is written.

EXAMPLES
========

Here are examples of confguration file for  the  Master  Agent  or  Local
Agents.

* Configuration file for the Master Agent:

::

  # file MA example.cfg, configuration file for an MA
  agentType = DIET MASTER AGENT
  name = MA example
  #traceLevel = 1 # default
  #dietPort = <port> # not needed
  #dietHostname = <hostname|IP>
  # not needed fastUse = 1
  #ldapUse = 0 # default
  nwsUse = 1
  nwsNameserver = nwshost:9001
  #useLogService = 0 # default
  #lsOutbuffersize = 0 # default
  #lsFlushinterval = 10000 # default

* Configuration file for the Local Agent

::

  # file MA example.cfg, configuration file for an MA
  agentType = DIET MASTER AGENT
  name = MA example
  #traceLevel = 1 # default
  #dietPort = <port> # not needed
  #dietHostname = <hostname|IP> # not needed
  fastUse = 1
  #ldapUse = 0 # default
  nwsUse = 1
  nwsNameserver = nwshost:9001
  #useLogService = 0 # default
  #lsOutbuffersize = 0 # default
  #lsFlushinterval = 10000 # default

REPORTING BUGS
==============

If you find that software interesting, or if you find a bug,
please send us a mail : <diet-dev@ens-lyon.fr> with the descrip‐
tion of the problem, the version of the program and/or any infor‐
mation that could help us fixing it.

LICENSE AND COPYRIGHT
=====================
    
Copyright
---------    
(C)2010, GRAAL, INRIA Rhone-Alpes, 46 allee d'Italie, 69364 Lyon
cedex 07, France all right reserved <diet-dev@ens-lyon.fr>

License
-------
This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation, either version 3 of the
License, or (at your option) any later version. This program is
distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
for more details. You should have received a copy of the GNU
General Public License along with this program. If not, see
<http://www.gnu.org/licenses/>.

AUTHORS
=======
GRAAL
INRIA Rhone-Alpes
46 allee d'Italie 69364 Lyon cedex 07, FRANCE
Email: <diet-dev@ens-lyon.fr>
WWW: http://graal.ens-lyon.fr/DIET

SEE ALSO
========
omniNames(1), DIETForwarder(1)
