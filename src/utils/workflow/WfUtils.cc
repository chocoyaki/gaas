/****************************************************************************/
/* Some stuffs used for workflow management                                 */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.18  2010/07/20 09:20:11  bisnard
 * integration with eclipse gui and with dietForwarder
 *
 * Revision 1.17  2010/04/28 14:12:03  bdepardo
 * fscanf warnings correction.
 *
 * Revision 1.16  2009/10/13 14:50:08  bisnard
 * updated gwendia types
 *
 * Revision 1.15  2009/10/02 07:46:15  bisnard
 * conversion of types for gwendia wf language
 *
 * Revision 1.14  2009/07/07 09:05:49  bisnard
 * added some const's
 *
 * Revision 1.13  2009/06/15 12:25:24  bisnard
 * added missing mappings for TYPE_UNKNOWN
 *
 * Revision 1.12  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.11  2009/04/24 11:04:07  bisnard
 * added conversion to XSchema types
 *
 * Revision 1.10  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.9  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.8  2008/12/02 10:05:27  bisnard
 * new conversion method
 *
 * Revision 1.7  2008/10/22 11:01:28  bdepardo
 * missing include
 *
 * Revision 1.6  2008/10/14 13:31:36  bisnard
 * new conversion method
 *
 * Revision 1.5  2008/10/02 07:34:20  bisnard
 * new constants definitions (matrix order)
 *
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
 * Revision 1.3  2007/04/17 13:34:54  ycaniou
 * Error in debug.tex header
 * Removes some warnings during doc generation
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
#include <iostream>
#include <stack>
#include <list>
#include <sstream>
#include <cstdlib>

#include "WfUtils.hh"
#include <map>

using namespace std;

string
WfStructException::ErrorMsg() const {
  string errorMsg;
  switch(Type()) {
    case eUNKNOWN_DAG:
      errorMsg = "Unknown dag (" + Info() + ")"; break;
    case eUNKNOWN_NODE:
      errorMsg = "Unknown node (" + Info() + ")"; break;
    case eUNKNOWN_PORT:
      errorMsg = "Unknown port (" + Info() + ")"; break;
    case eDUPLICATE_NODE:
      errorMsg = "Duplicate node (" + Info() + ")"; break;
    case eDUPLICATE_PORT:
      errorMsg = "Duplicate port (" + Info() + ")"; break;
    case eTYPE_MISMATCH:
      errorMsg = "Type mismatch (" + Info() + ")"; break;
    case eDEPTH_MISMATCH:
      errorMsg = "Depth mismatch (" + Info() + ")"; break;
    case eINVALID_EXPR:
      errorMsg = "Invalid expression (" + Info() + ")"; break;
    case eWF_UNDEF:
      errorMsg = "Workflow not defined (" + Info() + ")"; break;
    case eOTHER:
      errorMsg = Info(); break;
  }
  return errorMsg;
}

/**
 * Conversion for data types
 * WF -> DIET
 * STRING -> WF
 */
string WfCst::dietTypePrefix = "DIET_";

static const pair<short,short> wf2DietTypes[] = {
  pair<short,short>( WfCst::TYPE_CHAR, DIET_CHAR ),
  pair<short,short>( WfCst::TYPE_SHORT, DIET_SHORT),
  pair<short,short>( WfCst::TYPE_INT, DIET_INT),
  pair<short,short>( WfCst::TYPE_LONGINT, DIET_LONGINT),
  pair<short,short>( WfCst::TYPE_FLOAT, DIET_FLOAT),
  pair<short,short>( WfCst::TYPE_DOUBLE, DIET_DOUBLE),
  pair<short,short>( WfCst::TYPE_MATRIX, DIET_MATRIX),
  pair<short,short>( WfCst::TYPE_STRING, DIET_STRING),
  pair<short,short>( WfCst::TYPE_PARAMSTRING, DIET_PARAMSTRING),
  pair<short,short>( WfCst::TYPE_FILE, DIET_FILE),
  pair<short,short>( WfCst::TYPE_CONTAINER, DIET_CONTAINER)
};

