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


/*===========================================================================*/
/*                                                                           */
/* Data structure conversions                                                */
/*                                                                           */
/*===========================================================================*/


/***  Data descriptors ***/


#if 0
void
sf2corbaDataDesc(corba_data_desc_t *dest, const sf_data_desc_t *src)
{
  dest->id = (CORBA::Long) src->id;
  dest->base_type = src->base_type;

  switch (src->type) {
    
  case sf_type_cons_vect: {
    corba_data_desc_vect_ctn_t vect;
    dest->ctn.vect(vect);
    dest->ctn.vect().size = src->ctn.vect.size;;
    break;
  }
  case sf_type_cons_mat: {
    corba_data_desc_mat_ctn_t  mat;
    dest->ctn.mat(mat);
    dest->ctn.mat().nb_l  = src->ctn.mat.nb_l;
    dest->ctn.mat().nb_c  = src->ctn.mat.nb_c;
    dest->ctn.mat().trans = src->ctn.mat.trans;
    break;
  }
  case sf_type_cons_scal: {
    corba_data_desc_scal_ctn_t scal;

    dest->ctn.scal(scal);
    switch (src->base_type) {
    case sf_type_base_char: {
      char scal = *((char *)(src->ctn.scal.value));
      dest->ctn.scal().value <<= (short) scal;
      break;
    }
    case sf_type_base_int: {
      int scal = *((int *)(src->ctn.scal.value));
      dest->ctn.scal().value <<= (long) scal;
      break;
    }
    case sf_type_base_double: {
      double scal = *((double *)(src->ctn.scal.value));
      dest->ctn.scal().value <<= (double) scal;
      break;
    }
    default:
      cerr << "sf2corbaDataDesc: Error in base type conversion "
	   << src->base_type << " \n";
    }
    break;
  }
  default:
    cerr << "sf2corbaDataDesc: Error in cons type conversion "
	 << src->type << " \n";
  }
}

void
corba2sfDataDesc(sf_data_desc_t *dest, const corba_data_desc_t *src)
{
  dest->id = (long) src->id;
  dest->type      = (sf_type_cons_t) src->ctn._d();
  dest->base_type = (sf_type_base_t) src->base_type;
  
  switch (dest->type) {
    
  case sf_type_cons_vect:
    dest->ctn.vect.size = src->ctn.vect().size;
    break;
    
  case sf_type_cons_mat:
#ifndef withoutfast
    sf_dd_mat_set(dest, src->ctn.mat().nb_l,
		  src->ctn.mat().nb_c, src->ctn.mat().trans);
#else  // withoutfast
    dest->ctn.mat.nb_l  = src->ctn.mat().nb_l;
    dest->ctn.mat.nb_c  = src->ctn.mat().nb_c;
    dest->ctn.mat.trans = src->ctn.mat().trans;
#endif // withoutfast
    break;

  case sf_type_cons_scal:
    switch (dest->base_type) {
    case sf_type_base_char:
      dest->ctn.scal.value = (void *) new char;
      src->ctn.scal().value >>= *((short *)(dest->ctn.scal.value));
      break;
    case sf_type_base_int:
      dest->ctn.scal.value = (void *) new int;
      src->ctn.scal().value >>= *((long *)(dest->ctn.scal.value));
      break;
    case sf_type_base_double:
      dest->ctn.scal.value = (void *) new double;
      src->ctn.scal().value >>= *((double *)(dest->ctn.scal.value));
      break;
    default:
      cerr << "\ncorba2sfDataDesc: Error in type conversion "
	   << dest->base_type << " \n";
    }
    break;
    
  default:
    cerr << "\ncorba2sfDataDesc: Error in type conversion "
	 << dest->type << " \n";
  }
}


/*** Data themselves ***/

