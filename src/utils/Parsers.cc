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
 * Revision 1.22  2004/10/04 13:55:06  hdail
 * - Added AccessController class, an enhanced counting semaphore.
 * - Added config file options for controlling concurrent SeD access.
 *
 * Revision 1.21  2004/09/29 13:35:32  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.20  2004/09/14 12:50:26  hdail
 * Commented out free of Results::params[i].value.  Should be cleaned with
 * delete, but the type is unknown at this point so can't be.
 *
 * Revision 1.19  2004/07/05 14:56:13  rbolze
 * correct bug on 64 bit plat-form, when parsing cfg file :
 * remplace size_t by unsigned int for config options
 *
 * Revision 1.18  2004/05/28 10:53:21  mcolin
 * change the endpoint option names for agents and servers
 *  endPointPort -> dietPort
 *  endPointHostname -> dietHostname
 *
 * Revision 1.17  2004/04/16 19:04:40  mcolin
 * Fix patch for the vthd demo with the endPoint option in config files.
 * This option is now replaced by two options:
 *   endPointPort: precise the listening port of the agent/server
 *   endPointHostname: precise the listening interface of the agent/server
 ****************************************************************************/

#include <iostream>
using namespace std;

#include "Parsers.hh"
#include "assert.h"
#include "debug.hh"
#include "DIET_config.h"
#include "ms_function.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;


// !!! None of these keywords can be a prefix for another !!!
Parsers::Results::param_t Parsers::Results::params[] =
  {/* [0]  */ {"traceLevel", 10, Parsers::parseTraceLevel, 0, NULL},
   /* [1]  */ {"MAName", 6, Parsers::parseName, 0, NULL},
   /* [2]  */ {"agentType", 9, Parsers::parseAgentType, 0, NULL},
   /* [3]  */ {"dietPort", 8, Parsers::parsePort, 0, NULL},
   /* [4]  */ {"dietHostname", 12, Parsers::parseName, 0, NULL},
   /* [5]  */ {"name", 4, Parsers::parseName, 0, NULL},
   /* [6]  */ {"parentName", 10, Parsers::parseName, 0, NULL},
   /* [7]  */ {"fastUse", 7, Parsers::parseUse, 0, NULL},
   /* [8]  */ {"ldapUse", 7, Parsers::parseUse, 0, NULL},
   /* [9]  */ {"ldapBase", 8, Parsers::parseAddress, 0, NULL},
   /* [10] */ {"ldapMask", 8, Parsers::parseName, 0, NULL},
   /* [11] */ {"nwsUse", 6, Parsers::parseUse, 0, NULL},
   /* [12] */ {"nwsNameserver", 13, Parsers::parseAddress, 0, NULL},
   /* [13] */ {"nwsForecaster", 13, Parsers::parseAddress, 0, NULL},
   /* [14] */ {"useAsyncAPI", 11, Parsers::parseUse, 0, NULL},
   /* [15] */ {"useLogService", 13, Parsers::parseUse, 0, NULL},
   /* [16] */ {"lsOutbuffersize", 15, Parsers::parsePort, 0, NULL},
   /* [17] */ {"lsFlushinterval", 15, Parsers::parsePort, 0, NULL},
   /* [18] */ {"neighbours", 10, Parsers::parseName, 0, NULL},
   /* [19] */ {"maximumNeighbours", 17, Parsers::parsePort, 0, NULL},
   /* [20] */ {"minimumNeighbours", 17, Parsers::parsePort, 0, NULL},
   /* [21] */ {"updateLinkPeriod", 16, Parsers::parsePort, 0, NULL},
   /* [22] */ {"bindServicePort", 15, Parsers::parsePort, 0, NULL},
   /* [23] */ {"useConcJobLimit", 15, Parsers::parseUse, 0, NULL},
   /* [24] */ {"maxConcJobs", 11, Parsers::parseInt, 0, NULL}};

#define IS_ADDRESS(i) ((i == Results::LDAPBASE) || (i == Results::NWSNAMESERVER) || (i == Results::NWSFORECASTER))


ifstream Parsers::file;
char*    Parsers::path = "";
// size_t --> unsigned int
unsigned int   Parsers::noLine = 0;


