/****************************************************************************/
/* JXTA Multi Master Agent source code                                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

import java.io.*;
import java.net.*;
import java.util.*;
import java.lang.reflect.Array;

import net.jxta.platform.*;
import net.jxta.document.*;
import net.jxta.endpoint.*;
import net.jxta.pipe.*;
import net.jxta.id.*;
import net.jxta.peergroup.*;
import net.jxta.util.ClassFactory;
import net.jxta.protocol.*;
import net.jxta.discovery.*;
import net.jxta.exception.*;
import net.jxta.document.MimeMediaType;

public class JXTAMultiMA implements PipeMsgListener
{
  private DiscoveryService discoServ;
  private PipeService pipeServ;

  private InputPipe inPipe;
  private PipeAdvertisement pipeAdv; 

  private InputPipe propagateInPipe;
  private PipeAdvertisement propagatePipeAdv;

  private PeerGroup netPeerGroup = null;    
  
  private ReqMgr reqMgr;

  public native int startDIETAgent(String config_file);
  public native String[] submitJXTA(String pbName, String nbRow, String nbCol);
    
  static
  {

    System.err.print("Loading MA library... ");
    System.loadLibrary("dietMasterAgent");
    System.err.println("done.");
    
  } // static load libraries

  public InputPipe getInputPipe() {return (inPipe);}

  public void pipeMsgEvent (PipeMsgEvent e)
  {
    Message msg = e.getMessage();

    if (!reqMgr.isElement(msg.getMessageElement("reqId").toString())) {
      
      System.out.println("MA JXTA: Request received from AGENT.");
      Request req = new Request
	  (msg.getMessageElement("reqId").toString(), 
	   msg.getMessageElement("fatherId").toString());
      reqMgr.addElement(req);
      
      /* Lauching a thread to process the query */
      ProcessQuery processQuery = new ProcessQuery 
	  (this, msg, netPeerGroup, propagatePipeAdv);
      processQuery.start();
    }
  } // pipeMsgEvent

  public static void main(String args[])
  {

    if (args.length < 1)
    {
      System.out.println("USAGE: java JXTAMultiMA <config_file>");
      System.exit(0);
    }

    JXTAMultiMA mma = new JXTAMultiMA();
    System.out.println ("MA JXTA: Loading MA DIET...");

    int r;
    if ((r = mma.startDIETAgent(args[0])) != 0) {
      System.out.println("MA JXTA: loading MA DIET failed.");
      System.out.println("STOP.");
      System.exit(1);
    }
    System.out.println("MA JXTA: MA DIET is running.");

    System.out.println("MA JXTA: Starting JXTA... ");    
    mma.startJxta();
    System.out.println("done.");
    mma.run();
    System.exit(0);
  } //main

  private void startJxta()
  {
    try {
     System.setProperty("net.jxta.tls.password", "agentpwd");
     System.setProperty("net.jxta.tls.principal", "agent");
     netPeerGroup = PeerGroupFactory.newNetPeerGroup();
    } 
    catch (PeerGroupException e) {
      e.printStackTrace();
      System.exit(0);
    }

    /* Get the discovery and pipe services */
    discoServ = netPeerGroup.getDiscoveryService();
    pipeServ = netPeerGroup.getPipeService();
	
  } // startJxta

  private void run()
  {
    MimeMediaType mimeType = new MimeMediaType("text", "xml");
    try {

      /* Searching the propagate pipe advertisement*/
      Enumeration enumPipeAdv = null;

      System.out.print("MA JXTA: Looking for the propagate pipe advertisement... ");
      int nbDisco = 3;
      while (--nbDisco > 0) {
        try {
	  discoServ.getRemoteAdvertisements
	      (null, DiscoveryService.ADV, "Name", "DIET_MA_propagate", 1, null);
	  Thread.sleep(2500);
	  enumPipeAdv = discoServ.getLocalAdvertisements
	      (DiscoveryService.ADV, "Name", "DIET_MA_propagate");
	  if (enumPipeAdv.hasMoreElements())
	      break;
	}
	catch (Exception e) {
	    e.printStackTrace();
	}
      }

      /* if not found, create it */
      if (enumPipeAdv.hasMoreElements()) {
	System.out.println("found.");
	propagatePipeAdv = (PipeAdvertisement)enumPipeAdv.nextElement();
      }
      else {
	System.out.println ("not found, creating it.");
	propagatePipeAdv = (PipeAdvertisement)
	    AdvertisementFactory.newAdvertisement
	    (PipeAdvertisement.getAdvertisementType());
	propagatePipeAdv.setName("DIET_MA_propagate");
	propagatePipeAdv.setPipeID(IDFactory.newPipeID(netPeerGroup.getPeerGroupID())); // usefull ?
	propagatePipeAdv.setType(PipeService.PropagateType);
      }

      /* build input pipe for propagations */
      System.out.print ("MA JXTA: Creating propagate input pipe... ");
      propagateInPipe = pipeServ.createInputPipe(propagatePipeAdv, this);
      System.out.println("done.");

      /* publish the propagate pipe advertisement */
      PublishThread propagatePipeAdvPublishThread =
	  new PublishThread(discoServ, propagatePipeAdv);
      propagatePipeAdvPublishThread.start();

      /* Build the input pipe advertisement */
      pipeAdv = (PipeAdvertisement) 
	  AdvertisementFactory.newAdvertisement
	  (PipeAdvertisement.getAdvertisementType());
      PipeID inPipeId = IDFactory.newPipeID(netPeerGroup.getPeerGroupID());
      pipeAdv.setName("DIET_MA");
      pipeAdv.setPipeID(inPipeId);
      pipeAdv.setType(PipeService.UnicastType);

      /* Create the input pipe to listen to request */
      System.out.print ("MA JXTA: Creating input pipe for clients... ");
      inPipe = pipeServ.createInputPipe(pipeAdv);
      System.out.println("done.");
	    
      /* Publish the agent advertisement */
      PublishThread publishThread = new PublishThread(discoServ, pipeAdv);
      publishThread.start();
      
      /* Create the request manager */
      reqMgr = new ReqMgr();

      /* Wait for queries */
      System.out.println("MA JXTA: Ready to process queries.");
      while(true) {
	Message msg = inPipe.waitForMessage();
	System.out.println("MA JXTA: Request received from a CLIENT");
	ProcessQuery processQuery = new ProcessQuery 
	    (this, msg, netPeerGroup, propagatePipeAdv);
	processQuery.start();
      }
    }
    catch(Exception e) {
	e.printStackTrace();
    }
  } // run

  public String[] callDIETAgent(String pbName, String nbRow, String nbCol)
  {
    String[] resp = this.submitJXTA(pbName, nbRow, nbCol);
    return (resp);
    
  } // call DIET Agent

} // JXTAMultiMA


