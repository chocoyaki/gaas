/****************************************************************************/
/* $Id$ */
/* DIET CORBA marshalling source code                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Frederic LOMBARD          - LIFC Besançon (France)                  */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.21  2003/01/22 17:11:54  pcombes
 * API 0.6.4 : istrans -> order (row- or column-major)
 *
 * Revision 1.20  2003/01/17 18:08:43  pcombes
 * New API (0.6.3): structures are not hidden, but the user can ignore them.
 *
 * Revision 1.19  2003/01/13 18:06:13  pcombes
 * Add inout files management.
 *
 * Revision 1.18  2002/12/03 19:08:23  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
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
 *
 * Revision 1.9  2002/10/03 17:58:13  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
 *
 * Revision 1.5  2002/08/30 16:50:12  pcombes
 * This version works as well as the alpha version from the user point of view,
 * but the API is now the one imposed by the latest specifications (GridRPC API
 * in its sequential part, config file for all parts of the platform, agent
 * algorithm, etc.)
 *  - Reduce marshalling by using CORBA types internally
 *  - Creation of a class ServiceTable that is to be replaced
 *    by an LDAP DB for the MA
 *  - No copy for client/SeD data transfers
 *  - ...
 *
 ****************************************************************************/


#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "marshalling.hh"
#include "debug.hh"

/**
 * Trace level
 */
// FIXME: should be used for traceLevel >= TRACE_STRUCTURES only ...
static int traceLevel;

void mrsh_set_trace_level(int level)
{
  traceLevel = level;
}


/****************************************************************************/
/* Data structure marshalling                                               */
/****************************************************************************/

inline int mrsh_scalar_desc(corba_data_desc_t *dest, diet_data_desc_t *src)
{
  corba_scalar_specific_t scal;

  dest->specific.scal(scal);
  if (!(src->specific.scal.value)) {
    dest->specific.scal().value <<= (CORBA::Double) 0;
  } else {
    switch (src->generic.base_type) {
    case DIET_CHAR:
    case DIET_BYTE: {
      char scal = *((char *)(src->specific.scal.value));
      dest->specific.scal().value <<= (short) scal;
      break;
    }
    case DIET_INT: {
      long scal = (long) *((int *)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Long) scal;
      break;
    }
    case DIET_LONGINT: {
      long long int scal = *((long long int *)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Long) scal;
      break;
    }
    case DIET_FLOAT: {
      float scal = *((float *)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Float) scal;
      break;
    }
    case DIET_DOUBLE: {
      double scal = *((double *)(src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Double) scal;
      break;
    }
#if HAVE_COMPLEX
    case DIET_SCOMPLEX:
    case DIET_DCOMPLEX:
#endif // HAVE_COMPLEX
    default:
      cerr << "mrsh_scalar_desc: Base type "
	   << src->generic.base_type << " not implemented.\n";
      return 1;
    }
  }
  
  return 0;
}


int mrsh_data_desc(corba_data_desc_t *dest, diet_data_desc_t *src)
{
  dest->id = 0;
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
    cerr << "mrsh_data_desc: Type "
	 << src->generic.type << " not implemented\n";
    return 1;
  }
  return 0;
}


