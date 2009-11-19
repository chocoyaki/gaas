/****************************************************************************/
/* DIET service table source code (this is used by agents and SeDs)         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.34  2009/11/19 07:27:41  ycaniou
 * Remove warnings
 *
 * Revision 1.33  2009/11/19 06:31:54  ycaniou
 * Correct initializing bug in AddService
 *
 * Revision 1.32  2009/10/17 08:28:08  bdepardo
 * Removed compilation warnings
 *
 * Revision 1.31  2009/04/22 06:21:21  ycaniou
 * Correct a bug: when adding at least 20 services, matching_children[20] wasn't
 *   initialized and matching_children[20].children wasn't allocated thus a
 *   seg fault.
 *
 * Revision 1.30  2008/11/18 10:19:37  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 * Revision 1.29  2008/11/08 19:12:38  bdepardo
 * A few warnings removal
 *
 * Revision 1.28  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.27  2008/04/19 09:16:46  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Rewrite (and addition) of the propagation of the response concerning:
 *   job parallel_flag and serverType (batch or serial for the moment)
 * Complete debug info with batch stuff
 *
 * Revision 1.26  2008/04/07 15:33:44  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.25  2008/03/28 13:17:17  rbolze
 * update code to avoid warning with the intel compiler
 *
 * Revision 1.24  2008/02/21 12:05:20  rbolze
 * add tips info in order to help for debugging :)
 *
 * Revision 1.23  2008/01/01 19:43:49  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 * Revision 1.22  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.21  2007/03/26 13:44:48  glemahec
 * Adds the DIET_AGG_USER case to the "addService" method.
 * Very little change.
 *
 * Revision 1.20  2006/09/18 19:46:08  ycaniou
 * Corrected a bug in file_transfer:server.c
 * Corrected memory leaks due to incorrect free of char *
 * ServiceTable prints if service is sequential or parallel
 * Fully complete examples, whith a batch, a parallel and a sequential server and
 *  a unique client
 *
 * Revision 1.19  2006/09/11 11:04:36  ycaniou
 * - Commented getChildren(const corba_profile_desc_t* profile) which is unused
 * - Added new function getChildren(const corba_pb_desc_t * pb_desc) used in AgentImpl.cc
 * - getChildren(const ServiceReference_t ref) becomes const
 *
 * Revision 1.18  2006/08/27 18:40:11  ycaniou
 * Modified parallel submission API
 * - client: diet_call_batch() -> diet_parallel_call()
 * - SeD: diet_profile_desc_set_batch() -> [...]_parallel()
 * - from now, internal fields are related to parallel not batch
 * and corrected a bug:
 * - 3 types of submission: request among only seq, only parallel, or all
 *   available services (second wasn't implemented, third bug)
 *
 * Revision 1.17  2005/08/30 07:24:23  ycaniou
 * Changed the test in profile_match to enable the possibility for DIET to
 *   decide if a 'normal' job sould be submitted via batch or not.
 * Add the parsing of 'batchName' in config file.
 * Some type precisions in estVector (but real code untouched) because of
 *   compilation warnings I had.
 *
 * Revision 1.16  2005/05/15 15:43:55  alsu
 * ensure aggregators are equivalent when registering services
 *
 * Revision 1.15  2005/04/27 01:49:41  ycaniou
 * Added the necessary for initialisation of batch profile, for profiles to
 * match
 * Added the functions diet_profile_set_parallel(), diet_profile_set_nbprocs(),
 * diet_profile_desc_set_batch(), diet_profile_desc_set_parallel() that the
 * client needs to define a parallel/batch job
 * Added the parsing of the batch scheduler name that must be provided in the
 * server configuration file, accordingly to the elagi library, plus some
 * checkings about the server that can only submit batch or non-batch jobs
 *
 * Revision 1.14  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.13  2004/11/25 21:26:27  hdail
 * Changed initial allocation of solvers, eval_functions, convertors, and
 * perfmetrics from new to malloc to match our use of realloc for resizing.
 *
 * Revision 1.12.2.2  2004/11/30 13:57:12  alsu
 * minor problems during FAST testing on plugin schedulers
 *
 * Revision 1.12.2.1  2004/11/26 15:20:38  alsu
 * minor additions to enforce const-ness
 *
 * Revision 1.12  2004/10/06 16:42:01  rbolze
 * add function to getProfiles with the number of this profiles available
 *
 * Revision 1.11  2004/05/18 21:13:25  alsu
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
 * Revision 1.10  2003/12/01 14:49:31  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.9  2003/07/04 09:48:06  pcombes
 * Use new ERROR and WARNING macros.
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
 ****************************************************************************/


