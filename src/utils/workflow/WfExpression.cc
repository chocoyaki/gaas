/****************************************************************************/
/* The class used to parse and execute conditional expressions used in      */
/* workflow control structures (If, While, ...)                             */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.7  2011/02/09 14:34:57  bdepardo
 * Use <array><item>0</item></array> instead of <array><item value="0"/></array>
 * in default value.
 *
 * Revision 1.6  2011/02/09 14:27:11  bdepardo
 * Fixed default value when using a container
 *
 * Revision 1.5  2009/08/26 10:32:11  bisnard
 * corrected  warnings
 *
 * Revision 1.4  2009/07/10 12:55:17  bisnard
 * updated list of XQuery separators
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
#include <sstream>

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
/*                        WfExprVariable class                               */
/*****************************************************************************/

WfExprVariable::WfExprVariable(const std::string& varName,
                               const WfCst::WfDataType varType)
  : myName(varName), myType(varType), myValue() {}

void
WfExprVariable::getXQDeclaration(ostream& output) {
  output << "declare variable $" << myName;
  if (myType != WfCst::TYPE_CONTAINER) {

    string xsType = WfCst::cvtWfToXSType(myType);
    string quotes = (xsType == "xs:string") ? "'" : "";
    output << " as " << xsType << " := " << quotes << myValue << quotes << "; " << endl;

  } else {
    // in this case the value is supposed to be XML-encoded
    output << " := document { " << myValue << " }; " << endl;
  }
}

void
WfExprVariable::setValue(const std::string& varValue) {
  myValue = varValue;
}

void
WfExprVariable::setDefaultValue() {
  if (myType != WfCst::TYPE_CONTAINER) {
    myValue =  "0";
  } else {
    myValue = "<array><item>0</item></array>";
  }
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

/**
 * List of all characters than can separate words in XQuery
 * (used to avoid false positive in next method when a variable is prefix of another)
 */
string WfExpression::XQVarSeparators = string(" ,)+*-/");

bool
WfExpression::isVariableUsed(const string& varName) {
  // find occurence of variable (may be positive in case variable is prefix of another)
  string::size_type pos = myExpression.find("$"+varName);
  // find character following the variable (if not at the end)
  string nextChar;
  string::size_type nextPos = pos + varName.length() + 1;
  if (nextPos < myExpression.length())
    nextChar = myExpression.substr(nextPos,1);
  // find occurence of this next character in the separators list
  string::size_type nextCharIsSep = XQVarSeparators.find(nextChar);

  return ((pos != string::npos)
          && (nextChar.empty() || (nextCharIsSep != string::npos)));
}

void
WfExpression::addVariable(WfExprVariable*  var) {
  myVariables.push_back(var);
}

const std::string&
WfExpression::getQueryString() {
  return myQueryStr;
}

void
WfExpression::initQuery() {
  cout << "Initialize query..." << endl;
  ostringstream queryStream;

  for (list<WfExprVariable*>::iterator varIter = myVariables.begin();
       varIter != myVariables.end();
       ++varIter) {
    ((WfExprVariable*) *varIter)->getXQDeclaration(queryStream);
  }
  queryStream << myExpression;
  myQueryStr = queryStream.str();
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
  if (dynContext == NULL) {
    INTERNAL_ERROR("Error in WfExpression::evaluate() : void context",1);
  }
  Result result = myQuery->execute(dynContext);
  if (result.isNull()) {
    INTERNAL_ERROR("Error in WfExpression::evaluate() : void result",1);
  }
  Item::Ptr item = result->next(dynContext);
  if (item != NULL) {
    myResult = UTF8(item->asString(dynContext));
  } else {
    myResult = "";
  }
  delete dynContext;
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