/**
 *
 * This thread process a query
 *
 */
class ProcessQuery extends Thread implements PipeMsgListener
{
  private JXTAMultiMA mma;
  private Message msg;
    
  private PeerGroup netPeerGroup;
  private DiscoveryService discoServ;
  private PipeService pipeServ;
  
  private PipeAdvertisement pipeAdv;
  private InputPipe inPipe;

  private PipeAdvertisement propagatePipeAdv;
  private OutputPipe outPipe;

  private OutputPipe backOutPipe;

  private MimeMediaType mimeType;

  private UuidsMgr seds;
  private int nbForwards;
  private int nbResponses;
  private boolean allRspsReceived;

  ProcessQuery (JXTAMultiMA _mma, Message _msg, PeerGroup _netPeerGroup, PipeAdvertisement _propagatePipeAdv)
  {
    mma = _mma;
    msg = _msg;
    netPeerGroup = _netPeerGroup;
    
    allRspsReceived = false;
    nbResponses = 0;

    propagatePipeAdv = _propagatePipeAdv;

    discoServ = netPeerGroup.getDiscoveryService();
    pipeServ = netPeerGroup.getPipeService();

    /* Build an input pipe to listen to agent responses */
    try {
      pipeAdv = (PipeAdvertisement)
	  AdvertisementFactory.newAdvertisement
	  (PipeAdvertisement.getAdvertisementType());
      PipeID inPipeId = IDFactory.newPipeID(netPeerGroup.getPeerGroupID());
      pipeAdv.setPipeID(inPipeId);
      pipeAdv.setType(PipeService.UnicastType);
      inPipe = pipeServ.createInputPipe(pipeAdv, this);
    }
    catch (Exception e) {
      e.printStackTrace();
    }

    mimeType = new MimeMediaType("text", "xml");

  } // ctor

