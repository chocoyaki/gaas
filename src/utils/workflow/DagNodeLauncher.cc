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
 * Revision 1.2  2010/07/12 16:14:13  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
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
  myChosenServer = NULL;
}

void
DagNodeLauncher::setSeD(const char* sed,
                        const unsigned long reqID,
                        corba_estimation_t& ev)
{
  myChosenServer = CORBA::string_dup(sed);
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
	return NULL;
};

void
DagNodeLauncher::finishNode()
{
}
