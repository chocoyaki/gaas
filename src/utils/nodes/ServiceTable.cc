/**
 * @file ServiceTable.cc
 *
 * @brief  DIET service table source code (this is used by agents and SeDs)
 *
 * @author  - Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#include "ServiceTable.hh"
#include <iostream>
using namespace std;
#include <cstdlib>
#include <cstring>

#include "debug.hh"
#include "DIET_data_internal.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define SRVT_ERROR(formatted_text)                      \
  INTERNAL_WARNING("ServiceTable::" << __FUNCTION__     \
                                    << ": " << formatted_text)


ServiceTable::ServiceTable() {
  ServiceTableInit(MAX_NB_SERVICES, MAX_NB_CHILDREN);
}

ServiceTable::ServiceTable(CORBA::ULong max_nb_services) {
  ServiceTableInit(max_nb_services, 0);
}

ServiceTable::ServiceTable(CORBA::ULong max_nb_services,
                           CORBA::ULong max_nb_children) {
  ServiceTableInit(max_nb_services, max_nb_children);
}

ServiceTable::~ServiceTable() {
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
ServiceTable::maxSize() {
  return max_nb_s;
}

ServiceTable::ServiceReference_t
ServiceTable::lookupService(const corba_profile_desc_t *sv_profile) {
  /* Called from ServiceTable::addService(), rmService(), getSolver(),
     getEvalf(), getConvertor(), getPerfMetric(), getChildren()
   ** DIET_server::diet_service_table_lookup_by_profile()
   ** Cori_Fast::diet_service_table_lookup_by_profile()
   */
  size_t i(0);
  for (; (i < nb_s) && (!profile_desc_match(&(profiles[i]), sv_profile)); i++) ;
  return (ServiceReference_t) ((i == nb_s) ? -1 : (int) i);
}

ServiceTable::ServiceReference_t
ServiceTable::lookupService(const corba_pb_desc_t *pb_desc) {
  /* Called from MasterAgent::submitLocal(), AgentImpl::findServer()
  ** SeDImpl::getRequest(), SeDImpl::checkContract()
  ** Thus, the check must be performed in special way concerning parallel tasks
  **  - if parallel or sequential is asked, strict check
  **  - if nothing specified, both // and non-// must be considered
  */
  size_t i(0);
  for (; (i < nb_s) && (!profile_match(&(profiles[i]), pb_desc)); i++) ;
  return (ServiceReference_t) ((i == nb_s) ? -1 : (int) i);
}

ServiceTable::ServiceReference_t
ServiceTable::lookupService(const char *path, const corba_profile_t *pb) {
  /* Called from SeDImpl::solve() and solveAsync() */
  size_t i(0);
  for (; (i < nb_s) && (!profile_match(&(profiles[i]), path, pb)); i++) ;
  return (ServiceReference_t) ((i == nb_s) ? -1 : (int) i);
}

