/****************************************************************************/
/* $Id$                  */
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

/* $Log$
/* Revision 1.1.1.1  2002/05/16 10:49:09  pcombes
/* Set source tree for DIET alpha version
/* */


#ifndef _DEBUG_HH_
#define _DEBUG_HH_

#include <stdio.h>

#include <types.hh>
#include <dietTypes.hh>

// DEBUG trace: print variable name and value
#define TRACE(var) cout << #var << " = " << (var) << endl


/*--------------------------------------------------------------------------*/
/* All the functions used to display Diet structures (for logging purposes) */
/* can be found in this library.                                            */
/*--------------------------------------------------------------------------*/

void displayResponse(FILE *os,const diet_response_t *resp);

void displayMAList(FILE *os,dietMADescList *MAs);

#endif // _DEBUG_HH_
