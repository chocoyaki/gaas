/**
 * @file ExitClass.hh
 *
 * @brief  DIET Agent Exit Class header
 *
 * @author  Cyrille PONTVIEUX (cyrille.pontvieux@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _EXITCLASS_HH_
#define _EXITCLASS_HH_

#define EXIT_FUNCTION ExitClass::exitFunction()

#include "debug.hh"
#include "ORBMgr.hh"
#include "AgentImpl.hh"

class ExitClass {
public:
  /**
   * You must initialize the ExitClass with all informations needed by the
   * ExitFunction
   */
  static void
  init(AgentImpl *agent);

  /**
   * The ExitFunction called by the INTERNAL_ERROR macro.
   */
  static void
  exitFunction();

private:
  static AgentImpl *agent;
};

#endif  // _EXITCLASS_HH_