int mrsh_data(corba_data_t *dest, diet_data_t *src, int release)
{
  long unsigned int size = (long unsigned int) data_sizeof(&(src->desc));
  CORBA::Char *value;
  char *path = src->desc.specific.file.path;
  
  if (mrsh_data_desc(&(dest->desc), &(src->desc)))
    return 1;
  if (src->desc.generic.type == DIET_FILE) {
    if (path && strcmp("", path)) {
      ifstream infile(path);
      value = SeqChar::allocbuf(size);
      if (!infile) {
	cerr << "DIET Error: cannot open file "
	     << path << " for reading.\n";
	return 1;
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
    value = (CORBA::Char *)src->value;
  }
  dest->value.replace(size, size, value, release);
  return 0;
}


/****************************************************************************/
/* Data structure unmarshalling                                             */
/****************************************************************************/


inline int unmrsh_scalar_desc(diet_data_desc_t *dest, const corba_data_desc_t *src)
{
  void *value;
  diet_base_type_t bt = (diet_base_type_t)src->base_type;
  
  switch(bt) {
  case DIET_CHAR:
  case DIET_BYTE: {
    value = new char;
    src->specific.scal().value >>= *((CORBA::Short *)(value));
    scalar_set_desc(dest, DIET_VOLATILE, bt, value);
    break;
  }
  case DIET_INT: {
    value = (void *) new int;
    src->specific.scal().value >>= *((CORBA::Long *)(value));
    scalar_set_desc(dest, DIET_VOLATILE, bt, value);
    break;
  }
  case DIET_LONGINT: {
    value = (void *) new long long int;
    src->specific.scal().value >>= *((CORBA::Long *)(value));
    scalar_set_desc(dest, DIET_VOLATILE, bt, value);
    break;
  }
  case DIET_FLOAT: {
    value = (void *) new float;
    src->specific.scal().value >>= *((float *)(value));
    scalar_set_desc(dest, DIET_VOLATILE, bt, value);
    break;
  }
  case DIET_DOUBLE: {
    value = (void *) new double;
    src->specific.scal().value >>= *((double *)(value));
    scalar_set_desc(dest, DIET_VOLATILE, bt, value);
    break;
  }
#if HAVE_COMPLEX
  case DIET_SCOMPLEX:
  case DIET_DCOMPLEX:
#endif // HAVE_COMPLEX
  default:
    cerr << "unmrsh_scalar_desc: Base type " << bt << " not implemented.\n";
    return 1;
  }
  return 0;
}

int unmrsh_data_desc(diet_data_desc_t *dest, const corba_data_desc_t *src)
{
  diet_base_type_t bt = (diet_base_type_t)src->base_type;

  switch ((diet_data_type_t) src->specific._d()) {
  case DIET_SCALAR: {
    if (unmrsh_scalar_desc(dest, src))
      return 1;
    break;
  }
  case DIET_VECTOR: {
    vector_set_desc(dest, DIET_VOLATILE, bt,
		    src->specific.vect().size);
    break;
  }
  case DIET_MATRIX: {
    matrix_set_desc(dest, DIET_VOLATILE, bt,
		    src->specific.mat().nb_r, src->specific.mat().nb_c,
		    (diet_matrix_order_t) src->specific.mat().order);
    break;
  }
  case DIET_STRING: {
    string_set_desc(dest, DIET_VOLATILE, src->specific.str().length);
    break;
  }
  case DIET_FILE: {
    diet_generic_desc_set(&(dest->generic), DIET_FILE, DIET_CHAR);
    dest->specific.file.size = src->specific.file().size;
    dest->specific.file.path = NULL;
    break;
  }
  default:
    cerr << "unmrsh_data_desc: Type "
	 << src->specific._d() << " not implemented\n";
    return 1;
  }
  return 0;
}

inline sf_type_cons_t diet_to_sf_type(const diet_data_type_t t)
{
  switch (t) {
  case DIET_SCALAR:
    return sf_type_cons_scal;
  case DIET_VECTOR:
  case DIET_STRING:
    return sf_type_cons_vect;
  case DIET_MATRIX:
    return sf_type_cons_mat;
  case DIET_FILE:
    return sf_type_cons_file;
  default:
    return sf_type_cons_count;
  }
}

inline sf_type_base_t diet_to_sf_base_type(const diet_base_type_t t)
{
  switch (t) {
  case DIET_CHAR:
  case DIET_BYTE:
    return sf_type_base_char;
  case DIET_INT:
  case DIET_LONGINT:
    return sf_type_base_int;
  case DIET_FLOAT:
  case DIET_DOUBLE:
    return sf_type_base_double;
#if HAVE_COMPLEX
  case DIET_SCOMPLEX:
  case DIET_DCOMPLEX:
#endif // HAVE_COMPLEX
  default:
    return sf_type_base_count;  
  }
}

int unmrsh_data_desc_to_sf(sf_data_desc_t *dest, const diet_data_desc_t *src) 
{
  dest->id        = 0;
  dest->type      = diet_to_sf_type(src->generic.type);
  dest->base_type = diet_to_sf_base_type(src->generic.base_type);
  
  switch (dest->type) {
     
  case sf_type_cons_vect:
    dest->ctn.vect.size = src->specific.vect.size;
    if (src->generic.type == DIET_STRING)
      dest->ctn.vect.size++;
    break;
    
  case sf_type_cons_mat:
    dest->ctn.mat.nb_l  = src->specific.mat.nb_r;
    dest->ctn.mat.nb_c  = src->specific.mat.nb_c;
    dest->ctn.mat.trans = src->specific.mat.order;
    break;
    
  case sf_type_cons_file:
    if (src->specific.file.path) {
      dest->ctn.file.size = src->specific.file.size;
      strcpy(dest->ctn.file.path, src->specific.file.path);
    } else {
      dest->ctn.file.size = 0;
      dest->ctn.file.path[0] = '\0';
    }
    break;

  case sf_type_cons_scal:
    switch (dest->base_type) {
    case sf_type_base_char:
      dest->ctn.scal.value = (void *) new char;
      *((short *)dest->ctn.scal.value) = *((short *)src->specific.scal.value);
      break;
    case sf_type_base_int:
      dest->ctn.scal.value = (void *) new int;
      *((long *)dest->ctn.scal.value) = *((long *)src->specific.scal.value);
      break;
    case sf_type_base_double:
      dest->ctn.scal.value = (void *) new double;
      *((double *)dest->ctn.scal.value) = *((double *)src->specific.scal.value);
      break;
    default:
      cerr << "unmrsh_data_desc_to_sf: Base type "
	   << dest->base_type << " unknown for FAST\n";
      return 1;
    }
    break;
    
  default:
    cerr << "unmrsh_data_desc_to_sf: Type "
	 << src->generic.type << " unknown for FAST\n";
    return 1;
  }
  return 0;
}


int unmrsh_data(diet_data_t *dest, corba_data_t *src, int only_value)
{
  if ((!only_value) && unmrsh_data_desc(&(dest->desc), &(src->desc)))
    return 1;
  if (src->desc.specific._d() == (long) DIET_FILE) {
    dest->desc.specific.file.size = src->desc.specific.file().size;
    if ((src->desc.specific.file().path != NULL)
	&& strcmp("", src->desc.specific.file().path)) {
      char *in_path   = CORBA::string_dup(src->desc.specific.file().path);
      char *file_name = strrchr(in_path, '/');
      char *out_path  = new char[256];
      pid_t pid = getpid();
      sprintf(out_path, "/tmp/DIET_%d_%s", pid,
	      (file_name) ? (char *)(1 + file_name) : in_path);
    
      ofstream outfile(out_path);
      for (int i = 0; i < src->desc.specific.file().size; i++) {
	outfile.put(src->value[i]);
      }
      CORBA::string_free(in_path);
      dest->desc.specific.file.path = out_path;
    } else if (src->desc.specific.file().size != 0) {
      cerr << "DIET unmrsh_data WARNING: file structure is vicious !\n";
    } else {
      dest->desc.specific.file.path = strdup("");
    }
  } else {
    if (src->value.length() == 0) { // OUT case
      dest->value = malloc(data_sizeof(&(dest->desc)));
    } else {
      CORBA::Boolean orphan = 1; //(src->persistence_mode != DIET_VOLATILE);
      dest->value = (char *)src->value.get_buffer(orphan);
    }
  }
  return 0;
}


/****************************************************************************/
/* Service profile -> Corba profile descriptor  (for service descriptions)  */
/****************************************************************************/


int mrsh_profile_desc(corba_profile_desc_t *dest,
		      diet_profile_desc_t *src, char *src_name)
{
  dest->path       = CORBA::string_dup(src_name);
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

int mrsh_pb_desc(corba_pb_desc_t *dest, diet_profile_t *src, char *src_name)
{
  dest->path       = CORBA::string_dup(src_name);
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
/* Problem profile -> FAST sf_inst_desc (for client requests)              */
/****************************************************************************/

int cvt_arg_desc(sf_data_desc_t *dest,
		 diet_data_desc_t *src, diet_convertor_function_t f)
{
  diet_data_desc_t *ddd = new diet_data_desc_t;
  
  switch(f) {
  case DIET_CVT_IDENTITY: {
    delete ddd;
    ddd = src;
    break;
  }
  case DIET_CVT_VECT_SIZE: {
    scalar_set_desc(ddd, DIET_VOLATILE, DIET_INT, src->specific.scal.value);
    break;
  }
  case DIET_CVT_MAT_NB_ROW: {
    scalar_set_desc(ddd, DIET_VOLATILE, DIET_INT, &src->specific.mat.nb_r);
    break;
  }
  case DIET_CVT_MAT_NB_COL: {
    scalar_set_desc(ddd, DIET_VOLATILE, DIET_INT, &src->specific.mat.nb_c);
    break;
  }
  case DIET_CVT_MAT_ORDER: {
    char t;
    // FIXME test on order !!!!
    switch (src->specific.mat.order) {
    case DIET_ROW_MAJOR: t = 'N';
    case DIET_COL_MAJOR: t = 'T';
    default: {
      cerr << "DIET conversion error: invalid order for matrix.\n";
      return 1;
    }
    }
    scalar_set_desc(ddd, DIET_VOLATILE, DIET_CHAR, &t);
    break;
  }
  case DIET_CVT_STR_LEN: {
    scalar_set_desc(ddd, DIET_VOLATILE, DIET_INT, &src->specific.str.length);
    break;
  }
  case DIET_CVT_FILE_SIZE: {
    scalar_set_desc(ddd, DIET_VOLATILE, DIET_INT, &src->specific.file.size);
    break;
  }
  default: {
    cerr << "DIET conversion error: invalid convertor function.\n";
    return 1;
  }
  }
  
  return unmrsh_data_desc_to_sf(dest, ddd);
}


int unmrsh_pb_desc_to_sf(sf_inst_desc_t *dest, const corba_pb_desc_t *src,
			 const diet_convertor_t *cvt)
{
  diet_data_desc_t **src_params =
    (diet_data_desc_t **) calloc(src->last_out + 1, sizeof(diet_data_desc_t **));
  
  dest->path       = CORBA::string_dup(cvt->path);
  dest->last_in    = cvt->last_in;
  dest->last_inout = cvt->last_inout;
  dest->last_out   = cvt->last_out;
  dest->param_desc = new sf_data_desc_t[cvt->last_out + 1];

  for (int i = 0; i <= cvt->last_out; i++) {
    diet_data_desc_t *ddd_tmp;
    int arg_idx = cvt->arg_convs[i].arg_idx;
    
    if ((arg_idx >= 0) && (arg_idx <= src->last_out)) {
      if (!src_params[arg_idx]) {
	ddd_tmp = new diet_data_desc_t;
	src_params[arg_idx] = ddd_tmp;
	unmrsh_data_desc(ddd_tmp, &(src->param_desc[arg_idx]));
      } else {
	ddd_tmp = src_params[arg_idx];
      }
    } else {
      ddd_tmp = &(cvt->arg_convs[i].arg->desc);
    }
    if (cvt_arg_desc(&(dest->param_desc[i]), ddd_tmp, cvt->arg_convs[i].f)) {
      cerr << "DIET conversion error: Cannot convert client profile "
	   << "to server profile.\n";
      delete src_params;
      return 1;
    }
  }
  
  delete src_params;
  return 0;
}


/****************************************************************************/
/* Client sends its data ...                                                */
/****************************************************************************/

int mrsh_profile_to_in_args(corba_profile_t *dest, const diet_profile_t *src)
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
    dest->parameters[i].value.replace(0, 0, NULL);
  }
  return 0;
}


/****************************************************************************/
/* Server receives client data ...                                          */
/****************************************************************************/
  
int cvt_arg(diet_data_t *dest, diet_data_t *src, diet_convertor_function_t f)
{
  switch(f) {
  case DIET_CVT_IDENTITY: {
    (*dest) = (*src);
    break;
  }
  case DIET_CVT_VECT_SIZE: {
    diet_scalar_set(dest, src->desc.specific.scal.value,
		    DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_MAT_NB_ROW: {
    diet_scalar_set(dest, &src->desc.specific.mat.nb_r,
		    DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_MAT_NB_COL: {
    diet_scalar_set(dest, &src->desc.specific.mat.nb_c,
		    DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_MAT_ORDER: {
    char *t;
    // FIXME test on order !!!!
    switch (src->desc.specific.mat.order) {
    case DIET_ROW_MAJOR: t = new char('N');
    case DIET_COL_MAJOR: t = new char('T');
    default: {
      cerr << "DIET conversion error: invalid order for matrix.\n";
      return 1;
    }
    }
    diet_scalar_set(dest, t, DIET_VOLATILE, DIET_CHAR);
    break;
  }
  case DIET_CVT_STR_LEN: {
    diet_scalar_set(dest, &src->desc.specific.str.length,
		    DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_FILE_SIZE: {
    diet_scalar_set(dest, &src->desc.specific.file.size,
		    DIET_VOLATILE, DIET_INT);
    break;
  }
  default: {
    cerr << "DIET conversion error: invalid convertor function.\n";
    return 1;
  }
  }
  
  return 0;
}


int unmrsh_in_args_to_profile(diet_profile_t *dest, corba_profile_t *src,
			      const diet_convertor_t *cvt)
{
  diet_data_t **src_params =
    (diet_data_t **) calloc(src->last_out + 1, sizeof(diet_data_t *));

  dest->last_in    = cvt->last_in;
  dest->last_inout = cvt->last_inout;
  dest->last_out   = cvt->last_out;
  dest->parameters = new diet_data_t[cvt->last_out + 1];

  
  for (int i = 0; i <= cvt->last_out; i++) {
    diet_data_t *dd_tmp;
    int arg_idx = cvt->arg_convs[i].arg_idx;
    
    if ((arg_idx >= 0) && (arg_idx <= src->last_out)) {
      if (!src_params[arg_idx]) {
	src_params[arg_idx] = new diet_data_t;
	unmrsh_data(src_params[arg_idx], &(src->parameters[arg_idx]), 0);
      }
      dd_tmp = src_params[arg_idx];
    } else {
      dd_tmp = cvt->arg_convs[i].arg;
    }
    if (cvt_arg(&(dest->parameters[i]), dd_tmp, cvt->arg_convs[i].f)) {
      cerr << "DIET conversion error: Cannot convert client problem profile"
	   << "to server profile.\n";
      delete src_params;
      return 1;
    }
  }

  delete src_params;
  return 0;
}
			      


/****************************************************************************/
/* Server sends results ...                                                 */
/****************************************************************************/

  
int recvt_arg(diet_data_t *dest, diet_data_t *src, diet_convertor_function_t f)
{
  switch(f) {
  case DIET_CVT_IDENTITY: {
    (*dest) = (*src);
    return 0;
  }
  case DIET_CVT_VECT_SIZE:
  case DIET_CVT_MAT_NB_ROW:
  case DIET_CVT_MAT_NB_COL:
  case DIET_CVT_MAT_ORDER:
  case DIET_CVT_STR_LEN:
  case DIET_CVT_FILE_SIZE:
    return 1;
  default: {
    cerr << "DIET conversion error: invalid convertor function.\n";
    return 2;
  }
  }
}


int mrsh_profile_to_out_args(corba_profile_t *dest, const diet_profile_t *src,
			     const diet_convertor_t *cvt)
{
  int i, arg_idx, res;
  int *args_filled = (int *) calloc(dest->last_out + 1, sizeof(int));
  diet_data_t dd;

  for (i = 0; i <= cvt->last_out; i++) {
    arg_idx = cvt->arg_convs[i].arg_idx;
    if ((arg_idx >= 0) && (arg_idx <= dest->last_out)) {
      res = recvt_arg(&dd, &(src->parameters[i]), cvt->arg_convs[i].f);
      if (res == 2) {
	cerr << "DIET conversion error: Cannot convert server profile "
	     << "to client problem profile.\n";
	return 1;
      }
      if ((!args_filled[arg_idx]) && (!res)) {
	// For IN arguments, reset value fields to NULL, so that the ORB does
	// not carry in data backwards when the RPC returns.
	if (arg_idx <= dest->last_in) {
	  dest->parameters[arg_idx].value.replace(0, 0, NULL);
	  // For INOUT arguments, release is false,
	  // For OUT arguments,   release is true.
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
	  if (mrsh_data(&(dest->parameters[arg_idx]),
			&dd, (arg_idx > dest->last_inout)))
	    return 1;
	}
	args_filled[arg_idx] = 1;
      }
    }
  }
  for (i = dest->last_in + 1; i <= dest->last_out; i++) {
    if (!args_filled[i]) {
      cerr << "DIET conversion WARNING: could not reconvert all arguments.\n";
      break;
    }
  }

  return 0;
}


/****************************************************************************/
/* Client receives server data ...                                          */
/****************************************************************************/

// INOUT parameters should have been set correctly by the ORB.
// But their descriptions could have been slightly altered
//  (matrix dimensions, for instance)
int unmrsh_out_args_to_profile(diet_profile_t *dpb, corba_profile_t *cpb)
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
    corba_data_desc_t *cdd = &(cpb->parameters[i].desc);
    // Special case for INOUT files: rewrite in the same file.
    if (cdd->specific._d() == (long) DIET_FILE) {
      char *inout_path = dpb->parameters[i].desc.specific.file.path;
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

  // Unmarshal OUT parameters, but not the descriptions to save time,
  // EXCEPT for files !!
  for (; i <= dpb->last_out; i++) {
    unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]), 1);
  }
  return 0;
}

