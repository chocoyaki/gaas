/****************************************************************************/
/* DIET Agent Exit Class implementation                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cyrille PONTVIEUX (cyrille.pontvieux@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2010/03/03 10:19:03  bdepardo
 * Changed \n into endl
 *
 * Revision 1.2  2004/03/01 18:45:28  rbolze
 * change in exitFunction()
 *
 * Revision 1.1  2003/10/06 10:11:55  cpontvie
 * Class to manage a clean exit, call by EXITFUNCTION
 *
 ****************************************************************************/

using namespace std;
#include "ExitClass.hh"

#include <iostream>
#include <stdlib.h>

#include "Parsers.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

AgentImpl* ExitClass::agent = NULL;

void
ExitClass::init(AgentImpl* agent)
{
  ExitClass::agent = agent;
}

void
ExitClass::exitFunction()
{
  TRACE_TEXT(TRACE_MAIN_STEPS, "______________________________" << endl);
  TRACE_TEXT(TRACE_MAIN_STEPS, "Stopping the Agent..." << endl);
  // Deactivate and destroy the agent
  try {
    if (ExitClass::agent != NULL) {
      delete ExitClass::agent;
    }
    ORBMgr::destroy();
  } catch (...) {}
  TRACE_TEXT(TRACE_MAIN_STEPS, "Agent stopped !" << endl);

  /* We do not need the parsing results any more */
  Parsers::endParsing();
}

