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
 * Revision 1.6  2011/02/04 15:20:48  hguemar
 * fixes to new configuration parser
 * some cleaning
 *
 * Revision 1.5  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.4  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
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

#include "ExitClass.hh"

#include <cstdlib>
#include <iostream>
#include "configuration.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

AgentImpl* ExitClass::agent = 0;

void
ExitClass::init(AgentImpl* agent) {
  ExitClass::agent = agent;
}

void
ExitClass::exitFunction() {
  TRACE_TEXT(TRACE_MAIN_STEPS, "______________________________\n");
  TRACE_TEXT(TRACE_MAIN_STEPS, "Stopping the Agent...\n");
  // Deactivate and destroy the agent
  try {
    if (ExitClass::agent) {
      delete ExitClass::agent;
    }
    delete ORBMgr::getMgr();
  } catch (...) {}
  TRACE_TEXT(TRACE_MAIN_STEPS, "Agent stopped !\n");
  /* We do not need the parsing results any more */
  delete configPtr;
}

