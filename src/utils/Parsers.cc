/****************************************************************************/
/* DIET parsers source code                                                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.10  2003/06/30 11:43:54  cpera
 * Fix incorrect async use code.
 *
 * Revision 1.9  2003/06/30 11:15:12  cpera
 * Fix bugs in ReaderWriter and new internal debug macros.
 *
 * Revision 1.8  2003/06/23 13:35:49  pcombes
 * useAsyncApi -> useAsyncAPI
 *
 * Revision 1.7  2003/06/02 08:08:11  cpera
 * Beta version of asynchronize DIET API.
 *
 * Revision 1.6  2003/05/22 11:27:47  sdahan
 * adds a missing header in Parsers
 *
 * Revision 1.5  2003/05/15 14:42:16  pcombes
 * Still bug fixes in compilation with FAST. Sorry !
 *
 * Revision 1.2  2003/05/10 08:49:33  pcombes
 * New Parsers for new configuration files.
 *
 * Revision 1.1  2003/04/10 12:51:22  pcombes
 * Static class for configuration files parsing.
 ****************************************************************************/

#include <iostream>
using namespace std;

#include "Parsers.hh"
#include "assert.h"
#include "debug.hh"
#include "DIET_config.h"

extern unsigned int TRACE_LEVEL;

// !!! None of these keywords can be a prefix for another !!!
Parsers::Results::param_t Parsers::Results::params[] =
  {/* [0]  */ {"traceLevel", 10, Parsers::parseTraceLevel, 0, NULL},
   /* [1]  */ {"MAName", 6, Parsers::parseName, 0, NULL},
   /* [2]  */ {"agentType", 9, Parsers::parseAgentType, 0, NULL},
   /* [3]  */ {"endPoint", 8, Parsers::parsePort, 0, NULL},
   /* [4]  */ {"name", 4, Parsers::parseName, 0, NULL},
   /* [5]  */ {"parentName", 10, Parsers::parseName, 0, NULL},
   /* [6]  */ {"fastUse", 7, Parsers::parseUse, 0, NULL},
   /* [7]  */ {"ldapUse", 7, Parsers::parseUse, 0, NULL},
   /* [8]  */ {"ldapBase", 8, Parsers::parseAddress, 0, NULL},
   /* [9]  */ {"ldapMask", 8, Parsers::parseName, 0, NULL},
   /* [10] */ {"nwsUse", 6, Parsers::parseUse, 0, NULL},
   /* [11] */ {"nwsNameserver", 13, Parsers::parseAddress, 0, NULL},
   /* [12] */ {"nwsForecaster", 13, Parsers::parseAddress, 0, NULL},
   /* [13] */ {"useAsyncAPI", 11, Parsers::parseUse, 0, NULL}};

#define IS_ADDRESS(i) ((i == 8) || (i == 11) || (i == 12))


ifstream Parsers::file;
char*    Parsers::path = "";
size_t   Parsers::noLine = 0;


#define DIET_PARSE_ERROR        1
#define DIET_FILE_IO_ERROR      2
#define DIET_MISSING_PARAMETERS 3

#define CHECK_PARAM(type) \
  if (Results::params[(type)].noLine > 0) {                           \
    cerr << "Warning: " << FUNCTION_NAME << ": "                    \
	 << Results::params[(type)].kwd  << " already set at line " \
	 << Results::params[(type)].noLine << " - ignored.\n";      \
    return 0;                                                       \
  }


/**
 * Prepare the parsing of the file \c filePath (open the file and initialize
 * the parameter structures)
 */
int
Parsers::beginParsing(char* filePath)
{
  static char* FUNCTION_NAME = "Parsers::beginParsing";
  if ((filePath == NULL) || (*filePath == '\0')) {
    cerr << "Error: " << FUNCTION_NAME << ": no file to parse !\n";
    return DIET_FILE_IO_ERROR;
  }
  Parsers::path = filePath;
  Parsers::file.open(filePath);
  // FIXME: this test should be wrong !!!
  if (Parsers::file == NULL) {
    cerr << "Error: " << FUNCTION_NAME << ": could not open "
	 << filePath << endl;
    return DIET_FILE_IO_ERROR;
  }
  Parsers::noLine = 0;
  return 0;
}

