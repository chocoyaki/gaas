/****************************************************************************/
/* Service example for dynamic loading.                                     */
/* This is only a 'Hello world' service                                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Depardon (benjamin.depardon@ens-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/12/08 15:32:43  bdepardo
 * Added an example to dynamically load a service given a library:
 * the library is sent by the client, and the SeD loads it and uses the new
 * service(s) added.
 *
 *
 ****************************************************************************/

#include "service2.hh"

#include <iostream>
#include <cstring>

#include "DIET_server.h"
#include "DIET_Dagda.h"


static const char service_name[] = "Dynamic_creation2";

static diet_profile_desc_t *
set_profile_desc()
{
  diet_profile_desc_t* profile = NULL;
  unsigned int pos = 0;

  /* Set profile parameters: */
  profile = diet_profile_desc_alloc(strdup(service_name),0,0,0);

  diet_generic_desc_set(diet_param_desc(profile,pos++),DIET_STRING, DIET_CHAR);
  return profile;
}

const char * const serviceName()
{
  return service_name;
}


/*
 * SOLVE FUNCTION
 */
int
service(diet_profile_t* pb)
{
  char *str;

  if (pb->pb_name)
    std::cout << "## Executing " << pb->pb_name << std::endl;
  else {
    std::cout << "## ERROR: No name for the service" << std::endl;
    return -1;
  }
    
  diet_string_get(diet_parameter(pb,0), &str, NULL);

  std::cout << "*****************************************************" << std::endl
	    << service_name << std::endl
	    << " Message: '" << str << "'" << std::endl
	    << "*****************************************************" << std::endl;

  return 0;
}


/*
 * add_service function:
 * declare the DIET's service
 */
int
addService()
{
  diet_profile_desc_t* profile = set_profile_desc();

  /* Add service to the service table */
  if (diet_service_table_add(profile, NULL, service )) return 1;

  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);

  return 0;
}



int
removeService()
{
  diet_profile_desc_t* profile = set_profile_desc();

  std::cout << "## Removing service " << profile->path << std::endl;
  diet_service_table_remove_desc(profile);
  std::cout << "## Service removed" << std::endl;

  diet_profile_desc_free(profile);

  return 0;
}