int
ServiceTable::addService(const corba_profile_desc_t *profile,
                         const diet_convertor_t *const cvt,
                         diet_solve_t solver,
                         diet_eval_t evalf,
                         diet_perfmetric_t perfmetric_fn) {
  /* Called from DIET_server::diet_service_table_add */
  ServiceReference_t service_idx(-1);

  if (matching_children) {
    SRVT_ERROR(
      "attempting to add a service with" << endl
                                         <<
      "  solver in a table initialized with children");
  }

  if ((service_idx = lookupService(profile)) == -1) {
    // Then add a brand new service
    if (nb_s == max_nb_s) {
      max_nb_s += max_nb_s_step;
      profiles.length(max_nb_s);
      diet_solve_t *newSolvers = (diet_solve_t *)
                                 realloc(solvers, max_nb_s *
                                         sizeof(diet_solve_t));
      if (newSolvers == NULL) {
        // What do we do??
        // free(solvers);
        return 1;
      }
      solvers = newSolvers;

      diet_eval_t *newEval = (diet_eval_t *)
                             realloc(eval_functions, max_nb_s *
                                     sizeof(diet_eval_t));
      if (newEval == NULL) {
        // What do we do??
        // free(eval_functions);
        return 1;
      }
      eval_functions = newEval;

      diet_convertor_t *newConvertors = (diet_convertor_t *)
                                        realloc(convertors, max_nb_s *
                                                sizeof(diet_convertor_t));
      if (newConvertors == NULL) {
        // What do we do??
        // free(convertors);
        return 1;
      }
      convertors = newConvertors;

      diet_perfmetric_t *newPerf = (diet_perfmetric_t *)
                                   realloc(perfmetrics, max_nb_s *
                                           sizeof(diet_perfmetric_t));
      if (newPerf == NULL) {
        // What do we do??
        // free(perfmetrics);
        return -1;
      }
      perfmetrics = newPerf;

      for (size_t i = nb_s; i < max_nb_s; i++) {
        profiles[i].param_desc.length(0);
        solvers[i] = NULL;
        eval_functions[i] = NULL;
        convertors[i].arg_convs = NULL;
        perfmetrics[i] = NULL;
      }
    }

    profiles[nb_s] = *profile;              // deep copy
    solvers[nb_s] = solver;
    eval_functions[nb_s] = evalf;
    // duplicate path and arg_convs, since the user should deallocate them with
    // diet_convertor_free.
    convertors[nb_s] = *cvt;
    convertors[nb_s].path = strdup(cvt->path);
    convertors[nb_s].arg_convs = new diet_arg_convertor_t[cvt->last_out + 1];
    for (int i = 0; i <= cvt->last_out; i++) {
      convertors[nb_s].arg_convs[i] = cvt->arg_convs[i];
    }
    perfmetrics[nb_s] = perfmetric_fn;
    nb_s++;
  } else if (solver == solvers[(size_t) service_idx]) {
    return -1;
    // service is already in table
  } else {
    SRVT_ERROR(
      "attempting to add 2 services with" << endl
                                          <<
      "  same path and profile, but with different solvers");
  }
  return 0;
} // addService

/* Called from AgentImpl::addServices() */
int
ServiceTable::addService(const corba_profile_desc_t *profile,
                         CORBA::ULong child) {
  ServiceReference_t service_idx(-1);

  if (solvers) {
    SRVT_ERROR(
      "attempting to add a service with" << endl
                                         <<
      "  child in a table initialized with solvers");
  }

  if ((service_idx = lookupService(profile)) == -1) {
    // Then add a brand new service
    if ((nb_s != 0) && (nb_s % max_nb_s) == 0) {
      max_nb_s += max_nb_s_step;
      profiles.length(max_nb_s);
      matching_children_t *newChildren = (matching_children_t *)
                                         realloc(
        matching_children, max_nb_s * sizeof(matching_children_t));
      if (newChildren == NULL) {
        // What do we do??
        return 1;
      }
      matching_children = newChildren;

      for (size_t i = nb_s; i < max_nb_s; i++) {
        profiles[i].param_desc.length(0);
        matching_children[i].nb_children = 0;
        matching_children[i].children = new CORBA::ULong[max_nb_children];
      }
    }
    profiles[nb_s] = *profile;  // deep copy
    matching_children[nb_s].children[matching_children[nb_s].nb_children++]
      = child;
    nb_s++;
  } else {
    { /* verify that the aggregators are equivalent */
      corba_profile_desc_t *storedProfile = &(profiles[service_idx]);
      const corba_aggregator_desc_t *a1 = &(storedProfile->aggregator);
      const corba_aggregator_desc_t *a2 = &(profile->aggregator);
      if (a1->agg_specific._d() != a2->agg_specific._d()) {
        ERROR(__FUNCTION__ << ": aggregator type mismatch" << endl, -2);
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
                ")" << endl, -2);
        }
        for (unsigned int pvIter = 0;
             pvIter < p1->priorityList.length();
             pvIter++) {
          if (p1->priorityList[pvIter] != p2->priorityList[pvIter]) {
            ERROR(
              __FUNCTION__ <<
              ": priority list value mismatch, index " <<
              pvIter <<
              " (" <<
              p1->priorityList[pvIter] <<
              " != " <<
              p2->priorityList[pvIter] <<
              ")" << endl <<
              "TIPS : check diet_aggregator_priority_ function in all SeD",
              -2);
          }
        }
      }
      break;
      default:
        ERROR(__FUNCTION__ <<
              ": unexpected aggregator type (" <<
              a1->agg_specific._d(), -2);
      } // switch
    }

    CORBA::ULong nb_children =
      matching_children[(size_t) service_idx].nb_children;
    CORBA::ULong *children = matching_children[(size_t) service_idx].children;

    for (size_t i = 0; i < nb_children; i++)
      if (children[i] == child) {
        return -1;
      }             // service already associated to child
    // Here, we must add the child in matching_children[service_idx].children
    if ((nb_children % max_nb_children) == 0) {
      // Then realloc children array
      children = (CORBA::ULong *)
                 realloc(children,
                         (nb_children + max_nb_children) * sizeof(CORBA::ULong));
      matching_children[service_idx].children = children;
    }
    children[nb_children] = child;
    matching_children[service_idx].nb_children++;
  }
  return 0;
} // addService



