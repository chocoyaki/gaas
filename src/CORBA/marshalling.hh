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

#include "dietTypes.hh"
#include "common_types.hh"

#include "DIET_config.h"


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
mrsh_pb_desc(corba_pb_desc_t* dest, diet_profile_t* src);


// To send the input data from client to SeD
int
mrsh_profile_to_in_args(corba_profile_t* dest, const diet_profile_t* src);

// To convert client data profile to service profile
int
unmrsh_in_args_to_profile(diet_profile_t* dest, corba_profile_t* src,
			  const diet_convertor_t* cvt);

// To reconvert service output data to client data profile
int
mrsh_profile_to_out_args(corba_profile_t* dest, const diet_profile_t* src,
			 const diet_convertor_t* cvt);

// To receive output data on the client
int
unmrsh_out_args_to_profile(diet_profile_t* dpb, corba_profile_t* cpb);

int
unmrsh_data(diet_data_t* dest, corba_data_t* src);

#endif // _MARSHALLING_HH_
