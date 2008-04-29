/****************************************************************************/
/* DIET parsers header file                                                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.34  2008/04/29 22:22:02  glemahec
 * DAGDA improvements :
 *   - Asynchronous API.
 *   - Data ID alias managing.
 *   - Data manager state backup and restore.
 *
 * Revision 1.33  2008/04/28 07:08:31  glemahec
 * The DAGDA API.
 *
 * Revision 1.32  2008/04/22 08:24:02  glemahec
 * Cache replacement algorithms for DAGDA + Shared file management.
 *
 * Revision 1.31  2008/04/09 12:52:54  gcharrie
 * Adding Specific Client Scheduling into the parser to use burst mode
 *
 * Revision 1.30  2008/04/07 15:33:44  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.29  2008/02/12 11:38:13  glemahec
 * Les references aux SeDs sont sorties du marshalling. Le parametre
 * "storageDirectory" est maintenant partage par DAGDA et les batchs.
 *
 * !!! Attention : Il faut donc remplacer le parametre "pathToTmp" par
 * "storageDirectory" dans les fichiers de configuration pour les SeDs
 * batchs !!!
 *
 * Revision 1.28  2008/01/14 13:49:28  glemahec
 * CMakeLists.txt files modified to compile DIET with DAGDA.
 * Bugs corrections.
 *
 * Revision 1.27  2008/01/14 11:46:52  glemahec
 * Adds the DAGDA parameters to the possible ones in the config files.
 *
 * Revision 1.26  2008/01/01 19:43:49  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 * Revision 1.25  2007/12/07 08:44:42  bdepardo
 * Added AckFile support in CMake files.
 * No longer need to add -DADAGE to use it, instead -DHAVE_ACKFILE is automatically added when the option is selected.
 * /!\ Parsing problem on Mac: do not recognize the parameter ackFile within the configuration file.
 *
 * Revision 1.24  2007/07/31 14:25:11  bdepardo
 * Added option ackFile in the configuration file, in order for the agents to touch a file at the end of their initialization.
 * Currently needs to use -D ADAGE when compiling to support this feature.
 *
 * Revision 1.23  2007/06/28 14:59:22  ycaniou
 * Add the parsing of the keyword initRequestID from which request counter
 * will begin.
 *
 * Revision 1.22  2007/04/17 20:44:58  dart
 * - move #define from Parsers.cc to Parsers.hh
 * - define the maximum length of getline as MAXCFGLINE
 * - change tests about config file
 * - insert HUGE_VAL definition if not defined to compile under AIX
 *
 * Revision 1.21  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.20  2007/03/26 13:41:21  glemahec
 * Adds the options "schedulerModule" and "moduleConfigFile" to the allowed options of a DIET config file.
 *
 * Revision 1.19  2007/02/16 20:43:17  ycaniou
 * Add type to parsed value to correct memory leaks
 *
 * Revision 1.18  2006/07/11 08:59:10  ycaniou
 * .Batch queue is now read in the serveur config file (only one queue
 * supported).
 * .Transfered perf evaluation in diet server (still dummy function)
 *
 * Revision 1.17  2006/07/10 11:12:05  aamar
 * Adding the workflow monitoring parameter USEWFLOGSERVICE
 *
 * Revision 1.16  2006/04/14 14:26:22  aamar
 * Adding the MADAGNAME parameter for configuration file.
 * Adding the DIET_MA_DAG in agent_type_t enumeration.
 *
 * Revision 1.15  2005/09/05 16:09:14  hdail
 * Addition of locationId to configuration file options.
 *
 * Revision 1.14  2005/05/02 16:52:50  ycaniou
 * Added code commentaries
 *
 * Revision 1.13  2005/04/29 15:03:11  ecaron
 * Delete friend definition to class Result to avoid one problem with gcc 3.4.x
 *
 * Revision 1.12  2005/04/27 01:49:41  ycaniou
 * Added the necessary for initialisation of batch profile, for profiles to
 * match
 * Added the functions diet_profile_set_parallel(), diet_profile_set_nbprocs(),
 * diet_profile_desc_set_batch(), diet_profile_desc_set_parallel() that the
 * client needs to define a parallel/batch job
 * Added the parsing of the batch scheduler name that must be provided in the
 * server configuration file, accordingly to the elagi library, plus some
 * checkings about the server that can only submit batch or non-batch jobs
 *
 * Revision 1.11  2004/10/04 13:55:06  hdail
 * - Added AccessController class, an enhanced counting semaphore.
 * - Added config file options for controlling concurrent SeD access.
 *
 * Revision 1.10  2004/09/29 13:35:32  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.9  2004/07/05 14:56:13  rbolze
 * correct bug on 64 bit plat-form, when parsing cfg file :
 * remplace size_t by unsigned int for config options
 *
 * Revision 1.8  2004/05/28 10:53:21  mcolin
 * change the endpoint option names for agents and servers
 *  endPointPort -> dietPort
 *  endPointHostname -> dietHostname
 *
 * Revision 1.7  2004/04/16 19:04:40  mcolin
 * Fix patch for the vthd demo with the endPoint option in config files.
 * This option is now replaced by two options:
 *   endPointPort: precise the listening port of the agent/server
 *   endPointHostname: precise the listening interface of the agent/server
 *
 * Revision 1.6  2004/03/01 19:02:30  rbolze
 * change to enable new options relative to logservice in the config file for MA, LA and SeD
 *
 * Revision 1.5  2003/06/02 08:08:11  cpera
 * Beta version of asynchronize DIET API.
 *
 * Revision 1.4  2003/05/22 11:27:47  sdahan
 * adds a missing header in Parsers
 *
 * Revision 1.3  2003/05/15 11:48:41  pcombes
 * Fix includes.
 *
 * Revision 1.2  2003/05/10 08:49:33  pcombes
 * New Parsers for new configuration files.
 *
 * Revision 1.1  2003/04/10 12:51:22  pcombes
 * Static class for configuration files parsing.
 ****************************************************************************/

