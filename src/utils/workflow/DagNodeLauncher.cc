/****************************************************************************/
/* The virtual class used for dagNode execution                             */
/*                                                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/09/25 12:36:18  bisnard
 * created new classes DagNodeLauncher & childs
 *
 ****************************************************************************/

#include "DagNodeLauncher.hh"
#include "DagNode.hh"

DagNodeLauncher::DagNodeLauncher(DagNode * parent,
                                 DagScheduler * scheduler)
  : myNode(parent), myDagScheduler(scheduler), isSedDefined(false), myReqID(0)
{
  myChosenServer = SeD::_nil();
}

void
DagNodeLauncher::setSeD(const SeD_var& sed,
                        const unsigned long reqID,
                        corba_estimation_t& ev)
{
  myChosenServer = sed;
  myReqID = reqID;
  myEstimVect = ev; // COPY of estimation vector (required)
  isSedDefined = true;
}

void *
DagNodeLauncher::run()
{
  bool ok = false;

  execNode(ok);
  if (ok)   myNode->setAsDone(myDagScheduler);
  else      myNode->setAsFailed(myDagScheduler);
  finishNode();
};

void
DagNodeLauncher::finishNode()
{
}