#include "ServiceTable.hh"
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <string.h>

#include "debug.hh"
#include "DIET_data_internal.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define SRVT_ERROR(formatted_text)                \
  INTERNAL_ERROR("ServiceTable::" << __FUNCTION__ \
                 << ": " << formatted_text, 1)


ServiceTable::ServiceTable() 
{
  ServiceTableInit(MAX_NB_SERVICES, MAX_NB_CHILDREN);
}

ServiceTable::ServiceTable(CORBA::ULong max_nb_services) 
{
  ServiceTableInit(max_nb_services, 0);
}

ServiceTable::ServiceTable(CORBA::ULong max_nb_services,
                           CORBA::ULong max_nb_children)
{
  ServiceTableInit(max_nb_services, max_nb_children);
}

ServiceTable::~ServiceTable()
{
  if (solvers) {
    for (size_t i = 0; i < max_nb_s; i++) {
      free(profiles[i].path);
      profiles[i].param_desc.length(0);
      free(convertors[i].path);
      delete [] convertors[i].arg_convs;
    }
    free(solvers);
    free(eval_functions);
    free(convertors);
    free(perfmetrics);
  } else if (matching_children) {
    for (size_t i = 0; i < max_nb_s; i++) {
      free(profiles[i].path);
      profiles[i].param_desc.length(0);
      free(matching_children[i].children);
    }
    free(matching_children);
  }
  profiles.length(0);
}

CORBA::ULong
ServiceTable::maxSize()
{
  return max_nb_s;
}

ServiceTable::ServiceReference_t
ServiceTable::lookupService(const corba_profile_desc_t* sv_profile)
{
  /* Called from ServiceTable::addService(), rmService(), getSolver(),
     getEvalf(), getConvertor(), getPerfMetric(), getChildren()
  ** DIET_server::diet_service_table_lookup_by_profile()
  ** Cori_Fast::diet_service_table_lookup_by_profile() 
  */
  size_t i(0);
  for (; (i < nb_s) && (!profile_desc_match(&(profiles[i]), sv_profile)); i++) ;
  return (ServiceReference_t) ((i == nb_s) ? -1 : (int)i);
}

ServiceTable::ServiceReference_t
ServiceTable::lookupService(const corba_pb_desc_t* pb_desc)
{
  /* Called from MasterAgent::submitLocal(), AgentImpl::findServer()
  ** SeDImpl::getRequest(), SeDImpl::checkContract()
  ** Thus, the check must be performed in special way concerning parallel tasks
  **  - if parallel or sequential is asked, strict check
  **  - if nothing specified, both // and non-// must be considered
  */
  size_t i(0);
  for (; (i < nb_s) && (!profile_match(&(profiles[i]), pb_desc)); i++) ;
  return (ServiceReference_t) ((i == nb_s) ? -1 : (int)i);
}

ServiceTable::ServiceReference_t
ServiceTable::lookupService(const char* path, const corba_profile_t* pb)
{
  /* Called from SeDImpl::solve() and solveAsync() */
  size_t i(0);
  for (; (i < nb_s) && (!profile_match(&(profiles[i]), path, pb)); i++) ;
  return (ServiceReference_t) ((i == nb_s) ? -1 : (int)i);
}

