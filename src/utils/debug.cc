/****************************************************************************/
/* DIET debug utils source code                                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.22  2004/12/15 13:53:22  sdahan
 * - the trace function are now thread safe.
 * - add "extern unsigned int TRACE_LEVEL" in debug.hh
 *
 * Revision 1.21  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.20.2.3  2004/11/24 09:30:15  alsu
 * - adding new datatype DIET_PARAMSTRING, which allows users to define
 *   strings for which the value is important for performance evaluation
 *   (and which is consequently stored in the argument description, much
 *   like what is done for DIET_SCALAR arguments)
 * - adding functions to access the type-specific data structures stored
 *   in the diet_data_desc_t.specific union (for use in custom
 *   performance metrics to access data such as those that are described
 *   above)
 *
 * Revision 1.20.2.2  2004/11/06 16:22:55  alsu
 * estimation vector access functions now have parameter-based default
 * return values
 *
 * Revision 1.20.2.1  2004/11/02 00:44:34  alsu
 * minor changes to accomodate the new estimation structure
 *
 * Revision 1.20  2004/10/15 08:22:18  hdail
 * Removed references to corba_response_t->sortedIndexes - no longer useful.
 * Added displayResponseShort(...) for easier debugging.
 *
 * Revision 1.19  2004/10/14 15:03:47  hdail
 * Added shorter, cleaner response debug message.
 *
 * Revision 1.18  2004/10/04 09:40:43  sdahan
 * warning fix :
 *  - debug.cc : change the printf format from %ul to %lu and from %l to %ld
 *  - ReferenceUpdateThread and BindService : The omniORB documentation said that
 *    it's better to create private destructor for the thread subclasses. But
 *    private destructors generate warning, so I set the destructors public.
 *  - CORBA.h and DIET_config.h define the same macros. So I include the CORBA.h
 *    before the DIET_config.h to avoid to define two times the same macros.
 *  - remove the deprecated warning when including iostream.h and set.h
 *
 * Revision 1.17  2004/09/14 12:51:01  hdail
 * Corrected output types for print statements to agree with variable types.
 *
 * Revision 1.16  2003/12/01 14:49:31  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.15  2003/09/27 07:54:01  pcombes
 * Replace silly base type DIET_BYTE by DIET_SHORT.
 *
 * Revision 1.14  2003/09/24 09:10:01  pcombes
 * Merge corba_DataMgr_desc_t and corba_data_desc_t.
 *
 * Revision 1.12  2003/04/10 12:45:44  pcombes
 * Set TRACE_LEVEL as a static variable, used by all other modules.
 * Update displayResponse to the new corba_response_t structure.
 *
 * Revision 1.9  2003/01/22 17:06:43  pcombes
 * API 0.6.4 : istrans -> order (row- or column-major)
 *
 * Revision 1.8  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 ****************************************************************************/


#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.hh"
#include "marshalling.hh"
#include "DIET_data_internal.hh"
#include "DIET_server.h"


/**
 * The trace level itself: it must be set by the Parsers.
 */
unsigned int TRACE_LEVEL = TRACE_DEFAULT;

// see debug.hh
omni_mutex debug_log_mutex ;