static const pair<short,short> diet2WfTypes[] = {
  pair<short,short>( DIET_CHAR, WfCst::TYPE_CHAR),
  pair<short,short>( DIET_SHORT, WfCst::TYPE_SHORT),
  pair<short,short>( DIET_INT, WfCst::TYPE_INT),
  pair<short,short>( DIET_LONGINT, WfCst::TYPE_LONGINT),
  pair<short,short>( DIET_FLOAT, WfCst::TYPE_FLOAT),
  pair<short,short>( DIET_DOUBLE, WfCst::TYPE_DOUBLE),
  pair<short,short>( DIET_MATRIX, WfCst::TYPE_MATRIX),
  pair<short,short>( DIET_STRING, WfCst::TYPE_STRING),
  pair<short,short>( DIET_PARAMSTRING, WfCst::TYPE_PARAMSTRING),
  pair<short,short>( DIET_FILE, WfCst::TYPE_FILE),
  pair<short,short>( DIET_CONTAINER, WfCst::TYPE_CONTAINER)
};

static const pair<string,short> wfTypes[] = {
  pair<string,short>( "", WfCst::TYPE_UNKNOWN),
  pair<string,short>( "DIET_CHAR", WfCst::TYPE_CHAR),
  pair<string,short>( "DIET_SHORT", WfCst::TYPE_SHORT),
  pair<string,short>( "DIET_INT", WfCst::TYPE_INT),
  pair<string,short>( "DIET_LONGINT", WfCst::TYPE_LONGINT),
  pair<string,short>( "DIET_FLOAT", WfCst::TYPE_FLOAT),
  pair<string,short>( "DIET_DOUBLE", WfCst::TYPE_DOUBLE),
  pair<string,short>( "DIET_MATRIX", WfCst::TYPE_MATRIX),
  pair<string,short>( "DIET_STRING", WfCst::TYPE_STRING),
  pair<string,short>( "DIET_PARAMSTRING", WfCst::TYPE_PARAMSTRING),
  pair<string,short>( "DIET_FILE", WfCst::TYPE_FILE),
  pair<string,short>( "DIET_CONTAINER", WfCst::TYPE_CONTAINER)
};

static const pair<short, const string> strTypes[] = {
  pair<short, const string>( WfCst::TYPE_CHAR, "DIET_CHAR"),
  pair<short, const string>( WfCst::TYPE_SHORT, "DIET_SHORT"),
  pair<short, const string>( WfCst::TYPE_INT, "DIET_INT"),
  pair<short, const string>( WfCst::TYPE_LONGINT, "DIET_LONGINT"),
  pair<short, const string>( WfCst::TYPE_FLOAT, "DIET_FLOAT"),
  pair<short, const string>( WfCst::TYPE_DOUBLE, "DIET_DOUBLE"),
  pair<short, const string>( WfCst::TYPE_MATRIX, "DIET_MATRIX"),
  pair<short, const string>( WfCst::TYPE_STRING, "DIET_STRING"),
  pair<short, const string>( WfCst::TYPE_PARAMSTRING, "DIET_PARAMSTRING"),
  pair<short, const string>( WfCst::TYPE_FILE, "DIET_FILE"),
  pair<short, const string>( WfCst::TYPE_CONTAINER, "DIET_CONTAINER"),
  pair<short, const string>( WfCst::TYPE_UNKNOWN,"_UNKNOWN_")
};

static const pair<short, const string> XSTypes[] = {
  pair<short, const string>( WfCst::TYPE_CHAR, "xs:byte"),
  pair<short, const string>( WfCst::TYPE_SHORT, "xs:short"),
  pair<short, const string>( WfCst::TYPE_INT, "xs:integer"),
  pair<short, const string>( WfCst::TYPE_LONGINT, "xs:long"),
  pair<short, const string>( WfCst::TYPE_FLOAT, "xs:float"),
  pair<short, const string>( WfCst::TYPE_DOUBLE, "xs:double"),
  pair<short, const string>( WfCst::TYPE_MATRIX, ""), // not defined
  pair<short, const string>( WfCst::TYPE_STRING, "xs:string"),
  pair<short, const string>( WfCst::TYPE_PARAMSTRING, "xs:string"),
  pair<short, const string>( WfCst::TYPE_FILE, ""),
  pair<short, const string>( WfCst::TYPE_CONTAINER, ""),
  pair<short, const string>( WfCst::TYPE_UNKNOWN,"")
};