int
ServiceTable::addService(const corba_profile_desc_t* profile,
                         const diet_convertor_t* const cvt,
                         diet_solve_t solver,
                         diet_eval_t evalf,
                         diet_perfmetric_t perfmetric_fn)
{
  /* Called from DIET_server::diet_service_table_add */
  ServiceReference_t service_idx(-1);

  if (matching_children) {
    SRVT_ERROR("attempting to add a service with\n"
               << "  solver in a table initialized with children");
  }

  if ((service_idx = lookupService(profile)) == -1) {
    // Then add a brand new service
    if (nb_s == max_nb_s) {
      max_nb_s += max_nb_s_step;
      profiles.length(max_nb_s);
      solvers = (diet_solve_t *)
        realloc(solvers, max_nb_s * sizeof(diet_solve_t));
      eval_functions = (diet_eval_t *)
        realloc(eval_functions, max_nb_s * sizeof(diet_eval_t));
      convertors = (diet_convertor_t *)
        realloc(convertors, max_nb_s * sizeof(diet_convertor_t));
      perfmetrics = (diet_perfmetric_t *)
        realloc(perfmetrics, max_nb_s * sizeof(diet_perfmetric_t));
      for (size_t i = nb_s; i < max_nb_s; i++) {
        profiles[i].param_desc.length(0);
        solvers[i]              = NULL;
        eval_functions[i]       = NULL;
        convertors[i].arg_convs = NULL;
        perfmetrics[i]          = NULL;
      }
    }

    profiles[nb_s]             = *profile; // deep copy
    solvers[nb_s]              = solver;
    eval_functions[nb_s]       = evalf;
    // duplicate path and arg_convs, since the user should deallocate them with
    // diet_convertor_free.
    convertors[nb_s]            = *cvt;
    convertors[nb_s].path       = strdup(cvt->path);
    convertors[nb_s].arg_convs  = new diet_arg_convertor_t[cvt->last_out + 1];
    for (int i = 0; i <= cvt->last_out; i++) {
      convertors[nb_s].arg_convs[i] = cvt->arg_convs[i];
    }
    perfmetrics[nb_s]          = perfmetric_fn;
    nb_s++;
  } else if (solver == solvers[(size_t)service_idx]) {
    return -1;
    // service is already in table
  } else {
    SRVT_ERROR("attempting to add 2 services with\n"
               << "  same path and profile, but with different solvers");
  }
  return 0;
}

/* Called from AgentImpl::addServices() */
int
ServiceTable::addService(const corba_profile_desc_t* profile,
                         CORBA::ULong child)
{
  ServiceReference_t service_idx(-1);
  
  if (solvers) {
    SRVT_ERROR("attempting to add a service with\n"
               << "  child in a table initialized with solvers");
  }

  if ((service_idx = lookupService(profile)) == -1) {
    // Then add a brand new service
    if( (nb_s != 0) && (nb_s % max_nb_s) == 0 ) {
      max_nb_s += max_nb_s_step;
      profiles.length(max_nb_s);
      matching_children = (matching_children_t*)
        realloc(matching_children, max_nb_s * sizeof(matching_children_t));
      for (size_t i = nb_s ; i < max_nb_s; i++) {
        profiles[i].param_desc.length(0);
        matching_children[i].nb_children = 0;
        matching_children[i].children    = new CORBA::ULong[max_nb_children];
      }
    }
    profiles[nb_s] = *profile; // deep copy
    matching_children[nb_s].children[matching_children[nb_s].nb_children++]
      = child;
    nb_s++;
  
  } else {

    { /* verify that the aggregators are equivalent */
      corba_profile_desc_t *storedProfile = &(profiles[service_idx]);
      const corba_aggregator_desc_t *a1 = &(storedProfile->aggregator);
      const corba_aggregator_desc_t *a2 = &(profile->aggregator);
      if (a1->agg_specific._d() != a2->agg_specific._d()) {
        ERROR(__FUNCTION__ << ": aggregator type mismatch\n", -2);
      }
      switch (a1->agg_specific._d()) {
/* New : The user aggregator case.                           */
/*       It doesn't need more than the default aggregator... */
#ifdef USERSCHED
	  case DIET_AGG_USER:
#endif
      case DIET_AGG_DEFAULT:
        break;
      case DIET_AGG_PRIORITY:
        {
          const corba_agg_priority_t *p1 = &(a1->agg_specific.agg_priority());
          const corba_agg_priority_t *p2 = &(a2->agg_specific.agg_priority());
          if (p1->priorityList.length() != p2->priorityList.length()) {
            ERROR(__FUNCTION__ <<
                  ": priority list length mismatch (" <<
                  p1->priorityList.length() <<
                  " != " <<
                  p2->priorityList.length() <<
                  ")\n", -2);
          }
          for (unsigned int pvIter = 0 ;
               pvIter < p1->priorityList.length() ;
               pvIter++) {
            if (p1->priorityList[pvIter] != p2->priorityList[pvIter]) {
              ERROR(__FUNCTION__ <<
                    ": priority list value mismatch, index " <<
                    pvIter <<
                    " (" <<
                    p1->priorityList[pvIter] <<
                    " != " <<
                    p2->priorityList[pvIter] <<
                    ")\n" <<
		    "TIPS : check diet_aggregator_priority_ function in all SeD", -2);
            }
          }
        }
        break;
      default:
        ERROR(__FUNCTION__ <<
              ": unexpected aggregator type (" <<
              a1->agg_specific._d(), -2);
      }
    }

    CORBA::ULong nb_children =
      matching_children[(size_t)service_idx].nb_children;
    CORBA::ULong* children   = matching_children[(size_t)service_idx].children;
    
    for (size_t i = 0; i < nb_children; i++)
      if (children[i] == child)
        return -1; // service already associated to child
    // Here, we must add the child in matching_children[service_idx].children
    if ((nb_children % max_nb_children) == 0) {
      // Then realloc children array
      children = (CORBA::ULong*)
        realloc(children,
                (nb_children + max_nb_children) * sizeof(CORBA::ULong));
      matching_children[service_idx].children = children;
    }
    children[nb_children] = child;
    matching_children[service_idx].nb_children++;
  }
  return 0;
}



