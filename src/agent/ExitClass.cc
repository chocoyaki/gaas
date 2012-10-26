/**
 * @file  ExitClass.cc
 *
 * @brief  DIET Agent Exit Class implementation
 *
 * @author  Cyrille PONTVIEUX (cyrille.pontvieux@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include "ExitClass.hh"

#include <cstdlib>
#include <iostream>
#include "configuration.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

AgentImpl *ExitClass::agent = 0;

void
ExitClass::init(AgentImpl *agent) {
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
  } catch (...) {
  }
  TRACE_TEXT(TRACE_MAIN_STEPS, "Agent stopped !\n");
  /* We do not need the parsing results any more */
  delete GetConfigPtr();
} // exitFunction
