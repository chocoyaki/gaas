/**
* @file service2.hh
* 
* @brief  Service example for dynamic loading. This is only a 'Hello world'  
* 
* @author  - Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.1  2008/12/08 15:32:43  bdepardo
 * Added an example to dynamically load a service given a library:
 * the library is sent by the client, and the SeD loads it and uses the new
 * service(s) added.
 *
 *
 ****************************************************************************/

#ifndef SERVICE2_HH
#define SERVICE2_HH

#include "DIET_server.h"


extern "C" {
  int
  service(diet_profile_t *pb);

  int
  addService();

  int
  removeService();

  const char * const
  serviceName();
}

#endif
