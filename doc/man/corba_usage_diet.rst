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
