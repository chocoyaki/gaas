/****************************************************************************/
/* $Id$ */
/* DIET CORBA marshalling header                                            */
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
 * Revision 1.10  2002/10/25 14:31:17  ecaron
 * FAST support: convertors implemented and compatible to --without-fast
 *               configure option, but still not tested with FAST !
 *
 * Revision 1.10  2002/10/25 10:50:22  pcombes
 * FAST support: convertors implemented and compatible to --without-fast
 *               configure option, but still not tested with FAST !
 *
 * Revision 1.9  2002/10/15 18:43:48  pcombes
 * Implement convertor API and file transfer.
 *
 * Revision 1.8  2002/10/03 17:58:13  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
 *
 * Revision 1.7  2002/09/17 15:23:11  pcombes
 * Bug fixes on inout arguments and examples
 * Add support for omniORB 4.0.0
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
 ****************************************************************************/


#ifndef _MARSHALLING_HH_
#define _MARSHALLING_HH_

#include "DIET_client.h"
#include "DIET_data.h"
#include "DIET_server.h"

#include "dietTypes.hh"
#include "types.hh"


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

int unmrsh_data(diet_data_t *dest, corba_data_t *src, int only_value);


/*
 * Trace Level
 */
void mrsh_set_trace_level(int level);


/*
 * Profile conversion
 */

// To add a CORBA service into the SeD service table
int mrsh_profile_desc(corba_profile_desc_t *dest,
		      diet_profile_desc_t *src, char *src_name);

// To submit a request from the client DIET profile
int mrsh_pb_desc(corba_pb_desc_t *dest, diet_profile_t *src, char *src_name);

// To convert a request profile to FAST sf_inst_desc for evaluation
int unmrsh_pb_desc_to_sf(sf_inst_desc_t *dest, const corba_pb_desc_t *src,
			 const diet_convertor_t *cvt);

// To send the input data from client to SeD
int mrsh_profile_to_in_args(corba_profile_t *dest, const diet_profile_t *src);

// To convert client data profile to service profile
int unmrsh_in_args_to_profile(diet_profile_t *dest, corba_profile_t *src,
			      const diet_convertor_t *cvt);

// To reconvert service output data to client data profile
int mrsh_profile_to_out_args(corba_profile_t *dest, const diet_profile_t *src,
			     const diet_convertor_t *cvt);

// To receive output data on the client
int unmrsh_out_args_to_profile(diet_profile_t *dpb, corba_profile_t *cpb);


#endif // _MARSHALLING_HH_
