/****************************************************************************/
/* Jxta dmat_manips example: a Jxta client for MatSUM and MatPROD           */
/*   problems (MatSUM is expanded to SqMatSUM and SqMatSUM_bis).            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/*  $LICENSE$                                                               */
/****************************************************************************/

import java.net.*;
import java.io.*;
import java.util.*;
import java.lang.reflect.Array;

import net.jxta.document.*;
import net.jxta.pipe.*;
import net.jxta.id.*;
import net.jxta.peergroup.*;
import net.jxta.protocol.*;
import net.jxta.discovery.*;
import net.jxta.exception.*;
import net.jxta.endpoint.*;
import net.jxta.peer.*;

public class JXTAClient
{

  /* peer informations */
  private PeerGroup netPeerGroup = null;
  private PeerID peerID = null;
    
  /* services */
  private DiscoveryService discoServ;
  private PipeService pipeServ;
    
  /* pipes informations */
  private OutputPipe outPipe;
  private InputPipe inPipe;
  private PipeAdvertisement pipeAdvIn;
  private PipeID inPipeId;

  /* problem to be submitted and solved */
  private String pbName;

  public static void 
  main(String args[])
  {
    if(args.length < 1) {
      System.out.println ("USAGE: java JXTAClientLoader Pb");
      System.exit(0);
    }

    JXTAClient client = new JXTAClient();
    client.startJxta();
    client.run(args[0]);
    
    System.exit(0);
  }
   
  private void 
  startJxta()
  {
    /* Start the JXTA platform */
    try {   
      System.out.println("Starting JXTA... ");
      System.setProperty("net.jxta.tls.password", "clientpwd");
      System.setProperty("net.jxta.tls.principal", "client");
      netPeerGroup = PeerGroupFactory.newNetPeerGroup();
      peerID = netPeerGroup.getPeerID();
      System.out.println("done.");
    }
    catch (PeerGroupException ex) {
      ex.printStackTrace();
      System.exit(1);
    }	
    /* Get the peer services */
    discoServ = netPeerGroup.getDiscoveryService();
    pipeServ = netPeerGroup.getPipeService();

  } // startJxta

  private void 
  run(String _pbName)
  {
    MimeMediaType mimeType = new MimeMediaType("text", "xml");
    pbName = new String(_pbName);

    Enumeration jxtaMA = null;
    PipeAdvertisement agentPipeAdv = null;

    System.out.print("Searching a DIET-MA advertisement... ");
    while (true) {
	try {
	  jxtaMA = discoServ.getLocalAdvertisements
	    (DiscoveryService.ADV, "Name", "DIET_MA");
	  if ((jxtaMA != null) && jxtaMA.hasMoreElements()) 
	    break;
	  discoServ.getRemoteAdvertisements
	    (null, DiscoveryService.ADV, "Name", "DIET_MA", 1, null);
	}
	catch (Exception e) {
         e.printStackTrace();    
	}
    }
    System.out.println("found.");

    System.out.print("Extracting Agent input pipe... ");
    agentPipeAdv = (PipeAdvertisement)jxtaMA.nextElement();
    System.out.println("done.");

    /* Create the pipe advertisement */
    try {
      pipeAdvIn = (PipeAdvertisement) 
	  AdvertisementFactory.newAdvertisement
	  (PipeAdvertisement.getAdvertisementType());
      inPipeId = IDFactory.newPipeID(netPeerGroup.getPeerGroupID());
      pipeAdvIn.setPipeID(inPipeId);
      pipeAdvIn.setType(PipeService.UnicastType);

      /* create the input pipe to receive the agent response */
      inPipe = pipeServ.createInputPipe(pipeAdvIn);  
    }
    catch (Exception e) {
      e.printStackTrace();
    }

    try {
	outPipe = pipeServ.createOutputPipe(agentPipeAdv, 20000);
    }
    catch (IOException e) {
      System.out.print("Unable to connect the MA, ");
      System.out.println("should have been stopped recently.");
      /* FIXME : try others MA ? */
      System.out.println("STOP.");
      System.exit(1);
    }

    /* submit */
    this.submitPb();

  } // run

