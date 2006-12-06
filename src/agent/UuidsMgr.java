/****************************************************************************/
/* UUIDs manager source code                                                */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/* $LICENCE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2006/12/06 18:01:17  ecaron
 * Header bug fix
 *
 * Revision 1.3  2006/11/28 16:25:11  ctedesch
 * add old logs in header
 *
 * Revision 1.2  2006/11/28 16:24:06  ctedesch
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

/**************************************************************************/
/* Class UuidsMgr :                                                       */
/*                                                                        */
/* class managing a list of Uuids able to solve the problem               */
/* described in a Request processed by an agent                           */
/**************************************************************************/
class UuidsMgr
{
    private String reqId;
    private String[] uuids;
    private int nbUuids;

    UuidsMgr (String _reqId)
    {
	reqId = new String (_reqId);
	uuids = new String [100];
	nbUuids = 0;
    } // ctor

    public synchronized String getReqId()
    {
	return(reqId);
    } // getreqId

    public synchronized String[] getUuids()
    {
	return (uuids);
    } // getUuids

    public synchronized int getNbUuids()
    {
	return(nbUuids);
    } // getNbUuids

    public synchronized boolean addUuid(String uuid)
    {
	if (nbUuids < 30)
	{
	    uuids[nbUuids++] = new String(uuid);
	    return(true);
	}
	System.out.println("Unable to add the SeD - vector is full");
	return(false);
    } // addUuid
    
} // UuidsMgr