/**
 * Free the parameters structures: all calls to Results::getParamValue must
 * have been done before call to this method.
 */
int
Parsers::endParsing()
{
  //  static char* FUNCTION_NAME = "Parsers::endParsing";

//   if (Parsers::file == NULL)
//     cerr << "Warning: " << FUNCTION_NAME << ": file was already closed !\n";
//   else {
    Parsers::file.close();
    // FIXME: this test is wrong
    // if (Parsers::file != NULL)
    //   cerr << "Warning: " << FUNCTION_NAME << ": could not close file !\n";
//  }
  for (size_t i = Results::TRACELEVEL; i < Results::NB_PARAM_TYPE; i++) {
    if (Results::params[i].value != NULL) {
      if (IS_ADDRESS(i))
	delete((Results::Address*)Results::params[i].value);
      else
	free(Results::params[i].value);
    }
    Results::params[i].noLine = 0;
  }
  return 0;
}

/**
 * Parse the file \c filePath and fill in the parameters structure. 
 * It is possible to specify a list of compulsory parameters, so that the
 * parsers can check itself for presence of these parameters.
 * FAST parameters are processed apart, since they depend on each other.
 * @param checkFASTEntries   tells if FAST entries must be checked.
 * @param nbCompulsoryParams is the length of the list \c compulsoryParams
 * @param compulsoryParams   is the list of compulsory parameters
 */
int
Parsers::parseCfgFile(bool checkFASTEntries, size_t nbCompulsoryParams,
		      Results::param_type_t* compulsoryParams)
{
  static char* FUNCTION_NAME = "Parsers::parseCfgFile";
  static char full_line[512];
  char* ptr;
  int parse_res;

  if (Parsers::file == NULL) {
    cerr << "Error: " << FUNCTION_NAME << ": no file has been opened."
	 << " Please consider calling beginParsing first !\n";
    return DIET_FILE_IO_ERROR;
  }

  while (Parsers::file.getline(full_line, 512)) {

    Parsers::noLine++;

    // Set Parsers::line
    ptr = (char*) full_line;
    // Skip leading blank characters
    while ((*ptr == ' ') || (*ptr == '\t'))
      ptr++;
    if (*ptr == '#')  /* comment lines */
      continue;
    if (*ptr == '\0') /* empty lines */
      continue;
    parse_res = Parsers::parseCfgLine(ptr);
    if (parse_res)
      return parse_res;
  }

  /* If no traceLevel specified, set it to default */
  if (Results::params[Results::TRACELEVEL].noLine == 0)
    TRACE_LEVEL = TRACE_DEFAULT;
  
	if ((parse_res =
       Parsers::checkValidity(checkFASTEntries,
			      nbCompulsoryParams, compulsoryParams)))
    return parse_res;

  return 0;
}


/**
 * Check the coherence of the FAST parameters, essentially depending on the
 * various "Use" parameters entered.
 */
