/****************************************************************************/
/* DIET debug utils header                                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.12  2003/04/10 12:45:44  pcombes
 * Set TRACE_LEVEL as a static variable, used by all other modules.
 * Update displayResponse to the new corba_response_t structure.
 *
 * Revision 1.11  2003/02/04 10:08:23  pcombes
 * Apply Coding Standards
 *
 * Revision 1.10  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 *
 * Revision 1.8  2002/10/03 17:58:21  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
 *
 * Revision 1.7  2002/09/17 15:23:19  pcombes
 * Bug fixes on inout arguments and examples
 * Add support for omniORB 4.0.0
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
 ****************************************************************************/


#ifndef _DEBUG_HH_
#define _DEBUG_HH_

#include <iostream>
using namespace std;
#include <stdio.h>

#include "common_types.hh"
#include "response.hh"
#include "dietTypes.hh"

/**
 * Various trace levels
 */
#define NO_TRACE            0
#define TRACE_MAIN_STEPS    1
#define TRACE_ALL_STEPS     5
#define TRACE_STRUCTURES   10
#define TRACE_MAX_VALUE    TRACE_STRUCTURES
#define TRACE_DEFAULT      TRACE_MAIN_STEPS

/**
 * Always useful
 */
#define MAX(a,b) ((a) < (b)) ? (b) : (a)
#define MIN(a,b) ((a) > (b)) ? (b) : (a)



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

#define print_matrix(mat, m, n, rm)        \
  {                                        \
    size_t i, j;                           \
    printf("%s (%s-major) = \n", #mat,     \
           (rm) ? "row" : "column");       \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
        if (rm)                            \
	  printf("%3f ", (mat)[j + i*(n)]);\
        else                               \
	  printf("%3f ", (mat)[i + j*(m)]);\
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

/*--------------------------------------------------------------------------*/
/* All the functions used to display Diet structures (for logging purposes) */
/* can be found in this library.                                            */
/*--------------------------------------------------------------------------*/

void
displayResponse(FILE* os, const corba_response_t* resp);

#if 0
void
displayMAList(FILE* os, dietMADescList* MAs);
#endif // 0


void
displayArgDesc(FILE* f, int type, int base_type);
void
displayArg(FILE* f, const corba_data_desc_t* arg);
void
displayArg(FILE* f, const diet_data_desc_t* arg);

void
displayProfileDesc(const diet_profile_desc_t* desc, const char* path);
void
displayProfileDesc(const corba_profile_desc_t* desc);
void
displayProfile(const diet_profile_t* profile, const char* path);
void
displayProfile(const corba_profile_t* profile, const char* path);
void
displayPbDesc(const corba_pb_desc_t* pb_desc);

void
displayConvertor(FILE* f, const diet_convertor_t* cvt);

#endif // _DEBUG_HH_