int
ServiceTable::rmService(const corba_profile_desc_t *profile) {
  ServiceReference_t ref(-1);

  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR("attempting to rm a service that is not in table");
  }
  return this->rmService(ref);
}


int
ServiceTable::rmService(const ServiceReference_t ref) {
  size_t i(0);

  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }

  if (solvers) {
    profiles[ref].param_desc.length(0);
    for (i = (size_t) ref; i < (nb_s - 1); i++) {
      profiles[i] = profiles[i + 1];
      solvers[i] = solvers[i + 1];
      eval_functions[i] = eval_functions[i + 1];
      convertors[i] = convertors[i + 1];
      perfmetrics[i] = perfmetrics[i + 1];
    }
    profiles[i].param_desc.length(0);
    solvers[i] = NULL;
    eval_functions[i] = NULL;
    convertors[i].arg_convs = NULL;
    perfmetrics[i] = NULL;
  } else {
    profiles[ref].param_desc.length(0);
    free(matching_children[ref].children);
    for (i = (size_t) ref; i < (nb_s - 1); i++) {
      profiles[i] = profiles[i + 1];
      matching_children[i] = matching_children[i + 1];
    }
    profiles[i].param_desc.length(0);
    matching_children[i].nb_children = 0;
    matching_children[i].children = new CORBA::ULong[max_nb_children];
  }

  nb_s--;
  return 0;
} // rmService


/** removes a service on a given child */
int
ServiceTable::rmChildService(const corba_profile_desc_t *profile,
                             CORBA::ULong childID) {
  ServiceReference_t ref(-1);

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
    size_t i(0), j(0);
    for (i = (size_t) 0;
         i < matching_children[ref].nb_children &&
         matching_children[ref].children[i] != childID;
         i++) ;

    /* if we didn't found it return -1 */
    if (i == matching_children[ref].nb_children) {
      SRVT_ERROR(
        "attempting to rm a service to a child which do not possess it");
    }

    /* If the child is the only one to possess this service, we remove it completely */
    if (matching_children[ref].nb_children == 1) {
      return this->rmService(ref);
    }

    /* Otherwise we need to shift the array of children */
    for (j = i; j < matching_children[ref].nb_children; j++) {
      matching_children[ref].children[j] =
        matching_children[ref].children[j + 1];
    }
    matching_children[ref].nb_children--;
  }

  return 0;
} // rmChildService

int
ServiceTable::rmChild(const CORBA::ULong child) {
  ServiceReference_t ref(-1);

  if (solvers) {
    SRVT_ERROR(
      "attempting to remove a child from" << endl
                                          <<
      "  a table initialized with solvers");
  }
  for (ref = 0; (size_t) ref < nb_s; ref++) {
    size_t i;
    for (i = 0;
         ((i < matching_children[ref].nb_children) &&
          (matching_children[(size_t) ref].children[i] != child));
         i++) ;
    if (i < matching_children[ref].nb_children) {
      for (size_t j = i; j < matching_children[ref].nb_children; j++) {
        matching_children[ref].children[j]
          = matching_children[ref].children[j + 1];
      }
      // FIXME: Is it necessary ?
      // Remove service if there is no child left
      // if ((--(matching_children[ref].nb_children)) == 0)
      // rmService(ref);
      // replaced by:
      (matching_children[ref].nb_children)--;
    }
  }
  return 0;
} // rmChild

