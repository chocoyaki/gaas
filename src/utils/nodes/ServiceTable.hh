/**
 * @file ServiceTable.hh
 *
 * @brief  DIET service table header (this is used by agents and SeDs)
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


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

class ServiceTable {
public:
  typedef int ServiceReference_t;

  typedef struct {
    CORBA::ULong nb_children;
    CORBA::ULong *children;
  } matching_children_t;

  // Equivalent to ServiceTable(MAX_NB_SERVICES, MAX_NB_CHILDREN)
  ServiceTable();

  // Equivalent to ServiceTable(max_nb_services, 0)
  ServiceTable(CORBA::ULong max_nb_services);
  // Allocate memory with given inital numbers, but:
  // - the solvers part of the table is nil if max_nb_children > 0
  // - the matching children part is nil either
  ServiceTable(CORBA::ULong max_nb_services, CORBA::ULong max_nb_children);

  virtual
  ~ServiceTable();

  CORBA::ULong
  maxSize();

  ServiceReference_t
  lookupService(const corba_profile_desc_t *sv_profile);
  ServiceReference_t
  lookupService(const corba_pb_desc_t *pb_desc);
  ServiceReference_t
  lookupService(const char *path, const corba_profile_t *pb);

  // All data structures are duplicated in add methods
  int
  addService(const corba_profile_desc_t *profile,
             const diet_convertor_t *const cvt,
             diet_solve_t solver,
             diet_eval_t evalf,
             diet_perfmetric_t perfmetric_fn);
  int
  addService(const corba_profile_desc_t *profile, const CORBA::ULong child);

  int
  rmService(const corba_profile_desc_t *profile);

  int
  rmService(const ServiceReference_t ref);

  int
  rmChildService(const corba_profile_desc_t *profile, CORBA::ULong childID);

  int
  rmChild(const CORBA::ULong child);

  const corba_profile_desc_t &
  getProfile(const ServiceReference_t index);
  // Return a pointer to a copy of all profiles.
  // Caller is responsible for freeing the result.
  SeqCorbaProfileDesc_t *
  getProfiles();

  SeqCorbaProfileDesc_t *
  getProfiles(CORBA::Long &length);

  diet_solve_t
  getSolver(const corba_profile_desc_t *profile);

  diet_solve_t
  getSolver(const ServiceReference_t ref);

  diet_eval_t
  getEvalf(const corba_profile_desc_t *profile);

  diet_eval_t
  getEvalf(const ServiceReference_t ref);

  diet_convertor_t *
  getConvertor(const corba_profile_desc_t *profile);

  diet_convertor_t *
  getConvertor(const ServiceReference_t ref);

  diet_perfmetric_t
  getPerfMetric(const corba_profile_desc_t *profile);

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
  getChildren(const corba_pb_desc_t *pb_desc,
              ServiceTable::ServiceReference_t serviceRef,
              CORBA::ULong *frontier);
#else
  const matching_children_t *
  getChildren(const ServiceReference_t ref);
#endif
  void
  dump(FILE *f);

  CORBA::ULong max_nb_children;

#if HAVE_ALT_BATCH
  int
  testIfAllParallelServices();
#endif

private:

  // number of couples {service, profile} in the table
  CORBA::ULong nb_s, max_nb_s, max_nb_s_step;
  // max number of capable children for one service
  // array of name and generic data description (a profile description)
  SeqCorbaProfileDesc_t profiles;
  // array of solving functions
  diet_solve_t *solvers;
  // array of evaluation functions
  diet_eval_t *eval_functions;
  // array of convertors (from a client pb profile to a "solved" pb profile)
  diet_convertor_t *convertors;
  // array of performance metric functions
  diet_perfmetric_t *perfmetrics;
  // array of int arrays: each element is an array of children ID,
  // which offer the corresponding service
  matching_children_t *matching_children;

  // private methods
  inline int
  ServiceTableInit(CORBA::ULong max_nb_services, CORBA::ULong max_nb_children);
};

#endif  // _SERVICETABLE_HH_