int
ServiceTable::rmService(const corba_profile_desc_t* profile)
{
  ServiceReference_t ref(-1);
  
  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR("attempting to rm a service that is not in table");
  }
  return this->rmService(ref);
}


int
ServiceTable::rmService(const ServiceReference_t ref)
{
  size_t i(0);

  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  
  if (solvers) {
    profiles[ref].param_desc.length(0);
    for (i = (size_t) ref; i < (nb_s - 1); i++) {
      profiles[i]        = profiles[i+1];
      solvers[i]         = solvers[i+1];
      eval_functions[i]  = eval_functions[i+1];
      convertors[i]      = convertors[i+1];
      perfmetrics[i]     = perfmetrics[i+1];
    }
    profiles[i].param_desc.length(0);
    solvers[i]              = NULL;
    eval_functions[i]       = NULL;
    convertors[i].arg_convs = NULL;
    perfmetrics[i]          = NULL;
  } else {
    profiles[ref].param_desc.length(0);
    free(matching_children[ref].children);
    for (i = (size_t) ref; i < (nb_s - 1); i++) {
      profiles[i] = profiles[i+1];
      matching_children[i] = matching_children[i+1];
    }
    profiles[i].param_desc.length(0);
    matching_children[i].nb_children = 0;
    matching_children[i].children = new CORBA::ULong[max_nb_children];

  }
  
  nb_s--;
  return 0;
}


#ifdef HAVE_DAGDA
/** removes a service on a given child */
int
ServiceTable::rmChildService(const corba_profile_desc_t* profile, CORBA::ULong childID)
{
  ServiceReference_t ref(-1);
  size_t i(0), j(0);
  
  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR("attempting to rm a service that is not in table");
  }

  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  
  if (solvers) {
    /* Nothing to do
     * Should we even be able to call this?? */
  } else {
    /* We need to verify that this child exists for this service */
    for (i = (size_t) 0;
	 i < matching_children[ref].nb_children && matching_children[ref].children[i] != childID;
	 i ++) ;

    /* if we didn't found it return -1 */
    if (i == matching_children[ref].nb_children) {
      SRVT_ERROR("attempting to rm a service to a child which do not possess it");
    }

    /* If the child is the only one to possess this service, we remove it completely */
    if (matching_children[ref].nb_children == 1)
      return this->rmService(ref);

    /* Otherwise we need to shift the array of children */
    for (j = i; j < matching_children[ref].nb_children; j++) {
      matching_children[ref].children[j] = matching_children[ref].children[j+1];
    }
    matching_children[ref].nb_children --;
  }

  return 0;
}
#endif // HAVE_DAGDA


int
ServiceTable::rmChild(const CORBA::ULong child)
{
  ServiceReference_t ref(-1);
  
  if (solvers) {
    SRVT_ERROR("attempting to remove a child from\n"
               << "  a table initialized with solvers");
  }
  for (ref = 0; (size_t)ref < nb_s; ref++) {
    size_t i;
    for (i = 0 ;
         ((i < matching_children[ref].nb_children) &&
          (matching_children[(size_t)ref].children[i] != child)) ;
         i++) ;
    if (i < matching_children[ref].nb_children) {
      for (size_t j = i; j < matching_children[ref].nb_children; j++) {
        matching_children[ref].children[j]
          = matching_children[ref].children[j+1];
      }
      // FIXME: Is it necessary ?
      // Remove service if there is no child left
      //if ((--(matching_children[ref].nb_children)) == 0)
      //rmService(ref);
      // replaced by:
      (matching_children[ref].nb_children)--;
    }
  }
  return 0;
}