void
displayResponse(FILE* os, const corba_response_t* resp)
{
  size_t i;

  fprintf(os, "\n----------------------------------------\n");
  fprintf(os, " Response structure for request %lu :\n\n", resp->reqID);
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    fprintf(os, " I'm son nb %ld\n", resp->myID);
  //  fprintf(os, " There are %ld parameters\n", (long)resp->nbIn);
  fprintf(os, " %ld servers are able to solve the problem\n",
          (long)resp->servers.length());

  if (resp->servers.length() > 0)
    fprintf(os," Estimated computation time:\n");

  for (i = 0; i < resp->servers.length(); i++) {
    estVector_t ev = new_estVector();

//     if (resp->servers[i].estim.tComp != HUGE_VAL) {
//       fprintf(os, "  %ldth server can solve the problem in %g seconds\n",
//               (long) i, resp->servers[i].estim.tComp);
//     }
//     else {
//       fprintf(os, "  %ldth server has %g free CPU and %g free memory\n",
//               (long) i, resp->servers[i].estim.freeCPU,
//               resp->servers[i].estim.freeMem);
//     }
    {
      unmrsh_estimation_to_estVector(&(resp->servers[i].estim), ev);
      if (estVector_getEstimationValue(ev, EST_TCOMP, HUGE_VAL) != HUGE_VAL) {
        fprintf(os,
                "  %ldth server can solve the problem in %g seconds\n",
                (long) i,
                estVector_getEstimationValue(ev, EST_TCOMP, HUGE_VAL));
      }
      else {
        fprintf(os,
                "  %ldth server has %g free CPU and %g free memory\n",
                (long) i,
                estVector_getEstimationValue(ev, EST_FREECPU, HUGE_VAL),
                estVector_getEstimationValue(ev, EST_FREEMEM, HUGE_VAL));
      }
    }

    fprintf(os, "  TComms for each parameter: ");
//     for (j = 0; j < resp->servers[i].estim.commTimes.length(); j++) {
//       fprintf(os, " %g", (double)resp->servers[i].estim.commTimes[j]);
//     }
    for (int commTimeIter = 0 ;
         commTimeIter < estVector_numEstimationsByType(ev, EST_COMMTIME) ;
         commTimeIter++) {
      fprintf(os,
              " %g",
              estVector_getEstimationValueNum(ev,
                                              EST_COMMTIME,
                                              HUGE_VAL,
                                              commTimeIter));
    }
    fprintf(os,"\n");

    free_estVector(ev);
  }
  fprintf(os, "----------------------------------------\n");
}

void
displayResponseShort(FILE* os, const corba_response_t* resp)
{
  fprintf(os, "\n---------- Responses for request %lu ----------\n",
      resp->reqID);

  for (size_t i = 0; i < resp->servers.length(); i++){
    estVector_t ev = new_estVector();
    unmrsh_estimation_to_estVector(&(resp->servers[i].estim), ev);

//     fprintf(stdout, 
//       "    %d: %s:%ld: tComp %g fCpu %g fMem %g\n",
//       i,
//       (const char *)(resp->servers[i].loc.hostName),
//       resp->servers[i].loc.port,
//       resp->servers[i].estim.tComp,
//       resp->servers[i].estim.freeCPU,
//       resp->servers[i].estim.freeMem);
    fprintf(stdout, 
            "    %d: %s:%ld: tComp %g fCpu %g fMem %g\n",
            i,
            (const char *)(resp->servers[i].loc.hostName),
            resp->servers[i].loc.port,
            estVector_getEstimationValue(ev, EST_TCOMP, HUGE_VAL),
            estVector_getEstimationValue(ev, EST_FREECPU, HUGE_VAL),
            estVector_getEstimationValue(ev, EST_FREEMEM, HUGE_VAL));

    free_estVector(ev);
  }
}


void
displayArgDesc(FILE* f, int type, int base_type)
{
  switch(type) {
  case DIET_SCALAR: fprintf(f, "scalar"); break;
  case DIET_VECTOR: fprintf(f, "vector"); break;
  case DIET_MATRIX: fprintf(f, "matrix"); break;
  case DIET_STRING: fprintf(f, "string"); break;
  case DIET_PARAMSTRING: fprintf(f, "paramstring"); break;
  case DIET_FILE:   fprintf(f, "file");   break;
  }
  if ((type != DIET_STRING) &&
      (type != DIET_PARAMSTRING) &&
      (type != DIET_FILE)) {
    fprintf(f, " of ");
    switch (base_type) {
    case DIET_CHAR:     fprintf(f, "char");           break;
    case DIET_SHORT:    fprintf(f, "short");          break;
    case DIET_INT:      fprintf(f, "int");            break;
    case DIET_LONGINT:  fprintf(f, "long int");       break;
    case DIET_FLOAT:    fprintf(f, "float");          break;
    case DIET_DOUBLE:   fprintf(f, "double");         break;
    case DIET_SCOMPLEX: fprintf(f, "float complex");  break;
    case DIET_DCOMPLEX: fprintf(f, "double complex"); break;
    }
  }
}

