/**
* @file  WfExpression.cc
* 
* @brief  The class used to parse and execute conditional expressions used in workflow control structures
* 
* @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/

#include <sstream>

#include "WfExpression.hh"
#include "debug.hh"
#include "DagWfParser.hh"


/*****************************************************************************
 *                     WfExpressionParser class                               *
 *****************************************************************************/
// Initialisation of static members
WfExpressionParser* WfExpressionParser::myInstance = NULL;

// Private constructor
WfExpressionParser::WfExpressionParser() {
  TRACE_TEXT(TRACE_ALL_STEPS, "Initializing XQilla...\n");
  myImpl = new XQilla();
}

// Destructor
WfExpressionParser::~WfExpressionParser() {
  delete myImpl;
}

// Static single instance accessor
WfExpressionParser*
WfExpressionParser::instance() {
  if (!myInstance) {
    myInstance = new WfExpressionParser();
  }
  return myInstance;
}

// Parse method
XQQuery*
WfExpressionParser::parse(const std::string& queryStr) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Parsing XQuery: " << queryStr << "\n");
  return myImpl->parse(X(queryStr.c_str()));
}

/*****************************************************************************/
/*                        WfExprVariable class                               */
/*****************************************************************************/

WfExprVariable::WfExprVariable(const std::string& varName,
                               const WfCst::WfDataType varType)
  : myName(varName), myType(varType), myValue() {}

void
WfExprVariable::getXQDeclaration(std::ostream& output) {
  output << "declare variable $" << myName;
  if (myType != WfCst::TYPE_CONTAINER) {
    std::string xsType = WfCst::cvtWfToXSType(myType);
    std::string quotes = (xsType == "xs:string") ? "'" : "";
    output << " as " << xsType
           << " := " << xsType << "(" << quotes << myValue << quotes
           << "); \n";
  } else {
    // in this case the value is supposed to be XML-encoded
    output << " := document { " << myValue << " }\n";
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
  if (!myParser) {
    INTERNAL_ERROR("Condition parser initialization failed", 1);
  }
}

WfExpression::~WfExpression() {
  delete myQuery;
}

void
WfExpression::setExpression(const std::string& exprStr) {
  myExpression = exprStr;
}

const std::string&
WfExpression::getExpression() {
  return myExpression;
}

/**
 * List of all characters than can separate words in XQuery
 * (used to avoid false positive in next method when a variable
 * is prefix of another)
 */
std::string WfExpression::XQVarSeparators = std::string(" ,)+*-/");

bool
WfExpression::isVariableUsed(const std::string& varName) {
  // find occurence of variable
  // (may be positive in case variable is prefix of another)
  std::string::size_type pos = myExpression.find("$"+varName);
  // find character following the variable (if not at the end)
  std::string nextChar;
  std::string::size_type nextPos = pos + varName.length() + 1;
  if (nextPos < myExpression.length()) {
    nextChar = myExpression.substr(nextPos, 1);
  }
  // find occurence of this next character in the separators list
  std::string::size_type nextCharIsSep = XQVarSeparators.find(nextChar);

  return ((pos != std::string::npos)
          && (nextChar.empty() || (nextCharIsSep != std::string::npos)));
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
  TRACE_TEXT(TRACE_MAIN_STEPS, "Initialize query...\n");
  std::ostringstream queryStream;

  std::list<WfExprVariable*>::iterator varIter = myVariables.begin();
  for (; varIter != myVariables.end(); ++varIter) {
    ((WfExprVariable*) *varIter)->getXQDeclaration(queryStream);
  }
  queryStream << myExpression;
  myQueryStr = queryStream.str();
}

void
WfExpression::parseQuery() {
  delete myQuery;
  myQuery = myParser->parse(myQueryStr);
}

void
WfExpression::evaluate() {
  initQuery();
  parseQuery();
  DynamicContext* dynContext = myQuery->createDynamicContext();
  if (!dynContext) {
    INTERNAL_ERROR("Error in WfExpression::evaluate() : void context", 1);
  }
  Result result = myQuery->execute(dynContext);
  if (result.isNull()) {
    INTERNAL_ERROR("Error in WfExpression::evaluate() : void result", 1);
  }
  Item::Ptr item = result->next(dynContext);
  if (item) {
    myResult = UTF8(item->asString(dynContext));
  } else {
    myResult = "";
  }
  delete dynContext;
}

/*****************************************************************************
 *                    WfBooleanExpression class                              *
 *****************************************************************************/
bool
WfBooleanExpression::testIfTrue() {
  evaluate();

  if (myResult == "true") {
    return true;
  } else if (myResult == "false") {
    return false;
  } else {
    WARNING("WfBooleanExpression: wrong result! (" << myResult << ")\n");
    return false;
  }
}