#define DIET_PARSE_ERROR        1
#define DIET_FILE_IO_ERROR      2
#define DIET_MISSING_PARAMETERS 3

#define PARSERS_ERROR(formatted_msg,return_value)                          \
  ERROR("Parsers::" << __FUNCTION__ << ": " << formatted_msg, return_value)

#define PARSERS_INTERNAL_ERROR(formatted_msg,return_value)             \
  INTERNAL_ERROR("Parsers::" << __FUNCTION__ << ": " << formatted_msg, \
		 return_value)

#define PARSERS_WARNING(formatted_msg)                         \
  WARNING("Parsers::" << __FUNCTION__ << ": " << formatted_msg)

#define PARSERS_INTERNAL_WARNING(formatted_msg)                         \
  INTERNAL_WARNING("Parsers::" << __FUNCTION__ << ": " << formatted_msg)

#define PARAM(type) Results::params[Results::type]

#define CHECK_PARAM(type)                                                 \
  if (Results::params[(type)].noLine > 0) {                               \
    PARSERS_WARNING(Results::params[(type)].kwd << " already set at line "\
	            << Results::params[(type)].noLine << " - ignored");   \
    return 0;                                                             \
  }


/**
 * Prepare the parsing of the file \c filePath (open the file and initialize
 * the parameter structures)
 */
