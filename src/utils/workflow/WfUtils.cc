/**
* @file  WfUtils.cc
* 
* @brief  Some stuffs used for workflow management 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/

#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <stack>
#include <sstream>
#include "WfUtils.hh"

using std::pair;

std::string
WfStructException::ErrorMsg() const {
  std::string errorMsg;
  switch (Type()) {
  case eUNKNOWN_DAG:
    errorMsg = "Unknown dag (" + Info() + ")";
    break;
  case eUNKNOWN_NODE:
    errorMsg = "Unknown node (" + Info() + ")";
    break;
  case eUNKNOWN_PORT:
    errorMsg = "Unknown port (" + Info() + ")";
    break;
  case eDUPLICATE_NODE:
    errorMsg = "Duplicate node (" + Info() + ")";
    break;
  case eDUPLICATE_PORT:
    errorMsg = "Duplicate port (" + Info() + ")";
    break;
  case eTYPE_MISMATCH:
    errorMsg = "Type mismatch (" + Info() + ")";
    break;
  case eDEPTH_MISMATCH:
    errorMsg = "Depth mismatch (" + Info() + ")";
    break;
  case eINVALID_EXPR:
    errorMsg = "Invalid expression (" + Info() + ")";
    break;
  case eWF_UNDEF:
    errorMsg = "Workflow not defined (" + Info() + ")";
    break;
  case eOTHER:
    errorMsg = Info();
    break;
  }
  return errorMsg;
}

/**
 * Conversion for data types
 * WF -> DIET
 * STRING -> WF
 */
std::string WfCst::dietTypePrefix = "DIET_";

static const pair<short, short> wf2DietTypes[] = {
  pair<short, short>(WfCst::TYPE_CHAR, DIET_CHAR),
  pair<short, short>(WfCst::TYPE_SHORT, DIET_SHORT),
  pair<short, short>(WfCst::TYPE_INT, DIET_INT),
  pair<short, short>(WfCst::TYPE_LONGINT, DIET_LONGINT),
  pair<short, short>(WfCst::TYPE_FLOAT, DIET_FLOAT),
  pair<short, short>(WfCst::TYPE_DOUBLE, DIET_DOUBLE),
  pair<short, short>(WfCst::TYPE_MATRIX, DIET_MATRIX),
  pair<short, short>(WfCst::TYPE_STRING, DIET_STRING),
  pair<short, short>(WfCst::TYPE_PARAMSTRING, DIET_PARAMSTRING),
  pair<short, short>(WfCst::TYPE_FILE, DIET_FILE),
  pair<short, short>(WfCst::TYPE_CONTAINER, DIET_CONTAINER)
};

static const pair<short, short> diet2WfTypes[] = {
  pair<short, short>(DIET_CHAR, WfCst::TYPE_CHAR),
  pair<short, short>(DIET_SHORT, WfCst::TYPE_SHORT),
  pair<short, short>(DIET_INT, WfCst::TYPE_INT),
  pair<short, short>(DIET_LONGINT, WfCst::TYPE_LONGINT),
  pair<short, short>(DIET_FLOAT, WfCst::TYPE_FLOAT),
  pair<short, short>(DIET_DOUBLE, WfCst::TYPE_DOUBLE),
  pair<short, short>(DIET_MATRIX, WfCst::TYPE_MATRIX),
  pair<short, short>(DIET_STRING, WfCst::TYPE_STRING),
  pair<short, short>(DIET_PARAMSTRING, WfCst::TYPE_PARAMSTRING),
  pair<short, short>(DIET_FILE, WfCst::TYPE_FILE),
  pair<short, short>(DIET_CONTAINER, WfCst::TYPE_CONTAINER)
};

