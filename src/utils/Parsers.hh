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
 * Revision 1.1  2003/04/10 12:51:22  pcombes
 * Static class for configuration files parsing.
 *
 ****************************************************************************/

#ifndef _PARSERS_HH_
#define _PARSERS_HH_

#include <stdio.h>
#include <unistd.h>

/**
 * Tool to simplify file parsing in DIET.
 */

class Parsers
{
  
public:
  static int
  beginParsing(char* filePath);
  
  static int
  endParsing();

  /**
   * Parse the name of a DIET entity.
   * name must point at an array of 257 characters.
   */
  static int
  parseName(char* name);

  /**
   * Parse the 'traceLevel = ' config line.
   * If the line is not matched, traceLevel is set to TRACE_DEFAULT,
   * and a warning is printed.
   */
  static int
  parseTraceLevel();//(unsigned int* traceLevel);

  /**
   * Parse all FAST entries for a SeD or an agent.
   * ldapHost, ldapMask, nwsNSHost ans nwsForecasterHost must point at an array
   * of 257 characters.
   */
  static int
  parseFASTEntries(int*  ldapUse, char* ldapHost,  int* ldapPort,
		   char* ldapMask,
		   int*  nwsUse,  char* nwsNSHost, int* nwsNSPort,
		   char* nwsForecasterHost, int* nwsForecasterPort);

  /**
   * Parse a 256-char line
   * @return 0 if a line has been read, 1 if EOF, and 2 when errors occur.
   */
  static int
  scanLine256(char* line);

private:
  static FILE* file;

};


#endif // _PARSERS_HH_
