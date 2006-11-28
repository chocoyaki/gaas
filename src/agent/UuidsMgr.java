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
 * Revision 1.2  2006/11/28 16:24:06  ctedesch
 * header
 *
/****************************************************************************/
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
