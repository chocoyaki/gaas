/****************************************************************************/
/* Implementation of the DIET/FAST interface Manager                        */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/03/31 21:15:41  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.15  2007/04/30 13:31:09  ycaniou
 * Cosmetic changes
 *
 * Revision 1.14  2006/11/16 09:55:56  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.13  2006/01/25 21:08:00  pfrauenk
 * CoRI - plugin scheduler: the type diet_est_tag_t est replace by int
 *        some new fonctions in DIET_server.h to manage the estVector
 *
 * Revision 1.12  2005/08/31 14:56:49  alsu
 * New plugin scheduling interface: using the new estimation vector
 * interface
 *
 * Revision 1.11  2005/05/15 15:44:08  alsu
 * minor changes from estimation vector reorganization
 *
 * Revision 1.10  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.9.2.6  2004/12/08 00:03:57  alsu
 * basic validation tests using FAST 0.8.5 / NWS 2.6.mt4 complete
 *
 * Revision 1.9.2.5  2004/12/01 20:17:45  alsu
 * in commTime and estimate methods, FASTMgr::use should be checked
 * before FASTMgr::initialized, since the initialized variable is only
 * set if the check on the use variable succeeded in the init method.
 *
 * Revision 1.9.2.4  2004/11/30 13:57:12  alsu
 * minor problems during FAST testing on plugin schedulers
 *
 * Revision 1.9.2.3  2004/11/26 15:20:26  alsu
 * change the estimate function to enable the SeD-level C interface
 *
 * Revision 1.9.2.2  2004/11/24 09:30:15  alsu
 * - adding new datatype DIET_PARAMSTRING, which allows users to define
 *   strings for which the value is important for performance evaluation
 *   (and which is consequently stored in the argument description, much
 *   like what is done for DIET_SCALAR arguments)
 * - adding functions to access the type-specific data structures stored
 *   in the diet_data_desc_t.specific union (for use in custom
 *   performance metrics to access data such as those that are described
 *   above)
 *
 * Revision 1.9.2.1  2004/11/02 00:45:45  alsu
 * new estimate method to store data into an estVector_t, rather than
 * directly into a corba_estimation_t
 *
 * Revision 1.9  2003/12/09 10:10:11  cpera
 * Fix an error from previous commit (add #if HAVE_FAST).
 *
 * Revision 1.8  2003/12/08 11:02:24  cpera
 * Delete PAUSE macro used to simulate FAST call ...
 *
 * Revision 1.7  2003/10/14 10:49:14  cpera
 * Fix unmodified DIET_BYTE type from a previous commit.
 *
 * Revision 1.6  2003/09/27 07:54:01  pcombes
 * Replace silly base type DIET_BYTE by DIET_SHORT.
 *
 * Revision 1.4  2003/08/26 14:58:58  pcombes
 * Fix bug in returned value of init (return codes of FAST are not standard !)
 *
 * Revision 1.1  2003/08/01 19:16:55  pcombes
 * Add a manager for the FAST API, compatible with FAST 0.4 and FAST 0.8.
 * Any later changes in the FAST API should be processed by this static class.
 ****************************************************************************/


#include <cmath>

#include "FASTMgr.hh"

#include "est_internal.hh"

#include "debug.hh"
#include "marshalling.hh"
#include "Parsers.hh"

#if HAVE_FAST
#if defined(__FAST_0_8__)
#include <fast.h>
#elif defined(__FAST_0_4__)
#include <slimfast_api.h>
#else
#error "This version of FAST is not supported !!!"
#endif
#endif // HAVE_FAST

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/* Initialize private static members. */
bool FASTMgr::initialized = false;
#if HAVE_FAST
size_t FASTMgr::use = 1;
#else  // HAVE_FAST
size_t FASTMgr::use = 0;
#endif // HAVE_FAST
omni_mutex FASTMgr::mutex;
// FIXME: how can I initialize a mutex ?


/**
 * Initialize the FAST library according to the configuration got in the
 * Parsers::Results.
 */
int
FASTMgr::init()
{
  int res(0);

#if HAVE_FAST
  if (FASTMgr::initialized) {
    INTERNAL_ERROR("attempt to call fast_init twice", 1);
  }

  FASTMgr::mutex.lock();
  FASTMgr::use =
    *((size_t*)Parsers::Results::getParamValue(Parsers::Results::FASTUSE));
  if (FASTMgr::use > 0) {
    Parsers::Results::Address* tmp;
    size_t ldapUse, ldapPort, nwsUse, nwsNSPort, nwsFcstPort;
    char*  ldapHost = "";
    char*  ldapMask = "";
    char*  nwsNSHost = "";
    char*  nwsFcstHost = "";
    ldapUse = 
      *((size_t*)Parsers::Results::getParamValue(Parsers::Results::LDAPUSE));
    if (ldapUse) {
      tmp = (Parsers::Results::Address*)
	Parsers::Results::getParamValue(Parsers::Results::LDAPBASE);
      ldapHost = tmp->host;
      ldapPort = tmp->port;
      ldapMask = (char*)
	Parsers::Results::getParamValue(Parsers::Results::LDAPMASK);
    }
    nwsUse =
      *((size_t*)Parsers::Results::getParamValue(Parsers::Results::NWSUSE));
    if (nwsUse) {
      tmp = (Parsers::Results::Address*)
	Parsers::Results::getParamValue(Parsers::Results::NWSNAMESERVER);
      nwsNSHost = tmp->host;
      nwsNSPort = tmp->port;
      tmp = (Parsers::Results::Address*)
	Parsers::Results::getParamValue(Parsers::Results::NWSFORECASTER);
      if (tmp) { // Check if tmp != NULL
	nwsFcstHost = tmp->host;
	nwsFcstPort = tmp->port;
      }
    }
    res = fast_init("ldap_use", ldapUse,
		    "ldap_server", ldapHost, ldapPort, "ldap_binddn", ldapMask,
		    "nws_use", nwsUse,
		    "nws_nameserver", nwsNSHost, nwsNSPort,
#if defined(__FAST_0_4__)
		    "nws_forecaster", nwsFcstHost, nwsFcstPort,
#endif // __FAST_0_4__
		    NULL);

    res = (int)(!res);
    FASTMgr::initialized = (res == 0);
    FASTMgr::mutex.unlock();

  } // if (FASTMgr::use > 0) 
#endif // HAVE_FAST

  return res;
}

/**
 * Get communication time for an amount of data of size \c size,<br>
 *  <ul><li> from \c host1 to \c host2 if \c to is true,</li>
 *      <li> from \c host2 to \c host1 else.            </li></ul>
 */
double
FASTMgr::commTime(char* host1, char* host2, unsigned long size, bool to)
{
  double time(HUGE_VAL);
#if HAVE_FAST
  char* dest_name, * src_name;

  if (FASTMgr::use == 0) {
    return (time);
  }

  if (!FASTMgr::initialized) {
    INTERNAL_ERROR("attempt to estimate a communication time"
		   << " without having initalized FAST first.", 1);
  }

  if (to) {
    dest_name = host2;
    src_name  = host1;
  } else {
    dest_name = host1;
    src_name  = host2;
  }
  FASTMgr::mutex.lock();
  if (!(fast_comm_time_best(src_name, dest_name, size, &time))) {
    time = HUGE_VAL;
    WARNING("cannot estimate communication time: set to inf");
  }
  FASTMgr::mutex.unlock();
#endif // HAVE_FAST
  
  return time;
}


#if HAVE_FAST

#if defined(__FAST_0_8__)

typedef data_argstack_t fast_pb_t;
#define fast_load     fast_avail_load
#define fast_memory   fast_avail_memory
#define fast_cpucount fast_avail_cpucount
#define fast_pb_free  data_argstack_free

#elif defined(__FAST_0_4__)

typedef sf_inst_desc_t fast_pb_t;
#define fast_pb_free   sf_inst_desc_free
#define fast_cpucount(host,nbCpu) 1
#endif

/**
 * Convert the description of a DIET problem into a FAST description, using hte
 * convertor developped by the user.
 */
// fast_pb_t*
// corbaPbDesc2fastPb(const corba_pb_desc_t* src, const diet_convertor_t* cvt);
fast_pb_t*
corbaPbDesc2fastPb(const diet_profile_t* const src,
                   const diet_convertor_t* cvt);

#endif // HAVE_FAST

/**
 * Estimate the computation time for \c pb. \c cvt is used to convert the DIET
 * profile into a FAST profile.
 */
// void
// FASTMgr::estimate(char* hostName,
// 		  const corba_pb_desc_t& pb,
//                   const diet_convertor_t* cvt,
//                   estVector_t ev)
void
FASTMgr::estimate(char* hostName,
		  const diet_profile_t* const profilePtr,
                  const ServiceTable* SrvT,
                  ServiceTable::ServiceReference_t ref,
#ifdef HAVE_CORI
		  int estTag,
#endif //HAVE_CORI
                  estVector_t ev)
{
  double time(HUGE_VAL);
  double freeCPU(0);
  double freeMem(0);
  int    nbCPU(1);

#if HAVE_FAST // with FAST

  if (FASTMgr::use == 0) {
    return;
  }

  if (!FASTMgr::initialized) {
    INTERNAL_ERROR("attempt to estimate a request"
		   << " without having initalized FAST first.", 1);
  }


  // casting away the const, because i *know* getConvertor doesn't
  // modify the service table, but i don't know how to declare that
  // in C++...-as
  const diet_convertor_t* cvt = ((ServiceTable*)SrvT)->getConvertor(ref);

  fast_pb_t* fastPb = corbaPbDesc2fastPb(profilePtr, cvt);

  if (!fastPb) {
    WARNING("cannot evaluate the server");
    return;
  }

  FASTMgr::mutex.lock();
    
  if ( !(fast_comp_time_best(hostName, fastPb, &time)) ) {
    WARNING("cannot estimate computation time. Try CPU and memory load");
    time = HUGE_VAL;
  }
  if ( !(fast_load(hostName, &freeCPU)) ) {
    WARNING("cannot estimate free CPU fraction");
    freeCPU = 0;
  }
  if ( !(fast_memory(hostName, &freeMem)) ) {
    WARNING("cannot estimate free memory");
    freeMem = 0;
  }
  if ( !(fast_cpucount(hostName, &nbCPU)) ) {
    WARNING("cannot estimate number of CPU");
    freeMem = 0;
  }

  FASTMgr::mutex.unlock();

  fast_pb_free(fastPb);

#else //HAVE_FAST
#endif //HAVE_FAST
  {
//     fprintf(stderr,
// 	    "****FASTTEST****** FASTMgr::estimate: (%.4f,%.4f,%.4f,%.4f)\n",
// 	    time,
// 	    freeCPU,
// 	    freeMem,
// 	    (double) nbCPU);
#ifdef HAVE_CORI
    if ((estTag==EST_TCOMP)||(estTag==EST_ALLINFOS))
    diet_est_set_internal(ev, EST_TCOMP, time);
    if ((estTag==EST_FREECPU)||(estTag==EST_ALLINFOS))
    diet_est_set_internal(ev, EST_FREECPU, freeCPU);
    if ((estTag==EST_FREEMEM)||(estTag==EST_ALLINFOS))
    diet_est_set_internal(ev, EST_FREEMEM, freeMem);
    if ((estTag==EST_NBCPU)||(estTag==EST_ALLINFOS))
    diet_est_set_internal(ev, EST_NBCPU, nbCPU);
    if ((estTag==EST_TCOMP)||(estTag==EST_ALLINFOS))
    diet_est_set_internal(ev, EST_TCOMP, time);

#else //HAVECORI
    diet_est_set_internal(ev, EST_TCOMP, time);
    diet_est_set_internal(ev, EST_FREECPU, freeCPU);
    diet_est_set_internal(ev, EST_FREEMEM, freeMem);
    diet_est_set_internal(ev, EST_NBCPU, nbCPU);
    diet_est_set_internal(ev, EST_TCOMP, time);
#endif //HAVECORI
  }
}




#if HAVE_FAST

/****************************************************************************/
/* DIET problem to FAST problem                                             */
/****************************************************************************/


#if defined(__FAST_0_8__)

#define fast_type_t data_cons_type_t
#define FAST_SCAL   data_cons_scal
#define FAST_VECT   data_cons_vect
#define FAST_MAT    data_cons_mat 
#define FAST_FILE   data_cons_file

#define fast_base_type_t data_scal_type_t
#define FAST_CHAR        data_char
#define FAST_INT         data_int
#define FAST_DOUBLE      data_double

#define fast_arg_desc_t  data_arg_t
#define fast_char_alloc(value)     data_arg_char_new(value)
#define fast_int_alloc(value)      data_arg_int_new(value)
#define fast_double_alloc(value)   data_arg_double_new(value)
#define fast_vect_alloc(bt,size)   data_arg_vect_new(bt,size)
#define fast_mat_alloc(bt,r,c,o)   data_arg_mat_new(bt,r,c)
#define fast_file_alloc(path,size) data_arg_file_new(path)
#define fast_arg_desc_free(arg)   data_arg_free(&arg)

#define fast_pb_alloc(path,li,lio,lo,where) \
        *where = data_argstack_new(path)
#define fast_pb_set_param(pb, idx, arg) \
        data_argstack_push(pb, arg)


#elif defined(__FAST_0_4__)

#define fast_type_t sf_type_cons_t
#define FAST_SCAL   sf_type_cons_scal
#define FAST_VECT   sf_type_cons_vect
#define FAST_MAT    sf_type_cons_mat 
#define FAST_FILE   sf_type_cons_file

#define fast_base_type_t sf_type_base_t
#define FAST_CHAR        sf_type_base_char
#define FAST_INT         sf_type_base_int
#define FAST_DOUBLE      sf_type_base_double

#define fast_arg_desc_t  sf_data_desc_t
#define fast_char_alloc(value)     sf_dd_char_alloc(0,value)
#define fast_int_alloc(value)      sf_dd_int_alloc(0,value)
#define fast_double_alloc(value)   sf_dd_double_alloc(0,value)
#define fast_vect_alloc(bt,size)   sf_dd_vect_alloc(0,bt,size)
#define fast_mat_alloc(bt,r,c,o)   sf_dd_mat_alloc(0,bt,r,c,o)
#define fast_file_alloc(path,size) sf_dd_file_alloc(0,size,path)
#define fast_arg_free(arg)         sf_data_desc_free(arg)

#define fast_pb_alloc sf_inst_desc_alloc
#define fast_pb_set_param(pb, idx, arg) \
        pb->param_desc[idx] = *arg; free(arg)

#endif

inline fast_type_t
diet_to_fast_type(const diet_data_type_t t)
{
  switch (t) {
  case DIET_SCALAR:
  case DIET_PARAMSTRING:
    return FAST_SCAL;
  case DIET_VECTOR:
  case DIET_STRING:
    return FAST_VECT;
  case DIET_MATRIX:
    return FAST_MAT;
  case DIET_FILE:
    return FAST_FILE;
  default:
    return (fast_type_t)10;
  }
}

inline fast_base_type_t
diet_to_fast_base_type(const diet_base_type_t t)
{
  switch (t) {
  case DIET_CHAR:
    return FAST_CHAR;
  case DIET_SHORT:
  case DIET_INT:
  case DIET_LONGINT:
    return FAST_INT;
  case DIET_FLOAT:
  case DIET_DOUBLE:
    return FAST_DOUBLE;
#if HAVE_COMPLEX
  case DIET_SCOMPLEX:
  case DIET_DCOMPLEX:
#endif // HAVE_COMPLEX
  default:
    return (fast_base_type_t)10;
  }
}

fast_arg_desc_t*
dietDataDesc2fastArgDesc(const diet_data_desc_t* src) 
{
  fast_arg_desc_t* dest(NULL);
  fast_base_type_t bt = diet_to_fast_base_type(src->generic.base_type);
  
  switch (diet_to_fast_type(src->generic.type)) {
     
  case FAST_VECT:
    size_t size;
    size = src->specific.vect.size + (src->generic.type == DIET_STRING) ? 1 : 0;
    dest = fast_vect_alloc(bt, size);
    break;
    
  case FAST_MAT:
    dest = fast_mat_alloc(bt, src->specific.mat.nb_r,
			  src->specific.mat.nb_c, src->specific.mat.order);
    break;
    
  case FAST_FILE:
    if (src->specific.file.path)
      dest = fast_file_alloc(src->specific.file.path, src->specific.file.size);
    else
      dest = fast_file_alloc("", 0);
    break;

  case FAST_SCAL:
    switch (bt) {
    case FAST_INT:
      dest = fast_int_alloc(*((long*)src->specific.scal.value));
      break;
    case FAST_DOUBLE:
      dest = fast_double_alloc(*((double*)src->specific.scal.value));
      break;
    case FAST_CHAR:
      dest = fast_char_alloc(*((short*)src->specific.scal.value));
      break;
    default:
      ERROR(__FUNCTION__ << ": base type " << bt << " unknown for FAST", NULL);
    }
    break;
    
  default:
    ERROR(__FUNCTION__  << ": type " << diet_to_fast_type(src->generic.type)
	  << " unknown for FAST", NULL);
  }
  return dest;
}

/****************************************************************************/
/* Problem profile -> FAST sf_inst_desc (for client requests)              */
/****************************************************************************/

int
cvt_arg_desc(fast_arg_desc_t** dest, 
	     diet_data_desc_t* src, diet_convertor_function_t f)
{
  diet_data_desc_t* ddd = new diet_data_desc_t;
  
  switch(f) {
  case DIET_CVT_IDENTITY: {
    delete ddd;
    ddd = src;
    break;
  }
  case DIET_CVT_VECT_SIZE: {
    scalar_set_desc(ddd, NULL, DIET_VOLATILE, DIET_INT, src->specific.scal.value);
    break;
  }
  case DIET_CVT_MAT_NB_ROW: {
    scalar_set_desc(ddd, NULL, DIET_VOLATILE, DIET_INT, &src->specific.mat.nb_r);
    break;
  }
  case DIET_CVT_MAT_NB_COL: {
    scalar_set_desc(ddd, NULL, DIET_VOLATILE, DIET_INT, &src->specific.mat.nb_c);
    break;
  }
  case DIET_CVT_MAT_ORDER: {
    char t;
    // FIXME test on order !!!!
    switch ((diet_matrix_order_t)(src->specific.mat.order)) {
    case DIET_ROW_MAJOR: t = 'N'; break;
    case DIET_COL_MAJOR: t = 'T'; break;
    default: {
      ERROR(__FUNCTION__ << ": invalid order for matrix", 1);
    }
    }
    scalar_set_desc(ddd, NULL, DIET_VOLATILE, DIET_CHAR, &t);
    break;
  }
  case DIET_CVT_STR_LEN: {
    scalar_set_desc(ddd, NULL, DIET_VOLATILE, DIET_INT, &src->specific.str.length);
    break;
  }
  case DIET_CVT_FILE_SIZE: {
    scalar_set_desc(ddd, NULL, DIET_VOLATILE, DIET_INT, &src->specific.file.size);
    break;
  }
  default: {
    ERROR(__FUNCTION__ << ": invalid convertor function", 1);
  }
  }
  
  *dest = dietDataDesc2fastArgDesc(ddd);

  return (*dest == NULL);
}

extern int
unmrsh_data_desc(diet_data_desc_t* dest, const corba_data_desc_t* src);


fast_pb_t*
corbaPbDesc2fastPb(const diet_profile_t* srcProf, const diet_convertor_t* cvt)
{
  fast_pb_t* res(NULL);
  /* This keeps all unmarshalled argument descriptions */
  diet_data_desc_t** srcProf_params = // Use calloc to set all elements to NULL
    (diet_data_desc_t**) calloc(srcProf->last_out + 1,
                                sizeof(diet_data_desc_t*));
  
  fast_pb_alloc(cvt->path, cvt->last_in, cvt->last_inout, cvt->last_out, &res);

//   displayPbDesc(src);
  displayConvertor(stderr, cvt);

  for (int i = 0; i <= cvt->last_out; i++) {
    diet_data_desc_t* ddd_tmp(NULL);
    int arg_idx = cvt->arg_convs[i].in_arg_idx;
    
    if ((arg_idx >= 0) && (arg_idx <= srcProf->last_out)) {
      if (!srcProf_params[arg_idx]) {
	ddd_tmp = new diet_data_desc_t;
	srcProf_params[arg_idx] = ddd_tmp;

// as: we now have a profile, so we don't need to do an unmrsh
//         unmrsh_data_desc(ddd_tmp, &(src->param_desc[arg_idx]));
        memcpy(ddd_tmp,
//                &(srcProf->param_desc[arg_idx]),
               &((srcProf->parameters[arg_idx]).desc),
               sizeof (diet_data_desc_t));
      } else {
	ddd_tmp = srcProf_params[arg_idx];
      }
    } else {
      ddd_tmp = &(cvt->arg_convs[i].arg->desc);
    }
    // DESTINATOR = &(dest->param_desc[i])

    fast_arg_desc_t* fastArg;
    
    if (cvt_arg_desc(&fastArg, ddd_tmp, cvt->arg_convs[i].f)) {
      for (int j = 0; j <= srcProf->last_out; j++)
	delete srcProf_params[j];
      delete [] srcProf_params;
      WARNING(__FUNCTION__
	      << ": cannot convert client profile to server profile");
      fast_pb_free(res);
      return NULL;
    } else {
      fast_pb_set_param(res, i, fastArg);
    }
  }
  
  for (int i = 0; i <= srcProf->last_out; i++)
    delete srcProf_params[i];
  delete [] srcProf_params;

#if defined(__FAST_0_8__)
  if (TRACE_LEVEL >= TRACE_STRUCTURES) {
    data_argstack_dump(res);
    data_argstack_check(res);
  }
#endif

  return res;
}



#endif // HAVE_FAST
