/**
* @file  WfUtils.hh
* 
* @brief  Some stuffs used for workflow management 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _WFUTILS_HH_
#define _WFUTILS_HH_

#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <omniconfig.h>
#include <omnithread.h>
#include "DIET_data.h"
#ifdef WIN32
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif

/*****************************************************************************/
/*                        CLASS WfStructException                            */
/*****************************************************************************/

class DIET_API_LIB WfStructException {
public:
  enum WfStructErrorType { eUNKNOWN_DAG,
                           eUNKNOWN_NODE,
                           eUNKNOWN_PORT,
                           eDUPLICATE_NODE,
                           eDUPLICATE_PORT,
                           eTYPE_MISMATCH,
                           eDEPTH_MISMATCH,
                           eINVALID_EXPR,
                           eWF_UNDEF,
                           eOTHER };
  WfStructException(WfStructErrorType t, const std::string& info) {
    this->why = t;
    this->info = info;
  }

  explicit WfStructException(const std::string& info) {
    this->why = eOTHER;
    this->info = info;
  }

  WfStructErrorType
  Type() const { return this->why; }

  const std::string&
  Info() const { return this->info; }

  std::string
  ErrorMsg() const;
private:
  WfStructErrorType why;
  std::string info;
};

/*****************************************************************************/
/*                              CLASS WfCst                                  */
/*****************************************************************************/

class DIET_API_LIB WfCst {
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
    TYPE_CONTAINER,

    TYPE_UNKNOWN
  };

  static short
  cvtWfToDietType(WfDataType wfType);

  static short
  cvtDietToWfType(short dietType);

  static short
  cvtStrToWfType(const std::string& strType);

  static const std::string&
  cvtWfToStrType(WfDataType wfType);

  static const std::string&
  cvtWfToXSType(WfDataType wfType);

  static const std::string&
  cvtGwendiaToDietType(const std::string& gwType);

  static bool
  isMatrixType(const std::string& strType);
  static bool
  isMatrixType(const WfDataType type);

  /************************************************************/
  /** METHODS FOR MATRIX HANDLING                             */
  /************************************************************/

  enum WfMatrixOrder {
    ORDER_COL_MAJOR,
    ORDER_ROW_MAJOR
  };

  static short
  cvtWfToDietMatrixOrder(WfMatrixOrder wfMatrixOrder);

  static short
  cvtStrToWfMatrixOrder(const std::string& strMatrixOrder);

  static const std::string&
  cvtWfToStrMatrixOrder(WfMatrixOrder wfMatrixOrder);

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

  static std::string  dietTypePrefix;
};

/***************************************************************/
/** For debugging                                              */
/***************************************************************/
/**
 * The following macro is similar to the one defined *
 * in debug.hh  but use the macro DEBUG_WF (to replace TRACE_LEVEL) *
 */

extern omni_mutex debug_log_mutex;

#ifdef DEBUG_WF
#define debug_wf(formatted_text)                \
  debug_log_mutex.lock();                       \
  cout << formatted_text << endl;               \
  debug_log_mutex.unlock();
#else
#define debug_wf(formatted_text)
#endif  // DEBUG_WF

/***************************************************************/


/**
 * get the matrix order by a string
 */
diet_matrix_order_t
getMatrixOrder(const std::string &matrix_order);


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
DIET_API_LIB std::string
itoa(long l);

#endif   /* not defined _WFUTILS_HH_ */
