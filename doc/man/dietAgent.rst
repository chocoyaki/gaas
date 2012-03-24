=========
dietAgent
=========

--------------------------
DIET grid middleware agent
--------------------------

:Author: david.loureiro@sysfera.com
:Date:   2010-09-07
:Copyright: DIET developers
:License: CeCILL
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

.. include:: diet_platform.rst

.. include:: corba_usage_diet.rst

OPTIONS
=======

**config_file**
  Configuration file used by the agent to launch the DIET entity

.. include:: config_file.rst

USE SPECIFIC SCHEDULING
=======================

Specific options setting scheduler policy used by the client whenever it submits 
a request:

* **BURST REQUEST**: round robin on the available SeD

* **BURST  LIMIT**:  only allow a certain number of request per SeD in
  parallel the limit can be set with "void setAllowedReqPerSeD(unsigned ix)"

  [Remark:  DIET  must  be compiled with the Custom Client Scheduling
  (CCS) option]

  [Remark: Option used by clients]

**clientMaxNbSeD**: 
  Integer value representing the maximum number of SeD 
  the client should receive.

  [Remark: Option used by clients]

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

Here are examples of configuration file for  the  Master  Agent  or  Local
Agents.

* Configuration file for the Master Agent:

::

  # file MA example.cfg, configuration file for an MA
  agentType = DIET_MASTER_AGENT
  name = MA
  #traceLevel = 2 # default
  #dietPort = <port> # not needed
  #dietHostname = <hostname|IP>
  useLogService = 0 # default
  lsOutbuffersize = 0 # default
  lsFlushinterval = 10000 # default

* Configuration file for the Local Agent

::

  # file LA example.cfg, configuration file for an LA
  agentType = DIET_LOCAL_AGENT
  name = LA
  useLogService = 0 # default
  lsOutbuffersize = 0 # default
  lsFlushinterval = 10000 # default

.. include:: reporting_bugs.rst

.. include:: licence_copyright.rst

.. include:: authors.rst

SEE ALSO
========
omniNames(1), dietForwarder(1), maDagAgent(1)
