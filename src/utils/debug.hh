/****************************************************************************/
/* $Id$                */
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
 * Revision 1.2  2002/05/17 20:35:18  pcombes
 * Version alpha without FAST
 * */


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
