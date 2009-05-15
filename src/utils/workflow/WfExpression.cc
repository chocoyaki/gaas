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

#include "WfExpression.hh"
#include "debug.hh"
#include "DagWfParser.hh"

using namespace std;


/*****************************************************************************/
/*                     WfExpressionParser class                               */
/*****************************************************************************/

// Initialisation of static members
WfExpressionParser* WfExpressionParser::myInstance = NULL;

// Private constructor
WfExpressionParser::WfExpressionParser() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Initializing XQilla..." << endl);
  myImpl = new XQilla();
}

// Destructor
WfExpressionParser::~WfExpressionParser() {
  if (myImpl) delete myImpl;
}

// Static single instance accessor
WfExpressionParser*
WfExpressionParser::instance() {
  if (myInstance == NULL) {
    myInstance = new WfExpressionParser();
  }
  return myInstance;
}

// Parse method
XQQuery*
WfExpressionParser::parse(const string& queryStr) {
  TRACE_TEXT (TRACE_ALL_STEPS,"Parsing XQuery: " << queryStr << endl);
  return myImpl->parse(X(queryStr.c_str()));
}

/*****************************************************************************/
/*                        WfExpression class                                 */
/*****************************************************************************/

WfExpression::WfExpression()
  : myQuery(NULL) {
  myParser = WfExpressionParser::instance();
  if (myParser == NULL) {
    INTERNAL_ERROR("Condition parser initialization failed",1);
  }
}

WfExpression::~WfExpression() {
  if (myQuery) delete myQuery;
}

void
WfExpression::setExpression(const string& exprStr) {
  myExpression = exprStr;
}

const string&
WfExpression::getExpression() {
  return myExpression;
}

bool
WfExpression::isVariableUsed(const string& varName) {
  string::size_type pos = myExpression.find("$"+varName);
  return (pos != string::npos);
}

void
WfExpression::setVariable(const string& varName,
                         const WfCst::WfDataType varType,
                         const bool isSequence,
                         const string& varValue) {
  if (isVariableUsed(varName)) {
    varInfo_t varInfo;
    varInfo.name = varName;
    // convert type
    string xsType = WfCst::cvtWfToXSType(varType);
    if (xsType.empty())
      xsType = "xs:string";
    if (isSequence)
      xsType += "*";
    // store variable information
    varInfo.xsType = xsType;
    varInfo.value = varValue;
    myVariables.push_back(varInfo);
  }
}

void
WfExpression::setVariableDefaultValue(const string& varName,
                                      const WfCst::WfDataType varType,
                                      const bool isSequence) {
  string defVal = isSequence ? "(\"0\",\"1\")" : "0";
  setVariable(varName, varType, isSequence, defVal);
}

const std::string&
WfExpression::getQueryString() {
  return myQueryStr;
}

void
WfExpression::addVariableDecl(const string& varName,
                             const string& XSType,
                             const string& varValue) {
  string quotes = (XSType == "xs:string") ? "'" : "";
  myQueryStr = "declare variable $" + varName
               + " as " + XSType
               + " := " + quotes + varValue + quotes + "; "
               + myQueryStr;
}

void
WfExpression::initQuery() {
  cout << "Initialize query..." << endl;
  myQueryStr = myExpression;
  for (list<varInfo_t>::iterator varIter = myVariables.begin();
       varIter != myVariables.end();
       ++varIter) {
    addVariableDecl((*varIter).name, (*varIter).xsType, (*varIter).value);
  }
}

void
WfExpression::parseQuery() {
  if (myQuery) delete myQuery;
  myQuery = myParser->parse(myQueryStr);
}

void
WfExpression::evaluate() {
  initQuery();
  parseQuery();
  DynamicContext* dynContext = myQuery->createDynamicContext();
  Result result = myQuery->execute(dynContext);
  Item::Ptr item = result->next(dynContext);
  if (item == NULL) {
    // no result
  }
  myResult = UTF8(item->asString(dynContext));
  delete dynContext;
}

void
WfExpression::reset() {
  myVariables.clear();
  myQueryStr = myExpression;
  myResult.clear();
}


/*****************************************************************************/
/*                    WfBooleanExpression class                              */
/*****************************************************************************/



bool
WfBooleanExpression::testIfTrue() {
  evaluate();

  if (myResult == "true") return true;
  else if (myResult == "false") return false;
  else {
    WARNING("WfBooleanExpression: wrong result! (" << myResult << ")" << endl);
    return false;
  }
}


