/****************************************************************************/
/* $Id$ */
/* DIET CORBA marshalling source code                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Frederic LOMBARD          - LIFC Besançon (France)                  */
/*    - Philippe COMBES           - LIP ENS Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.7  2002/09/09 15:57:48  pcombes
 * Fix bugs in marshalling
 *
 * Revision 1.6  2002/09/02 17:09:51  pcombes
 * Add free on OUT arguments.
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
 * Revision 1.4  2002/08/09 14:30:27  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 * Revision 1.3  2002/05/24 19:36:52  pcombes
 * Add BLAS/dgemm example (implied bug fixes)
 *
 * Revision 1.2  2002/05/17 20:35:16  pcombes
 * Version alpha without FAST 
 *
 ****************************************************************************/


#include <iostream.h>
#include <stdlib.h>

#include "marshalling.hh"
#include "debug.hh"


/*==========================================================================*/
/* Data structure marshalling                                               */
/*==========================================================================*/

inline int mrsh_scalar_desc(corba_data_desc_t *dest, diet_data_desc_t *src)
{
  corba_scalar_specific_t scal;

  dest->specific.scal(scal);
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
    dest->specific.scal().value <<= (long int) scal;
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
  default:
    cerr << "mrsh_scalar_desc: Base type "
	 << src->generic.base_type << " not implemented.\n";
    return 1;
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
    dest->specific.mat().nb_r = src->specific.mat.nb_r;
    dest->specific.mat().nb_c = src->specific.mat.nb_c;
    dest->specific.mat().istrans = src->specific.mat.istrans;
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
    dest->specific.file().size = src->specific.file.size;
    break;
  }
  default:
    cerr << "mrsh_data_desc: Type "
	 << src->generic.type << " not implemented\n";
    return 1;
  }
  return 0;
}


int mrsh_data(corba_data_t *dest, diet_data_t *src, int only_desc, int release)
{
  long unsigned int size = (long unsigned int) data_sizeof(&(src->desc));
  if (mrsh_data_desc(&(dest->desc), &(src->desc)))
    return 1;
  if (only_desc) {
    dest->value.length(0);
  } else {
    dest->value.replace(size, size, (CORBA::Char *)src->value, release);
  }
  return 0;
}


/*====[ mrsh_data_seq ]=====================================================*/

int mrsh_data_seq(SeqCorbaData_t *dest, diet_data_seq_t *src,
		  int only_desc, int release)
{
  size_t dest_length = dest->length();

  if (dest_length != 0) {
    if (dest_length != src->length) {
      cerr << "This usage of mrsh_data_seq should not occur in DIET\n"
	   << "Either mrsh_data_seq is called on an empty SeqCorbaData_t,\n"
	   << " or it is called on the initial sequence\n";
      return 1;
    }
  } else {
    dest->length(src->length);
  }

  for (size_t i = 0; i < src->length; i++) {
    /* Convert data_t */
    if (mrsh_data(&((*dest)[i]), &(src->seq[i]), only_desc, release))
      return 1;
  }
  return 0;
}


/*==========================================================================*/
/* Data structure unmarshalling                                             */
/*==========================================================================*/


inline int unmrsh_scalar_desc(diet_data_desc_t *dest, const corba_data_desc_t *src)
{
  void *value;
  diet_base_type_t bt = (diet_base_type_t)src->base_type;
  
  switch(bt) {
  case DIET_CHAR:
  case DIET_BYTE: {
    value = new char;
    src->specific.scal().value >>= *((short *)(value));
    scalar_desc_set(dest, DIET_VOLATILE, bt, value);
    break;
  }
  case DIET_INT: {
    value = (void *) new int;
    src->specific.scal().value >>= *((long *)(value));
    scalar_desc_set(dest, DIET_VOLATILE, bt, value);
    break;
  }
  case DIET_LONGINT: {
    value = (void *) new long long int;
    src->specific.scal().value >>= *((long int *)(value));
    scalar_desc_set(dest, DIET_VOLATILE, bt, value);
    break;
  }
  case DIET_FLOAT: {
    value = (void *) new float;
    src->specific.scal().value >>= *((float *)(value));
    scalar_desc_set(dest, DIET_VOLATILE, bt, value);
    break;
  }
  case DIET_DOUBLE: {
    value = (void *) new double;
    src->specific.scal().value >>= *((double *)(value));
    scalar_desc_set(dest, DIET_VOLATILE, bt, value);
    break;
  }
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
    vector_desc_set(dest, DIET_VOLATILE, bt,
		    src->specific.vect().size);
    break;
  }
  case DIET_MATRIX: {
    matrix_desc_set(dest, DIET_VOLATILE, bt,
		    src->specific.mat().nb_r, src->specific.mat().nb_c,
		    src->specific.mat().istrans);
    break;
  }
  case DIET_STRING: {
    string_desc_set(dest, DIET_VOLATILE, src->specific.str().length);
    break;
  }
  case DIET_FILE: {
    file_desc_set(dest, DIET_VOLATILE, "");
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
    return sf_type_cons_vect;
  case DIET_MATRIX:
    return sf_type_cons_mat;
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
  default:
    return sf_type_base_count;  
  }
}