// #if defined HAVE_ALT_BATCH
/* This method does NOT test the validity of the range index */
const corba_profile_desc_t &
ServiceTable::getProfile(const ServiceReference_t index) {
  return profiles[index];
}
// #endif

SeqCorbaProfileDesc_t *
ServiceTable::getProfiles() {
  SeqCorbaProfileDesc_t *res = new SeqCorbaProfileDesc_t(nb_s);
  res->length(nb_s);
  for (size_t i = 0; i < nb_s; i++) {
    (*res)[i] = profiles[i];
  }
  return res;
}

SeqCorbaProfileDesc_t *
ServiceTable::getProfiles(CORBA::Long &length) {
  SeqCorbaProfileDesc_t *res = new SeqCorbaProfileDesc_t(nb_s);
  res->length(nb_s);
  for (size_t i = 0; i < nb_s; i++) {
    (*res)[i] = profiles[i];
  }
  length = nb_s;
  return res;
}



diet_solve_t
ServiceTable::getSolver(const corba_profile_desc_t *profile) {
  ServiceReference_t ref(-1);

  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR(
      "attempting to get solver" << endl
                                 <<
      "  of a service that is not in table");
  }
  return getSolver(ref);
}


diet_solve_t
ServiceTable::getSolver(const ServiceReference_t ref) {
  if (!solvers) {
    SRVT_ERROR(
      "attempting to get a solver" << endl
                                   <<
      "  in a table initialized with children");
  }
  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  return solvers[ref];
}


diet_eval_t
ServiceTable::getEvalf(const corba_profile_desc_t *profile) {
  ServiceReference_t ref;

  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR(
      "attempting to get eval function" << endl
                                        <<
      "  of a service that is not in table");
  }
  return getEvalf(ref);
}


diet_eval_t
ServiceTable::getEvalf(const ServiceReference_t ref) {
  if (!solvers) {
    SRVT_ERROR(
      "attempting to get an eval function" << endl
                                           <<
      "  in a table initialized with children");
  }
  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  return eval_functions[ref];
}


diet_convertor_t *
ServiceTable::getConvertor(const corba_profile_desc_t *profile) {
  ServiceReference_t ref(-1);

  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR(
      "attempting to get convertor" << endl
                                    <<
      "  of a service that is not in table");
  }
  return getConvertor(ref);
}


diet_convertor_t *
ServiceTable::getConvertor(const ServiceReference_t ref) {
  if (!solvers) {
    SRVT_ERROR(
      "attempting to get a convertor" << endl
                                      <<
      "  in a table initialized with children");
  }
  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }

  return &(convertors[ref]);
} // getConvertor

diet_perfmetric_t
ServiceTable::getPerfMetric(const corba_profile_desc_t *profile) {
  ServiceReference_t ref(-1);

  if ((ref = lookupService(profile)) == -1) {
    SRVT_ERROR(
      "attempting to get performance metric" << endl
                                             <<
      "  of a service that is not in table");
  }
  return (this->getPerfMetric(ref));
}


diet_perfmetric_t
ServiceTable::getPerfMetric(const ServiceReference_t ref) {
  if (!solvers) {
    SRVT_ERROR(
      "attempting to get a performance metric" << endl
                                               <<
      "  in a table initialized with children");
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
   SRVT_ERROR("attempting to get children" << endl
   << "  of a service that is not in table");
   }
   return getChildren(ref);
   }
 */
