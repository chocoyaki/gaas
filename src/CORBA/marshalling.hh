/****************************************************************************/
/* DIET CORBA marshalling header                                            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.33  2011/05/09 13:10:09  bdepardo
 * Added method diet_get_SeD_services to retreive the services of a SeD given
 * its name
 *
 * Revision 1.32  2008/04/18 13:47:23  glemahec
 * Everything about DAGDA is now in utils/DAGDA directory.
 *
 * Revision 1.31  2008/01/14 11:08:26  glemahec
 * marshalling modifications to allow the use of dagda as data manager.
 * v1.0: A lot of modifications will be added in the v1.2 version (coming soon)
 *
 * Revision 1.30  2008/01/01 19:02:49  ycaniou
 * Make modifications in order for pathToTmp and pathToNFS set in the SeD.cfg
 *   to be taken into account when transfering data.
 *
 * Important Note: it compiles and runs well under Linux, but compiles and does
 *   not run on AIX: client and agent must have a reference to getBatch() which is
 *   in a lib only linked to the SeD. Should be ok when Dagda will be commited.
 *
 * Revision 1.29  2007/07/09 18:54:48  aamar
 * Adding Endianness support (CMake option).
 *
 * Revision 1.28  2006/11/27 13:27:53  aamar
 * Force the unmarshalling of inout parameters for the asynchronous mode.
 *
 * Revision 1.27  2006/11/16 09:55:51  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.26  2006/04/14 14:16:15  aamar
 * Adding the marshalling functions between C workflow data types and
 * the corresponding IDL types.
 *
 * Revision 1.25  2005/10/05 11:35:31  ecaron
 * Fix Warning (Bug in maintainer mode on MacOSX)
 * mrsh_data_desc_type(corba_data_desc_t*, const diet_data_desc_t*)' was declared 'static' but never defined
 *
 * Revision 1.24  2005/10/05 09:51:20  ecaron
 * Fix Warning (Bug in maintainer mode on MacOSX)
 * uninitialized type in function for DIET_VECTOR and DIET_STRING
 *
 * Revision 1.23  2005/08/31 14:51:35  alsu
 * New plugin scheduling interface: marshalling/unmarshalling functions
 * no longer needed for the estimation vector, since the C structure no
 * longer exists and the CORBA C++ structure is being used throughout the
 * DIET code base.
 *
 * Revision 1.22  2004/12/08 15:02:51  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.21.2.3  2004/11/26 15:19:13  alsu
 * making the src argument of mrsh_pb_desc const
 *
 * Revision 1.21.2.2  2004/11/24 09:32:38  alsu
 * adding CORBA support (new datatype in the IDL and corresponding
 * marshalling/unmarshalling functions) to deal with
 * DIET_PARAMSTRING-specific data description structure
 *
 * Revision 1.21.2.1  2004/11/02 00:30:59  alsu
 * marshalling/unmarshalling functions to translate between estimations
 * (CORBA structures) and estimation vectors (plain C structures)
 *
 * Revision 1.21  2003/12/01 14:49:30  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.20  2003/09/22 21:06:22  pcombes
 * Rollback after Bruno's too quick commit.
 *
 * Revision 1.18  2003/08/09 17:31:38  pcombes
 * Include path in the diet_profile_desc structure.
 *
 * Revision 1.17  2003/08/01 19:26:07  pcombes
 * The conversions to FAST problems are now managed by FASTMgr.
 *
 * Revision 1.16  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.15  2003/04/10 12:40:22  pcombes
 * Use the TRACE_LEVEL of the debug module. Manage the data ID.
 *
 * Revision 1.14  2003/02/07 17:04:12  pcombes
 * Refine convertor API: arg_idx is splitted into in_arg_idx and out_arg_idx.
 *
 * Revision 1.13  2003/02/04 10:08:22  pcombes
 * Apply Coding Standards
 *
 * Revision 1.12  2003/01/23 18:40:53  pcombes
 * Remove "only_value" argument to unmrsh_data, which is now useless
 *
 * Revision 1.11  2002/12/03 19:08:23  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 *
 * Revision 1.9  2002/10/15 18:43:48  pcombes
 * Implement convertor API and file transfer.
 *
 * Revision 1.8  2002/10/03 17:58:13  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
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
 ****************************************************************************/


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
__mrsh_data_desc_type(corba_data_desc_t* dest,
                      const diet_data_desc_t* const src);

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
mrsh_profile_desc(corba_profile_desc_t* dest, const diet_profile_desc_t* src);

// To submit a request from the client DIET profile
int
mrsh_pb_desc(corba_pb_desc_t* dest, const diet_profile_t* const src);


// To send the input data from client to SeD
int
mrsh_profile_to_in_args(corba_profile_t* dest, const diet_profile_t* src);

// To convert client data profile to service profile
int
unmrsh_in_args_to_profile(diet_profile_t* dest, corba_profile_t* src,
                          const diet_convertor_t* cvt);
/* Needed by DAGDA. */
int
mrsh_data_desc(corba_data_desc_t* dest, diet_data_desc_t* src);

// To reconvert service output data to client data profile
int
mrsh_profile_to_out_args(corba_profile_t* dest, const diet_profile_t* src,
                         const diet_convertor_t* cvt);

// To receive output data on the client
int
unmrsh_out_args_to_profile(diet_profile_t* dpb, corba_profile_t* cpb);

// Force the unmarshalling of INOUT parameters (useful in async mdode)
int
unmrsh_inout_args_to_profile(diet_profile_t* dpb, corba_profile_t* cpb);

// tmpDir is the prefix of where data are stored on server
#if defined HAVE_ALT_BATCH
int
unmrsh_data(diet_data_t* dest, corba_data_t* src, int, const char * tmpDir);
#else
int
unmrsh_data(diet_data_t* dest, corba_data_t* src, int);
#endif

// unmarshall only the data description (needed for custom
// performance metrics)
int unmrsh_data_desc(diet_data_desc_t* dest,
                     const corba_data_desc_t* const src);


#ifdef HAVE_WORKFLOW
// Workflow structure marshaling
int
mrsh_wf_desc(corba_wf_desc_t* dest,
             const diet_wf_desc_t* const src);
#endif


int unmrsh_profile_desc(diet_profile_desc_t* dest,
                        const corba_profile_desc_t* src);

#endif // _MARSHALLING_HH_