static const pair<std::string, short> wfTypes[] = {
  pair<std::string, short>("", WfCst::TYPE_UNKNOWN),
  pair<std::string, short>("DIET_CHAR", WfCst::TYPE_CHAR),
  pair<std::string, short>("DIET_SHORT", WfCst::TYPE_SHORT),
  pair<std::string, short>("DIET_INT", WfCst::TYPE_INT),
  pair<std::string, short>("DIET_LONGINT", WfCst::TYPE_LONGINT),
  pair<std::string, short>("DIET_FLOAT", WfCst::TYPE_FLOAT),
  pair<std::string, short>("DIET_DOUBLE", WfCst::TYPE_DOUBLE),
  pair<std::string, short>("DIET_MATRIX", WfCst::TYPE_MATRIX),
  pair<std::string, short>("DIET_STRING", WfCst::TYPE_STRING),
  pair<std::string, short>("DIET_PARAMSTRING", WfCst::TYPE_PARAMSTRING),
  pair<std::string, short>("DIET_FILE", WfCst::TYPE_FILE),
  pair<std::string, short>("DIET_CONTAINER", WfCst::TYPE_CONTAINER)
};

static const pair<short, const std::string> strTypes[] = {
  pair<short, const std::string>(WfCst::TYPE_CHAR, "DIET_CHAR"),
  pair<short, const std::string>(WfCst::TYPE_SHORT, "DIET_SHORT"),
  pair<short, const std::string>(WfCst::TYPE_INT, "DIET_INT"),
  pair<short, const std::string>(WfCst::TYPE_LONGINT, "DIET_LONGINT"),
  pair<short, const std::string>(WfCst::TYPE_FLOAT, "DIET_FLOAT"),
  pair<short, const std::string>(WfCst::TYPE_DOUBLE, "DIET_DOUBLE"),
  pair<short, const std::string>(WfCst::TYPE_MATRIX, "DIET_MATRIX"),
  pair<short, const std::string>(WfCst::TYPE_STRING, "DIET_STRING"),
  pair<short, const std::string>(WfCst::TYPE_PARAMSTRING, "DIET_PARAMSTRING"),
  pair<short, const std::string>(WfCst::TYPE_FILE, "DIET_FILE"),
  pair<short, const std::string>(WfCst::TYPE_CONTAINER, "DIET_CONTAINER"),
  pair<short, const std::string>(WfCst::TYPE_UNKNOWN, "_UNKNOWN_")
};

static const pair<short, const std::string> XSTypes[] = {
  pair<short, const std::string>(WfCst::TYPE_CHAR, "xs:byte"),
  pair<short, const std::string>(WfCst::TYPE_SHORT, "xs:short"),
  pair<short, const std::string>(WfCst::TYPE_INT, "xs:integer"),
  pair<short, const std::string>(WfCst::TYPE_LONGINT, "xs:long"),
  pair<short, const std::string>(WfCst::TYPE_FLOAT, "xs:float"),
  pair<short, const std::string>(WfCst::TYPE_DOUBLE, "xs:double"),
  pair<short, const std::string>(WfCst::TYPE_MATRIX, ""),  // not defined
  pair<short, const std::string>(WfCst::TYPE_STRING, "xs:string"),
  pair<short, const std::string>(WfCst::TYPE_PARAMSTRING, "xs:string"),
  pair<short, const std::string>(WfCst::TYPE_FILE, ""),
  pair<short, const std::string>(WfCst::TYPE_CONTAINER, ""),
  pair<short, const std::string>(WfCst::TYPE_UNKNOWN, "")
};

static const pair<const std::string, const std::string> gw2dietTypes[] = {
  pair<const std::string, const std::string>("", ""),
  pair<const std::string, const std::string>("string", "DIET_STRING"),
  pair<const std::string, const std::string>("file", "DIET_FILE"),
  pair<const std::string, const std::string>("integer", "DIET_INT"),
  pair<const std::string, const std::string>("short", "DIET_SHORT"),
  pair<const std::string, const std::string>("double", "DIET_DOUBLE"),
  pair<const std::string, const std::string>("longint", "DIET_LONGINT"),
  pair<const std::string, const std::string>("boolean", "DIET_CHAR"),
  pair<const std::string, const std::string>("float", "DIET_FLOAT"),
};

static std::map<short, short>
WfTypesToDietTypes(wf2DietTypes, wf2DietTypes
                   + sizeof(wf2DietTypes)/sizeof(wf2DietTypes[0]));

static std::map<short, short>
DietTypesToWfTypes(diet2WfTypes, diet2WfTypes
                   + sizeof(diet2WfTypes)/sizeof(diet2WfTypes[0]));

static std::map<std::string, short>
StrTypesToWfTypes(wfTypes, wfTypes
                  + sizeof(wfTypes)/sizeof(wfTypes[0]));