static const pair<const string, const string> gw2dietTypes[] = {
  pair<const string, const string>( "", ""),
  pair<const string, const string>( "string", "DIET_STRING"),
  pair<const string, const string>( "file", "DIET_FILE"),
  pair<const string, const string>( "integer", "DIET_INT"),
  pair<const string, const string>( "short", "DIET_SHORT"),
  pair<const string, const string>( "double", "DIET_DOUBLE"),
  pair<const string, const string>( "longint", "DIET_LONGINT"),
  pair<const string, const string>( "boolean", "DIET_CHAR"),
  pair<const string, const string>( "float", "DIET_FLOAT"),
};

static map<short,short> WfTypesToDietTypes(wf2DietTypes, wf2DietTypes
    + sizeof(wf2DietTypes)/sizeof(wf2DietTypes[0]));
static map<short,short> DietTypesToWfTypes(diet2WfTypes, diet2WfTypes
    + sizeof(diet2WfTypes)/sizeof(diet2WfTypes[0]));
static map<string,short> StrTypesToWfTypes(wfTypes, wfTypes
    + sizeof(wfTypes)/sizeof(wfTypes[0]));
static map<short, const string> WfTypesToStrTypes(strTypes, strTypes
    + sizeof(strTypes)/sizeof(strTypes[0]));
static map<short, const string> WfTypesToXSTypes(XSTypes, XSTypes
    + sizeof(XSTypes)/sizeof(XSTypes[0]));
static map<const string, const string> GwendiaToDietTypes(gw2dietTypes,
    gw2dietTypes + sizeof(gw2dietTypes)/sizeof(gw2dietTypes[0]));

short
WfCst::cvtWfToDietType(WfDataType wfType) {
  return WfTypesToDietTypes[wfType];
}

short
WfCst::cvtDietToWfType(short dietType) {
  return DietTypesToWfTypes[dietType];
}

short
WfCst::cvtStrToWfType(const string& strType) {
  if (strType.substr(0,5) == dietTypePrefix)
    return StrTypesToWfTypes[strType];
  else
    return StrTypesToWfTypes[cvtGwendiaToDietType(strType)];
}

const string&
WfCst::cvtWfToStrType(WfDataType wfType) {
  return WfTypesToStrTypes[wfType];
}

const string&
WfCst::cvtWfToXSType(WfDataType wfType) {
  return WfTypesToXSTypes[wfType];
}

const string&
WfCst::cvtGwendiaToDietType(const string& gwType) {
  return GwendiaToDietTypes[gwType];
}

bool
WfCst::isMatrixType(const string& strType) {
  return (strType == "DIET_MATRIX");
}

bool
WfCst::isMatrixType(const WfDataType type) {
  return (type == TYPE_MATRIX);
}

/**
 * Conversion for matrix orders
 * WF -> DIET
 * STRING -> WF
 */

static const pair<short,short> dietMatrixOrders[] = {
  pair<short,short>( WfCst::ORDER_COL_MAJOR, DIET_COL_MAJOR ),
  pair<short,short>( WfCst::ORDER_ROW_MAJOR, DIET_ROW_MAJOR )
};

static const pair<string,short> wfMatrixOrders[] = {
  pair<string,short>( "DIET_COL_MAJOR", WfCst::ORDER_COL_MAJOR),
  pair<string,short>( "DIET_ROW_MAJOR", WfCst::ORDER_ROW_MAJOR)
};

static const pair<short, const string> strMatrixOrders[] = {
  pair<short, const string>( WfCst::ORDER_COL_MAJOR, "DIET_COL_MAJOR"),
  pair<short, const string>( WfCst::ORDER_ROW_MAJOR, "DIET_ROW_MAJOR")
};

static map<short,short> WfToDietMatrixOrders(dietMatrixOrders, dietMatrixOrders
    + sizeof(dietMatrixOrders)/sizeof(dietMatrixOrders[0]));
static map<string,short> StrToWfMatrixOrders(wfMatrixOrders, wfMatrixOrders
    + sizeof(wfMatrixOrders)/sizeof(wfMatrixOrders[0]));
