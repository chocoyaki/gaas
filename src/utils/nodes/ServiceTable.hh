/****************************************************************************/
/* DIET service table header (this is used by agents and SeDs)              */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2010/08/04 09:06:21  glemahec
 * Parallel compilation
 *
 * Revision 1.3  2010/07/12 16:14:13  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.2  2010/03/31 21:15:41  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.20  2008/11/18 10:19:38  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 * Revision 1.19  2008/04/19 09:16:47  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Rewrite (and addition) of the propagation of the response concerning:
 *   job parallel_flag and serverType (batch or serial for the moment)
 * Complete debug info with batch stuff
 *
 * Revision 1.18  2008/04/07 15:33:44  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.17  2008/03/28 13:17:17  rbolze
 * update code to avoid warning with the intel compiler
 *
 * Revision 1.16  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.15  2006/09/18 19:46:08  ycaniou
 * Corrected a bug in file_transfer:server.c
 * Corrected memory leaks due to incorrect free of char *
 * ServiceTable prints if service is sequential or parallel
 * Fully complete examples, whith a batch, a parallel and a sequential server and
 *  a unique client
 *
 * Revision 1.14  2006/09/11 11:04:36  ycaniou
 * - Commented getChildren(const corba_profile_desc_t* profile) which is unused
 * - Added new function getChildren(const corba_pb_desc_t * pb_desc) used in AgentImpl.cc
 * - getChildren(const ServiceReference_t ref) becomes const
 *
 * Revision 1.13  2006/08/27 18:40:11  ycaniou
 * Modified parallel submission API
 * - client: diet_call_batch() -> diet_parallel_call()
 * - SeD: diet_profile_desc_set_batch() -> [...]_parallel()
 * - from now, internal fields are related to parallel not batch
 * and corrected a bug:
 * - 3 types of submission: request among only seq, only parallel, or all
 *   available services (second wasn't implemented, third bug)
 *
 * Revision 1.12  2005/04/27 01:49:41  ycaniou
 * Added the necessary for initialisation of batch profile, for profiles to
 * match
 * Added the functions diet_profile_set_parallel(), diet_profile_set_nbprocs(),
 * diet_profile_desc_set_batch(), diet_profile_desc_set_parallel() that the
 * client needs to define a parallel/batch job
 * Added the parsing of the batch scheduler name that must be provided in the
 * server configuration file, accordingly to the elagi library, plus some
 * checkings about the server that can only submit batch or non-batch jobs
 *
 * Revision 1.11  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.10.2.2  2004/11/30 13:57:12  alsu
 * minor problems during FAST testing on plugin schedulers
 *
 * Revision 1.10.2.1  2004/11/26 15:20:38  alsu
 * minor additions to enforce const-ness
 *
 * Revision 1.10  2004/10/06 16:42:01  rbolze
 * add function to getProfiles with the number of this profiles available
 *
 * Revision 1.9  2004/05/18 21:13:25  alsu
 * - added the perfmetrics field to the ServiceTable class to store
 *   custom performance metric functions
 *
 * - changed the ServiceTable::addService interface to require a
 *   performance metric function; NULL signifies the "old method" of
 *   performance estimation
 *     (TODO: decide whether the old interface needs to be reinstated)
 *
 * - added ServiceTable::getPerfMetric interface
 *
 * Revision 1.8  2003/04/10 12:51:36  pcombes
 * "son"->"child", and adapt to CORBA::ULong child IDs.
 *
 * Revision 1.7  2003/02/04 10:08:22  pcombes
 * Apply Coding Standards
 *
 * Revision 1.6  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 *
 * Revision 1.4  2002/10/15 18:41:39  pcombes
 * Implement convertor API.
 *
 * Revision 1.3  2002/10/03 17:58:21  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
 *
 * Revision 1.2  2002/08/30 16:50:16  pcombes
 * This version works as well as the alpha version from the user point of view,
 * but the API is now the one imposed by the latest specifications (GridRPC API
 * in its sequential part, config file for all parts of the platform, agent
 * algorithm, etc.)
 *  - Reduce marshalling by using CORBA types internally
 *  - Creation of a class ServiceTable that is to be replaced
 *    by an LDAP DB for the MA
 *  - No copy for client/SeD data transfers
 *  - ...
 ****************************************************************************/

#ifndef _SERVICETABLE_HH_
#define _SERVICETABLE_HH_

#include "DIET_server.h"

#include <cstdio>
#include <omniORB4/CORBA.h>

#include "common_types.hh"



/* This class should implement an STL "map" or "multimap" container.
   But I am not that performant with STL, so C++ experts, help ! */

/* This is initial numbers of children (for an agent) and offered services.
   They might be configured at compilation time or set at runtime. */
#define MAX_NB_CHILDREN 10
#define MAX_NB_SERVICES 20

class ServiceTable
{
  
public:
  typedef int ServiceReference_t;
  typedef struct {
    CORBA::ULong  nb_children;
    CORBA::ULong* children;
  } matching_children_t;

  // Equivalent to ServiceTable(MAX_NB_SERVICES, MAX_NB_CHILDREN)
  ServiceTable();
  // Equivalent to ServiceTable(max_nb_services, 0)  
  ServiceTable(CORBA::ULong max_nb_services);
  // Allocate memory with given inital numbers, but:
  //  - the solvers part of the table is nil if max_nb_children > 0
  //  - the matching children part is nil either
  ServiceTable(CORBA::ULong max_nb_services, CORBA::ULong max_nb_children);
  virtual
  ~ServiceTable();
  
  CORBA::ULong
  maxSize();
  
  ServiceReference_t
  lookupService(const corba_profile_desc_t* sv_profile);
  ServiceReference_t
  lookupService(const corba_pb_desc_t* pb_desc);
  ServiceReference_t
  lookupService(const char* path, const corba_profile_t* pb);

  // All data structures are duplicated in add methods
  /*
  int
  addService(const corba_profile_desc_t* profile, diet_convertor_t* cvt,
	     diet_solve_t solver, diet_eval_t evalf);
  */
  int
  addService(const corba_profile_desc_t* profile,
             const diet_convertor_t* const cvt,
             diet_solve_t solver,
             diet_eval_t evalf,
             diet_perfmetric_t perfmetric_fn);
  int
  addService(const corba_profile_desc_t* profile, const CORBA::ULong child);

  int
  rmService(const corba_profile_desc_t* profile);
  int
  rmService(const ServiceReference_t ref);
//#ifdef HAVE_DAGDA
  int
  rmChildService(const corba_profile_desc_t* profile, CORBA::ULong childID);
//#endif
  int
  rmChild(const CORBA::ULong child);

  const corba_profile_desc_t &
  getProfile( const ServiceReference_t index ) ;
  // Return a pointer to a copy of all profiles.
  // Caller is responsible for freeing the result.
  SeqCorbaProfileDesc_t*
  getProfiles();
  SeqCorbaProfileDesc_t*
  getProfiles(CORBA::Long& length);
  diet_solve_t
  getSolver(const corba_profile_desc_t* profile);
  diet_solve_t
  getSolver(const ServiceReference_t ref);
  diet_eval_t
  getEvalf(const corba_profile_desc_t* profile);
  diet_eval_t
  getEvalf(const ServiceReference_t ref);
  diet_convertor_t*
  getConvertor(const corba_profile_desc_t* profile);
  diet_convertor_t*
  getConvertor(const ServiceReference_t ref);
  diet_perfmetric_t
  getPerfMetric(const corba_profile_desc_t* profile);
  diet_perfmetric_t
  getPerfMetric(const ServiceReference_t ref);
  /* Unused!
  matching_children_t*
    getChildren(const corba_profile_desc_t* profile);
  */
#if defined HAVE_ALT_BATCH
  /* Returns the list of children that can solve parallel and/or sequential
     task, depending on parallel flag of profile
     => Caller must desallocate the resulting memory! 

     Returns the concatanation of the
     matching children corresponding to requested service depending on
     parallel flag in \c pb_desc. For value strict. inferior to \c frontier,
     children correspond to profiles with parallel flag equal to 1, which is
     known with the help of \c serviceRef
  */

  ServiceTable::matching_children_t *
  getChildren(const corba_pb_desc_t * pb_desc,
	      ServiceTable::ServiceReference_t serviceRef,
	      CORBA::ULong * frontier);
#else
  const matching_children_t*
  getChildren(const ServiceReference_t ref);
#endif
  void
  dump(FILE* f);

  CORBA::ULong max_nb_children;

#if HAVE_ALT_BATCH
  int 
  testIfAllParallelServices() ;
#endif

private:
  
  // number of couples {service,profile} in the table
  CORBA::ULong nb_s, max_nb_s, max_nb_s_step;
  // max number of capable children for one service
  // array of name and generic data description (a profile description)
  SeqCorbaProfileDesc_t profiles;
  // array of solving functions 
  diet_solve_t* solvers;
  // array of evaluation functions 
  diet_eval_t* eval_functions;
  // array of convertors (from a client pb profile to a "solved" pb profile) 
  diet_convertor_t* convertors;
  // array of performance metric functions
  diet_perfmetric_t* perfmetrics;
  // array of int arrays: each element is an array of children ID, which offer the
  // corresponding service
  matching_children_t* matching_children;

  // private methods
  inline int
  ServiceTableInit(CORBA::ULong max_nb_services, CORBA::ULong max_nb_children);
};

#endif // _SERVICETABLE_HH_



