/****************************************************************************/
/* CoRI: Simple example for EasyTester                                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Peter Frauenkron (Peter.Frauenkron@gmail.com)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2006/11/01 21:35:05  ecaron
 * Add missing header
 *
 * Revision 1.2  2006/01/25 21:39:06 pfrauenk
 * CoRI Example dmatmanips replaced by a simple fibonacci example
 *
 * Revision 1.1 2006/01/21 00:57:37 pfrauenk
 * CoRI : 2 examples are now available: a simple tester for cori_easy and
 *        one for testing the plug-in scheduler and cori
 *
 ****************************************************************************/
#include "DIET_server.h"

int 
main (int argc, char * argv []){
 diet_estimate_coriEasy_print();
}