int unmrsh_data_desc_to_sf(sf_data_desc_t *dest, const corba_data_desc_t *src) 
{
  dest->id        = 0;
  dest->type      = diet_to_sf_type((diet_data_type_t)src->specific._d());
  dest->base_type = diet_to_sf_base_type((diet_base_type_t)src->base_type);
  
  switch (dest->type) {
    
  case sf_type_cons_vect:
    dest->ctn.vect.size = src->specific.vect().size;
    break;
    
  case sf_type_cons_mat:
    //#ifdef withoutfast
    dest->ctn.mat.nb_l  = src->specific.mat().nb_r;
    dest->ctn.mat.nb_c  = src->specific.mat().nb_c;
    dest->ctn.mat.trans = src->specific.mat().istrans;
    //#else  // withoutfast
    //sf_dd_mat_set(dest, src->specific.mat().nb_r,
    //  	    src->specific.mat().nb_c, src->specific.mat().istrans);
    //#endif // withoutfast
    break;

  case sf_type_cons_scal:
    switch (dest->base_type) {
    case sf_type_base_char:
      dest->ctn.scal.value = (void *) new char;
      src->specific.scal().value >>= *((short *)(dest->ctn.scal.value));
      break;
    case sf_type_base_int:
      dest->ctn.scal.value = (void *) new int;
      src->specific.scal().value >>= *((long *)(dest->ctn.scal.value));
      break;
    case sf_type_base_double:
      dest->ctn.scal.value = (void *) new double;
      src->specific.scal().value >>= *((double *)(dest->ctn.scal.value));
      break;
    default:
      cerr << "unmrsh_data_desc_to_sf: Base type "
	   << dest->base_type << " unknown for FAST\n";
      return 1;
    }
    break;
    
  default:
    cerr << "unmrsh_data_desc_to_sf: Type "
	 << src->specific._d() << " unknown for FAST\n";
    return 1;
  }
  return 0;
}


int unmrsh_data(diet_data_t *dest, const corba_data_t *src)
{
  if (unmrsh_data_desc(&(dest->desc), &(src->desc)))
    return 1;
  if (src->value.length() == 0) {
    dest->value = malloc(data_sizeof(&(dest->desc)));
  } else {
    // CORBA::boolean orphan = (src->persistence_mode != DIET_VOLATILE);
    dest->value = (char *)src->value.get_buffer();
  }
  return 0;
}


/*====[ unmrsh_data_seq ]===================================================*/

int unmrsh_data_seq(diet_data_seq_t *dest, const SeqCorbaData_t *src)
{
  if (dest->length != 0) {
    if (dest->length != src->length()) {
      cerr << "This usage of unmrsh_data_seq should not occur in DIET\n"
	   << "Either unmrsh_data_seq is called on an empty diet_data_seq_t,\n"
	   << " or it is called on the initial sequence\n";
      return 1;
    }
  } else {
    dest->length = src->length();
    dest->seq = (diet_data_t *) malloc(dest->length * sizeof(diet_data_t));
  }
  for (size_t i = 0; i < dest->length; i++) {
    if (unmrsh_data(&(dest->seq[i]), &((*src)[i])))
      return 1;
  }
  return 0;
}


/*==========================================================================*/
/* Profile structure conversions                                            */
/*==========================================================================*/


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

int unmrsh_profile_desc(diet_profile_desc_t *dest, char **dest_name,
			corba_profile_desc_t *src)
{
  *dest_name       = CORBA::string_dup(src->path);
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc = new (diet_arg_desc_t)[src->last_out + 1];
  for (int i = 0; i <= src->last_out; i++) {
    (dest->param_desc[i]).base_type = 
      (diet_base_type_t)(src->param_desc[i]).base_type;
    (dest->param_desc[i]).type =
      (diet_data_type_t)(src->param_desc[i]).type;    
  }
  return 0;
}

