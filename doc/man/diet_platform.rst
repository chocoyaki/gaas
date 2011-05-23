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

**Master Agent DAG (MA DAG)**
  The Master Agent DAG (MADAG) provides DAG workflow scheduling. This
  agent serves as the entry point to the Diet Hierarchy for a client
  that wants to submit a workflow. The language supported by the MADAG
  is based on XML.
