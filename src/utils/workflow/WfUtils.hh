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
 * Revision 1.4  2008/09/30 09:24:27  bisnard
 * new static maps for converting workflow data types to diet data types
 *
 * Revision 1.3  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.2  2008/04/28 12:16:23  bisnard
 * timestamps diff not used anymore
 *
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
  /** PORT TYPE                                              **/
  /************************************************************/

  enum WfDataType {
   TYPE_CHAR,
   TYPE_SHORT,
   TYPE_INT,
   TYPE_LONGINT,
   TYPE_FLOAT,
   TYPE_DOUBLE,

   TYPE_MATRIX,
   TYPE_STRING,
   TYPE_PARAMSTRING,
   TYPE_FILE,
   TYPE_CONTAINER
  };

  static short
  cvtWfToDietType(WfDataType wfType);

  static short
  cvtStrToWfType(const std::string& strType);

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

private:
};

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