void
diet2corbaData(corba_data_t *dest, const diet_data_t *src)
{
  size_t size = macro_data_sizeof(&(src->desc));
  
  sf2corbaDataDesc(&(dest->desc), &(src->desc));

  switch (src->desc.base_type) {
  case sf_type_base_char:
    if (dest->value.sc().length() != size) {
      // this is in case of total reallocation in call to length
      dest->value.sc().length(size);
    }
    for (size_t i = 0; i < size; i++)
      dest->value.sc()[i] = (char) ((char *)(src->value))[i];
    break;

  case sf_type_base_int:
    if (dest->value.sl().length() != size) {
      dest->value.sl().length(size);
    }
    for (size_t i = 0; i < size; i++)
      dest->value.sl()[i] = (int) ((int *)(src->value))[i];
    break;

  case sf_type_base_double:
    if (dest->value.sd().length() != size) {
      dest->value.sd().length(size);
    }
    for (size_t i = 0; i < size; i++)
      dest->value.sd()[i] = (double) ((double *)(src->value))[i];
    break;

  default:
    cerr << "diet2corbaData: Error in type conversion (base type)\n";
  }
}


void
corba2dietData(diet_data_t *dest, const corba_data_t *src)
{
  int realloc_value = 0;
  size_t size, data_size;


  corba2sfDataDesc(&(dest->desc), &(src->desc));
  data_size = macro_data_sizeof(&(dest->desc));

  switch (dest->desc.base_type) {
  case sf_type_base_char: {
    size = src->value.sc().length();
    if ((realloc_value = ((!dest->value) || (size > data_size))))
      dest->value = realloc(dest->value, size * sizeof(char));
    for (size_t i = 0; i < size; i++)
      ((char *) dest->value)[i] = src->value.sc()[i];
    break;
  }
  case sf_type_base_int: {
    size = src->value.sl().length();
    if ((realloc_value = ((!dest->value) || (size > data_size))))
      dest->value = realloc(dest->value, size * sizeof(int));
    for (size_t i = 0; i < size; i++)
      ((int *) dest->value)[i] = src->value.sl()[i];
    break;
  }
  case sf_type_base_double: {
    size = src->value.sd().length();
    if ((realloc_value = ((!dest->value) || (size > data_size)))) {
      dest->value = realloc(dest->value, size * sizeof(double));
    }
    for (size_t i = 0; i < size; i++)
      ((double *) dest->value)[i] = src->value.sd()[i];
    break;
  }
  default:
    cerr << "corba2dietData: Error in type conversion (base type)\n";
  }
  if (realloc_value)
    dest->to_be_freed = 1;
}


/* Allocate dest sequence if necessary */
/* First "free" all value sequences :
 *   this could be optimized by freeing only values that have not the same
 *   type in old sequence(already in place) as in the new one.
 * Then set the new sequence length and init the values (with length() == 0). */

void
diet2corbaDataSeq(SeqCorba_data_t *dest, const diet_data_seq_t *src) 
{
  size_t dest_length = dest->length();
  int all_is_new = (dest_length != src->length);
  
  /* "Free" all value fields */
  for (size_t i = 0; i < dest_length; i++) {
    switch (src->seq[i].desc.base_type) {
    case sf_type_base_char:
      (*dest)[i].value.sc().length(0);
      break;
    case sf_type_base_int:
      (*dest)[i].value.sl().length(0);
      break;
    case sf_type_base_double:
      (*dest)[i].value.sd().length(0);
      break;
    default:
      cerr << "diet2corbaData: Error in type conversion (base type)\n";
    }
  }
  /* Set (new) length */
  dest->length(src->length);
  
  for (size_t i = 0; i < src->length; i++) {
    
    /* Initialize (*dest)[i].value sequence if necessary */
    switch (src->seq[i].desc.base_type) {
    case sf_type_base_char: {
      SeqChar sc;
      (*dest)[i].value.sc(sc);
      break;
    }
    case sf_type_base_int: {
      SeqLong sl;
      (*dest)[i].value.sl(sl);
      break;
    }
    case sf_type_base_double: {
      SeqDouble sd;
      (*dest)[i].value.sd(sd);
      break;
    }
    default:
      cerr << "diet2corbaData: Error in type conversion (base type)\n";
    }

    /* Convert data_t */
    diet2corbaData(&((*dest)[i]), &(src->seq[i]));
  }
}

/* Allocate dest sequence if necessary */
/* Check if the sequence will have to be reallocated (different length) :
 *  if reallocation is needed then first free all values,
 *  else                           let the data conversion realloc values
 *                                 as needed.                             */
