/****************************************************************************/
/* The class used to parse and execute conditional expressions used in      */
/* workflow control structures (If, While, ...)
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/05/15 11:01:24  bisnard
 * new class WfExpression used to evaluate expression in IF node
 *
 *
 */

#ifndef _WFEXPRESSION_HH_
#define _WFEXPRESSION_HH_

#include <string>
#include <list>
#include <xqilla/xqilla-simple.hpp>
#include "WfUtils.hh"


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


class WfExpression {

  public:

    WfExpression();
    virtual ~WfExpression();

    /**
     * Set the conditional expression
     * @param exprStr XQuery containing variables with syntax $var
     */
    virtual void setExpression(const std::string& exprStr);

    /**
     * Set the value of a variable for the expression
     * (if variable name is not found in the expression, nothing happens)
     * @param varName string containing the variable name
     * @param varType type of the variable
     * @param isSequence set to true if the value contains a list: ( value1, value2, ... )
     * @param varValue string containing the variable value
     */
    virtual void
        setVariable(const std::string& varName,
                    const WfCst::WfDataType varType,
                    const bool isSequence,
                    const std::string& varValue);

    /**
     * Idem above but give a default value to the variable
     * (used for testing the expression)
     */
    virtual void
        setVariableDefaultValue(const std::string& varName,
                                const WfCst::WfDataType varType,
                                const bool isSequence);
    /**
     * Get the expression
     */
    const std::string& getExpression();

    /**
     * Get the full query (used for exceptions)
     */
    const std::string& getQueryString();

    /**
     * Check if a variable name is used in the expression
     */
    bool isVariableUsed(const std::string& varName);

    /**
     * Evaluate the expression
     */
    virtual void evaluate();

    /**
     * Reset the variables of the expression
     */
    virtual void reset();

  protected:

    /**
     * Method to initialize the XQuery before parsing
     * (uses both the expression and the variables)
     */
    void initQuery();

    /**
     * Method to parse an expression containing variables
     */
    void parseQuery();

    /**
     * Method to add a variable declaration to the query
     */
    void
        addVariableDecl(const std::string& varName,
                        const std::string& XSType,
                        const std::string& varValue);

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
     * Variable record type
     */
    struct varInfo_t {
      std::string name;
      std::string xsType;
      std::string value;
    };

    /**
     * Variables storage
     */
    std::list<varInfo_t>  myVariables;

    /**
     * Parser
     */
    WfExpressionParser* myParser;

    /**
     * The query object
     */
    XQQuery* myQuery;

};

class WfBooleanExpression : public WfExpression {

  public:

    /**
     * Evaluate the expression
     * @return true if the expression evaluates to 'true', else false
     */
    bool
        testIfTrue();
};

#endif // _WFEXPRESSION_HH_