int
Parsers::checkFASTEntries()
{
  static char* FUNCTION_NAME = "Parsers::checkFASTEntries";
  size_t use(0);

#if HAVE_FAST

  Results::Address* addr;

  if (Results::params[Results::FASTUSE].value == NULL) {
    cerr << "Warning: " << FUNCTION_NAME << ": "
	 << Results::params[Results::FASTUSE].kwd
	 << " is missing. As DIET was compiled with FAST, I guess "
	 << Results::params[Results::FASTUSE].kwd << " = 1.\n";
    Results::params[Results::FASTUSE].value = new size_t(1);
  }
  use = *((size_t*)Results::params[Results::FASTUSE].value);

  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << Results::params[Results::FASTUSE].kwd << " = " << use << ".\n";

  if (use > 0) {

    /* Check LDAP entries */
    if (Results::params[Results::LDAPUSE].value == NULL) {
      // Display warning for SeDs only, when ldapUse is not set
      if (Results::params[Results::AGENTTYPE].value != NULL) {
	cerr << "Warning: " << FUNCTION_NAME << ": "
	     << Results::params[Results::LDAPUSE].kwd << " is missing."
	     << " As DIET was compiled with FAST, and "
	     << Results::params[Results::FASTUSE].kwd << " = 1, I guess "
	     << Results::params[Results::LDAPUSE].kwd << " = 1.\n";
	*((size_t*)Results::params[Results::LDAPUSE].value) = 1;
      } else {
	// for agents, default is 0
	*((size_t*)Results::params[Results::LDAPUSE].value) = 0;
      }
    }
    use = *((size_t*)Results::params[Results::LDAPUSE].value);
    
    if (TRACE_LEVEL >= TRACE_ALL_STEPS)
      cout << ' ' << Results::params[Results::LDAPUSE].kwd
	   << " = " << use << endl;
    
    if (use > 0) {
      if (Results::params[Results::LDAPBASE].value == NULL) {
	cerr << "Error: " << FUNCTION_NAME << ": "
	     << Results::params[Results::LDAPBASE].kwd << " is missing !\n";
	return DIET_MISSING_PARAMETERS;
      }
      if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	cout << "  " << Results::params[Results::LDAPBASE].kwd << " = "
	     << ((Results::Address*)
		 Results::params[Results::LDAPBASE].value)->host
	     << ':'
	     << ((Results::Address*)
		 Results::params[Results::LDAPBASE].value)->port
	     << endl; 

      if (Results::params[Results::LDAPMASK].value == NULL) {
	cerr << "Error: " << FUNCTION_NAME << ": "
	     << Results::params[Results::LDAPMASK].kwd << " is missing !\n";
	return DIET_MISSING_PARAMETERS;
      }
      if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	cout << "  " << Results::params[Results::LDAPBASE].kwd << " = "
	     << (char*)Results::params[Results::LDAPMASK].value << endl;
      
    }

    /* Check NWS entries */
    if (Results::params[Results::NWSUSE].value == NULL) {
      cerr << "Warning: " << FUNCTION_NAME << ": "
	   << Results::params[Results::NWSUSE].kwd << " is missing."
	   << " As DIET was compiled with FAST, and "
	   << Results::params[Results::FASTUSE].kwd << " = 1, I guess "
	   << Results::params[Results::NWSUSE].kwd << " = 1.\n";
      *((size_t*)Results::params[Results::NWSUSE].value) = 1;
    }
    use = *((size_t*)Results::params[Results::NWSUSE].value);

    if (TRACE_LEVEL >= TRACE_ALL_STEPS)
      cout << ' ' << Results::params[Results::LDAPUSE].kwd << " = " << use << endl;

    if (use > 0) {
      if (Results::params[Results::NWSNAMESERVER].value == NULL) {
	cerr << "Error: " << FUNCTION_NAME << ": "
	     << Results::params[Results::NWSNAMESERVER].kwd
	     << " is missing !\n";
	return DIET_MISSING_PARAMETERS;
      }
      if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	cout << "  " << Results::params[Results::NWSNAMESERVER].kwd << " = "
	     << ((Results::Address*)
		 Results::params[Results::NWSNAMESERVER].value)->host
	     << ':'
	     << ((Results::Address*)
		 Results::params[Results::NWSNAMESERVER].value)->port
	     << endl;
      if (Results::params[Results::NWSFORECASTER].value == NULL) {
	cerr << "Error: " << FUNCTION_NAME << ": "
	     << Results::params[Results::NWSFORECASTER].kwd
	     << " is missing !\n";
	return DIET_MISSING_PARAMETERS;
      }
      if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	cout << "  " << Results::params[Results::NWSFORECASTER].kwd << " = "
	     << ((Results::Address*)
		 Results::params[Results::NWSFORECASTER].value)->host
	     << ':'
	     << ((Results::Address*)
		 Results::params[Results::NWSFORECASTER].value)->port
	     << endl; 
    }
  }

#else  // HAVE_FAST

  if (Results::params[Results::FASTUSE].value != NULL)
    use = *((size_t*)Results::params[Results::FASTUSE].value);
  if (use > 0) {
    cerr << "Warning: " << FUNCTION_NAME
	 << ": fastUse is set to 1 at line " << Parsers::noLine
	 << " but DIET was compiled without FAST - ignored.\n";
    *((size_t*)Results::params[Results::FASTUSE].value) = 0;
  }

#endif // HAVE_FAST

  return 0;
}