void
displayArg(FILE* f, const corba_data_desc_t* arg)
{
  switch(arg->specific._d()) {
  case DIET_SCALAR: fprintf(f, "scalar");            break;
  case DIET_VECTOR: fprintf(f, "vector (%ld)",
                            (long)arg->specific.vect().size);  break;
  case DIET_MATRIX: fprintf(f, "matrix (%ldx%ld)",
                            (long)arg->specific.mat().nb_r,
                            (long)arg->specific.mat().nb_c);   break;
  case DIET_STRING: fprintf(f, "string (%ld)",
                            (long)arg->specific.str().length); break;
  case DIET_PARAMSTRING: fprintf(f, "paramstring (%ld)",
                            (long)arg->specific.pstr().length); break;
  case DIET_FILE:   fprintf(f, "file (%ld)",
                            (long)arg->specific.file().size);  break;
  }
  if ((arg->specific._d() != DIET_STRING) &&
      (arg->specific._d() != DIET_PARAMSTRING) &&
      (arg->specific._d() != DIET_FILE)) {
    fprintf(f, " of ");
    switch (arg->base_type) {
    case DIET_CHAR:     fprintf(f, "char");           break;
    case DIET_SHORT:    fprintf(f, "short");          break;
    case DIET_INT:      fprintf(f, "int");            break;
    case DIET_LONGINT:  fprintf(f, "long int");       break;
    case DIET_FLOAT:    fprintf(f, "float");          break;
    case DIET_DOUBLE:   fprintf(f, "double");         break;
    case DIET_SCOMPLEX: fprintf(f, "float complex");  break;
    case DIET_DCOMPLEX: fprintf(f, "double complex"); break;
    }
  }
  fprintf(f, ", id=|%s|", arg->id.idNumber.in());
}

void
displayArg(FILE* f, const diet_data_desc_t* arg)
{
  switch((int) arg->generic.type) {
  case DIET_SCALAR: fprintf(f, "scalar");                break;
  case DIET_VECTOR: fprintf(f, "vector (%ld)",
                            (long)arg->specific.vect.size);    break;
  case DIET_MATRIX: fprintf(f, "matrix (%ldx%ld)",
                            (long)arg->specific.mat.nb_r,
                            (long)arg->specific.mat.nb_c);   break;
  case DIET_STRING: fprintf(f, "string (%ld)",
                            (long)arg->specific.str.length); break;
  case DIET_PARAMSTRING: fprintf(f, "paramstring (%ld)",
                            (long)arg->specific.pstr.length); break;
  case DIET_FILE:   fprintf(f, "file (%ld)",
                            (long)arg->specific.file.size);  break;
  }
  if ((arg->generic.type != DIET_STRING) &&
      (arg->generic.type != DIET_PARAMSTRING) &&
      (arg->generic.type != DIET_FILE)) {
    fprintf(f, " of ");
    switch ((int) arg->generic.base_type) {
    case DIET_CHAR:     fprintf(f, "char");           break;
    case DIET_SHORT:    fprintf(f, "short");          break;
    case DIET_INT:      fprintf(f, "int");            break;
    case DIET_LONGINT:  fprintf(f, "long int");       break;
    case DIET_FLOAT:    fprintf(f, "float");          break;
    case DIET_DOUBLE:   fprintf(f, "double");         break;
    case DIET_SCOMPLEX: fprintf(f, "float complex");  break;
    case DIET_DCOMPLEX: fprintf(f, "double complex"); break;
    }
  }
  fprintf(f, "id=|%s|", arg->id);
}