static map<short,const string> WfToStrMatrixOrders(strMatrixOrders, strMatrixOrders
    + sizeof(strMatrixOrders)/sizeof(strMatrixOrders[0]));

short
WfCst::cvtWfToDietMatrixOrder(WfMatrixOrder wfMatrixOrder) {
  return WfToDietMatrixOrders[wfMatrixOrder];
}

short
WfCst::cvtStrToWfMatrixOrder(const std::string& strMatrixOrder) {
  return StrToWfMatrixOrders[strMatrixOrder];
}

const string&
WfCst::cvtWfToStrMatrixOrder(WfMatrixOrder wfMatrixOrder) {
  return WfToStrMatrixOrders[wfMatrixOrder];
}

/**
 * Matrix read
 */

void
WfCst::open_file(const char * fileName, FILE *& myFile) {
  myFile = fopen(fileName, "r");
  if (!myFile) {
    cerr << "FATAL ERROR" << endl << "Data file " << fileName <<
      " not found" << endl;
    exit(1);
  }
}

unsigned long
WfCst::readChar(const char * fileName, char * mat, unsigned long mat_size) {
  FILE * myFile;
  open_file(fileName, myFile);
  unsigned long p = 0;
  char c;
  int res;
  res = fscanf(myFile, "%c", &c);
  while (res && !feof(myFile) && (p<mat_size)) {
    if ((c != '\n') && (c != '\r') && (c!=' '))
      mat[p++] = c;
    res = fscanf(myFile, "%c", &c);
  }
  //  rewind(myFile);
  fclose(myFile);
  return p;
}
unsigned long
WfCst::readShort(const char * fileName, short * mat, unsigned long mat_size) {
  FILE * myFile;
  open_file(fileName, myFile);
  unsigned long p = 0;
  short i;
  int res;
  res = fscanf(myFile, "%hd", &i);
  while (res && !feof(myFile) && (p<mat_size)) {
    mat[p++] = i;
    res = fscanf(myFile, "%hd", &i);
  }
  //  rewind(myFile);
  fclose(myFile);
  return p;
}
unsigned long
WfCst::readInt(const char * fileName, int * mat, unsigned long mat_size) {
  FILE * myFile;
  open_file(fileName, myFile);
  unsigned long p = 0;
  int i;
  int res;
  res = fscanf(myFile, "%d", &i);
  while (res && !feof(myFile) && (p<mat_size)) {
    mat[p++] = i;
    res = fscanf(myFile, "%d", &i);
  }
  //  rewind(myFile);
  fclose(myFile);
  return p;
}
unsigned long
WfCst::readLong(const char * fileName, long * mat, unsigned long mat_size) {
  FILE * myFile;
  open_file(fileName, myFile);
  unsigned long p = 0;
  long l;
  int res;
  res = fscanf(myFile, "%ld", &l);
  while (res && !feof(myFile)&& (p<mat_size)) {
    mat[p++] = l;
    res = fscanf(myFile, "%ld", &l);
  }
  //  rewind(myFile);
  fclose(myFile);
  return p;
}

unsigned long
WfCst::readFloat(const char * fileName, float * mat, unsigned long mat_size) {
  FILE * myFile;
  open_file(fileName, myFile);

  unsigned long p = 0;
  float f;
  int res;
  res = fscanf(myFile, "%f", &f);
  while (res && !feof(myFile)&& (p<mat_size)) {
    mat[p++] = f;
    res = fscanf(myFile, "%f", &f);
  }

  fclose(myFile);
  return p;
}

unsigned long
WfCst::readDouble(const char * fileName, double * mat, unsigned long mat_size) {
  FILE * myFile;
  open_file(fileName, myFile);

  unsigned long p = 0;
  double d;
  int res;
  res = fscanf(myFile, "%lf", &d);
  while (res && !feof(myFile)&& (p<mat_size)) {
    mat[p++] = d;
    res = fscanf(myFile, "%lf", &d);
  }

  fclose(myFile);
  return p;
}