  public void run()
  {
      //String pID = null;
    try {
      if ((msg.getMessageElement("origin")).toString().compareTo("CLIENT") == 0) {

	/* Create a new UUIDs manager */
	String reqId = msg.getMessageElement("reqId").toString();
	seds = new UuidsMgr(reqId);

	/* Extract problem description */
	String pbName = (msg.getMessageElement("pbName")).toString();
        String nbRow = (msg.getMessageElement("nbRow")).toString();
        String nbCol = (msg.getMessageElement("nbCol")).toString();

        /* Submit */
        String[] resp = mma.callDIETAgent(pbName, nbRow, nbCol);
	for (int i = 0; i < Array.getLength(resp); i++)
	    seds.addUuid(resp[i]);

        /* Propagation if failure */
        if (Array.getLength(resp) == 0) {
          resp = new String [30];

          Enumeration otherMA = null;
	  System.out.print("MA JXTA: Looking for other MA(s)... ");
	  int nbTry = 3;
	  while (nbTry > 0) {
            try {
	      discoServ.getRemoteAdvertisements
		  (null, DiscoveryService.ADV, "Name", "DIET_MA", 10, null);
	      Thread.sleep(1000);
	      otherMA = discoServ.getLocalAdvertisements
		  (DiscoveryService.ADV, "Name", "DIET_MA");
	      if (otherMA != null && otherMA.hasMoreElements())
		break;
	      nbTry--;
	    }
	    catch (Exception e) {
	      e.printStackTrace();
	    }
	  }

	  if (otherMA.hasMoreElements()) {

	    String otherPipeIdStr = null;
	    PipeAdvertisement [] otherPipeAdvs = new PipeAdvertisement [100];
	    for (int i = 0; i < 100; i++)
	      otherPipeAdvs[i] = null;
	    
	    /* Extracting other MA from the discovery */
	    while (otherMA.hasMoreElements()) {
		String[] rsp = null;
	      PipeAdvertisement otherPipeAdv = (PipeAdvertisement)otherMA.nextElement();
	      otherPipeIdStr = otherPipeAdv.getPipeID().toString();

	      /* removing himself to discover only other MA(s) */
	      if (otherPipeIdStr.compareTo (mma.getInputPipe().
					    getPipeID().toString()) != 0) {
	      nbForwards++;
	      }
	    }

	    /* If other MAs found */
	    if (nbForwards > 0) {

	      /* Adding himself to have the right number of responses to wait */
	      nbForwards++;
	      System.out.println("found.");
	      /* get Request elements to be forwarded */
	      String fwdReqId = msg.getMessageElement("reqId").toString();
	      String fwdPbName = (msg.getMessageElement("pbName")).toString();
	      String fwdNbRow = (msg.getMessageElement("nbRow")).toString();
	      String fwdNbCol = (msg.getMessageElement("nbCol")).toString();
	      InputStream ip = pipeAdv.getDocument(mimeType).getStream();
	      InputStreamMessageElement smeFwdPipeAdv =
		  new InputStreamMessageElement("pipeAdv", mimeType, ip, 
						null);
	      StringMessageElement smeFwdReqId =
		  new StringMessageElement("reqId", fwdReqId, null);
	      
	      StringMessageElement smeFatherId =
		  new StringMessageElement("fatherId", 
					   mma.getInputPipe().getPipeID()
					   .toString(),
					   null);
	      StringMessageElement smeFwdPbName = 
		  new StringMessageElement("pbName", fwdPbName, null);
	      StringMessageElement smeFwdNbRow = 
		  new StringMessageElement("nbRow", fwdNbRow, null);
	      StringMessageElement smeFwdNbCol = 
		  new StringMessageElement("nbCol", fwdNbCol, null);
	      StringMessageElement smeOrigin = 
		  new StringMessageElement("origin", "AGENT", null);

	      Message fwdMsg = new Message();

	      fwdMsg.addMessageElement(smeFwdPbName);
	      fwdMsg.addMessageElement(smeFwdNbRow);
	      fwdMsg.addMessageElement(smeFwdNbCol);
	      fwdMsg.addMessageElement(smeOrigin);
	      fwdMsg.addMessageElement(smeFwdReqId);
	      fwdMsg.addMessageElement(smeFatherId);
	      fwdMsg.addMessageElement(smeFwdPipeAdv);
	      
	      try {
		outPipe = pipeServ.createOutputPipe
		    (propagatePipeAdv, 10000);
	      }
	      catch(Exception e) {
		  e.printStackTrace();
	      }
	      
	      /* send message */
	      outPipe.send(fwdMsg);
		
	      System.out.println("MA JXTA: Request forwarded to " + 
				 (nbForwards - 1) + " MA(s).");

	      /* Waiting all responses */
	      while(!allRspsReceived);
	      System.out.println("MA JXTA: All responses received");
	    }
	    else
		System.out.println("not found.");
	  }
        }

	/* Answer the client */
	InputStream ip = null;
        ip = (msg.getMessageElement("pipeAdv")).getStream();
	PipeAdvertisement clientPipeAdv = (PipeAdvertisement)
	    AdvertisementFactory.newAdvertisement(mimeType, ip);
	outPipe = pipeServ.createOutputPipe(clientPipeAdv, 20000);
        
	/* Create the response message */
	Message responseMsg = new Message();
	
	/* insert uuids of server found */
	String [] sedUuids = seds.getUuids();
	
	int i;
	for(i = 0; i < seds.getNbUuids(); i++) {
	  
          StringMessageElement smeSeDUuid = 
	      new StringMessageElement("SeDUuid", sedUuids[i], null);
	  responseMsg.addMessageElement(smeSeDUuid);
	}
	System.out.println("MA JXTA: " + i + " SeD(s) sent to client");
	
	/* Send the response message to client */    
	outPipe.send (responseMsg);
	outPipe.close();
      }
      else /* Query forwarded by an AGENT */ {

	/* Extract problem description */
	String pbName = (msg.getMessageElement("pbName")).toString();
	String nbRow = (msg.getMessageElement("nbRow")).toString();
	String nbCol = (msg.getMessageElement("nbCol")).toString();

	/* Submit */
	String[] resp = mma.callDIETAgent(pbName, nbRow, nbCol);

	/* Extract father pipe advertisement */
	InputStream ip3 = msg.getMessageElement("pipeAdv").getStream();
	PipeAdvertisement agentPipeAdv = (PipeAdvertisement)
	    AdvertisementFactory.newAdvertisement(mimeType, ip3);
	
	/* Binding father input pipe */
	backOutPipe = pipeServ.createOutputPipe(agentPipeAdv, 0);

	/* Create the response message */
	Message responseMsg = new Message();
	  
	/* insert uuids of server found */
	for(int i = 0; i < Array.getLength(resp); i++) {
	    StringMessageElement smeSeDUuid = 
		new StringMessageElement("SeDUuid", resp[i], null);
	    responseMsg.addMessageElement(smeSeDUuid);
	}

	/* Send the response message */
	backOutPipe.send (responseMsg);
	System.out.println ("MA JXTA: FEEDBACK - SENT");

        /* creating message to be propagated */
	String reqId2 = msg.getMessageElement("reqId").toString();
      
	InputStream ip = pipeAdv.getDocument(mimeType).getStream();
	InputStreamMessageElement smePipeAdv =
	    new InputStreamMessageElement("pipeAdv", mimeType, ip, null);
	StringMessageElement smeReqId =
	    new StringMessageElement("reqId", reqId2, null);
	StringMessageElement smePbName = 
	    new StringMessageElement("pbName", pbName, null);
	StringMessageElement smeNbRow = 
	    new StringMessageElement("nbRow", nbRow, null);
	StringMessageElement smeNbCol = 
	    new StringMessageElement("nbCol", nbCol, null);
	StringMessageElement smeOrigin = 
	    new StringMessageElement("origin", "AGENT", null);
	
	Message forwardMsg = new Message ();
	
	forwardMsg.addMessageElement(smeReqId);
	forwardMsg.addMessageElement(smePbName);
	forwardMsg.addMessageElement(smeNbRow);
	forwardMsg.addMessageElement(smeNbCol);
	forwardMsg.addMessageElement(smeOrigin);
	forwardMsg.addMessageElement(smePipeAdv);
	
	try {
	  /* create propagate output pipe */
	  outPipe = pipeServ.createOutputPipe
	      (propagatePipeAdv, 10000);
	  /* send message */
	  outPipe.send(forwardMsg);
	}
	catch(Exception e) {
	  e.printStackTrace();
	}
      }
    }
    catch (Exception e) {
      e.printStackTrace();
    }
  } // run

