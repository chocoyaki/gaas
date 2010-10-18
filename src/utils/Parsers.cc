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
 * Revision 1.59  2010/10/18 08:20:12  bisnard
 * Modified initialization of Parser to avoid crash due to calling
 * beginParsing twice.
 *
 * Revision 1.58  2010/09/02 12:07:47  bdepardo
 * Changed initialization of Parsers::path to remove a memory leak
 *
 * Revision 1.57  2010/07/12 16:14:12  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.56  2010/06/15 18:21:52  amuresan
 * Added separate compilation flag for Cloud features.
 * Removed gSOAP files from the kit, referencing an installed gSOAP package in cmake.
 * Added meta headers for cloud example files.
 * Added Cloud-related files to the release listing.
 *
 * Revision 1.55  2010/05/05 13:13:51  amuresan
 * First commit for the Eucalyptus BatchSystem.
 * Added SOAP client for the Amazon EC2 SOAP interface and
 * a new implementation of the BatchSystem base-class.
 *
 * Revision 1.54  2010/01/14 13:15:08  bdepardo
 * "\n" -> endl
 *
 * Revision 1.53  2009/09/25 13:44:43  bdepardo
 * Fixed a bug on clientMaxNbSeD: there was an inconsistency in the type of
 * the parameter.
 *
 * Revision 1.52  2009/09/07 14:34:53  bdepardo
 * Added an option in client configuration file to select, when launching the
 * client, the maximum number of SeD the client can receive.
 * Example in client.cfg:
 * clientMaxNbSeD = 100
 *
 * Revision 1.51  2008/11/18 09:39:59  bdepardo
 * Warning removal: the type of the parameters weren't in the intialization of
 * the array containing the different parameters.
 *
 * Revision 1.50  2008/07/08 22:14:23  rbolze
 * avoid  "conversion from string constant to «char*»" warning
 *
 * Revision 1.49  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.48  2008/05/06 10:49:37  bisnard
 * bug in checkValidity (compulsory param was not displayed)
 *
 * Revision 1.47  2008/04/29 22:22:02  glemahec
 * DAGDA improvements :
 *   - Asynchronous API.
 *   - Data ID alias managing.
 *   - Data manager state backup and restore.
 *
 * Revision 1.46  2008/04/28 07:08:31  glemahec
 * The DAGDA API.
 *
 * Revision 1.45  2008/04/22 08:24:02  glemahec
 * Cache replacement algorithms for DAGDA + Shared file management.
 *
 * Revision 1.44  2008/04/09 12:52:54  gcharrie
 * Adding Specific Client Scheduling into the parser to use burst mode
 *
 * Revision 1.43  2008/04/07 15:33:44  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.42  2008/02/12 11:38:13  glemahec
 * Les references aux SeDs sont sorties du marshalling. Le parametre
 * "storageDirectory" est maintenant partage par DAGDA et les batchs.
 *
 * !!! Attention : Il faut donc remplacer le parametre "pathToTmp" par
 * "storageDirectory" dans les fichiers de configuration pour les SeDs
 * batchs !!!
 *
 * Revision 1.41  2008/01/14 11:46:52  glemahec
 * Adds the DAGDA parameters to the possible ones in the config files.
 *
 * Revision 1.40  2008/01/01 19:43:49  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 * Revision 1.39  2007/12/07 08:44:42  bdepardo
 * Added AckFile support in CMake files.
 * No longer need to add -DADAGE to use it, instead -DHAVE_ACKFILE is automatically added when the option is selected.
 * /!\ Parsing problem on Mac: do not recognize the parameter ackFile within the configuration file.
 *
 * Revision 1.38  2007/07/31 14:25:11  bdepardo
 * Added option ackFile in the configuration file, in order for the agents to touch a file at the end of their initialization.
 * Currently needs to use -D ADAGE when compiling to support this feature.
 *
 * Revision 1.37  2007/06/29 15:13:19  ycaniou
 * Unused variable (warning--)
 *
 * Revision 1.36  2007/06/28 17:17:42  ycaniou
 * MAImpl.cc: reqIDCounter feature to begin the count with a given value
 * Parsers.cc: parseInt not a name
 *
 * Revision 1.35  2007/06/28 14:59:22  ycaniou
 * Add the parsing of the keyword initRequestID from which request counter
 * will begin.
 *
 * Revision 1.34  2007/04/30 13:30:25  ycaniou
 * Moved C++ commentaries to C commentaries because they were on multiple lines
 *   then the C compiler complained
 *
 * Revision 1.33  2007/04/17 20:44:58  dart
 * - move #define from Parsers.cc to Parsers.hh
 * - define the maximum length of getline as MAXCFGLINE
 * - change tests about config file
 * - insert HUGE_VAL definition if not defined to compile under AIX
 *
 * Revision 1.32  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.31  2007/03/26 13:41:20  glemahec
 * Adds the options "schedulerModule" and "moduleConfigFile" to the allowed options of a DIET config file.
 *
 * Revision 1.30  2007/02/16 20:43:17  ycaniou
 * Add type to parsed value to correct memory leaks
 *
 * Revision 1.29  2006/11/16 09:55:56  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.28  2006/07/11 08:59:10  ycaniou
 * .Batch queue is now read in the serveur config file (only one queue
 * supported).
 * .Transfered perf evaluation in diet server (still dummy function)
 *
 * Revision 1.27  2006/07/10 11:12:05  aamar
 * Adding the workflow monitoring parameter USEWFLOGSERVICE
 *
 * Revision 1.26  2006/04/14 14:22:28  aamar
 * Adding the DIET_MA_DAG in agent_type_t enumeration.
 * Adding MADAGNAME in param_type_t enumeration.
 * Adding the MADAGNAME parameter for configuration file.
 *
 * Revision 1.25  2005/09/05 16:09:14  hdail
 * Addition of locationId to configuration file options.
 *
 * Revision 1.24  2005/08/30 07:24:23  ycaniou
 * Changed the test in profile_match to enable the possibility for DIET to
 *   decide if a 'normal' job sould be submitted via batch or not.
 * Add the parsing of 'batchName' in config file.
 * Some type precisions in estVector (but real code untouched) because of
 *   compilation warnings I had.
 *
 * Revision 1.23  2005/06/28 15:55:06  hdail
 * Changed default config file settings so that even if FAST is compiled in,
 * defaults are the same as when its not compiled in.  Users should explicitly
 * state in config file that they require NWS, LDAP, and/or FAST since
 * requiring them can cause DIET to hang and/or crash.
 *
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
#include <fstream>
using namespace std;

