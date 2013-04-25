/**
 * @file  debug.cc
 *
 * @brief  DIET debug utils source code
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */



#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "debug.hh"
#include "common_types.hh"
#include "est_internal.hh"
#include "marshalling.hh"
#include "DIET_data_internal.hh"
#include "DIET_server.h"

#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif

/**
 * The trace level itself: it must be set by the Parsers.
 */
DIET_API_LIB unsigned int TRACE_LEVEL = TRACE_DEFAULT;

// see debug.hh
omni_mutex debug_log_mutex;

void
displayResponse(FILE *os, const corba_response_t *resp) {
#ifdef HAVE_ALT_BATCH
  const char *serverType;
  const char *jobSpec;
#endif

  fprintf(os, "\n----------------------------------------\n");
  fprintf(os, " Response structure for request %lu :\n\n",
          (long unsigned int) resp->reqID);
  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    fprintf(os, " I'm son nb %lu\n", (long unsigned int) resp->myID);
  }

  fprintf(os, " %ld servers are able to solve the problem:\n",
          (long) resp->servers.length());

  for (size_t i = 0; i < resp->servers.length(); i++) {
    estVectorConst_t ev = &(resp->servers[i].estim);

#ifdef HAVE_ALT_BATCH
    /* TODO: Should be called from somewhere in DIET_server.cc */
    if (resp->servers[i].loc.serverType == BATCH) {
      serverType = "Batch";
    } else if (resp->servers[i].loc.serverType == SERIAL) {
      serverType = "serial";
    } else {
      WARNING("Type of server is not well defined!\n");
      serverType = "";
    }
    if (resp->servers[i].loc.parallel_flag == 1) {
      jobSpec = "sequential";
    } else if (resp->servers[i].loc.parallel_flag == 2) {
      jobSpec = "parallel";
    } else {
      WARNING("Type of job is not well defined!\n");
      jobSpec = "";
    }


    // TODO: depending on server type, show pertinant information
    // -> no need of free CPU for a frontal, but nomber of resources
    // available or not
    if (diet_est_get_internal(ev, EST_TCOMP, HUGE_VAL) != HUGE_VAL) {
      fprintf(os,
              "  %ldth %s server can solve the %s problem in %g seconds\n",
              (long) i,
              serverType,
              jobSpec,
              diet_est_get_internal(ev, EST_TCOMP, HUGE_VAL));
    } else {
      fprintf(os,
              "  %ldth %s server can solve the %s problem and "
              "has %g free CPU and %g free memory\n",
              (long) i,
              serverType,
              jobSpec,
              diet_est_get_internal(ev, EST_FREECPU, HUGE_VAL),
              diet_est_get_internal(ev, EST_FREEMEM, HUGE_VAL));
    }
#else // ifdef HAVE_ALT_BATCH

    if (diet_est_get_internal(ev, EST_TCOMP, HUGE_VAL) != HUGE_VAL) {
      fprintf(os,
              "  %ldth server can solve the problem in %g seconds\n",
              (long) i,
              diet_est_get_internal(ev, EST_TCOMP, HUGE_VAL));
    } else {
      fprintf(os,
              "  %ldth server can solve the problem and "
              "has %g free CPU and %g free memory\n",
              (long) i,
              diet_est_get_internal(ev, EST_FREECPU, HUGE_VAL),
              diet_est_get_internal(ev, EST_FREEMEM, HUGE_VAL));
    }
#endif // ifdef HAVE_ALT_BATCH

    int numComms = diet_est_array_size_internal(ev, EST_COMMTIME);
    if (numComms > 0) {
      fprintf(os, "       Parameter transfer times (sec): ");
      for (int commTimeIter = 0;
           commTimeIter < numComms;
           commTimeIter++) {
        fprintf(os,
                " %g |",
                diet_est_array_get_internal(ev,
                                            EST_COMMTIME,
                                            commTimeIter,
                                            HUGE_VAL));
      } // end for each comm time parameter
    }   // end check if comm times exist in est vector
  }     // end for each server
  fprintf(os, "\n");
  fprintf(os, "----------------------------------------\n");
} // displayResponse

