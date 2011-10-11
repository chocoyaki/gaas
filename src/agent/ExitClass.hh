/****************************************************************************/
/* DIET Agent Exit Class header                                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cyrille PONTVIEUX (cyrille.pontvieux@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2003/10/06 10:11:35  cpontvie
 * Class to manage a clean exit, call by EXITFUNCTION
 *
 ****************************************************************************/

/*
  !! YOU MUST INCLUDE THIS HEADER PRIOR TO THE OTHERS !!
*/

#ifndef _EXITCLASS_HH_
#define _EXITCLASS_HH_

#define EXIT_FUNCTION ExitClass::exitFunction()

#include "debug.hh"
#include "ORBMgr.hh"
#include "AgentImpl.hh"

class ExitClass
{
public:
  /**
   * You must initialize the ExitClass with all informations needed by the
   * ExitFunction
   */
  static void
  init(AgentImpl* agent);

  /**
   * The ExitFunction called by the INTERNAL_ERROR macro.
   */
  static void
  exitFunction();

private:
  static AgentImpl* agent;
};

#endif  // _EXITCLASS_HH_