#if defined HAVE_ALT_BATCH
ServiceTable::matching_children_t *
ServiceTable::getChildren(const corba_pb_desc_t *pb_desc,
                          ServiceTable::ServiceReference_t serviceRef,
                          CORBA::ULong *frontier) {
  if (solvers) {
    SRVT_ERROR(
      "attempting to get children" << endl
                                   <<
      "  in a table initialized with solvers");
  }
  if (((int) serviceRef < 0) || ((size_t) serviceRef >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }

  int first_found = -1;  // at most, two indices: // and seq
  size_t i(0);
  ServiceTable::matching_children_t *mc = NULL;

  /* Search for 1rst occurence of service in table */
  while ((i < nb_s) && (!profile_match(&(profiles[i]), pb_desc))) {
    i++;
  }
  if (i == nb_s) {
    SRVT_ERROR(
      "attempting to get children" << endl
                                   <<
      "  of a service that is not in table");
  }
  first_found = i;
  mc = new ServiceTable::matching_children_t();
  if (mc == NULL) {
    SRVT_ERROR("Not enough memory");
  }

  if (pb_desc->parallel_flag == 0) { /* Test if there is same profile with
                                         different parallel flag */
    int second_found = -1;
    size_t j(0);

    i++;
    /* Search for 2nd occurence of service in table */
    while ((i < nb_s) && (!profile_match(&(profiles[i]), pb_desc))) {
      i++;
    }
    if (i == nb_s) { /* No new occurence */
      mc->nb_children =
        matching_children[first_found].nb_children;
    } else {
      second_found = i;
      mc->nb_children =
        matching_children[first_found].nb_children
        + matching_children[second_found].nb_children;
    }
    /* Reserve memory for all children */
    mc->children =
      new CORBA::ULong[mc->nb_children];

    /* Copy children, ordered parallel flag = 1 first */
    if (profiles[serviceRef].parallel_flag == 1) {
      for (i = 0; i < matching_children[first_found].nb_children; i++) {
        mc->children[i] =
          matching_children[first_found].children[i];
      }
      if (second_found > 0) {
        for (j = 0; j < matching_children[second_found].nb_children; i++,
             j++) {
          mc->children[i] =
            matching_children[second_found].children[j];
        }
      }
      /* set frontier */
      (*frontier) = matching_children[first_found].nb_children;
    } else {
      if (second_found > 0) {
        for (i = 0; i < matching_children[second_found].nb_children; i++) {
          mc->children[i] =
            matching_children[second_found].children[i];
        }
        /* set frontier */
        (*frontier) = matching_children[second_found].nb_children;
      } else {
        for (j = 0, i = 0; j < matching_children[first_found].nb_children;
             i++, j++) {
          mc->children[i] =
            matching_children[first_found].children[j];
        }
        /* set frontier */
        (*frontier) = 0;
      }
    }
  } else { /* Only interested by a given profile (seq ORexclusive //)
              then first match is unique match */
    mc->nb_children =
      matching_children[first_found].nb_children;
    mc->children =
      new CORBA::ULong[mc->nb_children];
    for (i = 0; i < matching_children[first_found].nb_children; i++) {
      mc->children[i] =
        matching_children[first_found].children[i];
    }
    /* set frontier */
    if (profiles[serviceRef].parallel_flag == 1) {
      (*frontier) = matching_children[first_found].nb_children;
    } else {
      (*frontier) = 0;
    }
  }

  return mc;
} // getChildren
#else // if defined HAVE_ALT_BATCH
const ServiceTable::matching_children_t *
ServiceTable::getChildren(const ServiceReference_t ref) {
  if (solvers) {
    SRVT_ERROR(
      "attempting to get children" << endl
                                   <<
      "  in a table initialized with solvers");
  }
  if (((int) ref < 0) || ((size_t) ref >= nb_s)) {
    SRVT_ERROR("wrong service reference");
  }
  return &(matching_children[ref]);
}
#endif // if defined HAVE_ALT_BATCH

void
ServiceTable::dump(FILE *f) {
  char path[257];

  // FIXME: stat the FILE to check if write OK (take care that this method can
  // be called on stdout !

  fprintf(f, "\n--------------------------------------------------\n");
  fprintf(f, "Service Table (%lu services)\n", (unsigned long) nb_s);
  fprintf(f, "--------------------------------------------------\n\n");

  for (size_t i = 0; i < nb_s; i++) {
    strcpy(path, profiles[i].path);
    fprintf(f, "- Service %s", path);
#if defined HAVE_ALT_BATCH
    if (profiles[i].parallel_flag == 2) {
      fprintf(f, " (parallel service) ");
    } else if (profiles[i].parallel_flag == 1) {
      fprintf(f, " (sequential service) ");
    } else {fprintf(f, " Error? ");
    }
#endif

    if (matching_children) {
      fprintf(f, " offered by ");
      if (matching_children[i].nb_children == 0) {
        fprintf(f, "no child");
      } else {
        if (matching_children[i].nb_children == 1) {
          fprintf(f, "child %lu",
                  (long unsigned int) matching_children[i].children[0]);
        } else {
          size_t j;
          fprintf(f, "children %lu",
                  (long unsigned int) matching_children[i].children[0]);
          for (j = 1; j < (matching_children[i].nb_children - 1); j++)
            fprintf(f, ", %lu",
                    (long unsigned int) matching_children[i].children[j]);
          // Re-use j to shorten next line
          fprintf(f, " and %lu.",
                  (long unsigned int) matching_children[i].children[j]);
        }
      }
    }

    for (size_t j = 0; (int) j <= profiles[i].last_out; j++) {
      fprintf(f, "\n     %s ",
              ((int) j <= profiles[i].last_in) ? "IN   "
              : ((int) j <= profiles[i].last_inout) ? "INOUT"
              : "OUT  ");
      displayArgDesc(f, profiles[i].param_desc[j].type,
                     profiles[i].param_desc[j].base_type);
    }
    fprintf(f, "\n");

    if (!matching_children) {
      displayConvertor(f, &(convertors[i]));
    }
  }
} // dump


int
ServiceTable::ServiceTableInit(CORBA::ULong max_nb_services,
                               CORBA::ULong max_nb_children) {
  nb_s = 0;
  max_nb_s = (max_nb_services == 0) ? MAX_NB_SERVICES : max_nb_services;
  max_nb_s_step = max_nb_s;
  this->max_nb_children = max_nb_children;
  profiles.length(max_nb_s);

  if (max_nb_children > 0) {
    solvers = NULL;
    eval_functions = NULL;
    convertors = NULL;
    perfmetrics = NULL;
    matching_children = new matching_children_t[max_nb_s];
    for (size_t i = 0; i < max_nb_s; i++) {
      profiles[i].param_desc.length(0);
      matching_children[i].nb_children = 0;
      matching_children[i].children = new CORBA::ULong[max_nb_children];
    }
  } else {
    solvers = (diet_solve_t *)
              malloc(max_nb_s * sizeof(diet_solve_t));
    eval_functions = (diet_eval_t *)
                     malloc(max_nb_s * sizeof(diet_eval_t));
    convertors = (diet_convertor_t *)
                 malloc(max_nb_s * sizeof(diet_convertor_t));
    perfmetrics = (diet_perfmetric_t *)
                  malloc(max_nb_s * sizeof(diet_perfmetric_t));
    for (size_t i = 0; i < max_nb_s; i++) {
      profiles[i].param_desc.length(0);
      solvers[i] = NULL;
      eval_functions[i] = NULL;
      convertors[i].arg_convs = NULL;
      perfmetrics[i] = NULL;
    }
    matching_children = NULL;
  }
  return 0;
} // ServiceTableInit

#if defined HAVE_ALT_BATCH
int
ServiceTable::testIfAllParallelServices() {
  size_t i = 1;

  while ((i < nb_s) &&
         (profiles[0].parallel_flag == profiles[i].parallel_flag)) {
    i++;
  }
  if (i == nb_s) {
    return (profiles[0].parallel_flag == 2);
  } else {
    return -1;
  }
} // testIfAllParallelServices
#endif // if defined HAVE_ALT_BATCH