void
displayResponseShort(FILE *os, const corba_response_t *resp) {
#ifdef HAVE_ALT_BATCH

  const char *serverType;
  const char *jobSpec;
#endif

  fprintf(os, "\n---------- Responses for request %lu ----------\n",
          (unsigned long) resp->reqID);

  for (size_t i = 0; i < resp->servers.length(); i++) {
    estVectorConst_t ev = &(resp->servers[i].estim);

#ifdef HAVE_ALT_BATCH
    /* TODO: Should be called from somewhere in DIET_server.cc */
    if (resp->servers[i].loc.serverType == BATCH) {
      serverType = "Batch";
    } else if (resp->servers[i].loc.serverType == SERIAL) {
      serverType = "serial";
    } else {
      WARNING("Type of server is not well defined!\n");
      serverType = "";
    }
    if (resp->servers[i].loc.parallel_flag == 1) {
      jobSpec = "sequential";
    } else if (resp->servers[i].loc.parallel_flag == 2) {
      jobSpec = "parallel";
    } else {
      WARNING("Type of job is not well defined!\n");
      jobSpec = "";
    }
#endif // ifdef HAVE_ALT_BATCH

    fprintf(stdout,
#ifdef HAVE_ALT_BATCH
            "    %ld: %s:%ld:%s;%s: tComp %g fCpu %g fMem %g\n",
            (long int) i,
            (const char *) (resp->servers[i].loc.hostName),
            (long int) resp->servers[i].loc.port,
            serverType, jobSpec,
#else
            "    %ld: %s:%ld: tComp %g fCpu %g fMem %g\n",
            (long int) i,
            (const char *) (resp->servers[i].loc.hostName),
            (long int) resp->servers[i].loc.port,
#endif
            diet_est_get_internal(ev, EST_TCOMP, HUGE_VAL),
            diet_est_get_internal(ev, EST_FREECPU, HUGE_VAL),
            diet_est_get_internal(ev, EST_FREEMEM, HUGE_VAL));
  }
} // displayResponseShort


void
displayArgDesc(FILE *f, int type, int base_type) {
  switch (type) {
  case DIET_SCALAR: fprintf(f, "scalar");
    break;
  case DIET_VECTOR: fprintf(f, "vector");
    break;
  case DIET_MATRIX: fprintf(f, "matrix");
    break;
  case DIET_STRING: fprintf(f, "string");
    break;
  case DIET_PARAMSTRING: fprintf(f, "paramstring");
    break;
  case DIET_FILE:   fprintf(f, "file");
    break;
  case DIET_CONTAINER: fprintf(f, "container");
    break;
  } // switch
  if ((type != DIET_STRING) &&
      (type != DIET_PARAMSTRING) &&
      (type != DIET_FILE) &&
      (type != DIET_CONTAINER)) {
    fprintf(f, " of ");
    switch (base_type) {
    case DIET_CHAR:     fprintf(f, "char");
      break;
    case DIET_SHORT:    fprintf(f, "short");
      break;
    case DIET_INT:      fprintf(f, "int");
      break;
    case DIET_LONGINT:  fprintf(f, "long int");
      break;
    case DIET_FLOAT:    fprintf(f, "float");
      break;
    case DIET_DOUBLE:   fprintf(f, "double");
      break;
    case DIET_SCOMPLEX: fprintf(f, "float complex");
      break;
    case DIET_DCOMPLEX: fprintf(f, "double complex");
      break;
    } // switch
  }
} // displayArgDesc

