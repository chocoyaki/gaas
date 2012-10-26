/**
 * @file service2.hh
 *
 * @brief  Service example for dynamic loading. This is only a 'Hello world'
 *
 * @author  Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


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

#endif /* ifndef SERVICE2_HH */