void
displayProfileDesc(const diet_profile_desc_t* desc, const char* path)
{
  FILE* f = stdout;
  fprintf(f, " - Service %s", path);
  for (int i = 0; i <= desc->last_out; i++) {
    fprintf(f, "\n     %s ",
            (i <= desc->last_in) ? "IN   "
            : (i <= desc->last_inout) ? "INOUT"
            : "OUT  ");
    displayArgDesc(f, desc->param_desc[i].type, desc->param_desc[i].base_type);
  }
  fprintf(f, "\n");
}


void
displayProfileDesc(const corba_profile_desc_t* desc)
{
  FILE* f = stdout;
  char* path = CORBA::string_dup(desc->path);
  fprintf(f, " - Service %s", path);
  CORBA::string_free(path);
  for (int j = 0; j <= desc->last_out; j++) {
    fprintf(f, "\n     %s ",
            (j <= desc->last_in) ? "IN   "
            : (j <= desc->last_inout) ? "INOUT"
            : "OUT  ");
    displayArgDesc(f, desc->param_desc[j].type, desc->param_desc[j].base_type);
  }
  fprintf(f, "\n");
  free(path);
}

void
displayProfile(const diet_profile_t* profile, const char* path)
{
  FILE* f = stdout;
  fprintf(f, " - Service %s", path);
  for (int i = 0; i <= profile->last_out; i++) {
    fprintf(f, "\n     %s ",
            (i <= profile->last_in) ? "IN   "
            : (i <= profile->last_inout) ? "INOUT"
            : "OUT  ");
    displayArg(f, &(profile->parameters[i].desc));
  }
  fprintf(f, "\n");
}

void
displayProfile(const corba_profile_t* profile, const char* path)
{
  FILE* f = stdout;
  fprintf(f, " - Service %s", path);
  for (int i = 0; i <= profile->last_out; i++) {
    fprintf(f, "\n     %s ",
            (i <= profile->last_in) ? "IN   "
            : (i <= profile->last_inout) ? "INOUT"
            : "OUT  ");
    displayArg(f, &(profile->parameters[i].desc));
  }
  fprintf(f, "\n");
}

void
displayPbDesc(const corba_pb_desc_t* profile)
{
  FILE* f = stdout;
  char* path = CORBA::string_dup(profile->path);
  fprintf(f, " - Service %s", path);
  CORBA::string_free(path);
  for (int j = 0; j <= profile->last_out; j++) {
    fprintf(f, "\n     %s ",
            (j <= profile->last_in) ? "IN   "
            : (j <= profile->last_inout) ? "INOUT"
            : "OUT  ");
    displayArg(f, &(profile->param_desc[j]));
  }
  fprintf(f, "\n");
}

void
displayConvertor(FILE* f, const diet_convertor_t* cvt)
{
  fprintf(f, " - Convertor to %s", cvt->path);
  for (int i = 0; i <= cvt->last_out; i++) {
    fprintf(f, "\n     %s ",
            (i <= cvt->last_in) ? "IN   "
            : (i <= cvt->last_inout) ? "INOUT"
            : "OUT  ");
    switch((int)cvt->arg_convs[i].f) {
    case DIET_CVT_IDENTITY:   fprintf(f, "IDENT  of "); break;
    case DIET_CVT_FILE_SIZE:
    case DIET_CVT_VECT_SIZE:  fprintf(f, "SIZE   of "); break;
    case DIET_CVT_MAT_NB_ROW: fprintf(f, "NB_ROW of "); break;
    case DIET_CVT_MAT_NB_COL: fprintf(f, "NB_COL of "); break;
    case DIET_CVT_MAT_ORDER:  fprintf(f, "ORDER  of "); break;
    case DIET_CVT_STR_LEN:    fprintf(f, "LENGTH of "); break;
    }
    if (cvt->arg_convs[i].arg)
      displayArg(f, &(cvt->arg_convs[i].arg->desc));
    else
      fprintf(f, "argument %d", cvt->arg_convs[i].in_arg_idx);
    fprintf(f, " (out: %d)", cvt->arg_convs[i].out_arg_idx);
  }
  fprintf(f, "\n");
}