//#if defined HAVE_ALT_BATCH
/* This method does NOT test the validity of the range index */
const corba_profile_desc_t &
ServiceTable::getProfile( const ServiceReference_t index )
{
  return profiles[ index ] ;
}
//#endif

SeqCorbaProfileDesc_t*
ServiceTable::getProfiles()
{
  SeqCorbaProfileDesc_t* res = new SeqCorbaProfileDesc_t(nb_s);
  res->length(nb_s);
  for (size_t i = 0; i < nb_s; i++) {
    (*res)[i] = profiles[i];
  }
  return res;
}

SeqCorbaProfileDesc_t*
ServiceTable::getProfiles(CORBA::Long& length)
{
  SeqCorbaProfileDesc_t* res = new SeqCorbaProfileDesc_t(nb_s);
  res->length(nb_s);
  for (size_t i = 0; i < nb_s; i++) {
    (*res)[i] = profiles[i];
  }
  length = nb_s;
  return res;
}



diet_solve_t
ServiceTable::getSolver(const corba_profile_desc_t* profile)
{
  ServiceReference_t ref(-1);
  
  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR("attempting to get solver\n"
               << "  of a service that is not in table");
  }
  return getSolver(ref);
}


diet_solve_t
ServiceTable::getSolver(const ServiceReference_t ref)
{
  if (!solvers) {
    SRVT_ERROR("attempting to get a solver\n"
               << "  in a table initialized with children");
  }
  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  return solvers[ref];
}


diet_eval_t
ServiceTable::getEvalf(const corba_profile_desc_t* profile)
{
  ServiceReference_t ref;
  
  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR("attempting to get eval function\n"
               << "  of a service that is not in table");
  }
  return getEvalf(ref);
}


diet_eval_t
ServiceTable::getEvalf(const ServiceReference_t ref)
{
  if (!solvers) {
    SRVT_ERROR("attempting to get an eval function\n"
               << "  in a table initialized with children");
  }
  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  return eval_functions[ref];
}


diet_convertor_t* 
ServiceTable::getConvertor(const corba_profile_desc_t* profile)
{
  ServiceReference_t ref(-1);
  
  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR("attempting to get convertor\n"
               << "  of a service that is not in table");
  }
  return getConvertor(ref);
}


diet_convertor_t*
ServiceTable::getConvertor(const ServiceReference_t ref)
{
  if (!solvers) {
    SRVT_ERROR("attempting to get a convertor\n"
               << "  in a table initialized with children");
  }
  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }

  return &(convertors[ref]);
}

diet_perfmetric_t
ServiceTable::getPerfMetric(const corba_profile_desc_t* profile)
{
  ServiceReference_t ref(-1);
  
  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR("attempting to get performance metric\n"
               << "  of a service that is not in table");
  }
  return (this->getPerfMetric(ref));
}


diet_perfmetric_t
ServiceTable::getPerfMetric(const ServiceReference_t ref)
{
  if (!solvers) {
    SRVT_ERROR("attempting to get a performance metric\n"
               << "  in a table initialized with children");
  }
  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  return (this->perfmetrics[ref]);
}