int
Parsers::beginParsing(char* filePath)
{
  if ((filePath == NULL) || (*filePath == '\0')) {
    PARSERS_ERROR("no file to parse", DIET_FILE_IO_ERROR);
  }
  Parsers::path = filePath;
  Parsers::file.open(filePath);
  // FIXME: this test might be wrong !!!
  if (Parsers::file == NULL) {
    PARSERS_ERROR("could not open " << filePath, DIET_FILE_IO_ERROR);
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
//   if (Parsers::file == NULL)
//     PARSERS_WARNING("file was already closed");
//   else {
    Parsers::file.close();
    // FIXME: this test is wrong
    // if (Parsers::file != NULL)
    //   PARSERS_WARNING("could not close file");
//  }

    // size_t --> unsigned int
  for (unsigned int i = Results::TRACELEVEL; i < Results::NB_PARAM_TYPE; i++) {
    if (Results::params[i].value != NULL) {
      if (IS_ADDRESS(i)) {
	delete((Results::Address*)Results::params[i].value);
      } else {
        // TODO: should be deleted with delete, but delete can not be  
        // used with void*.  Identify pointer type and delete.
	//free(Results::params[i].value);
      }
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

    // size_t --> unsigned int
int
Parsers::parseCfgFile(bool checkFASTEntries, unsigned int nbCompulsoryParams,
		      Results::param_type_t* compulsoryParams)
{
  static char full_line[512];
  char* ptr;
  int parse_res;

  if (Parsers::file == NULL) {
    PARSERS_INTERNAL_ERROR("no file has been opened. Please consider calling "
			   << "Parsers::beginParsing first",
			   DIET_FILE_IO_ERROR);
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
  if (PARAM(TRACELEVEL).noLine == 0)
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
  //size_t use(0);
 // size_t --> unsigned int
  unsigned int use = 0;
#if HAVE_FAST

  if (PARAM(FASTUSE).value == NULL) {
    PARSERS_WARNING(PARAM(FASTUSE).kwd << " is missing.\n As DIET was compiled"
		    << " with FAST, I guess " << PARAM(FASTUSE).kwd << " = 1");
    PARAM(FASTUSE).value = new size_t(1);
  }

    // size_t --> unsigned int
  use = *((unsigned int*)PARAM(FASTUSE).value);
  TRACE_TEXT(TRACE_ALL_STEPS, PARAM(FASTUSE).kwd << " = " << use << ".\n");

  if (use > 0) {

    /* Check LDAP entries */
    if (PARAM(LDAPUSE).value == NULL) {
      // Display warning for SeDs only, when ldapUse is not set
      if (PARAM(AGENTTYPE).value != NULL) {
	PARSERS_WARNING(PARAM(LDAPUSE).kwd << " is missing.\n As DIET was "
			<< "compiled with FAST, and " << PARAM(FASTUSE).kwd
			<< " = 1, I guess " << PARAM(LDAPUSE).kwd << " = 1");
    // size_t --> unsigned int
	*((unsigned int*)PARAM(LDAPUSE).value) = 1;
      } else {
	// for agents, default is 0
    // size_t --> unsigned int
	*((unsigned int*)PARAM(LDAPUSE).value) = 0;
      }
    }

    // size_t --> unsigned int
    use = *((unsigned int*)PARAM(LDAPUSE).value);
    TRACE_TEXT(TRACE_ALL_STEPS,
	       ' ' << PARAM(LDAPUSE).kwd << " = " << use << ".\n");
    
    if (use > 0) {
      if (PARAM(LDAPBASE).value == NULL) {
	PARSERS_ERROR(PARAM(LDAPBASE).kwd << " is missing",
		      DIET_MISSING_PARAMETERS);
      }
      TRACE_TEXT(TRACE_ALL_STEPS, "  " << PARAM(LDAPBASE).kwd << " = "
		 << ((Results::Address*)PARAM(LDAPBASE).value)->host << ':'
		 << ((Results::Address*)PARAM(LDAPBASE).value)->port << endl);

      if (PARAM(LDAPMASK).value == NULL) {
	PARSERS_ERROR(PARAM(LDAPMASK).kwd << " is missing",
		      DIET_MISSING_PARAMETERS);
      }
      TRACE_TEXT(TRACE_ALL_STEPS, "  " << PARAM(LDAPBASE).kwd << " = "
		 << (char*)PARAM(LDAPMASK).value << endl);
    }

    /* Check NWS entries */
    if (PARAM(NWSUSE).value == NULL) {
      PARSERS_WARNING(PARAM(NWSUSE).kwd << " is missing.\n As DIET was "
		      << "compiled with FAST, and " << PARAM(FASTUSE).kwd
		      << " = 1, I guess " << PARAM(NWSUSE).kwd << " = 1");
    // size_t --> unsigned int
      *((unsigned int*)PARAM(NWSUSE).value) = 1;
    }

    // size_t --> unsigned int
    use = *((unsigned int*)PARAM(NWSUSE).value);
    TRACE_TEXT(TRACE_ALL_STEPS,
	       ' ' << PARAM(NWSUSE).kwd << " = " << use << ".\n");
    
    if (use > 0) {
      if (PARAM(NWSNAMESERVER).value == NULL) {
	PARSERS_ERROR(PARAM(NWSNAMESERVER).kwd << " is missing",
		      DIET_MISSING_PARAMETERS);
      }
      TRACE_TEXT(TRACE_ALL_STEPS, "  " << PARAM(NWSNAMESERVER).kwd << " = "
		 << ((Results::Address*)PARAM(NWSNAMESERVER).value)->host << ':'
		 << ((Results::Address*)PARAM(NWSNAMESERVER).value)->port
		 << endl);
      
      if (PARAM(NWSFORECASTER).value != NULL) {
	TRACE_TEXT(TRACE_ALL_STEPS, "  " << PARAM(NWSFORECASTER).kwd << " = "
		   << ((Results::Address*)PARAM(NWSFORECASTER).value)->host
		   << ':'
		   << ((Results::Address*)PARAM(NWSFORECASTER).value)->port
		   << endl);
#if defined(__FAST_0_4__)
      } else {
	PARSERS_ERROR(PARAM(NWSFORECASTER).kwd << " is missing",
		      DIET_MISSING_PARAMETERS);
#endif
      }
    }
  }

#else  // HAVE_FAST

  if (PARAM(FASTUSE).value != NULL)
    // size_t --> unsigned int
    use = *((unsigned int*)PARAM(FASTUSE).value);
  if (use > 0) {
    PARSERS_WARNING("fastUse is set to 1 at line " << Parsers::noLine
		    << " but DIET was compiled without FAST - ignored");
    // size_t --> unsigned int
    *((unsigned int*)PARAM(FASTUSE).value) = 0;
  }

#endif // HAVE_FAST

  return 0;
}

/**
 * Check for presence of compulsory params, and call checkFASTEntries.
 */
    // size_t --> unsigned int
int
Parsers::checkValidity(bool checkFASTEntries, unsigned int nbCompulsoryParams,
		       Results::param_type_t* compulsoryParams)
{
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
    cerr << "Error: " << "Parsers::" << __FUNCTION__
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
  char* ptr = line;
  int parse_res = DIET_PARSE_ERROR;

  // Set ptr to the character after the '='
  ptr = strchr(line, '=');
  if (ptr == NULL) {
    PARSERS_ERROR("no '=' at line " << Parsers::noLine, DIET_PARSE_ERROR);
  }
  ptr++;
  // Call the parser associated to the first keyword matched.
    // size_t --> unsigned int
  for (unsigned int i = Results::TRACELEVEL; i < Results::NB_PARAM_TYPE; i++) {
    if (!strncmp(line, Results::params[i].kwd, Results::params[i].len)) {
      parse_res =
	(*Results::params[i].parser)(ptr, (Results::param_type_t)i);
      break;
    }
  }

  if (parse_res) {
    PARSERS_ERROR("syntax error at " << Parsers::path << ':' << Parsers::noLine,
		  parse_res);
  }
  return parse_res;
}

/**
 * Parse a <host:port> address and fill in Results::params with an Address.
 */
int
Parsers::parseAddress(char* address, Results::param_type_t type)
{
  static char buf[257];
  // size_t --> unsigned int
  unsigned int port;
  char* ptr;

  CHECK_PARAM(type);

  ptr = strchr(address, ':');
  if ((ptr == address) || (ptr == NULL)) {
    PARSERS_ERROR(Results::params[type].kwd << " should be <host:port>",
		  DIET_PARSE_ERROR);
  }
  *ptr = '\0';
  if (sscanf(address, "%256s", (char*)buf) != 1) {
    PARSERS_ERROR(Results::params[type].kwd << " should be <host:port>",
		  DIET_PARSE_ERROR);
  }
  *ptr = ':';
  if (sscanf(ptr + 1, "%u ", &port) != 1) {
    PARSERS_ERROR(Results::params[type].kwd << " should be <host:port>",
		  DIET_PARSE_ERROR);
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
   if (res) {
     PARSERS_ERROR(Results::params[type].kwd << " should be DIET_LOCAL_AGENT "
		   << "(or LA) or DIET_MASTER_AGENT (or MA)", res);
   } else {
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
  // size_t --> unsigned int
  unsigned int port;

  CHECK_PARAM(type);
  if (sscanf(port_str, "%u ", &port) != 1) {
    return DIET_PARSE_ERROR;
  }
  Results::params[type].noLine = Parsers::noLine;
  // size_t --> unsigned int
  Results::params[type].value = new unsigned int(port);
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
  CHECK_PARAM(type);
  if (sscanf(traceLevel, "%u ", &TRACE_LEVEL) != 1) {
    PARSERS_WARNING("could not read traceLevel (set to "
		    << TRACE_DEFAULT << ')');
    TRACE_LEVEL = TRACE_DEFAULT;
  } else { // TRACE_LEVEL is set
    Results::params[type].noLine = Parsers::noLine;
  }
  return 0;
}

/**
 * Parse an integer.  If the integer conversion did not succeed, the
 * integer result is set to -1;
 */ 
int
Parsers::parseInt(char* intString, Results::param_type_t type)
{
  int value;

  CHECK_PARAM(type);
  if (sscanf(intString, "%d ", &value) != 1) {
    PARSERS_WARNING("could not read int from " << intString);
    Results::params[type].value = new int(-1);
  } else {
    Results::params[type].value = new int(value);
  }
  return 0;
} 

/**
 * Parse a use (for fastUse, ldapUse, etc.): 0 or 1.
 */
int
Parsers::parseUse(char* use_str, Results::param_type_t type)
{
  // size_t --> unsigned int
  unsigned int use;

  CHECK_PARAM(type);
  if ((sscanf(use_str, "%u ", &use) != 1) || (use != 0 && use != 1)) {
    PARSERS_ERROR(Results::params[type].kwd  << " must be 0 or 1",
		  DIET_PARSE_ERROR);
  }
  Results::params[type].noLine = Parsers::noLine;
  // size_t --> unsigned int
  Results::params[type].value  = new unsigned int(use);
  return 0;
}

