/****************************************************************************/
/* JXTA Multi Master Agent source code                                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/* $LICENCE$                                                                */
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

public class JXTAMultiMA
{
  private DiscoveryService discoServ;
  private PipeService pipeServ;

  private OutputPipe outPipe;
  private InputPipe inPipe;
  private PipeAdvertisement pipeAdv; 

  private PeerGroup netPeerGroup = null;    
    
  public native int 
  startDIETAgent(String config_file);
  
  public native String[]
  submitJXTA(String pbName, String nbRow, String nbCol);
    
  static
  {
    /*
      System.out.println("Loading omniORB libraries... ");
      System.loadLibrary("omniDynamic4");
      System.loadLibrary("omnithread");
      System.loadLibrary("omniORB4");
      System.out.println("done.\n");
    */
    System.err.print("Loading MA library... ");
    System.loadLibrary("dietMasterAgent");
    System.err.println("done.");
    
  } // static load libraries

  public InputPipe 
  getInputPipe()
  {
      return (inPipe);
  }

  public static void
  main(String args[])
  {
    JXTAMultiMA mma = new JXTAMultiMA();
    System.out.println ("Starting DIET Agent...");
    int r;
    if ((r = mma.startDIETAgent(args[0])) != 0) {
      System.out.println("JXTA Agent: loading DIET Agent failed.");
      System.out.println("STOP.");
      System.exit(1);
    }
    System.out.println("JXTA Agent: DIET Agent is running.");

    System.out.println("Starting JXTA... ");    
    mma.startJxta();
    System.out.println("done.");
    mma.run();
    System.exit(0);
  } //main

  private void 
  startJxta()
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
	/* Build the input pipe advertisement */
      pipeAdv = (PipeAdvertisement) 
	  AdvertisementFactory.newAdvertisement
	  (PipeAdvertisement.getAdvertisementType());
      PipeID inPipeId = IDFactory.newPipeID(netPeerGroup.getPeerGroupID());
      pipeAdv.setName("DIET_MA");
      pipeAdv.setPipeID(inPipeId);
      pipeAdv.setType(PipeService.UnicastType);

      /* Create the input pipe to listen to request */
      inPipe = pipeServ.createInputPipe(pipeAdv);    
      System.out.println("MA input pipe: " + inPipeId.toString());
	    
      /* Publish the agent advertisement */
      PublishThread publishThread = new PublishThread(discoServ, pipeAdv);
      publishThread.start();
      System.out.println("Publishing MA advertisement...");
      