/* Unused
   ServiceTable::matching_children_t*
   ServiceTable::getChildren(const corba_profile_desc_t* profile)
   {
   ServiceReference_t ref(-1);
   
   if ((ref = lookupService(profile)) == -1) {
   SRVT_ERROR("attempting to get children\n"
   << "  of a service that is not in table");
   }
   return getChildren(ref);
   }
*/
#if defined HAVE_ALT_BATCH
ServiceTable::matching_children_t *
ServiceTable::getChildren(const corba_pb_desc_t * pb_desc,
			  ServiceTable::ServiceReference_t serviceRef,
			  CORBA::ULong * frontier)
{
  if (solvers) {
    SRVT_ERROR("attempting to get children\n"
               << "  in a table initialized with solvers");
  }
  if (((int) serviceRef < 0) || ((size_t) serviceRef >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  
  int first_found = -1, second_found = -1 ; // at most, two indices: // and seq
  size_t i(0), j(0) ;
  ServiceTable::matching_children_t * mc = NULL ;
  
  /* Search for 1rst occurence of service in table */  
  while( (i < nb_s) && (!profile_match(&(profiles[i]), pb_desc)) )
    i++ ;
  if( i == nb_s ) {
    SRVT_ERROR("attempting to get children\n"
               << "  of a service that is not in table");
  }
  first_found = i ;
  mc = new ServiceTable::matching_children_t() ;
  if( mc == NULL ) {
    SRVT_ERROR("Not enough memory") ;
  }

  if( pb_desc->parallel_flag == 0 ) { /* Test if there is same profile with
					 different parallel flag */
    i++ ;
    /* Search for 2nd occurence of service in table */  
    while( (i < nb_s) && (!profile_match(&(profiles[i]), pb_desc)) )
      i++ ;
    if( i== nb_s ) /* No new occurence */
      mc->nb_children = 
	matching_children[ first_found ].nb_children ;
    else {
      second_found = i ;
      mc->nb_children = 
	matching_children[ first_found ].nb_children 
	+ matching_children[ second_found ].nb_children ;
    }
    /* Reserve memory for all children */
    mc->children =
      new CORBA::ULong[mc->nb_children] ;
  
    /* Copy children, ordered parallel flag = 1 first */
    if( profiles[ serviceRef ].parallel_flag == 1 ) {
#ifdef YC_DEBUG
      cout << "Identities of the children for seq service\n" ;
#endif
      for( i=0; i<matching_children[ first_found ].nb_children ; i++ ) {
	mc->children[ i ] =
	  matching_children[ first_found ].children[ i ] ;
#ifdef YC_DEBUG
	cout << matching_children[ first_found ].children[ i ] ;
#endif
      }
#ifdef YC_DEBUG
	cout << "\n" ;
#endif
      if( second_found > 0 ) {
#ifdef YC_DEBUG
	cout << "Identities of the children for parallel service\n" ;
#endif
	for(j=0 ; j<matching_children[ second_found ].nb_children ; i++, j++) {
	  mc->children[ i ] =
	    matching_children[ second_found ].children[ j ] ;
#ifdef YC_DEBUG
	cout << matching_children[ second_found ].children[ j ] ;
#endif
	}
#ifdef YC_DEBUG
	cout << "\n" ;
#endif
      }
      /* set frontier */
      (*frontier)=matching_children[ first_found ].nb_children ;
    } else {
      if( second_found > 0 ) {
#ifdef YC_DEBUG
      cout << "Identities of the children for seq service\n" ;
#endif
	for( i=0; i<matching_children[ second_found ].nb_children ; i++ ) {
	  mc->children[ i ] =
	    matching_children[ second_found ].children[ i ] ;
#ifdef YC_DEBUG
	  cout << matching_children[ second_found ].children[ i ] ;
#endif
	}
#ifdef YC_DEBUG
	cout << "\n" ;
#endif
	/* set frontier */
	(*frontier)=matching_children[ second_found ].nb_children ;
      } else {
#ifdef YC_DEBUG
      cout << "Identities of the children for parallel service\n" ;
#endif 
      for( j=0, i=0 ; j<matching_children[ first_found ].nb_children ;
	   i++, j++ ) {
	mc->children[ i ] =
	  matching_children[ first_found ].children[ j ] ;
#ifdef YC_DEBUG
	cout << matching_children[ first_found ].children[ j ] ;
#endif
      }
#ifdef YC_DEBUG
	cout << "\n" ;
#endif
      /* set frontier */
      (*frontier)=0 ;
      }
    }
  } else { /* Only interested by a given profile ( seq ORexclusive // )
	      then first match is unique match */
    mc->nb_children = 
      matching_children[ first_found ].nb_children ;
    mc->children =
      new CORBA::ULong[mc->nb_children] ;
#ifdef YC_DEBUG
    cout << "Identities of the children for service\n" ;
#endif
    for( i=0; i<matching_children[ first_found ].nb_children ; i++ ) {
      mc->children[ i ] =
	matching_children[ first_found ].children[ i ] ;
#ifdef YC_DEBUG
      cout << matching_children[ first_found ].children[ i ] ;
#endif
    }
#ifdef YC_DEBUG
	cout << "\n" ;
#endif
    /* set frontier */
    if( profiles[ serviceRef ].parallel_flag == 1 )
      (*frontier)= matching_children[ first_found ].nb_children ;
    else
      (*frontier)= 0 ;
  }

  return mc ;
}
#else
const ServiceTable::matching_children_t*
ServiceTable::getChildren(const ServiceReference_t ref)
{
  if (solvers) {
    SRVT_ERROR("attempting to get children\n"
               << "  in a table initialized with solvers");
  }
  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  return &(matching_children[ref]);
}
#endif

void
ServiceTable::dump(FILE* f)
{
  char path[257];
  
  // FIXME: stat the FILE to check if write OK (take care that this method can
  // be called on stdout !

  fprintf(f, "\n--------------------------------------------------\n");
  fprintf(f,   "Service Table (%lu services)\n", (unsigned long)nb_s);
  fprintf(f,   "--------------------------------------------------\n\n");

  for (size_t i = 0; i < nb_s; i++) {
    strcpy(path, profiles[i].path);
    fprintf(f, "- Service %s", path);
#if defined HAVE_ALT_BATCH
    if( profiles[i].parallel_flag == 2 )
      fprintf(f," (parallel service) ") ;
    else if ( profiles[i].parallel_flag == 1 )
      fprintf(f," (sequential service) ") ;
    else fprintf(f," Error? ") ;
#endif

    if (matching_children) {
      fprintf(f, " offered by ");
      if (matching_children[i].nb_children == 0)
        fprintf(f, "no child");
      else {
        if (matching_children[i].nb_children == 1)
          fprintf(f, "child %lu", matching_children[i].children[0]);
        else {
          size_t j;
          fprintf(f, "children %lu", matching_children[i].children[0]);
          for (j = 1; j < (matching_children[i].nb_children - 1); j++)
            fprintf(f, ", %lu", matching_children[i].children[j]);
          // Re-use j to shorten next line
          fprintf(f, " and %lu.", matching_children[i].children[j]);
        }
      }
    }

    for (size_t j = 0; (int)j <= profiles[i].last_out; j++) {
      fprintf(f, "\n     %s ",
              ((int)j <= profiles[i].last_in) ? "IN   "
              : ((int)j <= profiles[i].last_inout) ? "INOUT"
              : "OUT  ");
      displayArgDesc(f, profiles[i].param_desc[j].type,
                     profiles[i].param_desc[j].base_type);
    }
    fprintf(f, "\n");

    if (!matching_children)
      displayConvertor(f, &(convertors[i]));
  }
}


int
ServiceTable::ServiceTableInit(CORBA::ULong max_nb_services,
                               CORBA::ULong max_nb_children)
{
  nb_s          = 0;
  max_nb_s      = (max_nb_services == 0) ? MAX_NB_SERVICES : max_nb_services;
  max_nb_s_step = max_nb_s;
  this->max_nb_children = max_nb_children;
  profiles.length(max_nb_s);

  if (max_nb_children > 0) {
    solvers        = NULL;
    eval_functions = NULL;
    convertors     = NULL;
    perfmetrics    = NULL;
    matching_children  = new matching_children_t[max_nb_s];
    for (size_t i = 0; i < max_nb_s; i++) {
      profiles[i].param_desc.length(0);
      matching_children[i].nb_children = 0;
      matching_children[i].children    = new CORBA::ULong[max_nb_children];
    }
  } else {
    solvers        = (diet_solve_t *) 
        malloc(max_nb_s * sizeof(diet_solve_t));
    eval_functions = (diet_eval_t *) 
        malloc(max_nb_s * sizeof(diet_eval_t));
    convertors     = (diet_convertor_t *) 
        malloc(max_nb_s * sizeof(diet_convertor_t));
    perfmetrics    = (diet_perfmetric_t *) 
        malloc(max_nb_s * sizeof(diet_perfmetric_t));
    for (size_t i = 0; i < max_nb_s; i++) {
      profiles[i].param_desc.length(0);
      solvers[i]              = NULL;
      eval_functions[i]       = NULL;
      convertors[i].arg_convs = NULL;
      perfmetrics[i]          = NULL;
    }
    matching_children = NULL;
  }
  return 0;
}

#if defined HAVE_ALT_BATCH
int
ServiceTable::testIfAllParallelServices()
{
  size_t i=1 ;

  while( (i<nb_s) && (profiles[0].parallel_flag == profiles[i].parallel_flag) )
    i++ ;
  if( i==nb_s )
    return ( profiles[0].parallel_flag == 2 ) ;
  else
    return -1 ;
}
#endif
