/****************************************************************************/
/* JXTA SeD source code                                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

import java.io.*;
import java.net.*;
import java.lang.Thread;

import net.jxta.platform.*;
import net.jxta.document.*;
import net.jxta.endpoint.Message;
import net.jxta.pipe.*;
import net.jxta.id.*;
import net.jxta.peergroup.*;
import net.jxta.protocol.*;
import net.jxta.endpoint.*;
import net.jxta.util.ClassFactory;
import net.jxta.exception.*;
import net.jxta.document.MimeMediaType;

public class JXTASeD
{
  private PeerGroup netPeerGroup = null;
  private PipeService pipeServ = null;

  /* native methods allowing communication with DIET SeD */
  public native int 
  startDIETSeD(String config_file, String uuid, String pbs []);
  
  public native String 
  solveJXTA(String serialPb, String nbRow, String nbCol, 
	    String mat1, String mat2);
  
  public native void 
  ping();
    
    static
    {
	/* 
	System.out.println ("Loading omniORB libraries... ");
	System.loadLibrary("omniDynamic4");
	System.loadLibrary("omnithread");
	System.loadLibrary("omniORB4");
	System.out.println("done.\n");
	*/
	System.err.print("Loading SeDJNI library... ");
	System.loadLibrary("SeDJNI");
	System.err.println("done.");

    } // static libraries load
    

  public static void
  main(String args[])
  {

    if (args.length < 2) {
      System.out.println("Usage : java JXTASeDLoader <config_file> pbs");
      System.exit(0);
    }

    JXTASeD sed = new JXTASeD();

    System.out.println("Starting JXTA...");
    sed.startJxta();
    System.out.println("done.");
    sed.run(args);
    System.exit(0);

  } // main

  private void 
  startJxta()
  {
    try {
      /* Start jxta */
      System.setProperty("net.jxta.tls.password", "sedpwd00");
      System.setProperty("net.jxta.tls.principal", "sed");
      netPeerGroup = PeerGroupFactory.newNetPeerGroup();
    }
    catch (PeerGroupException e) {
      System.out.println("Unable to create the peer group.\n");
      e.printStackTrace();
      System.exit (0);
    }
    pipeServ = netPeerGroup.getPipeService();
  } // startJxta
    
  private void run(String args [])
  {
    try {
    /* Create an input pipe to wait for client pbs */
      PipeAdvertisement inPipeAdv = (PipeAdvertisement) 
	  AdvertisementFactory.newAdvertisement
	  (PipeAdvertisement.getAdvertisementType());
      PipeID inPipeId = IDFactory.newPipeID(netPeerGroup.getPeerGroupID());
      inPipeAdv.setPipeID(inPipeId);
      inPipeAdv.setType(PipeService.UnicastType);
      InputPipe inPipe = pipeServ.createInputPipe(inPipeAdv);
      System.out.println("Input pipe : " + inPipeId.toString());

      /* Start the DIET SeD process with Uuid */
      String pbs [] = new String [args.length - 1];
      for (int i = 1; i < args.length; i++) {
        pbs [i-1] = args [i];
      }
      int r;
      if ((r = this.startDIETSeD(args[0], inPipeId.toString(), pbs)) != 0) {
        System.out.println("JXTA SeD: loading DIET SeD returned " + r);
        System.out.println("STOP.");
        System.exit(1);
      }
      System.out.println("JXTA SeD: DIET SeD is running.");
	    
      /* Wait for clients call */
      System.out.println("JXTA SeD wait for problem messages..."); 
      while(true) {
        Message msg = inPipe.waitForMessage();
        System.out.print("Message Received : launching a thread to solve... ");
        SolveThread solveThread = new SolveThread(this, msg, pipeServ);
        solveThread.start();
        System.out.println("done.");
      }
    }
    catch (Exception e) {
      e.printStackTrace();
      System.exit(1);
    }
  } // run
} // SeD JXTA


class SolveThread extends Thread
{
  private PipeService pipeServ;
  private JXTASeD sed;
  private Message msg;

  SolveThread(JXTASeD _sed, Message _msg, PipeService _pipeServ)
  {
    sed = _sed;
    msg = _msg;
    pipeServ = _pipeServ;
  } // ctor

  public void run()
  {
    MimeMediaType mimeType = new MimeMediaType("text", "xml");
    try {
      /* Getting pb to be solved */
      InputStream ip = null;
      ip = msg.getMessageElement("pipeAdv").getStream();
      //String clientUuid = (msg.getMessageElement("clientUuid")).toString();
      String pb = (msg.getMessageElement("pbName")).toString();
      String nbRow = (msg.getMessageElement("nbRow")).toString();
      String nbCol = (msg.getMessageElement("nbCol")).toString();
      String mat1 = (msg.getMessageElement("mat1")).toString();
      String mat2 = (msg.getMessageElement("mat2")).toString();

      /* Ping DIET SeD via JXTA SeD */
      sed.ping();
      
      /* Call DIET SeD to solve the pb */
      System.out.println("JXTA SeD: Call DIET SeD solve on " + pb);
      String resultat = sed.solveJXTA(pb, nbRow, nbCol, mat1, mat2);
      
      /* Send back the response to client */
      System.out.println ("JXTA SeD : response : " + resultat);
      PipeAdvertisement clientPipeAdv = (PipeAdvertisement)
	  AdvertisementFactory.newAdvertisement(mimeType, ip);

      OutputPipe oppToClient = pipeServ.createOutputPipe(clientPipeAdv, 10000);
      Message respToClient = new Message();
      StringMessageElement smeresultat = new StringMessageElement
	  ("response", resultat, null);
      respToClient.addMessageElement(smeresultat);
      System.out.print("SeD JXTA: Send response to client...");
      oppToClient.send(respToClient);
      System.out.println("done.");
    }
    catch (Exception e) {
      e.printStackTrace();
      System.exit(1);
    }
  } // run
} // SolveThread
