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

/*
 * Data conversion
 */

/* This functions allocate each member of the destination sequence. But they
   reuse as much as possible already allocated memory */
int mrsh_data_seq(SeqCorbaData_t *dest, diet_data_seq_t *src,
		  int only_desc, int release);
int unmrsh_data_seq(diet_data_seq_t *dest, const SeqCorbaData_t *src);


/*
 * Profile conversion
 */

int mrsh_profile_desc(corba_profile_desc_t *dest,
		      diet_profile_desc_t *src, char *src_name);
/* dest_name is allocated: caller is responsible for freeing it */
int unmrsh_profile_desc(diet_profile_desc_t *dest, char **dest_name,
			corba_profile_desc_t *src);
int unmrsh_profile_desc_to_name(char **dest_name, corba_profile_desc_t *src);


int mrsh_profile(corba_profile_t *dest, diet_profile_t *src, char *src_name);
int unmrsh_profile_to_desc(diet_profile_desc_t *dest, char **dest_name,
			   corba_profile_t *src);
int unmrsh_profile_to_sf(sf_inst_desc_t *dest, const corba_profile_t *src);


/*
 * Profile -> corba data sequences
 */

int mrsh_profile_to_in_args(SeqCorbaData_t *in,
			    SeqCorbaData_t *inout,SeqCorbaData_t *out,
			    const diet_profile_t *profile);

int unmrsh_out_args_to_profile(diet_profile_t *profile,
			       SeqCorbaData_t *inout, SeqCorbaData_t *out);

#endif // _MARSHALLING_HH_