void
corba2dietDataSeq(diet_data_seq_t *dest, const SeqCorba_data_t *src)
{ 
  int all_is_new = (dest->length != src->length());
  
  if (!dest->length)
    dest->seq = NULL;
  
  if (all_is_new) {
    /* First free all value fields */
    for (size_t i = 0; i < dest->length; i++) {
      if (dest->seq[i].to_be_freed)
	free(dest->seq[i].value);
    }
    /* Then realloc sequence */
    dest->length = src->length();
    dest->seq = (diet_data_t *) realloc(dest->seq,
					dest->length * sizeof(diet_data_t));
  }
  
  for (size_t i = 0; i < dest->length; i++) {
    if (all_is_new) {
      /* Initialize dest->seq[i].value to force allocation*/
      dest->seq[i].value = NULL;
    }
    corba2dietData(&(dest->seq[i]), &((*src)[i]));
  }
}
#endif // 0

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

int mrsh_data(corba_data_t *dest, diet_data_t *src, int only_desc)
{
  long unsigned int size = (long unsigned int) data_sizeof(&(src->desc));
  if (mrsh_data_desc(&(dest->desc), &(src->desc)))
    return 1;
  if (only_desc) {
    dest->value.length(0);
  } else {
    SeqChar value(size, size, (CORBA::Char *)src->value);
    dest->value = value;
  }
  return 0;
}


/*====[ mrsh_data_seq ]=====================================================*/

int mrsh_data_seq(SeqCorbaData_t *dest, diet_data_seq_t *src, int only_desc)
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
    if (mrsh_data(&((*dest)[i]), &(src->seq[i]), only_desc))
      return 1;
  }
  return 0;
}


/*==========================================================================*/
/* Data structure unmarshalling                                             */
/*==========================================================================*/


inline int unmrsh_scalar_desc(diet_data_desc_t *dest, corba_data_desc_t *src)
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

int unmrsh_data_desc(diet_data_desc_t *dest, corba_data_desc_t *src)
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


int unmrsh_data(diet_data_t *dest, corba_data_t *src)
{
  if (unmrsh_data_desc(&(dest->desc), &(src->desc)))
    return 1;
  if (src->value.length() == 0)
    dest->value = malloc(data_sizeof(&(dest->desc)));
  else {
    // CORBA::boolean orphan = (src->persistence_mode != DIET_VOLATILE);
    dest->value = src->value.get_buffer(true);
  }
  return 0;
}

/*====[ unmrsh_data_seq ]===================================================*/

