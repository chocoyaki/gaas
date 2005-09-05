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
#if HAVE_BATCH
      BATCHNAME,
#endif
      LOCATIONID,         // For alternative transfer cost prediction
      NB_PARAM_TYPE
    } param_type_t;

    /* Some of these parameters are complex types ... */
    
    /** The agent type: MA or LA. */
    typedef enum {DIET_LOCAL_AGENT, DIET_MASTER_AGENT} agent_type_t;

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
    static inline void*
    getParamValue(param_type_t param_type) {
      return Results::params[param_type].value;
    };
    
    
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
  static ifstream file;
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

};

#endif // _PARSERS_HH_
