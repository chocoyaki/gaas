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
 * Revision 1.1  2003/04/10 12:51:22  pcombes
 * Static class for configuration files parsing.
 *
 ****************************************************************************/

#include <iostream>
using namespace std;

#include "Parsers.hh"
#include "debug.hh"
#include "DIET_config.h"


FILE* Parsers::file = NULL;

extern unsigned int TRACE_LEVEL;

int
Parsers::beginParsing(char* filePath)
{
  if (file)
    cerr << "Warning: Parsers::beginParsing: file was not closed !\n";
  file = fopen(filePath, "r");
  if (!file) {
    cerr << "Error: Parsers::beginParsing: could not open <<"
	 << filePath << endl;
    return 1;
  }
  return 0;
}

int
Parsers::endParsing()
{
  if (!file)
    cerr << "Warning: Parsers::endParsing: file was already closed !\n";
  if (fclose(file))
    cerr << "Warning: Parsers::endParsing: could not close file !\n";
  file = NULL;
  return 0;
}

int
Parsers::parseName(char* name)
{
  if (fscanf(file, "%256s ", name) != 1) {
    cerr << "Error: Parsers::parseName: failed to read name.\n";
    return 1;
  }
  return 0;
}

int
Parsers::parseTraceLevel()//(unsigned int* traceLevel)
{
  if (fscanf(file, "traceLevel = %ud ", &TRACE_LEVEL) != 1) {
    cerr << "Warning: Parsers::ParseTraceLevel: could not read traceLevel "
	 << "(set to " << TRACE_DEFAULT << ")\n";
    TRACE_LEVEL = TRACE_DEFAULT;
  }
  return 0;
}


int
Parsers::parseFASTEntries(int*  ldapUse, char* ldapHost,  int* ldapPort,
			  char* ldapMask,
			  int*  nwsUse,  char* nwsNSHost, int* nwsNSPort,
			  char* nwsForecasterHost, int* nwsForecasterPort)
{

  /* Parse LDAP config lines */

  *ldapUse = 0;
  *ldapHost = '\0';
  *ldapPort = 0;
  *ldapMask = '\0';

#if HAVE_FAST
#define TEST != 1 /* All the configurations must be valid */
#else  // HAVE_FAST
#define TEST && 0 /* Read each lines and ignore the result */
#endif // HAVE_FAST

  if (fscanf(file, "LDAP_USE = %d ", ldapUse) TEST) {
    cerr << "Error: Parsers::parseFASTEntries: failed to read LDAP_USE.\n";
    return 1;
  }
  if (fscanf(file, "LDAP_HOST = %s ", ldapHost) TEST && *ldapUse) {
    cerr << "Error: Parsers::parseFASTEntries: failed to read LDAP_HOST.\n";
    return 1;
  }
  if (fscanf(file, "LDAP_PORT = %d ", ldapPort) TEST && *ldapUse) {
    cerr << "Error: Parsers::parseFASTEntries: failed to read LDAP_PORT.\n";
    return 1;
  }
  if (fscanf(file, "LDAP_MASK = %s ", ldapMask) TEST && *ldapUse) {
    cerr << "Error: Parsers::parseFASTEntries: failed to read LDAP_MASK.\n";
    return 1;
  }

  /* Parse NWS config lines */

  *nwsUse    = 0;
  *nwsNSHost = '\0';
  *nwsNSPort = 0;
  *nwsForecasterHost = '\0';
  *nwsForecasterPort = 0;
  
  if (fscanf(file, "NWS_USE = %d ", nwsUse) TEST) {
    cerr << "Error: Parsers::parseFASTEntries: failed to read NWS_USE.\n";
    return 1;
  }
  if (fscanf(file, "NWS_NAMESERVER_HOST = %s ", nwsNSHost) TEST && *nwsUse) {
    cerr << "Error: Parsers::parseFASTEntries: "
	 << "failed to read NWS_NAMESERVER_HOST.\n";
    return 1;
  }
  if (fscanf(file, "NWS_NAMESERVER_PORT = %d ", nwsNSPort) TEST && *nwsUse) {
    cerr << "Error: Parsers::parseFASTEntries: "
	 << "failed to read NWS_NAMESERVER_PORT.\n";
    return 1;
  }
  if (fscanf(file, "NWS_FORECASTER_HOST = %s ", nwsForecasterHost) TEST
      && *nwsUse) {
    cerr << "Error: Parsers::parseFASTEntries: "
	 << "failed to read NWS_FORECASTER_HOST.\n";
    return 1;
  }
  if (fscanf(file, "NWS_FORECASTER_PORT = %d ", nwsForecasterPort) TEST
      && *nwsUse) {
    cerr << "Error: Parsers::parseFASTEntries: "
	 << "failed to read NWS_FORECASTER_PORT.\n";
    return 1;
  }
  
#undef TEST

  return 0;
}

int
Parsers::scanLine256(char* line)
{
  int res = fscanf(file, "%256s", line);
  if (res != 1)
    res = 2;
  else if (res == EOF)
    res = 1;
  else
    res = 0;
  return res;
}

