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
 * Revision 1.6  2002/09/09 15:58:03  pcombes
 * Fix bugs in marshalling
 *
 * Revision 1.5  2002/08/30 16:50:16  pcombes
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

#define print_matrix(mat, m, n)            \
  {                                        \
    size_t i, j;                           \
    printf("%s = \n", #mat);               \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
	printf("%3f ", (mat)[i*(n) + j]);  \
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

/*--------------------------------------------------------------------------*/
/* All the functions used to display Diet structures (for logging purposes) */
/* can be found in this library.                                            */
/*--------------------------------------------------------------------------*/

void displayResponse(FILE *os,const corba_response_t *resp);

void displayMAList(FILE *os,dietMADescList *MAs);

void displayProfileDesc(const diet_profile_desc_t *profile, const char *path);
void displayCorbaProfileDesc(const corba_profile_desc_t *profile);
void displayProfile(const corba_profile_t *profile);

#endif // _DEBUG_HH_
