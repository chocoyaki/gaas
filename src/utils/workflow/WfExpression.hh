/**
* @file  WfExpression.hh
* 
* @brief  The class used to parse and execute conditional expressions used in workflow control structures
* 
* @author  - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.5  2011/01/12 09:12:18  bdepardo
 * Removed warning during code documentation generation
 *
 * Revision 1.4  2009/08/26 10:32:11  bisnard
 * corrected  warnings
 *
 * Revision 1.3  2009/07/08 08:56:55  bisnard
 * avoid false variable detection when var name is prefix of another variable
 *
 * Revision 1.2  2009/06/15 12:14:08  bisnard
 * added new class WfExprVariable to handle expression variables in conditional nodes
 *
 * Revision 1.1  2009/05/15 11:01:24  bisnard
 * new class WfExpression used to evaluate expression in IF node
 *
 *
 */

#ifndef _WFEXPRESSION_HH_
#define _WFEXPRESSION_HH_

#include <list>
#include <string>
#include <xqilla/xqilla-simple.hpp>
#include "WfUtils.hh"


/*****************************************************************************/
/*                       WfExpressionParser class                            */
/*****************************************************************************/

class WfExpressionParser {
public:
  /**
   * Parse a string containing a XQuery and returns the XQilla query
   * object (query is not executed yet)
   * @param queryStr  string containing a XQuery
   */
  XQQuery*
  parse(const std::string& queryStr);

  /**
   * Returns the unique instance of this class
   */
  static WfExpressionParser*
  instance();

  /**
   * Destructor
   */
  ~WfExpressionParser();

private:

  WfExpressionParser();

  /**
   * Unique instance reference
   */
  static WfExpressionParser* myInstance;

  /**
   * Xqilla environment object
   */
  XQilla* myImpl;
};

/*****************************************************************************/
/*                        WfExprVariable class                               */
/*****************************************************************************/

class WfExprVariable {
public:
  /**
   * Initialize a variable
   * @param varName string containing the variable name
   * @param varType type of the variable
   */
  WfExprVariable(const std::string& varName, const WfCst::WfDataType varType);

  virtual ~WfExprVariable() {
  }

  /**
   * Get name
   */
  const std::string&
  getName() { return myName; }

  /**
   * Generate the XQuery declaration of the variable
   * @param output  an output stream
   */
  virtual void
  getXQDeclaration(std::ostream& output);

  /**
   * Set the value of the variable
   * @param varValue raw value if scalar, XML-encoded if container
   */
  virtual void
  setValue(const std::string& varValue);

  /**
   * Set a default value for the variable
   */
  virtual void
  setDefaultValue();

protected:
  std::string myName;
  WfCst::WfDataType myType;
  std::string myValue;
};

/*****************************************************************************/
/*                          WfExpression class                               */
/*****************************************************************************/

class WfExpression {
public:
  WfExpression();
  virtual ~WfExpression();

  /**
   * Set the conditional expression
   * @param exprStr XQuery containing variables with syntax $var
   */
  virtual void
  setExpression(const std::string& exprStr);

  /**
   * Add a variable to the expression
   * @param var ref to a WfExprVariable
   */
  virtual void
  addVariable(WfExprVariable*  var);

  /**
   * Get the expression
   */
  const std::string&
  getExpression();

  /**
   * Get the full query (used for exceptions)
   */
  const std::string&
  getQueryString();

  /**
   * Check if a variable name is used in the expression
   */
  // characters that can happen after a variable
  static std::string XQVarSeparators;

  bool
  isVariableUsed(const std::string& varName);

  /**
   * Evaluate the expression
   */
  virtual void
  evaluate();

protected:
  /**
   * Method to initialize the XQuery before parsing
   * (uses both the expression and the variables)
   */
  void
  initQuery();

  /**
   * Method to parse an expression containing variables
   */
  void
  parseQuery();

  /**
   * Original expression
   */
  std::string myExpression;

  /**
   * Query (in XQuery lang)
   */
  std::string myQueryStr;

  /**
   * Result
   */
  std::string myResult;

  /**
   * Variables storage
   */
  std::list<WfExprVariable*> myVariables;

  /**
   * Parser
   */
  WfExpressionParser* myParser;

  /**
   * The query object
   */
  XQQuery* myQuery;
};


/*****************************************************************************/
/*                       WfBooleanExpression class                           */
/*****************************************************************************/

class WfBooleanExpression : public WfExpression {
public:
  /**
   * Evaluate the expression
   * @return true if the expression evaluates to 'true', else false
   */
  bool
  testIfTrue();
};

#endif  // _WFEXPRESSION_HH_
