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
 * Revision 1.29  2003/08/09 17:31:38  pcombes
 * Include path in the diet_profile_desc structure.
 *
 * Revision 1.28  2003/08/01 19:26:07  pcombes
 * The conversions to FAST problems are now managed by FASTMgr.
 *
 * Revision 1.27  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.26  2003/07/04 09:47:55  pcombes
 * Use new ERROR and WARNING macros.
 *
 * Revision 1.25  2003/04/10 12:40:22  pcombes
 * Use the TRACE_LEVEL of the debug module. Manage the data ID.
 *
 * Revision 1.24  2003/02/07 17:04:12  pcombes
 * Refine convertor API: arg_idx is splitted into in_arg_idx and out_arg_idx.
 *
 * Revision 1.21  2003/01/22 17:11:54  pcombes
 * API 0.6.4 : istrans -> order (row- or column-major)
 *
 * Revision 1.19  2003/01/13 18:06:13  pcombes
 * Add inout files management.
 *
 * Revision 1.16  2002/11/22 13:36:12  lbertsch
 * Added alpha linux support
 * Added a package for statistics and some traces
 *
 * Revision 1.15  2002/11/15 17:15:32  pcombes
 * FAST integration complete ...
 *
 * Revision 1.10  2002/10/04 16:17:09  pcombes
 * Integrate former hardcoded.h in configuration files. If DIET is compiled
 * without FAST, the correspunding entries in the cfg files can be omitted.
 ****************************************************************************/


#include <iostream>
using namespace std;
#include <fstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "marshalling.hh"
#include "debug.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;


#define MRSH_ERROR(formatted_msg,return_value)                       \
  INTERNAL_ERROR(__FUNCTION__ << ": " << formatted_msg, return_value)


/****************************************************************************/
/* Data structure marshalling                                               */
/****************************************************************************/

