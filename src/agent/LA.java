/****************************************************************************/
/* Java Local Agent source code (to run a LA inside a DIET/JXTA tree)       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

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
    la.startDIETLA(args[0]);
    System.exit(0);
  } // main
} // LA
