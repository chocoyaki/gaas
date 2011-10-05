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
 * Revision 1.3  2010/07/20 09:20:11  bisnard
 * integration with eclipse gui and with dietForwarder
 *
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
  : myNode(parent), myDagScheduler(scheduler), isSeDDefinedFlag(false), myReqID(0),
    isSuccessfulFlag(false)
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
  isSeDDefinedFlag = true;
}

//FIXME should return the hostname not the dataMgr name (temp for gwendia exp.)
string 
DagNodeLauncher::getSeDName() const
{
  if (isSeDDefined())
    return myChosenServer;
  else
    return "";
}


string 
DagNodeLauncher::toString() const
{
  string s = "DagNode Launcher (";
  s += myNode->getId();
  s += ")";
  return s;
}


void *
DagNodeLauncher::run()
{
  execNode();
  if (isSuccessfulFlag)   
    myNode->setAsDone(myDagScheduler);
  else      
    myNode->setAsFailed(myDagScheduler);
  finishNode();
  return NULL;
};

void
DagNodeLauncher::finishNode()
{
}
