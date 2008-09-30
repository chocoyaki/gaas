/****************************************************************************/
/* Some stuffs used for workflow management                                 */
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

#include "WfUtils.hh"
#include <map>

using namespace std;


static const pair<short,short> dietTypes[] = {
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

static const pair<string,short> wfTypes[] = {
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

static map<short,short> WfTypesToDietTypes(dietTypes, dietTypes + sizeof(dietTypes)/sizeof(dietTypes[0]));
static map<string,short> StrTypesToWfTypes(wfTypes, wfTypes + sizeof(wfTypes)/sizeof(wfTypes[0]));

short
WfCst::cvtWfToDietType(WfDataType wfType) {
  return WfTypesToDietTypes[wfType];
}

short
WfCst::cvtStrToWfType(const string& strType) {
  return StrTypesToWfTypes[strType];
}

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
  fscanf(myFile, "%c", &c);
  while (!feof(myFile) && (p<mat_size)) {
    if ((c != '\n') && (c != '\r') && (c!=' '))
      mat[p++] = c;
    fscanf(myFile, "%c", &c);
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
  fscanf(myFile, "%hd", &i);
  while (!feof(myFile) && (p<mat_size)) {
    mat[p++] = i;
    fscanf(myFile, "%hd", &i);
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
  fscanf(myFile, "%d", &i);
  while (!feof(myFile) && (p<mat_size)) {
    mat[p++] = i;
    fscanf(myFile, "%d", &i);
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
  fscanf(myFile, "%ld", &l);
  while (!feof(myFile)&& (p<mat_size)) {
    mat[p++] = l;
    fscanf(myFile, "%ld", &l);
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
  fscanf(myFile, "%f", &f);
  while (!feof(myFile)&& (p<mat_size)) {
    mat[p++] = f;
    fscanf(myFile, "%f", &f);
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
  fscanf(myFile, "%lf", &d);
  while (!feof(myFile)&& (p<mat_size)) {
    mat[p++] = d;
    fscanf(myFile, "%lf", &d);
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
 * get the diet base type by a string
 */
diet_base_type_t
getBaseType(const std::string base_type) {
  static std::map<std::string, diet_base_type_t> baseTypeMap;
  baseTypeMap["DIET_CHAR"] = DIET_CHAR;
  baseTypeMap["DIET_SHORT"] = DIET_SHORT;
  baseTypeMap["DIET_INT"] = DIET_INT;
  baseTypeMap["DIET_LONGINT"] = DIET_LONGINT;
  baseTypeMap["DIET_FLOAT"] = DIET_FLOAT;
  baseTypeMap["DIET_DOUBLE"] = DIET_DOUBLE;
  baseTypeMap["DIET_SCOMPLEX"] = DIET_SCOMPLEX;
  baseTypeMap["DIET_DCOMPLEX"] = DIET_DCOMPLEX;
  baseTypeMap["DIET_BASE_TYPE_COUNT"] = DIET_BASE_TYPE_COUNT;

  map<string, diet_base_type_t>::iterator p =
    baseTypeMap.find(base_type);

  if (p == baseTypeMap.end()) {
    // TO FIX
    cerr << "Unknown base type " << endl;
    return ((diet_base_type_t)(-1));
  }
  else
    return (diet_base_type_t)(p->second);
}

/**
 * get the string representation of diet base type
 */
string
getBaseTypeStr(const diet_base_type_t base_type) {
  static std::map<diet_base_type_t, std::string> baseTypeMapStr;
  baseTypeMapStr[DIET_CHAR] = "DIET_CHAR";
  baseTypeMapStr[DIET_SHORT] = "DIET_SHORT";
  baseTypeMapStr[DIET_INT] = "DIET_INT";
  baseTypeMapStr[DIET_LONGINT] = "DIET_LONGINT";
  baseTypeMapStr[DIET_FLOAT] = "DIET_FLOAT";
  baseTypeMapStr[DIET_DOUBLE] = "DIET_DOUBLE";
  baseTypeMapStr[DIET_SCOMPLEX] = "DIET_SCOMPLEX";
  baseTypeMapStr[DIET_DCOMPLEX] = "DIET_DCOMPLEX";
  baseTypeMapStr[DIET_BASE_TYPE_COUNT] = "DIET_BASE_TYPE_COUNT";

  map<diet_base_type_t, string>::iterator p =
    baseTypeMapStr.find(base_type);

  if (p == baseTypeMapStr.end()) {
    // TO FIX
    cerr << "Unknown base type " << endl;
    return "UNKNOWN";
  }
  else
    return (string)(p->second);

}
/**
 * get the matrix order by a string
 */
diet_matrix_order_t
getMatrixOrder(const std::string matrix_order) {
  static std::map<std::string, diet_matrix_order_t> matrixOrderMap;
  matrixOrderMap["DIET_COL_MAJOR"] = DIET_COL_MAJOR;
  matrixOrderMap["DIET_ROW_MAJOR"] = DIET_ROW_MAJOR;
  matrixOrderMap["DIET_MATRIX_ORDER_COUNT"] = DIET_MATRIX_ORDER_COUNT;

  map<string, diet_matrix_order_t>::iterator p =
    matrixOrderMap.find(matrix_order);

  if (p == matrixOrderMap.end()) {
    // TO FIX
    cerr << "Unknown matrix order " << endl;
    return ((diet_matrix_order_t)(-1));
  }
  else
    return (diet_matrix_order_t)(p->second);

}

/**
 * get the string associated to a matrix order
 */
std::string
getMatrixOrderStr(const diet_matrix_order_t matrix_order) {
  static std::map<diet_matrix_order_t, std::string> matrixOrderMapStr;
  matrixOrderMapStr[DIET_COL_MAJOR] = "DIET_COL_MAJOR";
  matrixOrderMapStr[DIET_ROW_MAJOR] = "DIET_ROW_MAJOR";
  matrixOrderMapStr[DIET_MATRIX_ORDER_COUNT] = "DIET_MATRIX_ORDER_COUNT";

  map<diet_matrix_order_t, string>::iterator p =
    matrixOrderMapStr.find(matrix_order);

  if (p == matrixOrderMapStr.end()) {
    // TO FIX
    cerr << "Unknown matrix order " << endl;
    return "UNKNOWN";
  }
  else
    return (string)(p->second);

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
