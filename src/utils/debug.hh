/****************************************************************************/
/* $Id$ */
/* DIET debug utils header                                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Frederic LOMBARD          - LIFC Besançon (France)                  */
/*    - Philippe COMBES           - LIP ENS Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.4  2002/08/09 14:30:34  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 * Revision 1.3  2002/05/24 19:36:54  pcombes
 * Add BLAS/dgemm example (implied bug fixes)
 *
 * Revision 1.2  2002/05/17 20:35:18  pcombes
 * Version alpha without FAST 
 *
 ****************************************************************************/


#ifndef _DEBUG_HH_
#define _DEBUG_HH_

#include <stdio.h>

#include "types.hh"
#include "dietTypes.hh"

// DEBUG trace: print variable name and value
#define TRACE(var) cout << #var << " = " << (var) << endl

// DEBUG pause: insert a pause of duration <s>+<us>E-6 seconds
#define PAUSE(s,us)                 \
{                                   \
  struct timeval tv;                \
  tv.tv_sec  = s;                      \
  tv.tv_usec = us;                     \
  select(0, NULL, NULL, NULL, &tv); \
}

/*--------------------------------------------------------------------------*/
/* All the functions used to display Diet structures (for logging purposes) */
/* can be found in this library.                                            */
/*--------------------------------------------------------------------------*/

void displayResponse(FILE *os,const corba_response_t *resp);

void displayMAList(FILE *os,dietMADescList *MAs);

#endif // _DEBUG_HH_