      /* Wait for client or agent query */
      while(true) {
        System.out.println("JXTA Agent: waiting for client or agent queries...");
	Message msg = inPipe.waitForMessage();
	System.out.println("received, launching a thread to process it.");
	ProcessThread processThread = new ProcessThread (this, msg, netPeerGroup);
	processThread.start();
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


class ProcessThread extends Thread
{
  private JXTAMultiMA mma;
  private Message msg;
    
  private PeerGroup netPeerGroup;
  private DiscoveryService discoServ;
  private PipeService pipeServ;
  
  private PipeAdvertisement pipeAdv;
  private OutputPipe outPipe;
  private InputPipe inPipe;

  ProcessThread (JXTAMultiMA _mma, Message _msg, PeerGroup _netPeerGroup)
  {
    mma = _mma;
    msg = _msg;
    netPeerGroup = _netPeerGroup;
    
    discoServ = netPeerGroup.getDiscoveryService();
    pipeServ = netPeerGroup.getPipeService();

    /* Build an input pipe to listen to client or agent responses */
    try {
      pipeAdv = (PipeAdvertisement)
	  AdvertisementFactory.newAdvertisement
	  (PipeAdvertisement.getAdvertisementType());
      PipeID inPipeId = IDFactory.newPipeID(netPeerGroup.getPeerGroupID());
      pipeAdv.setName("Normal");
      pipeAdv.setPipeID(inPipeId);
      pipeAdv.setType(PipeService.UnicastType);
      inPipe = pipeServ.createInputPipe(pipeAdv);
    }
    catch (Exception e) {
      e.printStackTrace();
    }
  } // ctor

  public void 
  run()
  {
    String pID = null;
    MimeMediaType mimeType = new MimeMediaType("text", "xml");
    try {
      String component = (msg.getMessageElement("origin")).toString();
      if (component.compareTo("client") == 0) {
        /* Call DIET Agent to get a SeD reference */
        String pbName = (msg.getMessageElement("pbName")).toString();
        String nbRow = (msg.getMessageElement("nbRow")).toString();
        String nbCol = (msg.getMessageElement("nbCol")).toString();
	  
        String[] resp = mma.callDIETAgent(pbName, nbRow, nbCol);
        /* if no SeD is found in the own agent's tree, forward to others MA */
        if (Array.getLength(resp) == 0) {
          resp = new String [30];
          Enumeration otherMA = null;
          try {
            otherMA = discoServ.getLocalAdvertisements
		(DiscoveryService.ADV, "Name", "DIET_MA");
            discoServ.getRemoteAdvertisements
		(null, DiscoveryService.ADV, "Name", "DIET_MA", 10, null);
          }
          catch (Exception e) {
            e.printStackTrace();
          }
          int ind = 0;
          while (otherMA.hasMoreElements()) {
	      //String pType = null;
	    String[] rsp = null;
            PipeAdvertisement otherPipeAdv = (PipeAdvertisement)otherMA.nextElement();
	    pID = otherPipeAdv.getPipeID().toString();
	    
            if (pID.compareTo (mma.getInputPipe().getPipeID().toString()) != 0) {   
              System.out.println("Agent found input pipe uuid: " + pID.toString());

              try {
                outPipe = pipeServ.createOutputPipe(otherPipeAdv, -1);
              }
              catch(Exception e) {
                e.printStackTrace();
              }

              /* Create messages to forward */
              String forwardPbName = (msg.getMessageElement("pbName")).toString();
              String forwardNbRow = (msg.getMessageElement("nbRow")).toString();
              String forwardNbCol = (msg.getMessageElement("nbCol")).toString();

              Message forwardMsg = new Message ();

	      InputStream ip;
	      InputStreamMessageElement smeforwardPipeAdv;
	      ip = pipeAdv.getDocument(mimeType).getStream();
	      smeforwardPipeAdv = new InputStreamMessageElement("pipeAdv", 
							   mimeType, 
							   ip,
							   null);
              StringMessageElement smeforwardPbName = 
		  new StringMessageElement("pbName", forwardPbName, null);
              StringMessageElement smeforwardNbRow = 
		  new StringMessageElement("nbRow", forwardNbRow, null);
              StringMessageElement smeforwardNbCol = 
		  new StringMessageElement("nbCol", forwardNbCol, null);
              StringMessageElement smeorigin = 
		  new StringMessageElement("origin", "agent", null);
					
              forwardMsg.addMessageElement(smeforwardPbName);
              forwardMsg.addMessageElement(smeforwardNbRow);
              forwardMsg.addMessageElement(smeforwardNbCol);
              forwardMsg.addMessageElement(smeorigin);
              forwardMsg.addMessageElement(smeforwardPipeAdv);
					
              /* forward query to agent */ 
              outPipe.send(forwardMsg);
              System.out.println ("Problem forwarded to agent...");

              Message rspMsg = inPipe.waitForMessage();
					
              /* Response received from agent, send back to client */
              Message.ElementIterator respIter = rspMsg.getMessageElements("SeDUuid");
					
              /* extracting SeD Uuid(s) from message */
              while (respIter.hasNext()) {
                resp[ind++] = (respIter.next()).toString();
              }
            }
          }
        }
	InputStream ip = null;
        ip = (msg.getMessageElement("pipeAdv")).getStream();
	PipeAdvertisement clientPipeAdv = (PipeAdvertisement)
	    AdvertisementFactory.newAdvertisement(mimeType, ip);
	outPipe = pipeServ.createOutputPipe(clientPipeAdv, 20000);
        
	/* Create the response message */
	Message responseMsg = new Message();
	
	/* insert uuids of server found */
	int i;
	for(i = 0; ((i < Array.getLength(resp)) && (resp[i] != null)); i++) {
	  
          StringMessageElement smeSeDUuid = 
	      new StringMessageElement("SeDUuid", resp[i], null);
	  responseMsg.addMessageElement(smeSeDUuid);
	  System.out.println ("JXTA Agent: SeD sent to client : " + resp[i]);
	}
	System.out.println(i + " SeD sent to client");
	
	/* Send the response message to client */    
	outPipe.send (responseMsg);
      }
      else /* Query forwarded by an agent */ {
			
        String pbName;
        String nbRow;
        String nbCol;
			
        pbName = (msg.getMessageElement("pbName")).toString();
        nbRow = (msg.getMessageElement("nbRow")).toString();
        nbCol = (msg.getMessageElement("nbCol")).toString();
			
        /* Call DIET Agent to submit to DIET */
        String[] resp = mma.callDIETAgent(pbName, nbRow, nbCol);
	
        //String agentUuid = (msg.getMessageElement("agentUuid")).toString();
        InputStream ip = null;
        ip = msg.getMessageElement("pipeAdv").getStream();
   
	PipeAdvertisement agentPipeAdv = (PipeAdvertisement)
	    AdvertisementFactory.newAdvertisement(mimeType, ip);
	outPipe = pipeServ.createOutputPipe(agentPipeAdv, 20000);

        /* build an output pipe to connect on the agent input pipe */
        outPipe = pipeServ.createOutputPipe(agentPipeAdv, 20000);

	/* Create the response message */
	Message responseMsg = new Message();
	
	/* insert uuids of server found */
	System.out.println("JXTA Agent: " + Array.getLength(resp) + 
			   " SeD sent to agent.");
	for(int i = 0; i < Array.getLength(resp); i++) {
          StringMessageElement smeSeDUuid = 
	      new StringMessageElement("SeDUuid", resp[i], null);
	  responseMsg.addMessageElement(smeSeDUuid);
	  System.out.println ("JXTA Agent: SeD sent to agent: " + resp[i]);
	}
			    
	/* Send the response message */
        outPipe.send (responseMsg);
      }
    }
    catch (Exception e) {
      e.printStackTrace();
    }
  } // run
} // ProcessThread


class PublishThread extends Thread
{
  private DiscoveryService disco;
  private PipeAdvertisement pipeAdv;

  PublishThread(DiscoveryService _disco, PipeAdvertisement _pipeAdv) 
  {
    disco = _disco;
    pipeAdv = _pipeAdv;
  }
    
  public void 
  run()
  {
    try {
      while(true) {
        /* publish the advertisement with lifetime = 20s */
	  long lifetime = 20000;
        disco.publish(pipeAdv, lifetime, lifetime);
        disco.remotePublish(pipeAdv, lifetime);

        /* Wait 15s and re-publish the advertisement */
        this.sleep(15000);
      }
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  } // run
} // PublishThread
