/**
* @file  CltDagNodeLauncher.hh
* 
* @brief   Launcher for client-side dag node  
* 
* @author  - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.2  2010/07/20 09:22:33  bisnard
 * changes for dietForwarder
 *
 * Revision 1.1  2009/09/25 12:36:18  bisnard
 * created new classes DagNodeLauncher & childs
 *
 ****************************************************************************/

#ifndef _CLTDAGNODELAUNCHER_
#define _CLTDAGNODELAUNCHER_

#include <string>
#include "DagNodeLauncher.hh"

class CltDagNodeLauncher : public DagNodeLauncher {
public:
  explicit CltDagNodeLauncher(DagNode * parent);

  virtual std::string
  toString() const;

  virtual void
  execNode();

  virtual void
  finishNode();
};

#endif