static std::map<short, const std::string>
WfTypesToStrTypes(strTypes, strTypes
                  + sizeof(strTypes)/sizeof(strTypes[0]));

static std::map<short, const std::string>
WfTypesToXSTypes(XSTypes, XSTypes
                 + sizeof(XSTypes)/sizeof(XSTypes[0]));

// hack due o AIX shitty STL implementation
#ifdef __aix__
static std::map<std::string, std::string>
#else
static std::map<const std::string, const std::string>
#endif
GwendiaToDietTypes(gw2dietTypes,
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
WfCst::cvtStrToWfType(const std::string& strType) {
  if (strType.substr(0, 5) == dietTypePrefix) {
    return StrTypesToWfTypes[strType];
  } else {
    return StrTypesToWfTypes[cvtGwendiaToDietType(strType)];
  }
}

const std::string&
WfCst::cvtWfToStrType(WfDataType wfType) {
  return WfTypesToStrTypes[wfType];
}

const std::string&
WfCst::cvtWfToXSType(WfDataType wfType) {
  return WfTypesToXSTypes[wfType];
}

const std::string&
WfCst::cvtGwendiaToDietType(const std::string& gwType) {
  return GwendiaToDietTypes[gwType];
}

bool
WfCst::isMatrixType(const std::string& strType) {
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

static const pair<short, short> dietMatrixOrders[] = {
  pair<short, short>(WfCst::ORDER_COL_MAJOR, DIET_COL_MAJOR),
  pair<short, short>(WfCst::ORDER_ROW_MAJOR, DIET_ROW_MAJOR)
};

static const pair<std::string, short> wfMatrixOrders[] = {
  pair<std::string, short>("DIET_COL_MAJOR", WfCst::ORDER_COL_MAJOR),
  pair<std::string, short>("DIET_ROW_MAJOR", WfCst::ORDER_ROW_MAJOR)
};

static const pair<short, const std::string> strMatrixOrders[] = {
  pair<short, const std::string>(WfCst::ORDER_COL_MAJOR, "DIET_COL_MAJOR"),
  pair<short, const std::string>(WfCst::ORDER_ROW_MAJOR, "DIET_ROW_MAJOR")
};

static std::map<short, short>
WfToDietMatrixOrders(dietMatrixOrders, dietMatrixOrders
                     + sizeof(dietMatrixOrders)/sizeof(dietMatrixOrders[0]));

static std::map<std::string, short>
StrToWfMatrixOrders(wfMatrixOrders, wfMatrixOrders
                    + sizeof(wfMatrixOrders)/sizeof(wfMatrixOrders[0]));

static std::map<short, const std::string>
WfToStrMatrixOrders(strMatrixOrders, strMatrixOrders
                    + sizeof(strMatrixOrders)/sizeof(strMatrixOrders[0]));

short
WfCst::cvtWfToDietMatrixOrder(WfMatrixOrder wfMatrixOrder) {
  return WfToDietMatrixOrders[wfMatrixOrder];
}

short
WfCst::cvtStrToWfMatrixOrder(const std::string& strMatrixOrder) {
  return StrToWfMatrixOrders[strMatrixOrder];
}

const std::string&
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
    std::cerr << "FATAL ERROR\nData file " << fileName
              << " not found\n";
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
  while (res && !feof(myFile) && (p < mat_size)) {
    if ((c != '\n') && (c != '\r') && (c != ' ')) {
      mat[p++] = c;
    }
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
  res = fscanf(myFile, "%5hd", &i);
  while (res && !feof(myFile) && (p < mat_size)) {
    mat[p++] = i;
    res = fscanf(myFile, "%5hd", &i);
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
  res = fscanf(myFile, "%20d", &i);
  while (res && !feof(myFile) && (p < mat_size)) {
    mat[p++] = i;
    res = fscanf(myFile, "%20d", &i);
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
  while (res && !feof(myFile)&& (p < mat_size)) {
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
  while (res && !feof(myFile)&& (p < mat_size)) {
    mat[p++] = f;
    res = fscanf(myFile, "%f", &f);
  }

  fclose(myFile);
  return p;
}

unsigned long
WfCst::readDouble(const char * fileName, double * mat,
                  unsigned long mat_size) {
  FILE * myFile;
  open_file(fileName, myFile);

  unsigned long p = 0;
  double d;
  int res;
  res = fscanf(myFile, "%lf", &d);
  while (res && !feof(myFile)&& (p < mat_size)) {
    mat[p++] = d;
    res = fscanf(myFile, "%lf", &d);
  }

  fclose(myFile);
  return p;
}


long
WfCst::eval_expr(std::string& expr, int var) {
  std::map<std::string, int> op_priority;
  op_priority["+"] = 0;
  op_priority["-"] = 0;
  op_priority["*"] = 1;
  op_priority["/"] = 1;
  op_priority["%"] = 1;
  std::stack<std::string> tokens;
  std::list<std::string> post_fixe_exp;
  std::stack<std::string>  polish;
  std::string tok;
  std::string op;
  long total = 0;
  std::string::size_type p = std::string::npos;
  // remove blanks
  while (expr.find(" ") != std::string::npos) {
    expr.erase(expr.find(" "), 1);
  }
  // replace the unary - by a #
  if (expr[0] == '-') {
    expr[0]='#';
  }
  for (unsigned int i = 1; i < expr.size(); i++) {
    if ((expr[i] == '-') && (!isdigit(expr[i-1]))) {
      expr[i]='#';
    }
  }

  // read the expression
  while (expr.size() > 0) {
    if (isdigit(expr[0]) || isalpha(expr[0]) || (expr[0] == '#')) {
      p = expr.find_first_of("+-/*%()");
      tok = expr.substr(0, p);
      post_fixe_exp.push_back(tok);
      if (p !=  std::string::npos) {
        expr = expr.substr(p);
      } else {
        expr = "";
      }
      } else {
      if (expr[0] == '(') {
        tokens.push("(");
      } else {
        if (expr[0] == ')') {
          while (tokens.top() != "(") {
            post_fixe_exp.push_back(tokens.top());
            tokens.pop();
          }
          tokens.pop();
        } else {
          // it is an op
          op = expr.substr(0, 1);
          while ((!tokens.empty())&&
                 (op_priority[tokens.top()] > op_priority[op])) {
            post_fixe_exp.push_back(tokens.top());
            tokens.pop();
          }  // end while
          tokens.push(op);
        }
      }
      expr = expr.substr(1);
    }
  }  // end while
  while (!tokens.empty()) {
    post_fixe_exp.push_back(tokens.top());
    tokens.pop();
  }

  while (!post_fixe_exp.empty()) {
    tok = post_fixe_exp.front();
    post_fixe_exp.pop_front();
    if (tok == "+" || tok == "-" || tok == "*" || tok == "/" || tok == "%") {
      // pop the first value
      std::string v1s = polish.top().c_str();
      long v1 = 1;
      if (v1s[0] == '#') {
        v1 = -1;
        v1s = v1s.substr(1);
      }
      v1 = v1*atoi(v1s.c_str());
      polish.pop();
      // pop the second value
      std::string v2s = polish.top().c_str();
      long v2 = 1;
      if (v2s[0] == '#') {
        v2 = -1;
        v2s = v2s.substr(1);
      }
      v2 = v2*atoi(v2s.c_str());
      polish.pop();

      std::stringstream ss;
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
    } else {
      polish.push(tok);
    }
  }

  total = atoi(polish.top().c_str());
  return total;
}

/**
 * return a list of token composing a string
 * used to read the matrix value
 */
std::vector<std::string>
getStringToken(std::string str) {
  std::vector<std::string> v;
  std::string str2(str);
  bool b = true;
  while (b) {
    v.push_back(str.substr(0, str.find(" ")));
    // read the immediate following blanks
    int ix = str.find(" ")+1;
    while (str[ix] == ' ') {
      ix++;
    }

    // remove the read token
    str2 =  str.substr(ix);
    if (str2 == str) {
      b = false;
    } else {
      str = str2;
    }
  }
  return v;
}  // end getStringToken

/**
 * return a string representation on an integer
 */
std::string
itoa(long l) {
  // stringstream seems to be not thread safe !!!
  char str[128];
  #ifdef __WIN32__
  _snprintf(str, sizeof(str), "%ld", l);
  #else
  snprintf(str, sizeof(str), "%ld", l);
  #endif
  return std::string(str);
}