void
displayArg(FILE *f, const corba_data_desc_t *arg) {
  switch (arg->specific._d()) {
  case DIET_SCALAR: fprintf(f, "scalar");
    break;
  case DIET_VECTOR: fprintf(f, "vector (%ld)",
                            (long) arg->specific.vect().size);
    break;
  case DIET_MATRIX: fprintf(f, "matrix (%ldx%ld)",
                            (long) arg->specific.mat().nb_r,
                            (long) arg->specific.mat().nb_c);
    break;
  case DIET_STRING: fprintf(f, "string (%ld)",
                            (long) arg->specific.str().length);
    break;
  case DIET_PARAMSTRING: fprintf(f, "paramstring (%ld)",
                                 (long) arg->specific.pstr().length);
    break;
  case DIET_FILE:   fprintf(f, "file (%ld)",
                            (long) arg->specific.file().size);
    break;
  case DIET_CONTAINER:  fprintf(f, "container");
    break;
  } // switch
  if ((arg->specific._d() != DIET_STRING) &&
      (arg->specific._d() != DIET_PARAMSTRING) &&
      (arg->specific._d() != DIET_FILE) &&
      (arg->specific._d() != DIET_CONTAINER)) {
    fprintf(f, " of ");
    switch (arg->base_type) {
    case DIET_CHAR:     fprintf(f, "char");
      break;
    case DIET_SHORT:    fprintf(f, "short");
      break;
    case DIET_INT:      fprintf(f, "int");
      break;
    case DIET_LONGINT:  fprintf(f, "long int");
      break;
    case DIET_FLOAT:    fprintf(f, "float");
      break;
    case DIET_DOUBLE:   fprintf(f, "double");
      break;
    case DIET_SCOMPLEX: fprintf(f, "float complex");
      break;
    case DIET_DCOMPLEX: fprintf(f, "double complex");
      break;
    } // switch
  }
  fprintf(f, ", id=|%s|", arg->id.idNumber.in());
} // displayArg

void
displayArg(FILE *f, const diet_data_desc_t *arg) {
  switch ((int) arg->generic.type) {
  case DIET_SCALAR: fprintf(f, "scalar");
    break;
  case DIET_VECTOR: fprintf(f, "vector (%ld)",
                            (long) arg->specific.vect.size);
    break;
  case DIET_MATRIX: fprintf(f, "matrix (%ldx%ld)",
                            (long) arg->specific.mat.nb_r,
                            (long) arg->specific.mat.nb_c);
    break;
  case DIET_STRING: fprintf(f, "string (%ld)",
                            (long) arg->specific.str.length);
    break;
  case DIET_PARAMSTRING: fprintf(f, "paramstring (%ld)",
                                 (long) arg->specific.pstr.length);
    break;
  case DIET_FILE:   fprintf(f, "file (%ld)",
                            (long) arg->specific.file.size);
    break;
  case DIET_CONTAINER:  fprintf(f, "container");
    break;
  } // switch
  if ((arg->generic.type != DIET_STRING) &&
      (arg->generic.type != DIET_PARAMSTRING) &&
      (arg->generic.type != DIET_FILE) &&
      (arg->generic.type != DIET_CONTAINER)) {
    fprintf(f, " of ");
    switch ((int) arg->generic.base_type) {
    case DIET_CHAR:     fprintf(f, "char");
      break;
    case DIET_SHORT:    fprintf(f, "short");
      break;
    case DIET_INT:      fprintf(f, "int");
      break;
    case DIET_LONGINT:  fprintf(f, "long int");
      break;
    case DIET_FLOAT:    fprintf(f, "float");
      break;
    case DIET_DOUBLE:   fprintf(f, "double");
      break;
    case DIET_SCOMPLEX: fprintf(f, "float complex");
      break;
    case DIET_DCOMPLEX: fprintf(f, "double complex");
      break;
    } // switch
  }
  fprintf(f, "id=|%s|", arg->id);
} // displayArg