  public void pipeMsgEvent (PipeMsgEvent event)
  {

    Message rspMsg = event.getMessage();

    if (((msg.getMessageElement("origin")).toString().compareTo("CLIENT") == 0)) {
      /* FIXME : useful test because of reentrance */
      if (allRspsReceived == false) {
      
	Message.ElementIterator msgIter = rspMsg.getMessageElements("SeDUuid");      
	int nbSeDs = 0;
	while (msgIter.hasNext()) {
	  if(!seds.addUuid(msgIter.next().toString())) {
	    System.out.println("MA JXTA: Too many SeDs found!");
	    break;
	  }
	  nbSeDs++;
	}
	System.out.println("MA JXTA: " + nbSeDs + " SeD(s) added.");
	if (++nbResponses == nbForwards) {
	  allRspsReceived = true;
	}
      }
    }
    else {
      try {
	backOutPipe.send(rspMsg);
	System.out.println("MA JXTA: FEEDBACK - FORWARDED.");
      }
      catch (IOException ioe) {
	ioe.printStackTrace();
      }
    }
  } // pipeMsgEvent

} // ProcessQuery

/**
 *
 * Thred publishing advertisements with lifetime
 *
 */
class PublishThread extends Thread
{
  private DiscoveryService disco;
  private PipeAdvertisement pipeAdv;

  PublishThread(DiscoveryService _disco, PipeAdvertisement _pipeAdv) 
  {
    disco = _disco;
    pipeAdv = _pipeAdv;
  }
    
  public void run()
  {
    try {
      while(true) {
        /* publish the advertisement with lifetime = 15s every 7 seconds */
	long lifetime = 15000;
        disco.publish(pipeAdv, lifetime, lifetime);
        disco.remotePublish(pipeAdv, lifetime);
        this.sleep(7000);
      }
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  } // run
} // PublishThread
