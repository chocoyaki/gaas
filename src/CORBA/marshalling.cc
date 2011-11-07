/**
 * @file marshalling.cc
 *
 * @brief  DIET CORBA marshalling source code
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Frederic LOMBARD (Frederc.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "marshalling.hh"
#include "debug.hh"
#include "ms_function.hh"
#include "DIET_data_internal.hh"     // for data_sizeof()
#include "configuration.hh"

#if defined HAVE_ALT_BATCH
#include "BatchSystem.hh"
#endif

#include <boost/detail/endian.hpp>

extern unsigned int TRACE_LEVEL;

#define MRSH_ERROR(formatted_msg, return_value)                          \
  INTERNAL_ERROR(__FUNCTION__ << ": " << formatted_msg, return_value)


/****************************************************************************/
/* Data structure marshalling                                               */
/****************************************************************************/

inline int
mrsh_scalar_desc(corba_data_desc_t *dest,
                 const diet_data_desc_t *const src) {
  corba_scalar_specific_t scal;

  dest->specific.scal(scal);
  if (!(src->specific.scal.value)) {
    dest->specific.scal().value <<= (CORBA::Double) 0;
  } else {
    switch (src->generic.base_type) {
    case DIET_CHAR: { // Impossible to insert a Char or an Octet into an Any.
      char scal = *((char *) (src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Short) scal;
      break;
    }
    case DIET_SHORT: {
      short scal = *((short *) (src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Short) scal;
      break;
    }
    case DIET_INT: {
      long scal = *((long int *) (src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Long) scal;
      break;
    }
    case DIET_LONGINT: {
      long long int scal = *((long long int *) (src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Long) scal;
      break;
    }
    case DIET_FLOAT: {
      float scal = *((float *) (src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Float) scal;
      break;
    }
    case DIET_DOUBLE: {
      double scal = *((double *) (src->specific.scal.value));
      dest->specific.scal().value <<= (CORBA::Double) scal;
      break;
    }
#if HAVE_COMPLEX
    case DIET_SCOMPLEX:
    case DIET_DCOMPLEX:
#endif  // HAVE_COMPLEX
    default:
      MRSH_ERROR("base type " << src->generic.base_type
                              << " not implemented", 1);
    } // switch
  }

  return 0;
} // mrsh_scalar_desc

int
__mrsh_data_desc_type(corba_data_desc_t *dest,
                      const diet_data_desc_t *const src) {
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
    dest->specific.mat().nb_r = src->specific.mat.nb_r;
    dest->specific.mat().nb_c = src->specific.mat.nb_c;
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
    break;
  }
  case DIET_FILE: {
    corba_file_specific_t file;
    dest->specific.file(file);
    if (src->specific.file.path) {
      dest->specific.file().path = CORBA::string_dup(src->specific.file.path);
      // Compute the file size.
      std::ifstream file(dest->specific.file().path);
      if (file.is_open()) {
        file.seekg(0, std::ios::end);
        dest->specific.file().size = file.tellg();
        file.close();
      } else {dest->specific.file().size = 0;
      }
    } else {
      dest->specific.file().path = CORBA::string_dup("");
      dest->specific.file().size = 0;
    }
    break;
  }
  case DIET_CONTAINER:
    corba_container_specific_t cont;
    dest->specific.cont(cont);
    dest->specific.cont().size = 0;
    break;
  default:
    MRSH_ERROR("type " << src->generic.type << " not implemented", 1);
  } // switch

  return (0);
} // __mrsh_data_desc_type

int
mrsh_data_desc(corba_data_desc_t *dest, diet_data_desc_t *src) {
  if (src->id != NULL) {
    // /!\ We cannot free dest->id.idNumber, it is only set by DTM in persistent mode
    // CORBA::string_free(dest->id.idNumber);
    dest->id.idNumber = CORBA::string_dup(src->id);
  } else {
    dest->id.idNumber = CORBA::string_dup("");
  }
  // The default values are not always inner the enum types, which triggers an ABORT.
  dest->id.state = DIET_FREE;
  dest->id.dataCopy = DIET_ORIGINAL;
  dest->mode = src->mode;
  dest->base_type = src->generic.base_type;
  // Endianness management
  dest->byte_order = BOOST_BYTE_ORDER;
  switch (dest->base_type) {
  case DIET_CHAR:
    dest->base_type_size = 1;
    break;
  case DIET_SHORT:
    dest->base_type_size = sizeof(short);
    break;
  case DIET_INT:
    dest->base_type_size = sizeof(int);
    break;
  case DIET_LONGINT:
    dest->base_type_size = sizeof(long);
    break;
  case DIET_FLOAT:
    dest->base_type_size = sizeof(float);
    break;
  case DIET_DOUBLE:
    dest->base_type_size = sizeof(double);
    break;
  case DIET_SCOMPLEX:
    dest->base_type_size = sizeof(float) * 2;
    break;
  case DIET_DCOMPLEX:
    dest->base_type_size = sizeof(double) * 2;
    break;
  default:
    MRSH_ERROR("base type " << dest->base_type << " unknown", 1);
  } // switch

  if (__mrsh_data_desc_type(dest, src) != 0) {
    return (1);
  }
  return (0);
} // mrsh_data_desc


int
mrsh_data(corba_data_t *dest, diet_data_t *src, int release) {
  long unsigned int size = (long unsigned int) data_sizeof(&(src->desc));
  CORBA::Char *value(NULL);

  if (mrsh_data_desc(&(dest->desc), &(src->desc))) {
    return 1;
  }

  if (src->value != NULL) {
    value = (CORBA::Char *) src->value;
  }
  if (value == NULL) {
    dest->value.length(0);
  } else {
    dest->value.replace(size, size, value, release);  // 0 if persistent 1 elsewhere
  }

  return 0;
} // mrsh_data

/**************************************************************************
 *  Marshall/ Unmarshall data handler                                     *
 *************************************************************************/
static int
__mrsh_data_id_desc(corba_data_desc_t *dest,
                    const diet_data_desc_t *const src) {
  dest->id.idNumber = CORBA::string_dup(src->id);

  dest->id.state = DIET_FREE;
  dest->id.dataCopy = DIET_ORIGINAL;

  return (0);
}
/****************************************************************************/
/* Data structure unmarshalling                                             */
/****************************************************************************/


inline int
unmrsh_scalar_desc(diet_data_desc_t *dest, const corba_data_desc_t *src) {
  void *value(NULL);
  diet_base_type_t bt = (diet_base_type_t) src->base_type;
  char *id = CORBA::string_dup(src->id.idNumber);

  switch (bt) {
  case DIET_CHAR: // Impossible to extract a Char or an Octet from an Any.
  case DIET_SHORT: {
    // must be done C fashion, because freed in a C client?
    value = (void *) new short;
    src->specific.scal().value >>= *((CORBA::Short *) (value));
    scalar_set_desc(dest, id, (diet_persistence_mode_t) src->mode, bt, value);
    // delete((short*)value);
    break;
  }
  case DIET_INT: {
    CORBA::Long v;
    value = (void *) new long int;
    src->specific.scal().value >>= v;
    *((long int *) value) = v;
    scalar_set_desc(dest, id, (diet_persistence_mode_t) src->mode, bt, value);
    break;
  }
  case DIET_LONGINT: {
    CORBA::Long v;
    value = (void *) new long long int;
    src->specific.scal().value >>= v;
    *((long long int *) value) = v;
    scalar_set_desc(dest, id, (diet_persistence_mode_t) src->mode, bt, value);
    break;
  }
  case DIET_FLOAT: {
    CORBA::Float f;
    value = (void *) new float;
    src->specific.scal().value >>= f;
    *((float *) value) = f;
    scalar_set_desc(dest, id, (diet_persistence_mode_t) src->mode, bt, value);
    break;
  }
  case DIET_DOUBLE: {
    CORBA::Double d;
    value = (void *) new double;
    src->specific.scal().value >>= d;
    *((double *) value) = d;
    scalar_set_desc(dest, id, (diet_persistence_mode_t) src->mode, bt, value);
    break;
  }
#if HAVE_COMPLEX
  case DIET_SCOMPLEX:
  case DIET_DCOMPLEX:
#endif  // HAVE_COMPLEX
  default:
    MRSH_ERROR("base type " << bt << " not implemented", 1);
  } // switch
  return 0;
} // unmrsh_scalar_desc

int
unmrsh_data_desc(diet_data_desc_t *dest, const corba_data_desc_t *const src) {
  char *id = NULL;
  diet_base_type_t bt = (diet_base_type_t) src->base_type;

  id = CORBA::string_dup(src->id.idNumber);

  switch ((diet_data_type_t) src->specific._d()) {
  case DIET_SCALAR: {
    if (unmrsh_scalar_desc(dest, src)) {
      return 1;
    }
    break;
  }
  case DIET_VECTOR: {
    vector_set_desc(dest, id, (diet_persistence_mode_t) src->mode, bt,
                    src->specific.vect().size);
    break;
  }
  case DIET_MATRIX: {
    matrix_set_desc(dest, id, (diet_persistence_mode_t) src->mode, bt,
                    src->specific.mat().nb_r, src->specific.mat().nb_c,
                    (diet_matrix_order_t) src->specific.mat().order);
    break;
  }
  case DIET_STRING: {
    string_set_desc(dest, id, (diet_persistence_mode_t) src->mode,
                    src->specific.str().length);
    break;
  }
  case DIET_PARAMSTRING: {
    paramstring_set_desc(dest,
                         id,
                         (diet_persistence_mode_t) src->mode,
                         src->specific.pstr().length,
                         src->specific.pstr().param);
    break;
  }
  case DIET_FILE: {
    dest->id = id;
    dest->mode = (diet_persistence_mode_t) src->mode;
    diet_generic_desc_set(&(dest->generic), DIET_FILE, DIET_CHAR);
    dest->specific.file.size = src->specific.file().size;
    dest->specific.file.path = CORBA::string_dup(src->specific.file().path);
    break;
  }
  case DIET_CONTAINER: {
    container_set_desc(dest, id, (diet_persistence_mode_t) src->mode, 0);
    break;
  }
  default:
    MRSH_ERROR("type " << src->specific._d() << " not implemented", 1);
  } // switch
  return 0;
} // unmrsh_data_desc

#if defined HAVE_ALT_BATCH
int
unmrsh_data(diet_data_t *dest, corba_data_t *src, int upDown,
            const char *tmpDir)
#else
int
unmrsh_data(diet_data_t * dest, corba_data_t * src, int upDown)
#endif
{
  static omni_mutex uniqDataIDMutex;

  if ((src->desc.mode == DIET_VOLATILE) && (upDown == 1)) {
    char *tmp = NULL;
    src->desc.id.idNumber = CORBA::string_dup(tmp);  // CORBA frees old mem
  }
  if (unmrsh_data_desc(&(dest->desc), &(src->desc))) {
    return 1;
  }
  if (src->value.length() == 0) {
    // TODO: should be allocated with new x[] to match delete[] used
    // by omniORB.  But ... what is the type?
    dest->value = malloc(data_sizeof(&(dest->desc)));
  } else {
    if (upDown == 0) { /** Need to know if it is in the client -> SeD way or in the SeD-> client way */
      if (src->desc.mode != DIET_VOLATILE) {
        // int size = data_sizeof(&(src->desc));
        // char *p =(char *)malloc(size*sizeof(char));
        // for (int i = 0; i < size; i++)
        // p[i] = src->value[i];
        // dest->value = p;  //memcopy
        dest->value = (char *) src->value.get_buffer(0);
      } else {
        CORBA::Boolean orphan = (src->desc.mode == DIET_VOLATILE);
        dest->value = (char *) src->value.get_buffer(orphan);
      }
    } else { // for out args when send to client
      dest->value = (char *) src->value.get_buffer(1);
    }
  }

  return 0;
} // unmrsh_data


/****************************************************************************/
/* Aggregator descriptor -> Corba aggregator descriptor                     */
/****************************************************************************/

int
mrsh_aggregator_desc(corba_aggregator_desc_t *dest,
                     const diet_aggregator_desc_t *const src) {
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
    for (int pIter = 0; pIter < numPValues; pIter++) {
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
  } // switch
  return (0);
} // mrsh_aggregator_desc

/****************************************************************************/
/* Service profile -> Corba profile descriptor  (for service descriptions)  */
/****************************************************************************/


int
mrsh_profile_desc(corba_profile_desc_t *dest, const diet_profile_desc_t *src) {
  dest->path = CORBA::string_dup(src->path);        // deallocates old dest->path
  dest->last_in = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out = src->last_out;
  dest->param_desc.length(src->last_out + 1);
  for (int i = 0; i <= src->last_out; i++) {
    (dest->param_desc[i]).base_type = (src->param_desc[i]).base_type;
    (dest->param_desc[i]).type = (src->param_desc[i]).type;
  }
#if defined HAVE_ALT_BATCH
  dest->parallel_flag = src->parallel_flag;
#endif

  mrsh_aggregator_desc(&(dest->aggregator), &(src->aggregator));

  return 0;
} // mrsh_profile_desc


/****************************************************************************/
/* Problem profile -> corba profile description (client requests to agent)  */
/****************************************************************************/

int
mrsh_pb_desc(corba_pb_desc_t *dest, const diet_profile_t *const src) {
  dest->path = CORBA::string_dup(src->pb_name);        // frees old dest->path
  dest->last_in = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out = src->last_out;
  dest->param_desc.length(src->last_out + 1);
  for (int i = 0; i <= src->last_out; i++) {
    /** Hack for correctly building CORBA profile.
        JuxMem does not require a call to the MA */
    if (src->parameters[i].desc.id == NULL) {
      mrsh_data_desc(&(dest->param_desc[i]), &(src->parameters[i].desc));
    } else {
      __mrsh_data_id_desc(&(dest->param_desc[i]), &(src->parameters[i].desc));
    }
  }
#if defined HAVE_ALT_BATCH
  dest->parallel_flag = src->parallel_flag;
  dest->nbprocs = src->nbprocs;
  dest->nbprocess = src->nbprocess;
  // No walltime, user has no idea about it
#endif
  return 0;
} // mrsh_pb_desc


/****************************************************************************/
/* Client sends its data to server ...                                      */
/****************************************************************************/

int
mrsh_profile_to_in_args(corba_profile_t *dest, const diet_profile_t *src) {
  int i;

#if defined HAVE_ALT_BATCH
  dest->parallel_flag = src->parallel_flag;
  dest->nbprocs = src->nbprocs;
  dest->nbprocess = src->nbprocess;
  dest->walltime = src->walltime;
#endif
  dest->last_in = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out = src->last_out;
  dest->parameters.length(src->last_out + 1);
  dest->dietReqID = src->dietReqID;

  for (i = 0; i <= src->last_inout; i++) {
    // With Dagda, only the descriptions are sent to the server.
    mrsh_data_desc(&(dest->parameters[i].desc), &(src->parameters[i].desc));
    dest->parameters[i].value.length(0);
  }
  for (; i <= src->last_out; i++) {
    if (mrsh_data_desc(&(dest->parameters[i].desc),
                       &(src->parameters[i].desc))) {
      return 1;
    }
    dest->parameters[i].value.replace(0, 0, NULL, 1);
  }

  // YC: for the moment, dest data have id set to ""
  return 0;
} // mrsh_profile_to_in_args


/****************************************************************************/
/* Server receives client data ...                                          */
/****************************************************************************/

int
cvt_arg(diet_data_t *dest, diet_data_t *src,
        diet_convertor_function_t f, int duplicate_value) {
  switch (f) {
  case DIET_CVT_IDENTITY: {
    (*dest) = (*src);
    if (duplicate_value && src->value) {
      size_t size = data_sizeof(&(src->desc));
      dest->value = new char[size];
      memcpy(dest->value, src->value, size);
      if (dest->desc.generic.type == DIET_SCALAR) {
        dest->desc.specific.scal.value = dest->value;
      }
    }
    break;
  }
  case DIET_CVT_VECT_SIZE: {
    size_t *size = new size_t(src->desc.specific.vect.size);
    diet_scalar_set(dest, size, DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_MAT_NB_ROW: {
    size_t *nb_r = new size_t(src->desc.specific.mat.nb_r);
    diet_scalar_set(dest, nb_r, DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_MAT_NB_COL: {
    size_t *nb_c = new size_t(src->desc.specific.mat.nb_c);
    diet_scalar_set(dest, nb_c, DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_MAT_ORDER: {
    char *t(NULL);
    // FIXME test on order !!!!
    switch (src->desc.specific.mat.order) {
    case DIET_ROW_MAJOR: t = new char('N');
      break;
    case DIET_COL_MAJOR: t = new char('T');
      break;
    default: {
      MRSH_ERROR("invalid order for matrix", 1);
    }
    }
    diet_scalar_set(dest, t, DIET_VOLATILE, DIET_CHAR);
    break;
  }
  case DIET_CVT_STR_LEN: {
    size_t *lgth = new size_t(src->desc.specific.str.length);
    diet_scalar_set(dest, lgth, DIET_VOLATILE, DIET_INT);
    break;
  }
  case DIET_CVT_FILE_SIZE: {
    size_t *size = new size_t(src->desc.specific.file.size);
    diet_scalar_set(dest, size, DIET_VOLATILE, DIET_INT);
    break;
  }
  default: {
    MRSH_ERROR("invalid convertor function", 1);
  }
  } // switch

  return 0;
} // cvt_arg


int
unmrsh_in_args_to_profile(diet_profile_t *dest, corba_profile_t *src,
                          const diet_convertor_t *cvt) {
  /* This keeps all umarshalled arguments */
  diet_data_t **src_params = new diet_data_t *[src->last_out + 1];
  for (int i = 0; i < (src->last_out + 1); i++) {
    src_params[i] = NULL;
  }

#if defined HAVE_ALT_BATCH
  dest->parallel_flag = src->parallel_flag;
  dest->nbprocs = src->nbprocs;
  dest->nbprocess = src->nbprocess;
  dest->walltime = src->walltime;
#endif
  dest->pb_name = cvt->path;
  dest->last_in = cvt->last_in;
  dest->last_inout = cvt->last_inout;
  dest->last_out = cvt->last_out;
  dest->parameters = new diet_data_t[cvt->last_out + 1];
  dest->dietReqID = src->dietReqID;

  for (int i = 0; i <= cvt->last_out; i++) {
    diet_data_t *dd_tmp(NULL);
    int arg_idx = cvt->arg_convs[i].in_arg_idx;
    int duplicate_value = 0;

    if ((arg_idx >= 0) && (arg_idx <= src->last_out)) {
      // Each time the cvt function is IDENTITY, unmrsh the data, even if
      // it has already been done (ie duplicate the value)
      if (src_params[arg_idx] == NULL) {
        src_params[arg_idx] = new diet_data_t;

#if defined HAVE_ALT_BATCH
        // -- GLM : Bug correction --
        /*      if (((SeDImpl*)dest->SeDPtr)->getBatch() != NULL)
                unmrsh_data(src_params[arg_idx], &(src->parameters[arg_idx]), 0,
                ((SeDImpl*)dest->SeDPtr)->getBatch()->getTmpPath());
                else // Should be removed when all classes managed within SeD
                unmrsh_data(src_params[arg_idx], &(src->parameters[arg_idx]), 0,
                "/tmp/"); */
        std::string dataPath = "/tmp/";
        CONFIG_STRING(diet::STORAGEDIR, dataPath);
        unmrsh_data(src_params[arg_idx], &(src->parameters[arg_idx]), 0,
                    dataPath.c_str());
#else
        unmrsh_data(src_params[arg_idx], &(src->parameters[arg_idx]), 0);
#endif
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
} // unmrsh_in_args_to_profile

/****************************************************************************/
/* Server sends results ...                                                 */
/****************************************************************************/

int
mrsh_profile_to_out_args(corba_profile_t *dest, const diet_profile_t *src,
                         const diet_convertor_t *cvt) {
  int i, arg_idx;
  int *args_filled = // Use calloc to set all elements to NULL
                     (int *) calloc(dest->last_out + 1, sizeof(int));
  diet_data_t dd;

  for (i = cvt->last_in + 1; i <= cvt->last_out; i++) {
    arg_idx = cvt->arg_convs[i].out_arg_idx;
    if ((arg_idx >= 0) && (arg_idx <= dest->last_out)) {
      if (dest->parameters[arg_idx].desc.specific._d() == DIET_FILE
          && diet_is_persistent(dest->parameters[arg_idx])) {
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
              if ((status = stat(dd.desc.specific.file.path, &buf))) {
                return status;
              }
              if (!(buf.st_mode & S_IFREG)) {
                return 2;
              }
              dd.desc.specific.file.size = (size_t) buf.st_size;
            } else {
              dd.desc.specific.file.size = 0;
            }
          }

          if (mrsh_data(&(dest->parameters[arg_idx]), &dd,
                        !diet_is_persistent(dd))) {
            return 1;
          }
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
} // mrsh_profile_to_out_args


/****************************************************************************/
/* Client receives server data ...                                          */
/****************************************************************************/

// INOUT parameters should have been set correctly by the ORB.
// But their descriptions could have been slightly altered
// (matrix dimensions, for instance)
int
unmrsh_out_args_to_profile(diet_profile_t *dpb, corba_profile_t *cpb) {
  for (int i = cpb->last_in + 1; i <= cpb->last_out; ++i)
    unmrsh_data_desc(&(dpb->parameters[i].desc), &(cpb->parameters[i].desc));
  return 0;

  int i;

  if ((dpb->last_in != cpb->last_in)
      || (dpb->last_inout != cpb->last_inout)
      || (dpb->last_out != cpb->last_out)
      ) {
    return 1;
  }

#if defined HAVE_ALT_BATCH
  // In case client wants to know how many procs and process have been used
  // but other info like walltime is useless
  dpb->nbprocs = cpb->nbprocs;
  dpb->nbprocess = cpb->nbprocess;
#endif

  // Unmarshal INOUT parameters descriptions only; indeed, the ORB has filled
  // in the memory zone pointed at by the diet_data value field, since the
  // marshalling was performed with replace method.
  for (i = dpb->last_in + 1; i <= dpb->last_inout; i++) {
    corba_data_desc_t *cdd = &(cpb->parameters[i].desc);

    // Special case for INOUT files: rewrite in the same file.
    if (cdd->specific._d() == (long) DIET_FILE) {
      char *inout_path = dpb->parameters[i].desc.specific.file.path;

      int size = cdd->specific.file().size;
      std::ofstream inoutfile(inout_path);
      dpb->parameters[i].desc.specific.file.size = size;
      for (int j = 0; j < size; j++) {
        inoutfile.put(cpb->parameters[i].value[j]);
      }
    } else {
      if (cpb->parameters[i].desc.mode == DIET_VOLATILE) {
        char *tmp = NULL;
        cdd->id.idNumber = CORBA::string_dup(tmp);
      }

      unmrsh_data_desc(&(dpb->parameters[i].desc), cdd);
    }
  }

  // Unmarshal OUT parameters
  for (; i <= dpb->last_out; i++) {
#if defined HAVE_ALT_BATCH
    /* YC: for the moment, client always receives in /tmp/
       -> see with Gael if can be changed with DagDA */
    unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]), 1,
                "/tmp/");
#else
    unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]), 1);
#endif
  }
  return 0;
} // unmrsh_out_args_to_profile

// Dec 2006: In async mode, INOUT parameters are not changed
// Force the unmarshalling of such parameters
// We suppose that the previous function (unmrsh_out_args_to_profile) was
// already called
int
unmrsh_inout_args_to_profile(diet_profile_t *dpb, corba_profile_t *cpb) {
  int i;

  if ((dpb->last_in != cpb->last_in)
      || (dpb->last_inout != cpb->last_inout)
      || (dpb->last_out != cpb->last_out)
      ) {
    return 1;
  }

  // Unmarshal INOUT parameters
  for (i = dpb->last_in + 1; i <= dpb->last_inout; i++) {
#if defined HAVE_ALT_BATCH
    // -- GLM : bug correction --
    /*  if (((SeDImpl*)dpb->SeDPtr)->getBatch() != NULL)
        unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]), 1,
        ((SeDImpl*)dpb->SeDPtr)->getBatch()->getTmpPath());
        else // Should be removed when all classes managed within SeD
        unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]), 1,
        "/tmp/");*/
    std::string dataPath = "/tmp/";
    CONFIG_STRING(diet::STORAGEDIR, dataPath);
    unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]), 1,
                dataPath.c_str());
#else
    unmrsh_data(&(dpb->parameters[i]), &(cpb->parameters[i]), 1);
#endif
  }

  return 0;
} // end unmrsh_inout_args_to_profile


#ifdef HAVE_WORKFLOW
/*
 * Workflow structure marshaling *
 */
int
mrsh_wf_desc(corba_wf_desc_t *dest,
             const diet_wf_desc_t *const src) {
  dest->abstract_wf = CORBA::string_dup(src->abstract_wf);
  return 0;
}
#endif

// this function place is marshalling.cc file
// to fix if necessary
int
unmrsh_profile_desc(diet_profile_desc_t *dest,
                    const corba_profile_desc_t *src) {
  dest->path = strdup(src->path);

  dest->last_in = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out = src->last_out;
  if (src->last_out > -1) {
    dest->param_desc = new diet_arg_desc_t[src->last_out + 1];
  } else {
    dest->param_desc = NULL;
  }

  for (int i = 0; i <= src->last_out; i++) {
    (dest->param_desc[i]).base_type =
      (diet_base_type_t) ((src->param_desc[i]).base_type);
    (dest->param_desc[i]).type = (diet_data_type_t) (src->param_desc[i]).type;
  }
#if defined HAVE_ALT_BATCH
  dest->parallel_flag = src->parallel_flag;
#endif

  // unmarshall the aggregator field
  // TO FIX
  // Since this function is used only by GRPC client lib side, this is not
  // necessary

  return 0;
} // unmrsh_profile_desc