int unmrsh_profile_desc_to_name(char **dest_name, corba_profile_desc_t *src)
{
  *dest_name = CORBA::string_dup(src->path);
  return 0;
}

int mrsh_profile(corba_profile_t *dest, diet_profile_t *src, char *src_name)
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

int unmrsh_profile_to_desc(diet_profile_desc_t *dest, char **dest_name,
			   corba_profile_t *src)
{
  *dest_name       = CORBA::string_dup(src->path);
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc = new (diet_arg_desc_t)[src->last_out + 1];
  for (int i = 0; i <= src->last_out; i++) {
    (dest->param_desc[i]).base_type =
      (diet_base_type_t)(src->param_desc[i]).base_type;
    (dest->param_desc[i]).type =
      (diet_data_type_t)(src->param_desc[i]).specific._d();    
  }
  return 0;
}


int unmrsh_profile_to_sf(sf_inst_desc_t *dest, const corba_profile_t *src)
{
  //#ifdef withoutfast
  dest->path       = CORBA::string_dup(src->path);
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc = new (sf_data_desc_t)[src->last_out + 1]; 
  //#else  // withoutfast
  //sf_inst_desc_set(dest, CORBA::string_dup(src->path),
  //                 src->last_in, src->last_inout, src->last_out);
  //#endif // withoutfast
  for (int i = 0; i <= src->last_out; i++) {
    unmrsh_data_desc_to_sf(&(dest->param_desc[i]), &(src->param_desc[i]));
  }
  return 0;
}


/*==========================================================================*/
/* Profile -> corba data sequences                                          */
/*==========================================================================*/

int mrsh_profile_to_in_args(SeqCorbaData_t *in,
			    SeqCorbaData_t *inout, SeqCorbaData_t *out,
			    const diet_profile_t *profile)
{
  int i, j;
  
  in->length(profile->last_in + 1);
  for (i = 0; i <= profile->last_in; i++) {
    if (mrsh_data(&((*in)[i]), &(profile->parameters[i]), 0, 0))
      return 1;
  }
  inout->length(profile->last_inout - profile->last_in);
  for (i = 0; i < (profile->last_inout - profile->last_in); i++) {
    j = i + profile->last_in + 1;
    if (mrsh_data(&((*inout)[i]), &(profile->parameters[j]), 0, 0))
      return 1;
  }
  // For OUT parameters, marshal only descriptions for checking
  // purpose on SeD side, and let data emty.
  out->length(profile->last_out - profile->last_inout);
  for (i = 0; i < (profile->last_out - profile->last_inout); i++) {
    j = i + profile->last_inout + 1;
    if (mrsh_data(&((*out)[i]), &(profile->parameters[j]), 1, 0))
      return 1;
  }
  return 0;
}

// INOUT parameters should have been set correctly by the ORB.
// But their descriptions could have been slightly altered
//  (matrix dimensions, for instance)
int unmrsh_out_args_to_profile(diet_profile_t *profile,
			       SeqCorbaData_t *inout, SeqCorbaData_t *out)
{
  size_t inout_length = (size_t)(profile->last_inout - profile->last_in);
  size_t out_length   = (size_t)(profile->last_out   - profile->last_inout);
  corba_data_t *src;
  diet_data_t  *dest;
  
  if ((inout_length != inout->length()) || (out_length != out->length()))
    return 1;
  // Unmarshal INOUT parameters descriptions only.
  for (size_t i = 0; i < inout->length(); i++) {
    unmrsh_data_desc(&(profile->parameters[i + profile->last_in + 1].desc),
		     &((*inout)[i].desc));
  }
  // Unmarshal OUT parameters, but not the descriptions to save time.
  for (size_t i = 0; i < out->length(); i++) {
    // instead of using unmrsh_data, save time by 
    src  = &((*out)[i]);
    dest = &(profile->parameters[i + profile->last_inout + 1]);
    if (src->value.length() == 0) {
      dest->value = malloc(data_sizeof(&(dest->desc)));
    } else {
      // CORBA::boolean orphan = (src->persistence_mode != DIET_VOLATILE);
      dest->value = src->value.get_buffer();
    }
  }
  return 0;
}

