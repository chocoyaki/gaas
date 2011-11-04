/**
 * @file service.hh
 *
 * @brief  Service example for dynamic loading. This is only a 'Hello world'
 *
 * @author  Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef SERVICE_HH
#define SERVICE_HH

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

#endif /* ifndef SERVICE_HH */
