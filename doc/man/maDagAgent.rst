==========
maDagAgent
==========

--------------------------
DIET grid middleware agent
--------------------------

:Author: benjamin.depardon@sysfera.com
:Date:   2011-05-23
:Copyright: DIET developers
:License: GPLv3+
:Version: 0.1
:Manual section: 1
:Manual group:

NAME
====

maDagAgent - Main binary of the DIET Architecture for managing workflows

SYNOPSYS
========

  maDagAgent config_file [sched] [pfm] [IRD] ...

DESCRIPTION
===========

The maDagAgent is the daemon responsible for the management of
workflows execution.

.. include:: diet_platform.rst

.. include:: corba_usage_diet.rst

OPTIONS
=======

**config_file**
  Configuration file used by the agent to launch the DIET entity

**sched**
  The policy used for scheduling workflows. This option can take the
  following values:

  + -basic (default):

  + -g_heft:

  + -g_aging_heft:

  + -fairness:

  + -srpt: 

  + -fcfs: first come first serve

**pfm**
  + -pfm_any (default)

  + -pfm_sameservices

**IRD**
  + -IRD value


.. include:: config_file.rst


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

Here are examples of configuration file for the MA DAG Agent.

::

  traceLevel = 2
  agentType = DIET_MA_DAG
  name = mad
  parentName = MA1


.. include:: reporting_bugs.rst

.. include:: licence_copyright.rst

.. include:: authors.rst

SEE ALSO
========
omniNames(1), dietAgent(1), dietForwarder(1)
