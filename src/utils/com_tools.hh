/****************************************************************************/
/* DIET communication tools specification                                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2003/04/10 12:45:25  pcombes
 * Remove duplicated functions.
 *
 * Revision 1.3  2003/02/04 10:08:22  pcombes
 * Apply Coding Standards
 *
 * Revision 1.2  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 ****************************************************************************/


#ifndef _COM_TOOLS_H_
#define _COM_TOOLS_H_

#include "common_types.hh"
#include "DIET_config.h"

#if HAVE_CICHLID

long
profile_size(corba_pb_desc_t* p);

void
init_communications();

void
add_communication(char* node0, char* node1, long size);


#endif // HAVE_CICHLID
#endif // _COM_TOOLS_H_