#ifndef _PARSERS_HH_
#define _PARSERS_HH_

#include <fstream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "ts_container/ts_set.hh"
#include "ms_function.hh"

// Added by EQ for EC from Parsers.cc
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

#define MAXCFGLINE 512

/**
 * Tool which aims at make file parsing simpler in DIET.
 */

class Parsers
{  
public:

  /**
   * The structure filled in by Parsers.
   */
  class Results
  {
  public:
    friend class Parsers;
    
    /** The configuration parameter type. */
    typedef enum {
      TRACELEVEL = 0,
      MANAME,             // MA for client to attach to
      AGENTTYPE,          // Is this agent an LA or MA
      DIETPORT,           // End point definition
      DIETHOSTNAME,       // End point definition
      NAME,               // Agent name
      PARENTNAME,         // Parent to attach to
      FASTUSE,            
      LDAPUSE, LDAPBASE, LDAPMASK,
      NWSUSE, NWSNAMESERVER, NWSFORECASTER,
      USEASYNCAPI,
      USELOGSERVICE, LSOUTBUFFERSIZE, LSFLUSHINTERVAL,
      NEIGHBOURS, MAXNEIGHBOURS, MINNEIGHBOURS, UPDATELINKPERIOD,
      BINDSERVICEPORT, USECONCJOBLIMIT, MAXCONCJOBS,
      LOCATIONID,         // For alternative transfer cost prediction
      MADAGNAME,
      USEWFLOGSERVICE,
/* New : For user scheduler support. */
#ifdef USERSCHED
	  MODULENAME,
	  MODULECFG,
#endif
#if HAVE_ALT_BATCH
      BATCHNAME,
      BATCHQUEUE,
      PATHTONFS,
      PATHTOTMP,
#endif

      initRequestID,        // RequestID begins with this value

#ifdef HAVE_ACKFILE
      ACKFILE,              // file to touch at the end of the initialization
#endif
#if HAVE_DAGDA
	  MAXMSGSIZE,
	  MAXDISKSPACE,
	  MAXMEMSPACE,
	  CACHEALGORITHM,
	  SHAREFILES,
	  DATABACKUPFILE,
	  RESTOREONSTART,
#endif // HAVE_DAGDA
#if HAVE_DAGDA || HAVE_ALT_BATCH
	  STORAGEDIR,
#endif
#ifdef HAVE_CCS
      USE_SPECIFIC_SCHEDULING,
#endif
      NB_PARAM_TYPE
    } param_type_t;

    /* To store parameter C type, to free memory after use */
    typedef enum {
      NONE_PARAMETER,
      INT_PARAMETER,
      STRING_PARAMETER,
      ADDRESS_PARAMETER,
      AGENT_PARAMETER // like int
#if HAVE_DAGDA
	 ,ULONG_PARAMETER
#endif // HAVE_DAGDA
    } param_C_type_t ;

    /* Some of these parameters are complex types ... */
    
    /** The agent type: MA or LA. */
    typedef enum {DIET_LOCAL_AGENT, DIET_MASTER_AGENT, DIET_MA_DAG} agent_type_t;