long
WfCst::eval_expr(std::string& expr, int var) {
  map<string, int> op_priority;
  op_priority["+"] = 0;
  op_priority["-"] = 0;
  op_priority["*"] = 1;
  op_priority["/"] = 1;
  op_priority["%"] = 1;
  stack<string> tokens;
  list<string> post_fixe_exp;
  stack<string>  polish;
  string tok;
  string op;
  long total = 0;
  string::size_type p = string::npos;
  // remove blanks
  while (expr.find(" ") != string::npos)
    expr.erase(expr.find(" "), 1);
  // replace the unary - by a #
  if (expr[0] == '-') expr[0]='#';
  for (unsigned int i=1; i<expr.size(); i++)
    if ( (expr[i] == '-') && (!isdigit(expr[i-1])) ) expr[i]='#';

  //  cout << expr << endl;

  // read the expression
  while (expr.size() > 0) {
    if (isdigit(expr[0]) || isalpha(expr[0]) || (expr[0]=='#')) {
      p = expr.find_first_of("+-/*%()");
      tok = expr.substr(0, p);
      //	cout << "token = " << tok << endl;
      //	cout << "|" << tok << "|";
      post_fixe_exp.push_back(tok);
      if (p!= string::npos)
	expr = expr.substr(p);
      else
	expr = "";
    }
    else {
      //	cout << "not a value" << endl;
      if (expr[0] == '(') {
	tokens.push("(");
      }
      else {
	if (expr[0] == ')') {
	  while (tokens.top() != "(") {
	    //	      cout << tokens.top();
	    post_fixe_exp.push_back(tokens.top());
	    tokens.pop();
	  }
	  tokens.pop();
	} // if '('
	else {
	  // it is an op
	  op = expr.substr(0,1);
	  while ((!tokens.empty())&&
		 (op_priority[tokens.top()] > op_priority[op])
		 ) {
	    //	      cout << tokens.top();
	    post_fixe_exp.push_back(tokens.top());
	    tokens.pop();
	  } // end while
	  tokens.push(op);
	}
      }
      expr = expr.substr(1);
    }
  } // end while
  while (!tokens.empty()) {
    //      cout << tokens.top();
    post_fixe_exp.push_back(tokens.top());
    tokens.pop();
  }

  while (!post_fixe_exp.empty()) {
    //    cout << "|" << post_fixe_exp.front() <<"|";
    tok = post_fixe_exp.front();
    post_fixe_exp.pop_front();
    if (tok == "+" || tok == "-" || tok == "*" || tok == "/" || tok == "%") {
      // pop the first value
      string v1s = polish.top().c_str();
      long v1 = 1;
      if (v1s[0] == '#') {
	v1 = -1;
	v1s = v1s.substr(1);
      }
      v1 = v1*atoi(v1s.c_str());
      polish.pop();
      // pop the second value
      string v2s = polish.top().c_str();
      long v2 = 1;
      if (v2s[0] == '#') {
	v2 = -1;
	v2s = v2s.substr(1);
      }
      v2 = v2*atoi(v2s.c_str());
      polish.pop();
      //***
      stringstream ss;
      if (tok == "+") {
	ss << (v1+v2);
      }
      if (tok == "-") {
	ss << (v1-v2);
      }
      if (tok == "*") {
	ss << (v1*v2);
      }
      if (tok == "/") {
	ss << (v2/v1);
      }
      if (tok == "%") {
	ss << (v2%v1);
      }
      polish.push(ss.str());
    }
    else {
      polish.push(tok);
    }
  }

  //  cout<< endl;
  //  cout << "Final result " << polish.top() << endl;
  total = atoi(polish.top().c_str());
  return total;
}

/**
 * return a list of token composing a string
 * used to read the matrix value
 */
vector<string>
getStringToken(string str) {
  vector<string> v ;
  string str2(str);
  bool b = true;
  while (b) {
    v.push_back(str.substr(0, str.find(" ")));
    // read the immediate following blanks
    int ix = str.find(" ")+1;
    while (str[ix] == ' ')
      ix++;

    // remove the read token
    str2 =  str.substr(ix);
    if (str2 == str)
      b = false;
    else
      str = str2;
  }
  return v;
} // end getStringToken

/**
 * return a string representation on an integer
 */
string
itoa(long l) {
  // stringstream seems to be not thread safe !!!
  /*
  stringstream ss;
  ss << l;
  return ss.str();
  */
  char str[128];
  sprintf(str, "%ld", l);
  return string(str);
}