inline int
mrsh_scalar_desc(corba_data_desc_t* dest, diet_data_desc_t* src)
{
  corba_scalar_specific_t scal;

  dest->specific.scal(scal);
  if (!(src->specific.scal.value)) {
    dest->specific.scal().value <<= (CORBA::Double) 0;
  } else {
    switch (src->generic.base_type) {
    case DIET_CHAR:
    case DIET_BYTE: {
      char scal = *((char*)(src->specific.scal.value));
      dest->specific.scal().value <<= (short) scal;
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
mrsh_data_desc(corba_data_desc_t* dest, diet_data_desc_t* src)
{
  if (src->id)
    dest->id = CORBA::string_dup(src->id); // deallocates old dest->id
  dest->mode = src->mode;
  dest->base_type = src->generic.base_type;
  switch (src->generic.type) {
  case DIET_SCALAR: {
    if (mrsh_scalar_desc(dest, src))
      return 1;
    break;
  }
  case DIET_VECTOR: {
    corba_vector_specific_t vect;
    dest->specific.vect(vect);
    dest->specific.vect().size = src->specific.vect.size;
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
    dest->specific.str(str);
    dest->specific.str().length = src->specific.str.length;
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
  return 0;
}


int
mrsh_data(corba_data_t* dest, diet_data_t* src, int release)
{
  long unsigned int size = (long unsigned int) data_sizeof(&(src->desc));
  CORBA::Char* value(NULL);
  
  if (mrsh_data_desc(&(dest->desc), &(src->desc)))
    return 1;
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
    value = (CORBA::Char*)src->value;
  }
  dest->value.replace(size, size, value, release);
  return 0;
}


/****************************************************************************/
/* Data structure unmarshalling                                             */
/****************************************************************************/


inline int
unmrsh_scalar_desc(diet_data_desc_t* dest, const corba_data_desc_t* src)
{
  void* value(NULL);
  diet_base_type_t bt = (diet_base_type_t)src->base_type;
  char* id = CORBA::string_dup(src->id);

  switch(bt) {
  case DIET_CHAR:
  case DIET_BYTE: {
    value = (void*) new char;
    src->specific.scal().value >>= *((CORBA::Short*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
    break;
  }
  case DIET_INT: {
    value = (void*) new int;
    src->specific.scal().value >>= *((CORBA::Long*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
    break;
  }
  case DIET_LONGINT: {
    value = (void*) new long long int;
    src->specific.scal().value >>= *((CORBA::Long*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
    break;
  }
  case DIET_FLOAT: {
    value = (void*) new float;
    src->specific.scal().value >>= *((float*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
    break;
  }
  case DIET_DOUBLE: {
    value = (void*) new double;
    src->specific.scal().value >>= *((double*)(value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t)src->mode, bt, value);
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
unmrsh_data_desc(diet_data_desc_t* dest, const corba_data_desc_t* src)
{
  diet_base_type_t bt = (diet_base_type_t)src->base_type;
  char* id = CORBA::string_dup(src->id);
  
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
  case DIET_FILE: {
    dest->id = id;
    dest->mode = (diet_persistence_mode_t)src->mode;
    diet_generic_desc_set(&(dest->generic), DIET_FILE, DIET_CHAR);
    dest->specific.file.size = src->specific.file().size;
    dest->specific.file.path = NULL;
    break;
  }
  default:
    MRSH_ERROR("type " << src->specific._d() << " not implemented", 1);
  }
  return 0;
}


int
unmrsh_data(diet_data_t* dest, corba_data_t* src)
{
  if (unmrsh_data_desc(&(dest->desc), &(src->desc)))
    return 1;
  if (src->desc.specific._d() == (long) DIET_FILE) {
    dest->desc.specific.file.size = src->desc.specific.file().size;
    if ((src->desc.specific.file().path != NULL)
	&& strcmp("", src->desc.specific.file().path)) {
      char* in_path   = CORBA::string_dup(src->desc.specific.file().path);
      char* file_name = strrchr(in_path, '/');
      char* out_path  = new char[256];
      pid_t pid = getpid();
      sprintf(out_path, "/tmp/DIET_%d_%s", pid,
	      (file_name) ? (char*)(1 + file_name) : in_path);
    
      ofstream outfile(out_path);
      for (int i = 0; i < src->desc.specific.file().size; i++) {
	outfile.put(src->value[i]);
      }
      CORBA::string_free(in_path);
      dest->desc.specific.file.path = out_path;
    } else if (src->desc.specific.file().size != 0) {
      INTERNAL_WARNING(__FUNCTION__ << ": file structure is vicious");
    } else {
      dest->desc.specific.file.path = strdup("");
    }
  } else {
    if (src->value.length() == 0) { // OUT case
      dest->value = malloc(data_sizeof(&(dest->desc)));
    } else {
      CORBA::Boolean orphan = 1; //(src->mode != DIET_VOLATILE);
      dest->value = (char*)src->value.get_buffer(orphan);
    }
  }
  return 0;
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
  return 0;
}


/****************************************************************************/
/* Problem profile -> corba profile description (for client requests)       */
/****************************************************************************/

int
mrsh_pb_desc(corba_pb_desc_t* dest, diet_profile_t* src)
{
  dest->path       = CORBA::string_dup(src->pb_name); // frees old dest->path
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc.length(src->last_out + 1);
  for (int i = 0; i <= src->last_out; i++) {
    mrsh_data_desc(&(dest->param_desc[i]), &(src->parameters[i].desc));
  }
  return 0;
}



/****************************************************************************/
/* Client sends its data ...                                                */
/****************************************************************************/

int
mrsh_profile_to_in_args(corba_profile_t* dest, const diet_profile_t* src)
{
  int i;
  
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->parameters.length(src->last_out + 1);
  for (i = 0; i <= src->last_inout; i++) {
    if (mrsh_data(&(dest->parameters[i]), &(src->parameters[i]), 0))
      return 1;
  }
  // For OUT parameters, marshal only descriptions for checking
  // purpose on SeD side, and let data emty.
  for (; i <= src->last_out; i++) {
    if (mrsh_data_desc(&(dest->parameters[i].desc), &(src->parameters[i].desc)))
      return 1;
    dest->parameters[i].value.replace(0, 0, NULL, 1);
  }
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
      dest->value = (char*)malloc(size);
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
    switch ((diet_matrix_order_t)(src->desc.specific.mat.order)) {
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
  diet_data_t** src_params = // Use calloc to set all elements to NULL
    (diet_data_t**) calloc(src->last_out + 1, sizeof(diet_data_t*));

  dest->last_in    = cvt->last_in;
  dest->last_inout = cvt->last_inout;
  dest->last_out   = cvt->last_out;
  dest->parameters = new diet_data_t[cvt->last_out + 1];

  
  for (int i = 0; i <= cvt->last_out; i++) {
    diet_data_t* dd_tmp(NULL);
    int arg_idx = cvt->arg_convs[i].in_arg_idx;
    int duplicate_value = 0;
    
    if ((arg_idx >= 0) && (arg_idx <= src->last_out)) {
      // Each time the cvt function is IDENTITY, unmrsh the data, even if
      // it has already been done (ie duplicate the value)
      if (!src_params[arg_idx]) {
	src_params[arg_idx] = new diet_data_t;
	unmrsh_data(src_params[arg_idx], &(src->parameters[arg_idx]));
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

  delete src_params;
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
	  if (mrsh_data(&(dest->parameters[arg_idx]), &dd,
			!diet_is_persistent(dd)))
	    return 1;
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

  delete [] args_filled;
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

  if (   (dpb->last_in    != cpb->last_in)
      || (dpb->last_inout != cpb->last_inout)
      || (dpb->last_out   != cpb->last_out))
    return 1;

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
      unmrsh_data_desc(&(dpb->parameters[i].desc), cdd);
    }
  }

  // Unmarshal OUT parameters
  for (; i <= dpb->last_out; i++) {
    unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]));
  }
  return 0;
}