void
displayProfileDesc(const diet_profile_desc_t *desc, const char *path) {
#ifdef HAVE_ALT_BATCH
  const char *jobSpec;

  /* TODO: Should be called from somewhere in DIET_server.cc */
  if (desc->parallel_flag == 1) {
    jobSpec = "sequential";
  } else if (desc->parallel_flag == 2) {
    jobSpec = "parallel";
  } else {
    WARNING("Type of job is not well defined!\n");
    jobSpec = "";
  }
#endif // ifdef HAVE_ALT_BATCH

  FILE *f = stdout;
#ifdef HAVE_ALT_BATCH
  fprintf(f, " - Service %s (%s)", path, jobSpec);
#else
  fprintf(f, " - Service %s", path);
#endif
  for (int i = 0; i <= desc->last_out; i++) {
    fprintf(f, "\n     %s ",
            (i <= desc->last_in) ? "IN   "
            : (i <= desc->last_inout) ? "INOUT"
            : "OUT  ");
    displayArgDesc(f, desc->param_desc[i].type, desc->param_desc[i].base_type);
  }
  fprintf(f, "\n");

  switch ((desc->aggregator).agg_method) {
  case DIET_AGG_DEFAULT:
    fprintf(f, "   Aggregator: Default\n");
    break;
  case DIET_AGG_PRIORITY:
    fprintf(f, "   Aggregator: Priority (");
    {
      diet_aggregator_priority_t prior =
        (desc->aggregator).agg_specific.agg_specific_priority;
      if (prior.p_numPValues == 0) {
        fprintf(f, " No priorities assigned");
      } else {
        for (int i = 0; i < prior.p_numPValues; i++) {
          fprintf(f, " %d", prior.p_pValues[i]);
        }
      }
    }
    fprintf(f, ")\n");
    break;
  default:
    fprintf(f, "   Aggregator: ERROR\n");
  } // switch
} // displayProfileDesc

void
displayProfileDesc(const corba_profile_desc_t *desc) {
  FILE *f = stdout;
  char *path = CORBA::string_dup(desc->path);
#ifdef HAVE_ALT_BATCH
  const char *jobSpec;

  /* TODO: Should be called from somewhere in DIET_server.cc */
  if (desc->parallel_flag == 1) {
    jobSpec = "sequential";
  } else if (desc->parallel_flag == 2) {
    jobSpec = "parallel";
  } else {
    WARNING("Type of job is not well defined!\n");
    jobSpec = "";
  }
  fprintf(f, " - Service %s (%s)", path, jobSpec);
#else // ifdef HAVE_ALT_BATCH
  fprintf(f, " - Service %s", path);
#endif // ifdef HAVE_ALT_BATCH

  CORBA::string_free(path);
  for (int j = 0; j <= desc->last_out; j++) {
    fprintf(f, "\n     %s ",
            (j <= desc->last_in) ? "IN   "
            : (j <= desc->last_inout) ? "INOUT"
            : "OUT  ");
    displayArgDesc(f, desc->param_desc[j].type, desc->param_desc[j].base_type);
  }
  fprintf(f, "\n");

  switch ((desc->aggregator).agg_specific._d()) {
  case DIET_AGG_DEFAULT:
  {
    fprintf(f, "   Aggregator: Default\n");
    break;
  }
  case DIET_AGG_PRIORITY:
  {
    fprintf(f, "   Aggregator: Priority (");
    corba_agg_priority_t prior =
      (desc->aggregator).agg_specific.agg_priority();
    if (prior.priorityList.length() == 0) {
      fprintf(f, " No priorities assigned");
    } else {
      for (unsigned int i = 0; i < prior.priorityList.length(); i++) {
        fprintf(f, " %ld", (long int) prior.priorityList[i]);
      }
    }
    fprintf(f, ")\n");
    break;
  }
  default:
    fprintf(f, "   Aggregator: ERROR\n");
  } // switch
} // displayProfileDesc

void
displayProfile(const diet_profile_t *profile, const char *path) {
  FILE *f = stdout;

#ifdef HAVE_ALT_BATCH
  const char *jobSpec;

  /* TODO: Should be called from somewhere in DIET_server.cc */
  if (profile->parallel_flag == 1) {
    jobSpec = "sequential";
  } else if (profile->parallel_flag == 2) {
    jobSpec = "parallel";
  } else {
    WARNING("Type of job is not well defined!\n");
    jobSpec = "";
  }
  fprintf(f, " - Service %s (%s)", path, jobSpec);
#else // ifdef HAVE_ALT_BATCH
  fprintf(f, " - Service %s", path);
#endif // ifdef HAVE_ALT_BATCH

  for (int i = 0; i <= profile->last_out; i++) {
    fprintf(f, "\n     %s ",
            (i <= profile->last_in) ? "IN   "
            : (i <= profile->last_inout) ? "INOUT"
            : "OUT  ");
    displayArg(f, &(profile->parameters[i].desc));
  }
  fprintf(f, "\n");
} // displayProfile

