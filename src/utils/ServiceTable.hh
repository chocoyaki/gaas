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

#include <stdio.h>
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
  int
  rmChild(const CORBA::ULong child);

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
  const diet_convertor_t* const
  getConvertor(const corba_profile_desc_t* profile);
  diet_convertor_t*
  getConvertor(const ServiceReference_t ref);
  diet_perfmetric_t
  getPerfMetric(const corba_profile_desc_t* profile);
  diet_perfmetric_t
  getPerfMetric(const ServiceReference_t ref);
  matching_children_t*
  getChildren(const corba_profile_desc_t* profile);
  matching_children_t*
  getChildren(const ServiceReference_t ref);
  
  void
  dump(FILE* f);

  CORBA::ULong max_nb_children;

#if HAVE_BATCH
  int
  existBatchService() ;
  int 
  testIfAllBatchServices() ;
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