/**
 * Check for presence of compulsory params, and call checkFASTEntries.
 */
int
Parsers::checkValidity(bool checkFASTEntries, size_t nbCompulsoryParams,
		       Results::param_type_t* compulsoryParams)
{
  static char* FUNCTION_NAME = "Parsers::checkValidity";

  if (checkFASTEntries)
    Parsers::checkFASTEntries();
  
  if (nbCompulsoryParams == 0)
    return 0;
  assert(compulsoryParams != NULL);
  
  bool someAreMissing(false);

  for (size_t i = 0; i < nbCompulsoryParams; i++) {
    Results::param_type_t type = compulsoryParams[i];
    if (Results::params[type].noLine == 0) {
      someAreMissing = true;
      break;
    }
  }
  if (someAreMissing) {
    cerr << "Error: " << FUNCTION_NAME
	 << ": some compulsory parameters are missing:\n";
    for (size_t i = 0; i < nbCompulsoryParams - 1; i++) {
      Results::param_type_t type = compulsoryParams[i];
      if (Results::params[type].noLine == 0)
	cerr << " " << Results::params[type].kwd;
    }
    cerr << endl;
    return DIET_MISSING_PARAMETERS;
  }
  return 0;
}

/**
 * Parse a valid line (not empty, not commented), and call the parser
 * associated to the keyword found in first place.
 */
int
Parsers::parseCfgLine(char* line)
{
  static char* FUNCTION_NAME = "Parsers::parseCfgLine";
  char* ptr = line;
  int parse_res = DIET_PARSE_ERROR;

  // Set ptr to the character after the '='
  ptr = strchr(line, '=');
  if (ptr == NULL) {
    cerr << "Error: " << FUNCTION_NAME << ": no '=' at line "
	 << Parsers::noLine << ".\n";
    return DIET_PARSE_ERROR;
  }
  ptr++;
  // Call the parser associated to the first keyword matched.
  for (size_t i = Results::TRACELEVEL; i < Results::NB_PARAM_TYPE; i++) {
    if (!strncmp(line, Results::params[i].kwd, Results::params[i].len)) {
      parse_res =
	(*Results::params[i].parser)(ptr, (Results::param_type_t)i);
      break;
    }
  }

  if (parse_res) {
    cerr << "Error: " << FUNCTION_NAME << ": syntax error at "
	 << Parsers::path << ':' << Parsers::noLine << ".\n";
  }
  return parse_res;
}

/**
 * Parse a <host:port> address and fill in Results::params with an Address.
 */
int
Parsers::parseAddress(char* address, Results::param_type_t type)
{
  static char* FUNCTION_NAME = "Parsers::parseAddress";
  static char buf[257];
  size_t port;
  char* ptr;
  CHECK_PARAM(type);

  ptr = strchr(address, ':');
  if ((ptr == address) || (ptr == NULL)) {
    cerr << "Error: " << FUNCTION_NAME << ": "
	 << Results::params[type].kwd << " should be <host:port>.\n";
    return DIET_PARSE_ERROR;
  }
  *ptr = '\0';
  if (sscanf(address, "%256s", (char*)buf) != 1) {
    cerr << "Error: " << FUNCTION_NAME << ": "
	 << Results::params[type].kwd << " should be <host:port>.\n";
    return DIET_PARSE_ERROR;
  }
  *ptr = ':';
  if (sscanf(ptr + 1, "%u ", &port) != 1) {
    cerr << "Error: " << FUNCTION_NAME << ": "
	 << Results::params[type].kwd << " should be <host:port>.\n";
    return DIET_PARSE_ERROR;
  }
  Results::params[type].noLine = Parsers::noLine;
  Results::params[type].value = new Results::Address((char*)buf, port);
  return 0;
}