    /** A <host:port> address. */
    class Address {
    public:
      char*  host;
    // size_t --> unsigned int
     unsigned int port;
      inline
    // size_t --> unsigned int
      Address(char* host, unsigned int port) {
	this->host = strdup(host);
	this->port = port;
      }
      inline
      Address(const Address& a) {
	this->host = ms_strdup(a.host);
	this->port = a.port;
      }
      ~Address() {
	if (this->host)
	  free(this->host);
      }
      bool operator== (const Address& a) const {
	return port == a.port && (strcmp(host, a.host) == 0) ;
      }
      bool operator< (const Address& a) const {
	if (port != a.port)
	  return strcmp(host, a.host)<0 ;
	else
	  return port < a.port ;
      }
    }; // class Address


    /**
     * a set of addresses
     */
    typedef ts_set<Address> AddressesSet ;

    /**
     * Return a pointer to the value parsed for the parameter of type \c type.
     */
//     static inline void*
//     getParamValue(Results::param_type_t param_type) {
//       return Parsers::Results::params[param_type].value;
//     };
    static void * getParamValue(Results::param_type_t param_type) ;
    
  protected:

    /** Type of all private parsers */
    typedef int (*parser_t)(char*, Results::param_type_t);

    /** Basic element of the parameters structure */
    typedef struct {
      /** keyword */
      char*  kwd;
      /** length of the keyword */
    // size_t --> unsigned int
      unsigned int len;
      /** parser */
      parser_t parser;
      /** line where it is set */
    // size_t --> unsigned int
      unsigned int noLine;
      /** pointer to the value of the parameter */
      void* value;
      /** C type of parameter, in order to correctly free memory **/
      Results::param_C_type_t type ;
    } param_t;
    /** All keywords and their lengths */
    static param_t params[];
  }; // Class Results
  
  
  /**
   * Prepare the parsing of the file \c filePath (open the file and initialize
   * the parameter structures)
   */
  static int
  beginParsing(char* filePath);
  
  /**
   * Free the parameters structures: all calls to Results::getParamValue must
   * have been done before call to this method.
   */
  static int
  endParsing();

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
  static int
  parseCfgFile(bool checkFASTEntries, unsigned int nbCompulsoryParams,
	       Results::param_type_t* compulsoryParams);


private:

  /** Path of the file to parse */
  static char* path;
  /** Stream of the file to parse */
  static std::ifstream file;
  /** Current line number */
    // size_t --> unsigned int
  static unsigned int noLine;

  /**
   * Check the coherence of the FAST parameters, essentially depending on the
   * various "Use" parameters entered.
   */
  static int
  checkFASTEntries();

  /**
   * Check for presence of compulsory params, and call checkFASTEntries.
   */
    // size_t --> unsigned int
  static int
  checkValidity(bool checkFASTEntries, unsigned int nbCompulsoryParams,
		Results::param_type_t* compulsoryParams);
  
  /**
   * Parse a valid line (not empty, not commented), and call the parser
   * associated to the keyword found in first place.
   */
  static int
  parseCfgLine(char* line);

  /**
   * Parse a <host:port> address and fill in Results::params with an Address.
   */
  static int
  parseAddress(char* address, Results::param_type_t type);

  /**
   * Parse an agent type.
   * NB: parameter \c type is useless since it is called only for AGENTTYPE.
   */
  static int
  parseAgentType(char* agtType_str, Results::param_type_t type);

  /**
   * Parse the name of a DIET entity (mine or my parent's name).
   */
  static int
  parseName(char* name, Results::param_type_t type);

  /**
   * Parse a port entry (for endPoint, ldapPort, etc.)
   */
  static int
  parsePort(char* port_str, Results::param_type_t type);

  /**
   * Parse an unsigned int and set global variable TRACE_LEVEL.
   * If the integer was wrong, TRACE_LEVEL is set to TRACE_DEFAULT,
   * and a warning is printed.
   */
  static int
  parseTraceLevel(char* traceLevel,
		  Results::param_type_t type = Results::TRACELEVEL);

  /**
   * Parse an integer.  If the integer conversion did not succeed, the
   * integer result is set to -1;
   */ 
  static int
  parseInt(char* intString, Results::param_type_t type);

  /**
   * Parse a use (for fastUse, ldapUse, etc.): 0 or 1.
   */
  static int
  parseUse(char* use_str, Results::param_type_t type);

#if HAVE_DAGDA
  /**
   * Parse an unsigned long int. If the conversion did not succeed,
   * the result is set to 0.
   */
   static int
   parseULong(char* ulongString, Results::param_type_t type);
#endif // HAVE_DAGDA

};

#endif // _PARSERS_HH_
