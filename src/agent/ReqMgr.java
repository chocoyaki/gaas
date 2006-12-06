/****************************************************************************/
/* Requests manager for JXTA Multi-MA source code                           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/* $LICENCE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2006/12/06 17:59:21  ecaron
 * Header bug fix
 *
 * Revision 1.3  2006/11/28 16:23:10  ctedesch
 * add old logs in header
 *
 * Revision 1.2  2006/11/28 16:21:47  ctedesch
 * header
 *
 * Revision 1.1
 * date: 2004/08/27 16:28:18  ctedesch
 * - Use of the asynchronous PIF scheme for propagation of the requests inside
 * the DIET J multi-hierarchy
 * - DIET/JXTA -> DIET J
 * - Change the JXTA examples scripts to build a whole multi-hierarchy
 ****************************************************************************/
import java.io.*;


/***************************************************************************/
/* class Request :                                                         */
/*                                                                         */
/* A Request contains an Id, Id of the father (agent that first forwarded  */
/* the request) and the list of the neighbours (agents to which this agent */
/* forward the request) The number of neighbours is at max 30.             */
/***************************************************************************/

class Request
{
    private String reqId;
    private String fatherUuid;
    private String[] neighboursUuids;
    private int nbNeighbours;

    Request (String _reqId, String _fatherUuid)
    {
	reqId = new String (_reqId);
	fatherUuid = new String(_fatherUuid);
	neighboursUuids = new String [30];
	nbNeighbours = 0;
    } // ctor

    public String getReqId()
    {
	return(reqId);
    } // getreqId

    public String getFatherUuid()
    {
	return(fatherUuid);
    } // getFatheruuid

    public String[] getNeighboursUuids()
    {
	return (neighboursUuids);
    } // getNeighboursUuids

    public int getNbNeighbours()
    {
	return(nbNeighbours);
    } // getNbNeighbours

    public boolean addNeighbour(String neighbourId)
    {
	if (nbNeighbours < 30)
	{
	    neighboursUuids[nbNeighbours++] = new String(neighbourId);
	    return(true);
	}
	System.out.println("Unable to add the neighbour - vector is full");
	return(false);
    } // addNeighbour

} // Request

/**************************************************************************/
/* Class Requests :                                                       */
/*                                                                        */
/* class managing the list of the requests processed by an agent          */
/**************************************************************************/
class ReqMgr
{
    private Request[] reqs;
    private int reqNb;

    ReqMgr()
    {
	reqNb = 0;
	reqs = new Request [1000] ;
    } // ctor

    public synchronized boolean isEmpty()
    {
	return(reqNb == 0);
    } // isEmpty

    public synchronized boolean isFull()
    {
	return(reqNb == 1000);
    } // isFull

    public synchronized int getReqNb()
    {
	return (reqNb);
    } // getReqNb

    public synchronized boolean isElement(String _reqId)
    {
	for (int i = 0; i < reqNb; i++)
	{
	    if ((reqs[i].getReqId()).compareTo(_reqId) == 0)
	    {
		return (true);
	    }
	}
	return(false);
    } // isElement

    public synchronized boolean addElement (Request _req)
    {
	if (!isFull())
	{
	    reqs[reqNb++] = new Request (_req.getReqId(), 
					 _req.getFatherUuid());
	    return(true);
	}
	System.out.println("Unable to add the request - vector is full");
	return(false);
    } // addElement

    public synchronized Request getElement (int i)
    {
	return (reqs[i]);
    } // getElement(int)

    /* to be tested */
    public synchronized Request getElement (String id)
    {
	for(int ct = 0; ct < reqNb; ct++)
	{
	    if ((reqs[ct].getReqId()).compareTo(id) == 0)
	    {
		return(reqs[ct]);
	    }
	}
	return (null);
    } // getElement(string)
} // ReqMgr
