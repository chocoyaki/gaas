/****************************************************************************/
/* Java Local Agent source code (to run a LA inside a DIET/JXTA tree)       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

import java.net.*;
import java.io.IOException;
import java.io.*;
import java.util.*;

public class LA
{
  public native int 
  startDIETLA(String config_file);
    
  static {
    System.loadLibrary("dietLocalAgent");
  }

  public static void 
  main (String args[])
  {
    LA la = new LA();
    int ret = la.startDIETLA(args[0]);
    System.exit(0);
  } // main
} // LA