#include "Parsers.hh"
#include "assert.h"
#include "debug.hh"
#include "ms_function.hh"

#include <sstream>

/** The trace level. */
extern unsigned int TRACE_LEVEL;


// !!! None of these keywords can be a prefix for another !!!
Parsers::Results::param_t Parsers::Results::params[] =
  {/* [0]  */ {"traceLevel", 10, Parsers::parseTraceLevel, 0, NULL, Results::NONE_PARAMETER},
   /* [1]  */ {"MAName", 6, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER},
   /* [2]  */ {"agentType", 9, Parsers::parseAgentType, 0, NULL, Results::AGENT_PARAMETER},
   /* [3]  */ {"dietPort", 8, Parsers::parsePort, 0, NULL, Results::INT_PARAMETER},
   /* [4]  */ {"dietHostname", 12, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER},
   /* [5]  */ {"name", 4, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER},
   /* [6]  */ {"parentName", 10, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER},
   /* [7]  */ {"fastUse", 7, Parsers::parseUse, 0, NULL, Results::INT_PARAMETER},
   /* [8]  */ {"ldapUse", 7, Parsers::parseUse, 0, NULL, Results::INT_PARAMETER},
   /* [9]  */ {"ldapBase", 8, Parsers::parseAddress, 0, NULL, Results::ADDRESS_PARAMETER},
   /* [10] */ {"ldapMask", 8, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER},
   /* [11] */ {"nwsUse", 6, Parsers::parseUse, 0, NULL, Results::INT_PARAMETER},
   /* [12] */ {"nwsNameserver", 13, Parsers::parseAddress, 0, NULL, Results::ADDRESS_PARAMETER},
   /* [13] */ {"nwsForecaster", 13, Parsers::parseAddress, 0, NULL, Results::ADDRESS_PARAMETER},
   /* [14] */ {"useAsyncAPI", 11, Parsers::parseUse, 0, NULL, Results::INT_PARAMETER},
   /* [15] */ {"useLogService", 13, Parsers::parseUse, 0, NULL, Results::INT_PARAMETER},
   /* [16] */ {"lsOutbuffersize", 15, Parsers::parsePort, 0, NULL, Results::INT_PARAMETER},
   /* [17] */ {"lsFlushinterval", 15, Parsers::parsePort, 0, NULL, Results::INT_PARAMETER},
   /* [18] */ {"neighbours", 10, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER},
   /* [19] */ {"maximumNeighbours", 17, Parsers::parsePort, 0, NULL, Results::INT_PARAMETER},
   /* [20] */ {"minimumNeighbours", 17, Parsers::parsePort, 0, NULL, Results::INT_PARAMETER},
   /* [21] */ {"updateLinkPeriod", 16, Parsers::parsePort, 0, NULL, Results::INT_PARAMETER},
   /* [22] */ {"bindServicePort", 15, Parsers::parsePort, 0, NULL, Results::INT_PARAMETER},
   /* [23] */ {"useConcJobLimit", 15, Parsers::parseUse, 0, NULL, Results::INT_PARAMETER},
   /* [24] */ {"maxConcJobs", 11, Parsers::parseInt, 0, NULL, Results::INT_PARAMETER}
   /* [25] */ ,{"locationID", 10, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [26] */ ,{"MADAGNAME", 9, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [27] */ ,{"USEWFLOGSERVICE", 15, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
/* New : For user scheduler support. */
/*       schedulerModule  : The path to the scheduler library file. */
/*       moduleConfigFile : Optionnal configuration file for the module. */
#ifdef USERSCHED
   /* [28] */ ,{"schedulerModule", 15, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [29] */ ,{"moduleConfigFile", 16, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
#endif
#ifdef HAVE_ALT_BATCH
   /* [30] */ ,{"batchName", 9, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [31] */ ,{"batchQueue", 10, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [32] */ ,{"pathToNFS", 9, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [33] */ ,{"pathToTmp", 9, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
#endif
   /* [34] */ ,{"initRequestID", 13, Parsers::parseInt, 0, NULL, Results::INT_PARAMETER}
#ifdef HAVE_ACKFILE
   /* [35] */ ,{"ackFile", 7, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
#endif
#if HAVE_DAGDA
   /* [36] */ ,{"maxMsgSize", 10, Parsers::parseULong, 0, NULL, Results::ULONG_PARAMETER}
   /* [37] */ ,{"maxDiskSpace", 12, Parsers::parseULong, 0, NULL, Results::ULONG_PARAMETER}
   /* [38] */ ,{"maxMemSpace", 11, Parsers::parseULong, 0, NULL, Results::ULONG_PARAMETER}
   /* [39] */ ,{"cacheAlgorithm", 14, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [40] */ ,{"shareFiles", 10, Parsers::parseUse, 0, NULL, Results::INT_PARAMETER}
   /* [41] */ ,{"dataBackupFile", 14, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [42] */ ,{"restoreOnStart", 14, Parsers::parseUse, 0, NULL, Results::INT_PARAMETER}
#endif // HAVE_DAGDA
#if HAVE_DAGDA || HAVE_ALT_BATCH
   /* [43] */ ,{"storageDirectory", 16, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
#endif
#ifdef HAVE_CCS
   /* [44] */ ,{"USE_SPECIFIC_SCHEDULING", 23, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
#endif
#ifdef HAVE_ALT_BATCH
   /* [41] */ ,{"internOARbatchQueueName", 23, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
#endif
   /* [42] */ ,{"clientMaxNbSeD", 14, Parsers::parseULong, 0, NULL, Results::ULONG_PARAMETER}
#ifdef HAVE_CLOUD
   /* [43] */ ,{"cloudURL", 8, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [44] */ ,{"emiName", 7, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER} 
   /* [45] */ ,{"eriName", 7, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [46] */ ,{"ekiName", 7, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [47] */ ,{"keyName", 7, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [48] */ ,{"vmType", 6, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [49] */ ,{"vmMinCount", 10, Parsers::parseInt, 0, NULL, Results::INT_PARAMETER}
   /* [50] */ ,{"vmMaxCount", 10, Parsers::parseInt, 0, NULL, Results::INT_PARAMETER}
   /* [51] */ ,{"pathToCert", 10, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
   /* [52] */ ,{"pathToPK", 8, Parsers::parseName, 0, NULL, Results::STRING_PARAMETER}
#endif
} ;

#define IS_ADDRESS(i) ((i == Results::LDAPBASE) || (i == Results::NWSNAMESERVER) || (i == Results::NWSFORECASTER))


ifstream Parsers::file;
char*    Parsers::path = NULL;
// size_t --> unsigned int
unsigned int   Parsers::noLine = 0;

/*
Moved by EQ for EC in Parsers.hh

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
*/

void *
Parsers::Results::getParamValue(Results::param_type_t param_type)
{
  return Parsers::Results::params[param_type].value;
};


/**
 * Prepare the parsing of the file \c filePath (open the file and initialize
 * the parameter structures)
 */
int
Parsers::beginParsing(const char* filePath)
{
  if ((filePath == NULL) || (*filePath == '\0')) {
    PARSERS_ERROR("no file to parse", DIET_FILE_IO_ERROR);
  }
  Parsers::path = CORBA::string_dup(filePath);
  Parsers::file.clear();
  if (Parsers::file.is_open()) {
    Parsers::file.close();  // Closing the file before re-opening it
    Parsers::endParsing();  // Free all the parsed parameters
  }
  Parsers::file.open(filePath);

  if (! Parsers::file.good()) {
    PARSERS_ERROR("could not open " << filePath, DIET_FILE_IO_ERROR);
  }
  Parsers::noLine = 0;
  // printf("begin Parsing %i\n",Parsers::file.good());

  return 0;
}

/**
 * Free the parameters structures: all calls to Results::getParamValue must
 * have been done before call to this method.
 */
int
Parsers::endParsing()
{
    // size_t --> unsigned int
  for (unsigned int i = Results::TRACELEVEL; i < Results::NB_PARAM_TYPE; i++) {
    if (Results::params[i].value != NULL) {
	switch( Results::params[i].type ) {
	case Results::INT_PARAMETER:
	  delete((int*)(Results::params[i].value)) ;
	  break ;
	case Results::STRING_PARAMETER:
	  free((char*)(Results::params[i].value)) ;
	  break ;
	case Results::ADDRESS_PARAMETER:
	  delete((Results::Address*)(Results::params[i].value)) ;
	  break ;
	case Results::AGENT_PARAMETER:
	  delete((Parsers::Results::agent_type_t*)(Results::params[i].value)) ;
	  break ;
#if HAVE_DAGDA
    case Results::ULONG_PARAMETER:
	  delete((unsigned long*)(Results::params[i].value));
	  break;
#endif
	default:
	  break ;
//       if (IS_ADDRESS(i)) {
// 	delete((Results::Address*)Results::params[i].value);
//       } else {
//         // TODO: should be deleted with delete, but delete can not be
//         // used with void*.  Identify pointer type and delete.
// 	//free(Results::params[i].value);
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
int
Parsers::parseCfgFile(bool checkFASTEntries, unsigned int nbCompulsoryParams,
		      Results::param_type_t* compulsoryParams)
{
  static char full_line[MAXCFGLINE];
  char* ptr=NULL;
  int parse_res=0;

  if (! Parsers::file.is_open() || ! Parsers::file.good()) {
    PARSERS_INTERNAL_ERROR("no file has been opened. Please consider calling "
			   << "Parsers::beginParsing first",
			   DIET_FILE_IO_ERROR);
  }
  while (Parsers::file.getline(full_line, MAXCFGLINE)) {

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
    if (parse_res) {
      Parsers::file.close();
      return parse_res;
    }
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
    PARSERS_WARNING(PARAM(FASTUSE).kwd << " is missing."
		    << endl << "I guess "
		    << PARAM(FASTUSE).kwd << " = 0");
    PARAM(FASTUSE).value = new size_t(0);
  }

    // size_t --> unsigned int
  use = *((unsigned int*)PARAM(FASTUSE).value);
  TRACE_TEXT(TRACE_ALL_STEPS, PARAM(FASTUSE).kwd << " = " << use << "." << endl);

  if (use > 0) {

    /* Check LDAP entries */
    if (PARAM(LDAPUSE).value == NULL) {
      // Display warning for SeDs only, when ldapUse is not set
      if (PARAM(AGENTTYPE).value != NULL) {
	PARSERS_WARNING(PARAM(LDAPUSE).kwd << " is missing."
			<< endl << "I guess "
			<< PARAM(LDAPUSE).kwd << " = 0");
	*((unsigned int*)PARAM(LDAPUSE).value) = 0;
      } else {
	// for agents, default is 0
	*((unsigned int*)PARAM(LDAPUSE).value) = 0;
      }
    }

    use = *((unsigned int*)PARAM(LDAPUSE).value);
    TRACE_TEXT(TRACE_ALL_STEPS,
	       ' ' << PARAM(LDAPUSE).kwd << " = " << use << "." << endl);

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
      PARSERS_WARNING(PARAM(NWSUSE).kwd << " is missing."
		      << endl << "I guess "
		      << PARAM(NWSUSE).kwd << " = 0");
      *((unsigned int*)PARAM(NWSUSE).value) = 0;
    }

    use = *((unsigned int*)PARAM(NWSUSE).value);
    TRACE_TEXT(TRACE_ALL_STEPS,
	       ' ' << PARAM(NWSUSE).kwd << " = " << use << "." << endl);

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
    use = *((unsigned int*)PARAM(FASTUSE).value);

  if (use > 0) {
    PARSERS_WARNING("fastUse is set to 1 at line " << Parsers::noLine
		    << " but DIET was compiled without FAST - ignored");
    *((unsigned int*)PARAM(FASTUSE).value) = 0;
  }

#endif // HAVE_FAST

  return 0;
}

/**
 * Check for presence of compulsory params, and call checkFASTEntries.
 */
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
	 << ": some compulsory parameters are missing:" << endl;
    for (size_t i = 0; i < nbCompulsoryParams; i++) {
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
  Results::params[type].type = Results::ADDRESS_PARAMETER ;
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
   else if ((strncmp(buf, "MA_DAG", 6)) ||
	    (strncmp(buf, "DIET_MA_DAG", 11)))
     agtType = Results::DIET_MA_DAG;
   else
     res = DIET_PARSE_ERROR;
   if (res) {
     PARSERS_ERROR(Results::params[type].kwd << " should be DIET_LOCAL_AGENT "
		   << "(or LA) or DIET_MASTER_AGENT (or MA)", res);
   } else {
     Results::params[type].noLine = Parsers::noLine;
     Results::params[type].value =
       new Parsers::Results::agent_type_t(agtType);
     Results::params[type].type = Results::AGENT_PARAMETER ;
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
  Results::params[type].type = Results::STRING_PARAMETER ;
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
  Results::params[type].type = Results::INT_PARAMETER ;
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
  Results::params[type].type = Results::NONE_PARAMETER ;
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
  Results::params[type].type = Results::INT_PARAMETER ;
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
  Results::params[type].type = Results::INT_PARAMETER ;
  return 0;
}

/**
 * Parse an unsigned long int. If the conversion did not succeed,
 * the result is set to 0.
 */
int
Parsers::parseULong(char* ulongString, Results::param_type_t type) {
  unsigned long value;
  std::istringstream str(ulongString);
  str >> value;
  if (str.fail()) {
	PARSERS_WARNING("Could not read unsigned long int from " << ulongString);
    value=0;
  }
  Results::params[type].value = new unsigned long(value);
  Results::params[type].type = Results::ULONG_PARAMETER ;
  return 0;
}
