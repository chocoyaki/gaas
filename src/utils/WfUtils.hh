/****************************************************************************/
/* Some stuffs used for workflow management                                 */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2006/04/14 14:02:03  aamar
 * Some tools used for workflow support.
 *
 ****************************************************************************/

#ifndef _WFUTILS_HH_
#define _WFUTILS_HH_

//#include "DIET_data.h"
#include <stdio.h>

#include <omniconfig.h>
#include <omnithread.h>

class WfCst {
public:
  static const char * DIET_CHAR;
  static const char * DIET_SHORT;
  static const char * DIET_INT;
  static const char * DIET_LONGINT;
  static const char * DIET_FLOAT;
  static const char * DIET_DOUBLE;
  static const char * DIET_SCOMPLEX;
  static const char * DIET_DCOMPLEX;
  static const char * DIET_BASE_TYPE_COUNT;

  static const char * DIET_SCALAR;
  static const char * DIET_VECTOR;
  static const char * DIET_MATRIX;
  static const char * DIET_STRING;
  static const char * DIET_PARAMSTRING;
  static const char * DIET_FILE;
  static const char * DIET_DATA_TYPE_COUNT;
private:
};

typedef enum {
  WF_SCHED_SIMPLE, 
  WF_SCHED_EFT, 
  WF_SCHED_CUSTOM
} wf_sched_strat_t;

typedef enum {
  ARG_PORT,
  IN_PORT,
  INOUT_PORT,
  OUT_PORT
} wf_port_t;

/***************************************************************/
/** For debugging                                              */
/***************************************************************/
/**
 * The following macro is similar to the one defined *
 * in debug.hh  but use the macro DEBUG_WF (to replace TRACE_LEVEL) *
 */

extern omni_mutex debug_log_mutex ;

#ifdef DEBUG_WF
#define debug_wf(formatted_text)             \
    debug_log_mutex.lock();                  \
    cout << formatted_text << endl;          \
    debug_log_mutex.unlock();
#else 
#define debug_wf(formatted_text)
#endif // DEBUG_WF

/***************************************************************/

#endif   /* not defined _WFUTILS_HH_ */