  private void 
  submitPb()
  {
    MimeMediaType mimeType = new MimeMediaType("text", "xml");
    try {
      /* Create the description of the pb */
  
      String nbRow = "4";
      String nbCol = "4";

      int nbR = 4;
      int nbC = 4;
  
      String mat1 = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15";
      String mat2 = "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16";
    
      /* build a message containing this description */
      Message msg = new Message();

      InputStream ip = pipeAdvIn.getDocument(mimeType).getStream();
      InputStreamMessageElement smePipeAdv;
      smePipeAdv = new InputStreamMessageElement("pipeAdv", 
				      mimeType, 
				      ip, null);
      StringMessageElement smeorigin = 
	  new StringMessageElement("origin", "client", null);
      StringMessageElement smepbName = 
	  new StringMessageElement("pbName", pbName, null);
      StringMessageElement smenbRow = 
	  new StringMessageElement("nbRow", nbRow, null);
      StringMessageElement smenbCol = 
	  new StringMessageElement("nbCol", nbCol, null);

      msg.addMessageElement(smeorigin);
      msg.addMessageElement(smePipeAdv);
      msg.addMessageElement(smepbName);
      msg.addMessageElement(smenbRow);
      msg.addMessageElement(smenbCol);

      /* Send the message to the agent */
      System.out.print("Sending pb description to agent... ");
      outPipe.send(msg);
      System.out.println("done.");

      System.out.print("Waiting for the agent response... ");
      msg = inPipe.waitForMessage();
      System.out.println("done.");

      /* extract the SeD pipe advertisements */
      int nbSeD = 0;
      Message.ElementIterator iter = msg.getMessageElements("SeDUuid");
      
      /* Create an output pipe to send pb to SeD */	       
      if (iter.hasNext()) {
	System.out.print ("Creating output pipe to send pb to SeD... ");
	PipeAdvertisement pipeAdvOut = (PipeAdvertisement) 
	    AdvertisementFactory.newAdvertisement
	    (PipeAdvertisement.getAdvertisementType());
	URI SeDURI = null;
	boolean connected = false;
	
	/* Try to connect the SeDs, one by one */
	int k = 1;
	while (iter.hasNext() && !connected) {
	  SeDURI = new URI(iter.next().toString());
	  pipeAdvOut.setPipeID((PipeID)IDFactory.fromURI(SeDURI)); 
	  pipeAdvOut.setType(PipeService.UnicastType);
	  try {
	    connected = true;
	    outPipe = pipeServ.createOutputPipe(pipeAdvOut, 20000);
	  }
	  catch(IOException e) {
	    System.out.println("Unable to connect the SeD " + k);
	    connected = false;
	  }
	  k++;
	}

	if (!connected) {
	  System.out.println("All SeDs tried without success.");
	  System.out.println("STOP");
	  System.exit(1);
	}
	System.out.println ("done.");

        /* Create message containing the problem */
        System.out.print("Building the problem message for SeD... ");
        Message msgPb;

	ip = pipeAdvIn.getDocument(mimeType).getStream();
	InputStreamMessageElement pbsmePipeAdv;
	pbsmePipeAdv = new InputStreamMessageElement("pipeAdv", 
						     mimeType, 
						     ip,
						     null);
        StringMessageElement pbsmeorigin = 
	    new StringMessageElement("origin", "client", null);
        StringMessageElement pbsmepbName = 
	    new StringMessageElement("pbName", pbName, null);
        StringMessageElement pbsmenbRow = 
	    new StringMessageElement("nbRow", nbRow, null);
        StringMessageElement pbsmenbCol = 
	    new StringMessageElement("nbCol", nbCol, null);
        StringMessageElement pbsmemat1 = 
	    new StringMessageElement("mat1", mat1, null);
        StringMessageElement pbsmemat2 = 
	    new StringMessageElement("mat2", mat2, null);

        msgPb = new Message();

        msgPb.addMessageElement(pbsmePipeAdv);
        msgPb.addMessageElement(pbsmepbName);
        msgPb.addMessageElement(pbsmenbRow);
        msgPb.addMessageElement(pbsmenbCol);
        msgPb.addMessageElement(pbsmemat1);
        msgPb.addMessageElement(pbsmemat2);
       
        System.out.println("done.");

        /* Send Message to SeD */
        System.out.print("Sending pb to SeD... ");
        outPipe.send(msgPb);
        System.out.println("done.");
  
        System.out.println("Waiting the result of computation...");
        Message rspMsg;
        rspMsg = inPipe.waitForMessage();
        String rsp = (rspMsg.getMessageElement("response")).toString();
        System.out.println("Response received: " + rsp);
        System.out.println("i.e. : \n" );
        int cpt = 0;
	
        /* print the result of SeD computation */
        for (int i = 0; i < mat1.length(); i++) {
          System.out.print(mat1.charAt(i));
          if (mat1.charAt(i) == ' ') cpt++;
          if (cpt >= nbC) {
            System.out.print("\n");
            cpt = 0;
          }
        }
        System.out.print("\n");
        if (pbName.compareTo("MatSUM") == 0)
          System.out.println ("\t\t+");
        else
          System.out.println("\t\t*");

        cpt = 0;
        for (int i = 0; i < mat2.length(); i++) {
          System.out.print(mat2.charAt(i));
          if (mat2.charAt(i) == ' ') cpt++;
          if (cpt >= nbC) {
            System.out.print("\n");
            cpt = 0;
          }
        }
        System.out.println ("\n\t\t=");
        cpt = 0;
        for (int i = 0; i < rsp.length(); i++) {
          System.out.print(rsp.charAt(i));
          if (rsp.charAt(i) == ' ') cpt++;
          if (cpt >= nbC) {
            System.out.print("\n");
            cpt = 0;
          }
        }
        System.out.print("\n");
      }
      else
        System.out.println("No server to solve the problem");
    }
    catch (Exception e) {
      e.printStackTrace();
    }
  } // submit pb

} // JXTAClient