void
displayProfile(const corba_profile_t *profile, const char *path) {
  FILE *f = stdout;

#ifdef HAVE_ALT_BATCH
  const char *jobSpec;

  /* TODO: Should be called from somewhere in DIET_server.cc */
  if (profile->parallel_flag == 1) {
    jobSpec = "sequential";
  } else if (profile->parallel_flag == 2) {
    jobSpec = "parallel";
  } else {
    WARNING("Type of job is not well defined!\n");
    jobSpec = "";
  }
  fprintf(f, " - Service %s (%s)", path, jobSpec);
#else // ifdef HAVE_ALT_BATCH
  fprintf(f, " - Service %s", path);
#endif // ifdef HAVE_ALT_BATCH

  for (int i = 0; i <= profile->last_out; i++) {
    fprintf(f, "\n     %s ",
            (i <= profile->last_in) ? "IN   "
            : (i <= profile->last_inout) ? "INOUT"
            : "OUT  ");
    displayArg(f, &(profile->parameters[i].desc));
  }
  fprintf(f, "\n");
} // displayProfile

void
displayPbDesc(const corba_pb_desc_t *profile) {
  FILE *f = stdout;
  char *path = CORBA::string_dup(profile->path);

#ifdef HAVE_ALT_BATCH
  const char *jobSpec;

  /* TODO: Should be called from somewhere in DIET_server.cc */
  if (profile->parallel_flag == 1) {
    jobSpec = "sequential";
  } else if (profile->parallel_flag == 2) {
    jobSpec = "parallel";
  } else {
    WARNING("Type of job is not well defined!\n");
    jobSpec = "";
  }
  fprintf(f, " - Service %s (%s)", path, jobSpec);
#else // ifdef HAVE_ALT_BATCH
  fprintf(f, " - Service %s", path);
#endif // ifdef HAVE_ALT_BATCH

  CORBA::string_free(path);
  for (int j = 0; j <= profile->last_out; j++) {
    fprintf(f, "\n     %s ",
            (j <= profile->last_in) ? "IN   "
            : (j <= profile->last_inout) ? "INOUT"
            : "OUT  ");
    displayArg(f, &(profile->param_desc[j]));
  }
  fprintf(f, "\n");
} // displayPbDesc

void
displayConvertor(FILE *f, const diet_convertor_t *cvt) {
  fprintf(f, " - Convertor to %s", cvt->path);
  for (int i = 0; i <= cvt->last_out; i++) {
    fprintf(f, "\n     %s ",
            (i <= cvt->last_in) ? "IN   "
            : (i <= cvt->last_inout) ? "INOUT"
            : "OUT  ");
    switch ((int) cvt->arg_convs[i].f) {
    case DIET_CVT_IDENTITY:   fprintf(f, "IDENT  of ");
      break;
    case DIET_CVT_FILE_SIZE:
    case DIET_CVT_VECT_SIZE:  fprintf(f, "SIZE   of ");
      break;
    case DIET_CVT_MAT_NB_ROW: fprintf(f, "NB_ROW of ");
      break;
    case DIET_CVT_MAT_NB_COL: fprintf(f, "NB_COL of ");
      break;
    case DIET_CVT_MAT_ORDER:  fprintf(f, "ORDER  of ");
      break;
    case DIET_CVT_STR_LEN:    fprintf(f, "LENGTH of ");
      break;
    } // switch
    if (cvt->arg_convs[i].arg) {
      displayArg(f, &(cvt->arg_convs[i].arg->desc));
    } else {
      fprintf(f, "argument %d", cvt->arg_convs[i].in_arg_idx);
    }
    fprintf(f, " (out: %d)", cvt->arg_convs[i].out_arg_idx);
  }
  fprintf(f, "\n");
} // displayConvertor