/**
 * Parse an agent type.<br>
 * NB: parameter \c type is useless since it is called only for AGENTTYPE.
 */
int
Parsers::parseAgentType(char* agtType_str, Results::param_type_t type)
{
   static char* FUNCTION_NAME = "Parsers::parseAgentType";
   static char buf[257];
   // dummy initialization to avoid warning at compile time.
   Parsers::Results::agent_type_t agtType = Results::DIET_LOCAL_AGENT;
   int res(0);

   CHECK_PARAM(type);
   if (sscanf(agtType_str,"%256s", (char*)buf) != 1)
     res = DIET_PARSE_ERROR;
   else if ((!strncmp(buf, "LA", 2)) ||
	    (!strncmp(buf, "DIET_LOCAL_AGENT", 16)))
     agtType = Results::DIET_LOCAL_AGENT;
   else if ((!strncmp(buf, "MA", 2)) ||
	    (!strncmp(buf, "DIET_MASTER_AGENT", 17)))
     agtType = Results::DIET_MASTER_AGENT;
   else
     res = DIET_PARSE_ERROR;
   if (res)
    cerr << "Error: " << FUNCTION_NAME << ": " << Results::params[type].kwd
	 << " should be DIET_LOCAL_AGENT or DIET_MASTER_AGENT.\n";
   else {
     Results::params[type].noLine = Parsers::noLine;
     Results::params[type].value =
       new Parsers::Results::agent_type_t(agtType);
   }
   return res;
}

/**
 * Parse the name of a DIET entity (mine or my parent's name).
 */
int
Parsers::parseName(char* name, Results::param_type_t type)
{
  static char* FUNCTION_NAME = "Parsers::parseName";
  static char buf[257];

  CHECK_PARAM(type);
  if (sscanf(name, "%256s ", buf) != 1) {
    return DIET_PARSE_ERROR;
  }
  Results::params[type].noLine = Parsers::noLine;
  Results::params[type].value = strdup(buf);
  return 0;
}

/**
 * Parse a port entry (for endPoint, ldapPort, etc.)
 */
int
Parsers::parsePort(char* port_str, Results::param_type_t type)
{
  static char* FUNCTION_NAME = "Parsers::parsePort";
  size_t port;

  CHECK_PARAM(type);
  if (sscanf(port_str, "%u ", &port) != 1) {
    return DIET_PARSE_ERROR;
  }
  Results::params[type].noLine = Parsers::noLine;
  Results::params[type].value = new size_t(port);
  return 0;
}

/**
 * Parse an unsigned int and set global variable TRACE_LEVEL.
 * If the integer was wrong, TRACE_LEVEL is set to TRACE_DEFAULT,
 * and a warning is printed.
 */
int
Parsers::parseTraceLevel(char* traceLevel, Results::param_type_t type)
{
  static char* FUNCTION_NAME = "Parsers::parseTraceLevel";

  CHECK_PARAM(type);
  if (sscanf(traceLevel, "%u ", &TRACE_LEVEL) != 1) {
    cerr << "Warning: " << FUNCTION_NAME << ": could not read traceLevel "
	 << "(set to " << TRACE_DEFAULT << ")\n";
    TRACE_LEVEL = TRACE_DEFAULT;
  } else { // TRACE_LEVEL is set
    Results::params[type].noLine = Parsers::noLine;
  }
  return 0;
}

/**
 * Parse a use (for fastUse, ldapUse, etc.): 0 or 1.
 */
int
Parsers::parseUse(char* use_str, Results::param_type_t type)
{
  static char* FUNCTION_NAME = "Parsers::parseUse";
  size_t use;

  CHECK_PARAM(type);
  if ((sscanf(use_str, "%u ", &use) != 1) || (use != 0 && use != 1)) {
    cerr << "Error: " << FUNCTION_NAME << ": "
	 << Results::params[type].kwd  << " must be 0 or 1.\n";
    return DIET_PARSE_ERROR;
  }
  Results::params[type].noLine = Parsers::noLine;
  Results::params[type].value = new size_t(use);
  return 0;
}
