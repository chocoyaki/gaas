/****************************************************************************/
/* Java Local Agent source code (to run a LA inside a DIET/JXTA tree)       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2006/12/06 00:36:03  ecaron
 * add old logs in header
 *
 * Revision 1.4  2006/11/28 14:30:48  ctedesch
 * add old logs in header
 *
 * Revision 1.3  2006/11/28 14:29:22  ctedesch
 * Header
 *
 * Revision 1.2
 * date: 2006/10/25 21:33:52  eboix
 * Fixed compile time warnings. --- Injay2461
 *
 * Revision 1.1
 * date: 2004/06/11 15:45:39  ctedesch
 * add DIET/JXTA
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
