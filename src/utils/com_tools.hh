/****************************************************************************/
/* $Id$ */
/* DIET communication tools specification                                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.3  2003/02/04 10:08:22  pcombes
 * Apply Coding Standards
 *
 * Revision 1.2  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 ****************************************************************************/


#ifndef _COM_TOOLS_H_
#define _COM_TOOLS_H_

#include "DIET_client.h"

#if HAVE_CICHLID


int
base_type_size(diet_base_type_t base_type);

long
parameter_size(diet_arg_t* p);

long
profile_size(diet_profile_t* p);

void
init_communications();

void
add_communication(char* node0, char* node1, long size);


#endif // HAVE_CICHLID
#endif // _COM_TOOLS_H_
