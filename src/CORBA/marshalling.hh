/**
 * @file marshalling.hh
 *
 * @brief  DIET CORBA marshalling header
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Frederic LOMBARD (Frederc.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _MARSHALLING_HH_
#define _MARSHALLING_HH_

#include "DIET_data.h"
#include "DIET_server.h"

#include "DIET_data_internal.hh"
#include "common_types.hh"

/*--------------------------------------------------------------------------*/
/* <FIXME: This comments should be updated>                                 */
/* These functions allow to copy the content of diet structures between     */
/* Corba and standard versions. Whenever it's possible, no data should be   */
/* shared between the two copies. This should prevent the Corba stub/skel   */
/* from deallocating usefull datas.                                         */
/* </FIXME>                                                                 */
/*                                                                          */
/* Functions whose name begins with "mrsh" are marshalling functions, i.e.  */
/* they convert standard C or C++ data into CORBA structures.               */
/* Functions whose name begins with "unmrsh" are unmarshalling functions.   */
/* Conversion are not always from structures to their exact equivalent,     */
/* but their name and prototype should be relevant enough.                  */
/*--------------------------------------------------------------------------*/

int
__mrsh_data_desc_type(corba_data_desc_t *dest,
                      const diet_data_desc_t *const src);

/*
 * Trace Level
 */
void
mrsh_set_trace_level(int level);


/*
 * Profile conversion
 */

// To add a CORBA service into the SeD service table
int
mrsh_profile_desc(corba_profile_desc_t *dest, const diet_profile_desc_t *src);

// To submit a request from the client DIET profile
int
mrsh_pb_desc(corba_pb_desc_t *dest, const diet_profile_t *const src);


// To send the input data from client to SeD
int
mrsh_profile_to_in_args(corba_profile_t *dest, const diet_profile_t *src);

// To convert client data profile to service profile
int
unmrsh_in_args_to_profile(diet_profile_t *dest, corba_profile_t *src,
                          const diet_convertor_t *cvt);
/* Needed by DAGDA. */
int
mrsh_data_desc(corba_data_desc_t *dest, diet_data_desc_t *src);

// To reconvert service output data to client data profile
int
mrsh_profile_to_out_args(corba_profile_t *dest, const diet_profile_t *src,
                         const diet_convertor_t *cvt);

// To receive output data on the client
int
unmrsh_out_args_to_profile(diet_profile_t *dpb, corba_profile_t *cpb);

// Force the unmarshalling of INOUT parameters (useful in async mdode)
int
unmrsh_inout_args_to_profile(diet_profile_t *dpb, corba_profile_t *cpb);

// tmpDir is the prefix of where data are stored on server
#if defined HAVE_ALT_BATCH
int
unmrsh_data(diet_data_t *dest, corba_data_t *src, int, const char *tmpDir);
#else
int
unmrsh_data(diet_data_t *dest, corba_data_t *src, int);
#endif

// unmarshall only the data description (needed for custom
// performance metrics)
int
unmrsh_data_desc(diet_data_desc_t *dest, const corba_data_desc_t *const src);


#ifdef HAVE_WORKFLOW
// Workflow structure marshaling
int
mrsh_wf_desc(corba_wf_desc_t *dest, const diet_wf_desc_t *const src);
#endif

int
unmrsh_profile_desc(diet_profile_desc_t *dest,
                    const corba_profile_desc_t *src);

#endif  // _MARSHALLING_HH_
