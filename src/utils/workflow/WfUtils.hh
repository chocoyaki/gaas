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
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.3  2006/10/20 08:35:39  aamar
 * *** empty log message ***
 *
 * Revision 1.2  2006/07/10 11:10:30  aamar
 * - Adding some helpful functions for matrix handling and diet
 * data types manipulation.
 * - The eval_expr to evaluate arithmetic expressions
 *
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
#include <string>
#include <map>
#include <vector>

#include "DIET_data.h"

class WfCst {
public:
  /************************************************************/
  /** STATIC CONSTANTS                                       **/
  /************************************************************/

  /************************************************************/
  /** DIET TYPE                                              **/
  /************************************************************/
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

  /************************************************************/
  /** METHODS FOR MATRIX HANDLING                             */
  /************************************************************/
  static void
  open_file(const char * fileName, FILE *& myFile);

  static unsigned long
  readChar(const char * fileName, char * mat, unsigned long mat_size);

  static unsigned long
  readShort(const char * fileName, short * mat, unsigned long mat_size);

  static unsigned long
  readInt(const char * fileName, int * mat, unsigned long mat_size);

  static unsigned long
  readLong(const char * fileName, long * mat, unsigned long mat_size);

  static unsigned long
  readFloat(const char * fileName, float * mat, unsigned long mat_size);

  static unsigned long
  readDouble(const char * fileName, double * mat, unsigned long mat_size);


  static long
  eval_expr(std::string& expr, int var = 0);

  static long int
  diff(struct timeval tv, struct timeval beginning,
       const long int l);
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

/**
 * get the diet base type by a string 
 */
diet_base_type_t
getBaseType(const std::string base_type);

/**
 * get the string representation of diet base type  
 */
std::string
getBaseTypeStr(const diet_base_type_t base_type);

/**
 * get the matrix order by a string 
 */
diet_matrix_order_t
getMatrixOrder(const std::string matrix_order);


/**
 * get the string associated to a matrix order 
 */
std::string
getMatrixOrderStr(const diet_matrix_order_t matrix_order);


/**
 * return a list of token composing a string
 * used to read the matrix value
 */
std::vector<std::string> 
getStringToken(std::string str);


/**
 * return a string representation on an integer
 */
std::string
itoa(long l);

#endif   /* not defined _WFUTILS_HH_ */