int unmrsh_data_seq(diet_data_seq_t *dest, SeqCorbaData_t *src)
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

  for (size_t i = 0; i <= dest->length; i++) {
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

int mrsh_profile_to_in_args(SeqCorbaData_t *in, SeqCorbaData_t *inout,
			    const diet_profile_t *profile)
{
  in->length(profile->last_in + 1);
  for (int i = 0; i <= profile->last_in; i++) {
    if (mrsh_data(&((*in)[i]), &(profile->parameters[i]), 0))
      return 1;
  }
  inout->length(profile->last_inout - profile->last_in);
  for (int i = 0; i < (profile->last_inout - profile->last_in); i++) {
    if (mrsh_data(&((*inout)[i]),
		  &(profile->parameters[i + profile->last_in + 1]), 0))
      return 1;
  }
  return 0;
}

int unmrsh_out_args_to_profile(diet_profile_t *profile,
			       SeqCorbaData_t *inout, SeqCorbaData_t *out)
{
  size_t inout_length = (size_t)(profile->last_inout - profile->last_in);
  size_t out_length   = (size_t)(profile->last_out   - profile->last_inout);
  
  if ((inout_length != inout->length()) || (out_length != out->length()))
    return 1;
  for (size_t i = 0; i < inout->length(); i++) {
    if (unmrsh_data(&(profile->parameters[i + profile->last_in + 1]),
		    &((*inout)[i])))
      return 1;
  }
  for (size_t i = 0; i < out->length(); i++) {
    if (unmrsh_data(&(profile->parameters[i + profile->last_inout + 1]),
		    &((*out)[i])))
      return 1;
  }
  return 0;
}


#if 0

void
sf2corbaPbDesc(corba_pb_desc_t *dest, const sf_pb_desc_t *src)
{
  dest->path       = CORBA::string_dup(src->path);
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc.length(dest->last_out + 1);
  for (int i = 0; i <= src->last_out; i++) {
    sf2corbaDataDesc(&(dest->param_desc[i]), &(src->param_desc[i]));
  }
  dest->code_desc <<= (long) src->code_desc;
}

void
corba2sfPbDesc(sf_pb_desc_t *dest, const corba_pb_desc_t *src)
{
#ifndef withoutfast
  sf_pb_desc_set(dest, CORBA::string_dup(src->path),
		 src->last_in, src->last_inout, src->last_out);
#else  // withoutfast
  dest->path       = CORBA::string_dup(src->path);
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc = new (sf_data_desc_t)[dest->last_out + 1];
#endif // withoutfast

  for (int i = 0; i <= dest->last_out; i++) {
    corba2sfDataDesc(&(dest->param_desc[i]), &(src->param_desc[i]));
  }
  // FIXME: This line triggers compilation error - do not care until SLIM is not ready
  //src->code_desc >>= dest->code_desc;
}

void
sf2corbaInstDesc(corba_inst_desc_t *dest, const sf_inst_desc_t *src)
{
  dest->path       = CORBA::string_dup(src->path);
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc.length(dest->last_out);
  for (int i = 0; i <= src->last_out; i++) {
    sf2corbaDataDesc(&(dest->param_desc[i]), &(src->param_desc[i]));
  }
}

void
corba2sfInstDesc(sf_inst_desc_t *dest, const corba_inst_desc_t *src)
{
#ifndef withoutfast
  sf_inst_desc_set(dest, CORBA::string_dup(src->path),
		   src->last_in, src->last_inout, src->last_out);
#else  // withoutfast
  dest->path       = CORBA::string_dup(src->path);
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc = new (sf_data_desc_t)[dest->last_out + 1];
#endif // withoutfast

  for (int i = 0; i <= dest->last_out; i++) {
    corba2sfDataDesc(&(dest->param_desc[i]), &(src->param_desc[i]));
  }
  
}

#ifdef OLD_VERSION

void sf2corbaPbDesc(corba_pb_desc_t *dest,const sf_pb_desc_t *src)
{
  corba_dd_scal_t tmpScal;
  
  dest->path=CORBA::string_dup(src->path);
  
  dest->nb_in=src->nb_in;
  
  dest->datas.length(dest->nb_in+1);
  
  for (int i=0;i<dest->nb_in+1;i++)
    {
      switch (src->datas[i]->type)
	{
	case sf_type_cons_scal:
	  tmpScal.type=((sf_dd_scal_t *)(src->datas[i]))->type;
	  tmpScal.base_type=((sf_dd_scal_t *)(src->datas[i]))->base_type;
	  tmpScal.role=((sf_dd_scal_t *)(src->datas[i]))->role;
	  
	  switch (((sf_dd_scal_t *)(src->datas[i]))->base_type)
	    {
	    sf_type_base_int:
	      tmpScal.value<<=((int *)(((sf_dd_scal_t *)(src->datas[i]))->value));
	      break;
	    sf_type_base_double:
	      tmpScal.value<<=((double *)(((sf_dd_scal_t *)(src->datas[i]))->value));
	      break;
	    sf_type_base_char:
	      tmpScal.value<<=((char *)(((sf_dd_scal_t *)(src->datas[i]))->value));
	      break;
	    default:
	      cerr << "Data metatype conversion fatal error, exiting..." << endl;
	      exit(1);
	    }
	  
	  dest->datas[i]<<=((corba_dd_scal_t *)(&tmpScal));
	  break;
	case sf_type_cons_vect:
	  dest->datas[i]<<=(*((corba_dd_vect_t *)(src->datas[i])));
	  break;
	case sf_type_cons_mat:
	  dest->datas[i]<<=(*((corba_dd_mat_t *)(src->datas[i])));
	  break;
	default:
	  cerr << "Data metatype conversion fatal error, exiting..." << endl;
	  exit(1);
	}
    }
}

void corba2sfPbDesc(sf_pb_desc_t *dest,const corba_pb_desc_t *src)
{
  dest->path=CORBA::string_dup(src->path);
  
  dest->nb_in=src->nb_in;

  dest->datas=new (sf_data_desc_t *)[dest->nb_in+1];

  /* For each parametter */
  
  for (int i=0;i<dest->nb_in+1;i++)
    {
      corba_dd_scal_t *scalref;
      corba_dd_vect_t *vectref,*vect;
      corba_dd_mat_t *matref,*mat;
      
      sf_dd_scal_t *scal;

      if (src->datas[i]>>=scalref)
	{
	  scal=new sf_dd_scal_t;
	  scal->type=(sf_type_cons_t)scalref->type;
	  scal->base_type=(sf_type_base_t)scalref->base_type;
	  scal->role=scalref->role;
	  
	  switch (scal->base_type)
	    {
	    case sf_type_base_int:
	      scal->value=(void *)new int;
	      scalref->value>>=*((long*)(scal->value));
	      break;
	    case sf_type_base_double:
	      scal->value=(void *)new double;
	      scalref->value>>=*((double*)(scal->value));
	      break;
	    case sf_type_base_char:
	      scal->value=(void *)new char;
	      scalref->value>>=*((short*)(scal->value));
	      break;
	    default:
	      cerr << "Data metatype conversion fatal error, exiting..." << endl;
	      exit(1);
	    }
	  dest->datas[i]=(sf_data_desc_t *)scal;
	}
      else 
	if (src->datas[i]>>=vectref)
	  {
	    vect=new corba_dd_vect_t;
	    *vect=*vectref;
	    dest->datas[i]=(sf_data_desc_t *)vect;
	  }
	else
	  if (src->datas[i]>>=matref)
	    {
	      mat=new corba_dd_mat_t;
	      *mat=*matref;
	      dest->datas[i]=(sf_data_desc_t *)mat;
	    }
	  else
	    {
	      cerr << "Warning, unknown type in Corba unmarshalling!" << endl;
	    }
    } 
}



void diet2corbaRequest(corba_request_t *dest,const diet_request_t *src)
{
  dest->reqId=src->reqId;
  sf2corbaPbDesc(&(dest->pb),&(src->pb));  
}

#endif // OLD_VERSION


void corba2dietRequest(diet_request_t *dest, corba_request_t *src)
{
  dest->reqId = src->reqId;
  unmrsh_profile_to_sf(&(dest->pb_profile),&(src->pb_profile));
}

void diet2corbaResponse(corba_response_t *dest,const diet_response_t *src)
{
  int i;

  dest->reqId=src->reqId;
  dest->myId=src->myId;
  dest->nbIn=src->nbIn;
  dest->nbServers=src->nbServers;

  dest->data.length(dest->nbIn);
  
  for (i=0;i<dest->nbIn;i++)
    {
      dest->data[i].localization=SeD::_duplicate(src->data[i].localization);

      if (CORBA::is_nil(dest->data[i].localization))
      {
	dest->data[i].hostname=CORBA::string_dup("");
      }
      else
      {
	dest->data[i].hostname=CORBA::string_dup(src->data[i].hostname);
	dest->data[i].port=src->data[i].port;
	dest->data[i].timeToMe=src->data[i].timeToMe;
      }
    }

  if (dest->nbServers>0)
    {
      dest->comp.length(dest->nbServers);
      
      for (i=0;i<dest->nbServers;i++)
				{
					dest->comp[i].myRef=SeD::_duplicate(src->comp[i].myRef);

					dest->comp[i].hostname=CORBA::string_dup(src->comp[i].hostname);

					dest->comp[i].port=src->comp[i].port;

					dest->comp[i].implName=CORBA::string_dup(src->comp[i].implName);

					dest->comp[i].tComp=src->comp[i].tComp;
	  
					dest->comp[i].tComm.length(dest->nbIn);
	  
					for (int j=0;j<dest->nbIn;j++)
						{
							dest->comp[i].tComm[j]=src->comp[i].tComm[j];
						}
				}
    }
  else
    {
      dest->comp.length(0);
    }
}

void corba2dietResponse(diet_response_t *dest,const corba_response_t *src)
{
  int i;

  dest->reqId=src->reqId;
  dest->myId=src->myId;
  dest->nbIn=src->nbIn;
  dest->nbServers=src->nbServers;
  
  dest->data=new diet_data_loc_t[dest->nbIn];

  for (i=0;i<dest->nbIn;i++)
    {
      dest->data[i].localization=SeD::_duplicate(src->data[i].localization);

      if (CORBA::is_nil(dest->data[i].localization))
      {
	dest->data[i].hostname=CORBA::string_dup("");
      }
      else
      {
	dest->data[i].hostname=CORBA::string_dup(src->data[i].hostname);
	dest->data[i].port=src->data[i].port;
	dest->data[i].timeToMe=src->data[i].timeToMe;
      }
    }
  
  if (dest->nbServers>0)
    {
      dest->comp=new diet_server_comp_t [dest->nbServers];

      for (i=0;i<dest->nbServers;i++)
				{	 
					dest->comp[i].myRef=SeD::_duplicate(src->comp[i].myRef);

					dest->comp[i].hostname=CORBA::string_dup(src->comp[i].hostname);
	  
					dest->comp[i].port=src->comp[i].port;
	  
					dest->comp[i].implName=CORBA::string_dup(src->comp[i].implName);

					dest->comp[i].tComp=src->comp[i].tComp;
	  
					dest->comp[i].tComm=new double[dest->nbIn];
	  
					for (int j=0;j<dest->nbIn;j++)
						{
							dest->comp[i].tComm[j]=src->comp[i].tComm[j];
						}
				}
    }
  else 
    {
      dest->comp=NULL;
    }
}

void diet2corbaDecisionDesc(corba_decision_desc_t *dest,const diet_decision_desc_t *src)
{
  dest->chosenServer=SeD::_duplicate(src->chosenServer);
  dest->chosenServerName=CORBA::string_dup(src->chosenServerName);
  dest->chosenServerPort=src->chosenServerPort;

  dest->nbIn=src->nbIn;
  dest->dataLocs.length(dest->nbIn);
  for (int i=0;i<dest->nbIn;i++)
    {
      dest->dataLocs[i].localization=SeD::_duplicate(src->dataLocs[i].localization);
      dest->dataLocs[i].hostname=CORBA::string_dup(src->dataLocs[i].hostname);
      dest->dataLocs[i].port=src->dataLocs[i].port;
    }
  
  dest->implPath=CORBA::string_dup(src->implPath);
}

void corba2dietDecisionDesc(diet_decision_desc_t *dest,const corba_decision_desc_t *src)
{
  dest->chosenServer=SeD::_duplicate(src->chosenServer);
  dest->chosenServerName=CORBA::string_dup(src->chosenServerName);
  dest->chosenServerPort=src->chosenServerPort;

  dest->nbIn=src->nbIn;
  dest->dataLocs=new diet_data_loc_t[dest->nbIn];
  for (int i=0;i<dest->nbIn;i++)
    {
      dest->dataLocs[i].localization=SeD::_duplicate(src->dataLocs[i].localization);
      dest->dataLocs[i].hostname=CORBA::string_dup(src->dataLocs[i].hostname);
      dest->dataLocs[i].port=src->dataLocs[i].port;
    }
  
  dest->implPath=CORBA::string_dup(src->implPath);
}

void diet2corbaDecisionSequence(corba_decision_sequence_t *dest,const diet_decision_sequence_t *src)
{
  dest->nbElts=src->nbElts;

  dest->decisions.length(dest->nbElts);
  for (int i=0;i<dest->nbElts;i++)
    {
      diet2corbaDecisionDesc(&(dest->decisions[i]),&(src->decisions[i]));
    }
}



void corba2dietDecisionSequence(diet_decision_sequence_t *dest,const corba_decision_sequence_t *src)
{
  dest->nbElts=src->nbElts;
  
  dest->decisions=new diet_decision_desc_t[dest->nbElts];
  for (int i=0;i<dest->nbElts;i++)
    {
      corba2dietDecisionDesc(&(dest->decisions[i]),&(src->decisions[i]));
    }
}


#endif // 0
