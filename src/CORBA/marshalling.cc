/****************************************************************************/
/* DIET CORBA marshalling source code                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederc.Lombard@lifc.univ-fcomte.fr)              */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.78  2007/12/18 12:30:13  aamar
 * Using debug macros instead of cout
 *
 * Revision 1.77  2007/12/18 11:35:54  aamar
 * Correcting a bug with endianess management
 *
 * Revision 1.76  2007/07/11 08:42:08  aamar
 * Adding "custom client scheduling" mode (known as Burst mode). Need to be
 * activated in cmake.
 *
 * Revision 1.75  2007/07/09 18:54:48  aamar
 * Adding Endianness support (CMake option).
 *
 * Revision 1.74  2007/06/28 18:23:19  rbolze
 * add dietReqID in the profile.
 * and propagate this change to all functions that  have both reqID and profile parameters.
 * TODO : look at the asynchronous mechanism (client->SED) to propage this change.
 *
 * Revision 1.73  2007/05/16 07:46:14  mjan
 * Bug fix for JuxMem use and remove uneeded HAVE_JUXMEM
 *
 * Revision 1.72  2007/04/16 22:43:42  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.71  2007/03/27 07:55:58  glemahec
 * Adds the support of the new aggregator type (DIET_AGG_USER) in marshalling.cc
 *
 * Revision 1.70  2007/02/12 07:37:25  ycaniou
 * Undo last modifs
 *
 * Revision 1.69  2007/02/10 13:32:38  ycaniou
 * Memory leaks in unmrsh_scalar_desc()
 *
 * Revision 1.68  2007/01/24 20:16:30  ycaniou
 * Changed the name of the file received by a SeD: DIET_pid.uniqID.random.ficName
 * Indeed, when a client receives a file from a SeD, it keeps the SeD ficName.
 * Hence, if two SeD have the same pid, and use the same uniqID, we hope that the
 * seed will do it ;)
 *
 * Revision 1.67  2006/11/27 13:27:53  aamar
 * Force the unmarshalling of inout parameters for the asynchronous mode.
 *
 * Revision 1.66  2006/10/19 21:18:39  mjan
 * Contribute back last modifications for JuxMem use ... code only inside HAVE_JUXMEM
 *
 * Revision 1.65  2006/09/18 19:46:06  ycaniou
 * Corrected a bug in file_transfer:server.c
 * Corrected memory leaks due to incorrect free of char *
 * ServiceTable prints if service is sequential or parallel
 * Fully complete examples, whith a batch, a parallel and a sequential server and
 *  a unique client
 *
 * Revision 1.64  2006/08/27 18:40:07  ycaniou
 * Modified parallel submission API
 * - client: diet_call_batch() -> diet_parallel_call()
 * - SeD: diet_profile_desc_set_batch() -> [...]_parallel()
 * - from now, internal fields are related to parallel not batch
 * and corrected a bug:
 * - 3 types of submission: request among only seq, only parallel, or all
 *   available services (second wasn't implemented, third bug)
 *
 * Revision 1.63  2006/07/25 14:26:08  ycaniou
 * Changed dietJobID -> dietReqID
 *
 * Revision 1.62  2006/06/30 15:37:34  ycaniou
 * Code presentation, commentaries (nothing really "touched")
 *
 * Revision 1.61  2006/06/03 21:34:46  ycaniou
 * Correct DIET_FILE unmarshalling bug:
 * Put initialization of diet_data_desc_t in unmrsh_data_desc()
 * in case of DIET_FILE: it was the only case where it wasn't. This id is then
 * used in mrsh_profile_to_out_args().
 * -- tkx Gaël, Philippe & Mathieu
 *
 * Batch: update (un)marshalling stuff (nbprocess + bugs)
 *
 * Revision 1.60  2006/04/14 14:16:15  aamar
 * Adding the marshalling functions between C workflow data types and
 * the corresponding IDL types.
 *
 * Revision 1.59  2006/02/16 00:31:53  ecaron
 * If JUXMEM is available. __mrsh_data_id_desc and __mrsh_data_id are not used.
 *
 * Revision 1.58  2005/12/19 15:56:00  eboix
 *     FIX: BLAS and dmat_manips examples were broken since revision 1.57
 *     (date: 2005/12/10 09:37:36; author: bdelfabr). This "wild guess" fix
 *     hopefuly restores the examples and is also validated (thanks to
 *     Raphelkiller) for persistent file arguments.   --- Injay2461
 *
 * Revision 1.57  2005/12/10 09:37:36  bdelfabr
 * fix out persistent file bug
 *
 * Revision 1.56  2005/10/05 11:35:31  ecaron
 * Fix Warning (Bug in maintainer mode on MacOSX)
 * mrsh_data_desc_type(corba_data_desc_t*, const diet_data_desc_t*)' was declared 'static' but never defined
 *
 * Revision 1.55  2005/10/05 10:26:49  ecaron
 * Fix Warning (Bug in maintainer mode on MacOSX)
 * remove an internal warning about uninitalized corba_agg_default_t object in mrsh_aggregator_desc function.
 *
 * Revision 1.54  2005/10/05 09:51:20  ecaron
 * Fix Warning (Bug in maintainer mode on MacOSX)
 * uninitialized type in function for DIET_VECTOR and DIET_STRING
 *
 * Revision 1.53  2005/08/31 14:51:35  alsu
 * New plugin scheduling interface: marshalling/unmarshalling functions
 * no longer needed for the estimation vector, since the C structure no
 * longer exists and the CORBA C++ structure is being used throughout the
 * DIET code base.
 *
 * Revision 1.52  2005/08/30 12:05:15  ycaniou
 * Added a field dietJobId to make the correspondance between batch ID and
 *   diet job ID.
 * Added the corresponding management in marshalling.
 *
 * Revision 1.51  2005/08/04 09:04:29  alsu
 * adding an internal warning in the case where a marshalling operation
 * is requested on an uninitalized diet_aggregator_desc_t object.
 *
 * Revision 1.50  2005/06/14 16:17:11  mjan
 * Added support of DIET_FILE inside JuxMem-DIET for TLSE code
 *
 * Revision 1.48  2005/05/15 15:32:22  alsu
 * marshalling functions for aggregator
 *
 * Revision 1.47  2005/04/27 01:07:37  ycaniou
 * Added the necessary 'translations' for the new profiles structure fields
 *
 * Revision 1.46  2005/01/14 13:02:38  bdelfabr
 * removing useless cout
 *
 * Revision 1.45  2005/01/13 15:13:42  bdelfabr
 * unmrsh_data modified ro fix file transfer bug (whatever the persistence mode was the client never received an out file)
 *
 * Revision 1.44  2004/12/22 06:28:45  alsu
 * - slight modifications to make clear the static nature (i.e.,
 *   logically private to the marshalling code) of a few functions
 * - back out change introduced to __mrsh_data_id_desc that attempted to
 *   copy the type and base type of the src data.  previously, this was
 *   done to make the diet_service_table_lookup_by_profile function work;
 *   this function has now been rewritten
 *
 * Revision 1.43  2004/12/08 15:02:51  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.42  2004/11/25 21:46:01  hdail
 * Repaired various memory leaks and memory management errors.
 *
 * Revision 1.41.2.5  2004/12/06 16:08:33  alsu
 * in mrsh_data_id_desc, we still need to make sure the argument type
 * information is filled out (because the marshalling function is being
 * used in the new general-purpose FAST convertor lookup routine)
 *
 * Revision 1.41.2.4  2004/11/26 15:19:13  alsu
 * making the src argument of mrsh_pb_desc const
 *
 * Revision 1.41.2.3  2004/11/24 09:32:38  alsu
 * adding CORBA support (new datatype in the IDL and corresponding
 * marshalling/unmarshalling functions) to deal with
 * DIET_PARAMSTRING-specific data description structure
 *
 * Revision 1.41.2.2  2004/11/06 16:22:55  alsu
 * estimation vector access functions now have parameter-based default
 * return values
 *
 * Revision 1.41.2.1  2004/11/02 00:30:59  alsu
 * marshalling/unmarshalling functions to translate between estimations
 * (CORBA structures) and estimation vectors (plain C structures)
 *
 * Revision 1.41  2004/10/05 08:24:33  bdelfabr
 * change umrsh_data to avoid a wrong file name to be set and also to avoid useless file copy
 *
 * Revision 1.40  2004/09/29 12:45:53  bdelfabr
 * remove cout
 *
 * Revision 1.39  2004/09/27 09:20:50  bdelfabr
 * when marshalling in-out or our args, set id to null when data is volatile.
 *
 * Revision 1.38  2004/09/14 12:42:18  hdail
 * Shortened log, changed allocation of desc->value to match cleanup done by
 * omniORB, matched cleanup of src_params and args_filled to match alloc.
 *
 * Revision 1.37  2004/02/27 10:21:50  bdelfabr
 * adding mrsh_data_id and mrsh_data_id_desc,
 * modifying mrsh_data and umrsh_data to avoid memory copy
 *
 * Revision 1.36  2003/10/10 11:12:29  mcolin
 * Fix bug in function mrsh_profile_to_in_args in the case
 * of a volatile parameter of type DIET_FILE in IN or INOUT mode.
 * This is a temporary patch for the RNTL DEMO. We have to think
 * about another solution later.
 *
 * Revision 1.35  2003/09/30 15:26:36  bdelfabr
 * Manage the case of In argument with NUll value.
 ****************************************************************************/

#include <fstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "marshalling.hh"
#include "debug.hh"
#include "ms_function.hh"
#include "DIET_data_internal.hh"     // for data_sizeof()

extern unsigned int TRACE_LEVEL;

#define MRSH_ERROR(formatted_msg,return_value)                       \
  INTERNAL_ERROR(__FUNCTION__ << ": " << formatted_msg, return_value)


#ifdef WITH_ENDIANNESS
/****************************************************************************/
/* Swapping from Big Endian to Little Endian and vice-versa                 */
/****************************************************************************/
/**
 * Test the memory model (little or big endian)
 */
bool Little_Endian (void) {
   int x = 1;
   int r = *((char *)&x); /* return 1 if Little endian, otherwise it's Big endian */
   return (r == 1);
}

bool little_endian = Little_Endian();

void endian_swap(void * value, size_t size) {
  TRACE_TEXT (TRACE_ALL_STEPS, "Swapping value of size " << size << endl);
  char * p = (char *)value;
  char c;
  for (size_t i = 0; i<size/2; i++) {
    c = *(p+i);
    *(p+i) = *(p-i+size-1);
    *(p-i+size-1) = c;
  } // end for
}

void endian_swap(void * value, size_t size, size_t base_type_size) {
  char * p = (char *)value;
  if (p == NULL)
    return;
  size_t ix = 0;
  while (ix<size) {
    endian_swap(p + ix, base_type_size);
    ix += base_type_size;
  }
}

#endif // WITH_ENDIANNESS

/****************************************************************************/
/* Data structure marshalling                                               */
/****************************************************************************/

inline int
mrsh_scalar_desc(corba_data_desc_t* dest,
                 const diet_data_desc_t* const src)
{
  corba_scalar_specific_t scal;

  dest->specific.scal(scal);
  if (!(src->specific.scal.value)) {
    dest->specific.scal().value <<= (CORBA::Double) 0;
  } else {
    switch (src->generic.base_type) {
    case DIET_CHAR: { // Impossible to insert a Char or an Octet into an Any.
      char scal = *((char*)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Short) scal;
      break;
    }
    case DIET_SHORT: {
      short scal = *((short*)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Short) scal;
      break;
    }
    case DIET_INT: {
      long scal = (long) *((int*)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Long) scal;
      break;
    }
    case DIET_LONGINT: {
      long long int scal = *((long long int*)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Long) scal;
      break;
    }
    case DIET_FLOAT: {
      float scal = *((float*)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Float) scal;
      break;
    }
    case DIET_DOUBLE: {
      double scal = *((double*)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Double) scal;
      break;
    }
#if HAVE_COMPLEX
    case DIET_SCOMPLEX:
    case DIET_DCOMPLEX:
#endif // HAVE_COMPLEX
    default:
      MRSH_ERROR("base type " << src->generic.base_type
                 << " not implemented", 1);
    }
  }

  return 0;
}

int
__mrsh_data_desc_type(corba_data_desc_t* dest,
                      const diet_data_desc_t* const src)
{
  switch (src->generic.type) {
  case DIET_SCALAR: {

    if (mrsh_scalar_desc(dest, src)) {
      return (1);
    }
    break;
  }
  case DIET_VECTOR: {
    corba_vector_specific_t vect;
    
    vect.size = src->specific.vect.size;
    dest->specific.vect(vect);
    break;
  }
  case DIET_MATRIX: {
    corba_matrix_specific_t mat;

    dest->specific.mat(mat);
    dest->specific.mat().nb_r  = src->specific.mat.nb_r;
    dest->specific.mat().nb_c  = src->specific.mat.nb_c;
    dest->specific.mat().order = src->specific.mat.order;
    break;
  }
   case DIET_STRING: {
     corba_string_specific_t str;

     str.length = src->specific.str.length;
     dest->specific.str(str);
     break;
   }
  case DIET_PARAMSTRING: {
    corba_paramstring_specific_t pstr;

    dest->specific.pstr(pstr);
    dest->specific.pstr().length = src->specific.pstr.length;
    dest->specific.pstr().param = CORBA::string_dup(src->specific.pstr.param);
//     cout << "mrsh_data_desc: param is " << dest->specific.pstr().param << "\n";
    break;
  }
  case DIET_FILE: {
    corba_file_specific_t file;
    dest->specific.file(file);
    if (src->specific.file.path) {
   
      dest->specific.file().path = CORBA::string_dup(src->specific.file.path);
      dest->specific.file().size = src->specific.file.size;

    } else {
     
      dest->specific.file().path = CORBA::string_dup("");
      dest->specific.file().size = 0;

    }
    break;
  }
  default:
    MRSH_ERROR("type " << src->generic.type << " not implemented", 1);
  }

  return (0);
}

int
mrsh_data_desc(corba_data_desc_t* dest, diet_data_desc_t* src)
{
  if (src->id != NULL)
    // nxt line also deallocates old dest->id.idNumber
    dest->id.idNumber = CORBA::string_dup(src->id);
  else 
    dest->id.idNumber = CORBA::string_dup("");
  // The default values are not always inner the enum types, which triggers an ABORT.
  dest->id.state    = DIET_FREE;
  dest->id.dataCopy = DIET_ORIGINAL;
  dest->mode = src->mode;
  dest->base_type = src->generic.base_type;

  if (__mrsh_data_desc_type(dest, src) != 0) {
    return (1);
  }
  return (0);
}


int
mrsh_data(corba_data_t* dest, diet_data_t* src, int release)
{
  long unsigned int size = (long unsigned int) data_sizeof(&(src->desc));
  CORBA::Char* value(NULL);

  if (mrsh_data_desc(&(dest->desc), &(src->desc)))
    return 1;

#if ! HAVE_JUXMEM
  if (src->desc.generic.type == DIET_FILE) {
    char* path = src->desc.specific.file.path;
 
    if (path && strcmp("", path)) {
      ifstream infile(path);
      value = SeqChar::allocbuf(size);
      if (!infile) {
        MRSH_ERROR("cannot open file " << path << " for reading", 1);
      }
      for (unsigned int i = 0; i < size; i++) {
        value[i] = infile.get();
      }
      infile.close();
    } else {
      value = SeqChar::allocbuf(1);
      value[0] = '\0';
    }
  } else {
#endif // ! HAVE_JUXMEM
    if (src->value != NULL) {
      value = (CORBA::Char*)src->value;
    }
#if ! HAVE_JUXMEM
  }
#endif // ! HAVE_JUXMEM
  if(value == NULL) 
    dest->value.length(0);
  else {
#ifdef WITH_ENDIANNESS
    /*
     * Manage endianness
     */
    if ((!little_endian) && 
        (src->desc.generic.type != DIET_FILE) && 
        (src->desc.generic.type != DIET_STRING) && 
        (src->desc.generic.type != DIET_PARAMSTRING)
        ) {
      TRACE_TEXT (TRACE_ALL_STEPS, "  ** endian_swap 1" << endl);
      endian_swap(value, size, type_sizeof(src->desc.generic.base_type) );
    } // end if
#endif // WITH_ENDIANNESS
    dest->value.replace(size, size, value, release); // 0 if persistent 1 elsewhere
  }
//   cout << "mrsh_data: value is " << dest->value.get_buffer() << "\n";
  return 0;
}

/**************************************************************************
 *  Marshall/ Unmarshall data handler                                     *
 *************************************************************************/
static int
__mrsh_data_id_desc(corba_data_desc_t* dest,
                    const diet_data_desc_t* const src){

  dest->id.idNumber = CORBA::string_dup(src->id);
 
  dest->id.state    = DIET_FREE;
  dest->id.dataCopy = DIET_ORIGINAL;

  return (0);
}

static void
__mrsh_data_id(corba_data_t* dest, const diet_data_t* const src) {
  __mrsh_data_id_desc(&(dest->desc), &(src->desc));
  dest->value.length(0); 
}

/****************************************************************************/
/* Data structure unmarshalling                                             */
/****************************************************************************/


inline int
unmrsh_scalar_desc(diet_data_desc_t* dest, const corba_data_desc_t* src)
{
  void* value(NULL);
  diet_base_type_t bt = (diet_base_type_t)src->base_type;
  char* id = CORBA::string_dup(src->id.idNumber);

  switch(bt) {
  case DIET_CHAR: // Impossible to extract a Char or an Octet from an Any.
  case DIET_SHORT: {
    // must be done C fashion, because freed in a C client?
    value = (void*) new short;
    src->specific.scal().value >>= *((CORBA::Short*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
    //    delete((short*)value) ;
    break;
  }
  case DIET_INT: {
    value = (void*) new int;
    src->specific.scal().value >>= *((CORBA::Long*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
    //    delete((int*)value) ;
    break;
  }
  case DIET_LONGINT: {
    value = (void*) new long long int;
    src->specific.scal().value >>= *((CORBA::Long*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
    //    delete((long long int*)value) ;
    break;
  }
  case DIET_FLOAT: {
    value = (void*) new float;
    src->specific.scal().value >>= *((CORBA::Float*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
    //    delete((float*)value) ;
    break;
  }
  case DIET_DOUBLE: {
    value = (void*) new double;
    src->specific.scal().value >>= *((CORBA::Double*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
    //    delete((double*)value) ;
    break;
  }
#if HAVE_COMPLEX
  case DIET_SCOMPLEX:
  case DIET_DCOMPLEX:
#endif // HAVE_COMPLEX
  default:
    MRSH_ERROR("base type " << bt << " not implemented", 1);
  }
  return 0;
}

int
unmrsh_data_desc(diet_data_desc_t* dest, const corba_data_desc_t* const src)
{
  char *id = NULL;
  diet_base_type_t bt = (diet_base_type_t)src->base_type;

  id = CORBA::string_dup(src->id.idNumber);

  switch ((diet_data_type_t) src->specific._d()) {
  case DIET_SCALAR: {
    if (unmrsh_scalar_desc(dest, src))
      return 1;
    break;
  }
  case DIET_VECTOR: {
    vector_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt,
                    src->specific.vect().size);
    break;
  }
  case DIET_MATRIX: {
    matrix_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt,
                    src->specific.mat().nb_r, src->specific.mat().nb_c,
                    (diet_matrix_order_t) src->specific.mat().order);
    break;
  }
  case DIET_STRING: {
    string_set_desc(dest, id, (diet_persistence_mode_t)src->mode,
                    src->specific.str().length);
    break;
  }
  case DIET_PARAMSTRING: {
    paramstring_set_desc(dest,
                         id,
                         (diet_persistence_mode_t)src->mode,
                         src->specific.pstr().length,
                         src->specific.pstr().param);
//     cout << "unmrsh_data_desc: param is " << src->specific.pstr().param << "\n";
    break;
  }
  case DIET_FILE: {
    dest->id = id;
    dest->mode = (diet_persistence_mode_t)src->mode;
    diet_generic_desc_set(&(dest->generic), DIET_FILE, DIET_CHAR);
    dest->specific.file.size = src->specific.file().size;
#if HAVE_JUXMEM
    dest->specific.file.path = CORBA::string_dup(src->specific.file().path);
#else
    /* File name is different on SeD than on client. Here, init only */
    dest->specific.file.path = NULL;
#endif // HAVE_JUXMEM
    break;
  }
  default:
    MRSH_ERROR("type " << src->specific._d() << " not implemented", 1);
  }
  return 0;
}


int
unmrsh_data(diet_data_t* dest, corba_data_t* src, int upDown)
{  
  static int uniqDataID=0 ;
  static omni_mutex uniqDataIDMutex ;
  
  if( (src->desc.mode == DIET_VOLATILE) && (upDown == 1) ){ 
    char *tmp=NULL;
    src->desc.id.idNumber=CORBA::string_dup(tmp); // CORBA frees old mem
  }
  if ( unmrsh_data_desc(&(dest->desc),&(src->desc)) )
    return 1;
#if ! HAVE_JUXMEM
  if (src->desc.specific._d() == (long) DIET_FILE) {
    dest->desc.specific.file.size = src->desc.specific.file().size;
    if ((src->desc.specific.file().path != NULL)
        && strcmp("", src->desc.specific.file().path)) {
      char* in_path   = CORBA::string_dup(src->desc.specific.file().path);
      char* file_name = strrchr(in_path, '/');
      /* FIXME: erase if ok:   char* out_path  = new char[256]; */
      char * out_path  = ms_stralloc(256) ;
      pid_t pid = getpid();
      int temp ;
      
      if(strncmp(in_path,"/tmp/DIET_",10) != 0) {
	uniqDataIDMutex.lock() ;
	uniqDataID++ ;
	temp = uniqDataID ;
	uniqDataIDMutex.unlock() ;
	
	sprintf(out_path, "/tmp/DIET_%d.%d.%ld_%s",pid,temp, random()
		,(file_name) ? (char*)(1 + file_name) : in_path);
	ofstream outfile(out_path);

        for (int i = 0; i < src->desc.specific.file().size; i++) {
          outfile.put(src->value[i]);
        }
        dest->desc.specific.file.path = out_path;
        CORBA::string_free(in_path);
      } else {
	if(upDown == 1) {
	  sprintf(out_path,"%s",in_path);
	
	  ofstream outfile(out_path);  
	  for (int i = 0; i < src->desc.specific.file().size; i++) {
	    outfile.put(src->value[i]);
	  }
	  dest->desc.specific.file.path = out_path;
	  CORBA::string_free(in_path);
	}else{	
	
	  dest->desc.specific.file.path = in_path;
	  
	  /* FIXME: erase me if of: delete[] out_path; */
	  CORBA::string_free( out_path ) ;
	}	
      }
    } else if (src->desc.specific.file().size != 0) {
      INTERNAL_WARNING(__FUNCTION__ << ": file structure is vicious");
    } else {
      /* FIXME: erase me if ok: dest->desc.specific.file.path = strdup(""); */
      dest->desc.specific.file.path = CORBA::string_dup("");
    }
  } else {
#endif // ! HAVE_JUXMEM
    if (src->value.length() == 0) {
      // TODO: should be allocated with new x[] to match delete[] used
      // by omniORB.  But ... what is the type?
      dest->value = malloc(data_sizeof(&(dest->desc)));
 
    } else {
      if(upDown==0){ /** Need to know if it is in the client -> SeD way or in the SeD-> client way */
        if (src->desc.mode != DIET_VOLATILE) {
          //               int size = data_sizeof(&(src->desc));
          //        cout << "value of data size = " << size << endl;
          //char *p =(char *)malloc(size*sizeof(char));
          //for(int i=0; i < size; i++)
          //  p[i] = src->value[i];
          //  dest->value = p; //memcopy
          dest->value = (char*)src->value.get_buffer(0);
#ifdef WITH_ENDIANNESS
          /** 
           * Manage endianness
           */
          dest->value = (char*)src->value.get_buffer(0);
          if ( (!little_endian) && 
               (src->desc.specific._d() != DIET_FILE) && 
               (src->desc.specific._d() != DIET_STRING) && 
               (src->desc.specific._d() != DIET_PARAMSTRING)
               ) {
            TRACE_TEXT (TRACE_ALL_STEPS, "  ** endian_swap 2" << endl);
            endian_swap(dest->value, src->value.length(), 
                        type_sizeof(dest->desc.generic.base_type) );
          } // end if
#endif // WITH_ENDIANNESS
       } else {
          CORBA::Boolean orphan = (src->desc.mode == DIET_VOLATILE);
#ifdef WITH_ENDIANNESS
          size_t lenx =  src->value.length();
#endif // WITH_ENDIANNESS
          dest->value = (char*)src->value.get_buffer(orphan);
#ifdef WITH_ENDIANNESS
          if ((!little_endian) && 
              (src->desc.specific._d() != DIET_FILE) && 
              (src->desc.specific._d() != DIET_STRING) && 
              (src->desc.specific._d() != DIET_PARAMSTRING)
              ) {
            TRACE_TEXT (TRACE_ALL_STEPS, "  ** endian_swap 3" << lenx << endl);
            endian_swap(dest->value, lenx, 
                        type_sizeof(dest->desc.generic.base_type) );
          } // end if
#endif // WITH_ENDIANNESS
       } 
      } else { // for out args when send to client
#ifndef WITH_ENDIANNESS
        dest->value = (char*)src->value.get_buffer(1);
#else
        /** 
         * Manage endianness
         */
        size_t lenx =  src->value.length();
        dest->value = (char*)src->value.get_buffer(1);
        if ((!little_endian) &&
            (src->desc.specific._d() != DIET_FILE) && 
            (src->desc.specific._d() != DIET_STRING) && 
            (src->desc.specific._d() != DIET_PARAMSTRING)
            ) {
          TRACE_TEXT (TRACE_ALL_STEPS, "  ** endian_swap 4" << endl);
          endian_swap(dest->value, lenx, 
                      type_sizeof(dest->desc.generic.base_type) );
        } // end if
#endif
      }
    }
#if ! HAVE_JUXMEM
  }
#endif // ! HAVE_JUXMEM
//   cout << "unmrsh_data: value is " << dest->value << "\n";
  return 0;
}


/****************************************************************************/
/* Aggregator descriptor -> Corba aggregator descriptor                     */
/****************************************************************************/

int mrsh_aggregator_desc(corba_aggregator_desc_t* dest,
                         const diet_aggregator_desc_t* const src)
{
  switch (src->agg_method) {
  case DIET_AGG_DEFAULT:
    {
      corba_agg_default_t d;

	  d.dummy = 0; /* init value to avoid warning */
      dest->agg_specific.agg_default(d);
      break;
    }
  case DIET_AGG_PRIORITY:
    {
      /* set the type-specific structure */
      corba_agg_priority_t p;
      dest->agg_specific.agg_priority(p);

      /* set the lenth dynamic array */
      int numPValues = src->agg_specific.agg_specific_priority.p_numPValues;
      dest->agg_specific.agg_priority().priorityList.length(numPValues);

      /* fill in the values */
      for (int pIter = 0 ; pIter < numPValues ; pIter++) {
        dest->agg_specific.agg_priority().priorityList[pIter] =
          src->agg_specific.agg_specific_priority.p_pValues[pIter];
      }
      break;
    }
/* New : For scheduler load support. */
#ifdef USERSCHED
  case DIET_AGG_USER:
    {
      corba_agg_user_t d;
      d.dummy = 0;
      dest->agg_specific.agg_user(d);
      break;
    }

#endif
/*************************************/
  default:
    INTERNAL_WARNING(__FUNCTION__ <<
                     ": unrecognized aggregation method (" <<
                     src->agg_method <<
                     ")");
  }
  return (0);
}

/****************************************************************************/
/* Service profile -> Corba profile descriptor  (for service descriptions)  */
/****************************************************************************/


int
mrsh_profile_desc(corba_profile_desc_t* dest, const diet_profile_desc_t* src)
{
  dest->path       = CORBA::string_dup(src->path); // deallocates old dest->path
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc.length(src->last_out + 1);
  for (int i = 0; i <= src->last_out; i++) {
    (dest->param_desc[i]).base_type = (src->param_desc[i]).base_type;
    (dest->param_desc[i]).type      = (src->param_desc[i]).type;
  }
#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
  dest->parallel_flag = src->parallel_flag ;
#endif

  mrsh_aggregator_desc(&(dest->aggregator), &(src->aggregator));

  return 0;
}


/****************************************************************************/
/* Problem profile -> corba profile description (client requests to agent)  */
/****************************************************************************/

int
mrsh_pb_desc(corba_pb_desc_t* dest, const diet_profile_t* const src)
{
  dest->path       = CORBA::string_dup(src->pb_name); // frees old dest->path
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc.length(src->last_out + 1);
  for (int i = 0; i <= src->last_out; i++) {
    /** Hack for correctly building CORBA profile. 
	JuxMem does not require a call to the MA */
#if HAVE_JUXMEM
    mrsh_data_desc(&(dest->param_desc[i]), &(src->parameters[i].desc));
#else
    if(src->parameters[i].desc.id == NULL) {
      mrsh_data_desc(&(dest->param_desc[i]), &(src->parameters[i].desc));
    } else {
      __mrsh_data_id_desc(&(dest->param_desc[i]), &(src->parameters[i].desc));
    }
#endif // HAVE_JUXMEM
  }
#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
  dest->parallel_flag = src->parallel_flag ;
  dest->nbprocs    = src->nbprocs ;
  dest->nbprocess  = src->nbprocess ;
  // No walltime, user has no idea about it
#endif
  return 0;
}


/****************************************************************************/
/* Client sends its data to server ...                                      */
/****************************************************************************/

int
mrsh_profile_to_in_args(corba_profile_t* dest, const diet_profile_t* src)
{
  int i;

#if defined HAVE_BATCH || defined HAVE_ALT_BATCH 
  dest->parallel_flag = src->parallel_flag ;
  dest->nbprocs    = src->nbprocs ;
  dest->nbprocess  = src->nbprocess ;
  dest->walltime   = src->walltime ;
#endif
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->parameters.length(src->last_out + 1);
  dest->dietReqID  = src->dietReqID ;

   for (i = 0; i <= src->last_inout; i++) {
     if(src->parameters[i].desc.id) {
#if ! HAVE_JUXMEM
       __mrsh_data_id(&(dest->parameters[i]), &(src->parameters[i]));
#else 
       mrsh_data_desc(&(dest->parameters[i].desc), &(src->parameters[i].desc));
       dest->parameters[i].value.replace(0, 0, NULL, 1);
       dest->parameters[i].value.length(0);
#endif
     } else {
       if (src->parameters[i].value == NULL &&
           !diet_is_persistent(src->parameters[i]) &&
           src->parameters[i].desc.generic.type != DIET_FILE) {
         // Warning : For DIET_FILE parameters, value field is NULL
         // although it is volatile
         ERROR(__FUNCTION__ << ": IN or INOUT parameter "
               << i << " is volatile but contains no data", 1);
       } else {
         if (mrsh_data(&(dest->parameters[i]), &(src->parameters[i]), 0)) {
           return 1;
         }
       }
     }
   } 
   
   for (; i <= src->last_out; i++) {
     if(mrsh_data_desc(&(dest->parameters[i].desc),&(src->parameters[i].desc)))
       return 1;
     dest->parameters[i].value.replace(0, 0, NULL, 1);
   }

   // YC: for the moment, dest data have id set to ""
  return 0;
}


/****************************************************************************/
/* Server receives client data ...                                          */
/****************************************************************************/

int
cvt_arg(diet_data_t* dest, diet_data_t* src,
        diet_convertor_function_t f, int duplicate_value)
{
  switch(f) {
  case DIET_CVT_IDENTITY: {
    (*dest) = (*src);
    if (duplicate_value && src->value) {
      size_t size = data_sizeof(&(src->desc));
      dest->value = new char[size]; 
#ifdef WITH_ENDIANNESS
      /** 
       * Manage endianness
       */    
      if ((!little_endian) &&
          (src->desc.generic.type != DIET_FILE) && 
          (src->desc.generic.type != DIET_STRING) && 
          (src->desc.generic.type != DIET_PARAMSTRING)
          ) {
        // Not necessary ?
        //        endian_swap(src->value, size, type_sizeof(src->desc.generic.base_type) );
      } // end if
#endif // WITH_ENDIANNESS
      memcpy(dest->value, src->value, size);
      if (dest->desc.generic.type == DIET_SCALAR)
        dest->desc.specific.scal.value = dest->value;
    }
    break;
  }
  case DIET_CVT_VECT_SIZE: {
    size_t* size = new size_t(src->desc.specific.vect.size);
    diet_scalar_set(dest, size, DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_MAT_NB_ROW: {
    size_t* nb_r = new size_t(src->desc.specific.mat.nb_r);
    diet_scalar_set(dest, nb_r, DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_MAT_NB_COL: {
    size_t* nb_c = new size_t(src->desc.specific.mat.nb_c);
    diet_scalar_set(dest, nb_c, DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_MAT_ORDER: {
    char* t(NULL);
    // FIXME test on order !!!!
    switch (src->desc.specific.mat.order) {
    case DIET_ROW_MAJOR: t = new char('N'); break;
    case DIET_COL_MAJOR: t = new char('T'); break;
    default: {
      MRSH_ERROR("invalid order for matrix", 1);
    }
    }
    diet_scalar_set(dest, t, DIET_VOLATILE, DIET_CHAR);
    break;
  }
  case DIET_CVT_STR_LEN: {
    size_t* lgth = new size_t(src->desc.specific.str.length);
    diet_scalar_set(dest, lgth, DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_FILE_SIZE: {
    size_t* size = new size_t(src->desc.specific.file.size);
    diet_scalar_set(dest, size, DIET_VOLATILE, DIET_INT);
    break;
  }
  default: {
    MRSH_ERROR("invalid convertor function", 1);
  }
  }

  return 0;
}


int
unmrsh_in_args_to_profile(diet_profile_t* dest, corba_profile_t* src,
                          const diet_convertor_t* cvt)
{
  /* This keeps all umarshalled arguments */
  diet_data_t** src_params = new diet_data_t*[src->last_out + 1];
  for (int i = 0; i < (src->last_out + 1); i++) {
    src_params[i] = NULL;
  }

#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
  dest->parallel_flag = src->parallel_flag ;
  dest->nbprocs    = src->nbprocs ;
  dest->nbprocess  = src->nbprocess ;
  dest->walltime   = src->walltime ;
#endif
  dest->pb_name    = cvt->path;
  dest->last_in    = cvt->last_in;
  dest->last_inout = cvt->last_inout;
  dest->last_out   = cvt->last_out;
  dest->parameters = new diet_data_t[cvt->last_out + 1];
  dest->dietReqID  = src->dietReqID ;

  for (int i = 0; i <= cvt->last_out; i++) {
    diet_data_t* dd_tmp(NULL);
    int arg_idx = cvt->arg_convs[i].in_arg_idx;
    int duplicate_value = 0;

    if ((arg_idx >= 0) && (arg_idx <= src->last_out)) {
      // Each time the cvt function is IDENTITY, unmrsh the data, even if
      // it has already been done (ie duplicate the value)
      if (src_params[arg_idx] == NULL) {
        src_params[arg_idx] = new diet_data_t;

        unmrsh_data(src_params[arg_idx], &(src->parameters[arg_idx]),0); 

      } else if (cvt->arg_convs[i].f == DIET_CVT_IDENTITY) {
        duplicate_value = 1;
      }
      dd_tmp = src_params[arg_idx];
    } else {
      dd_tmp = cvt->arg_convs[i].arg;
      // Duplicate the value field, so that it can be freed by user
      duplicate_value = 1;
    }

    if (cvt_arg(&(dest->parameters[i]), dd_tmp,
                cvt->arg_convs[i].f, duplicate_value)) {
      delete src_params;
      MRSH_ERROR("cannot convert client problem profile"
                 << "to server profile", 1);
    }
  }

  for (int i = 0; i < (src->last_out + 1); i++) {
    if (src_params[i] != NULL) {
      delete src_params[i]; 
    }
  }
  delete[] src_params;

  return 0;
}


/****************************************************************************/
/* Server sends results ...                                                 */
/****************************************************************************/

int
mrsh_profile_to_out_args(corba_profile_t* dest, const diet_profile_t* src,
                         const diet_convertor_t* cvt)
{
  int i, arg_idx;
  int* args_filled = // Use calloc to set all elements to NULL
    (int*) calloc(dest->last_out + 1, sizeof(int));
  diet_data_t dd;

  for (i = cvt->last_in + 1; i <= cvt->last_out; i++) {
    arg_idx = cvt->arg_convs[i].out_arg_idx;
    if ((arg_idx >= 0) && (arg_idx <= dest->last_out)) {
      if( dest->parameters[arg_idx].desc.specific._d() == DIET_FILE
	  && diet_is_persistent(dest->parameters[arg_idx]) ) {
	src->parameters[i].desc.id = 
	  CORBA::string_dup(dest->parameters[arg_idx].desc.id.idNumber);
      }
      dd = src->parameters[i];
      if (!args_filled[arg_idx]) {
        // For IN arguments, reset value fields to NULL, so that the ORB does
        // not carry in data backwards when the RPC returns.
        if (arg_idx <= dest->last_in) {
          dest->parameters[arg_idx].value.replace(0, 0, NULL, 1);
        } else {
          // The size of files must be (re)computed
          if (dd.desc.generic.type == DIET_FILE) {
            if ((dd.desc.specific.file.path)
                && (strcmp("", dd.desc.specific.file.path))) {
              struct stat buf;
              int status;
              if ((status = stat(dd.desc.specific.file.path, &buf)))
                return status;
              if (!(buf.st_mode & S_IFREG))
                return 2;
              dd.desc.specific.file.size = (size_t) buf.st_size;
            } else {
              dd.desc.specific.file.size = 0;
            }
          }
	
#if HAVE_JUXMEM
	  if (diet_is_persistent(dd)) {
	    mrsh_data_desc(&(dest->parameters[arg_idx].desc), &(dd.desc));
	    dest->parameters[arg_idx].value.replace(0, 0, NULL, 1);
	    dest->parameters[arg_idx].value.length(0);
	  } else {
#endif
          if (mrsh_data(&(dest->parameters[arg_idx]), &dd,
                        !diet_is_persistent(dd)))
            return 1;
#if HAVE_JUXMEM
	  }
#endif
        }
        args_filled[arg_idx] = 1;
      }
    }
  }
  for (i = dest->last_in + 1; i <= dest->last_out; i++) {
    if (!args_filled[i]) {
      INTERNAL_WARNING(__FUNCTION__
                       << ": could not reconvert all INOUT and OUT arguments");
      break;
    }
  }

  free(args_filled);
  return 0;
}


/****************************************************************************/
/* Client receives server data ...                                          */
/****************************************************************************/

// INOUT parameters should have been set correctly by the ORB.
// But their descriptions could have been slightly altered
//  (matrix dimensions, for instance)
int
unmrsh_out_args_to_profile(diet_profile_t* dpb, corba_profile_t* cpb)
{
  int i;

  if (   (dpb->last_in       != cpb->last_in)
         || (dpb->last_inout != cpb->last_inout)
         || (dpb->last_out   != cpb->last_out)
	 )
    return 1;

#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
  // In case client wants to know how many procs and process have been used
  // but other info like walltime is useless
  dpb->nbprocs = cpb->nbprocs ;
  dpb->nbprocess = cpb->nbprocess ;
#endif

  // Unmarshal INOUT parameters descriptions only ; indeed, the ORB has filled
  // in the memory zone pointed at by the diet_data value field, since the
  // marshalling was performed with replace method.
  for (i = dpb->last_in + 1; i <= dpb->last_inout; i++) {
    corba_data_desc_t* cdd = &(cpb->parameters[i].desc);

    // Special case for INOUT files: rewrite in the same file.
    if (cdd->specific._d() == (long) DIET_FILE) {
      char* inout_path = dpb->parameters[i].desc.specific.file.path;
   
      int   size = cdd->specific.file().size;
      ofstream inoutfile(inout_path);
      dpb->parameters[i].desc.specific.file.size = size;
      for (int j = 0; j < size; j++) {
        inoutfile.put(cpb->parameters[i].value[j]);
      }
    } else {
      if(cpb->parameters[i].desc.mode == DIET_VOLATILE){ 
        char *tmp=NULL;
        cdd->id.idNumber=CORBA::string_dup(tmp);
    }
      unmrsh_data_desc(&(dpb->parameters[i].desc), cdd);
    }
  }

  // Unmarshal OUT parameters
  for (; i <= dpb->last_out; i++) {
    unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]),1);
  }
  return 0;
}

// Dec 2006: In async mode, INOUT parameters are not changed
// Force the unmarshalling of such parameters
// We suppose that the previous function (unmrsh_out_args_to_profile) was
// already called
int
unmrsh_inout_args_to_profile(diet_profile_t* dpb, corba_profile_t* cpb)
{
  int i;

  if (   (dpb->last_in       != cpb->last_in)
         || (dpb->last_inout != cpb->last_inout)
         || (dpb->last_out   != cpb->last_out)
	 )
    return 1;

  // Unmarshal INOUT parameters
  for (i = dpb->last_in + 1; i <= dpb->last_inout; i++) {
    unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]),1);
  }

  return 0;
} // end unmrsh_inout_args_to_profile


#ifdef HAVE_WORKFLOW
/*
 * Workflow structure marshaling *
 */
int
mrsh_wf_desc(corba_wf_desc_t* dest,
	     const diet_wf_desc_t* const src)
{
  dest->abstract_wf = CORBA::string_dup(src->abstract_wf);
  return 0;
}
#endif

#ifdef WITH_ENDIANNESS
/**
 * For big endian architecture, reswap in and inout parameters
 */
int 
post_call(diet_profile_t * profile) {
  for (unsigned int ix=0; ix<=profile->last_inout; ix++) {
    if ( (profile->parameters[ix].desc.generic.type != DIET_FILE) && 
         (profile->parameters[ix].desc.generic.type != DIET_STRING) && 
         (profile->parameters[ix].desc.generic.type != DIET_PARAMSTRING) ) {
      endian_swap(profile->parameters[ix].value, 
                  data_sizeof(&(profile->parameters[ix].desc)), 
                  type_sizeof(profile->parameters[ix].desc.generic.base_type) );
    } // end if
  } // end for
} // end post_call

#endif
