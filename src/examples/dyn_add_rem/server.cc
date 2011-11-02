/**
* @file server.cc
* 
* @brief  Server used to dynamically add and remove a service
* 
* @author  - Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.4  2010/01/14 10:57:48  bdepardo
 * Changes to compile with gcc 4.4
 *
 * Revision 1.3  2009/10/18 09:15:11  bdepardo
 * CheckUsage
 *
 * Revision 1.2  2008/12/22 12:14:06  bdepardo
 * Corrected example in case Diet is compiled with Batch support.
 *
 * Revision 1.1  2008/11/18 10:18:13  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 *
 ****************************************************************************/

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>

#include "DIET_server.h"
#include "DIET_Dagda.h"

/* begin function prototypes*/
int service(diet_profile_t *pb);
int add_service(const char* service_name);
/* end function prototypes*/

static unsigned int NB = 1;

template <typename T>
std::string toString(T t)
{
  std::ostringstream oss;
  oss << t;
  return oss.str();
}


/*
 * SOLVE FUNCTION
 */
int
service(diet_profile_t* pb)
{
  int *nb;

  if (pb->pb_name)
    std::cout << "## Executing " << pb->pb_name << std::endl;
  else {
    std::cout << "## ERROR: No name for the service" << std::endl;
    return -1;
  }
    

  diet_scalar_get(diet_parameter(pb, 0), &nb, NULL);
  std::cout << "## Will create " << *nb << " services." << std::endl;


  for (int i = 0; i < *nb; i++) {
    add_service(std::string("dyn_add_rem_" + toString(NB++)).c_str());
  }

  std::cout << "## Services added" << std::endl;
  diet_print_service_table();

  /* Removing */
  std::cout << "## Removing service " << pb->pb_name << std::endl;
#ifdef HAVE_ALT_BATCH
  pb->parallel_flag = 1;
#endif
  diet_service_table_remove(pb);
  std::cout << "## Service removed" << std::endl;

  /* Print service table */
  diet_print_service_table();

  return 0;
}




//////////////////////////////////////////////////////////////////////

/*
 * usage function:
 * tell how to launch the SeD
 */
int
usage(char* cmd)
{
  std::cerr << "Usage: " << cmd << " <SeD.cfg>" << std::endl;
  return -1;
}



/*
 * add_service function:
 * declare the DIET's service
 */
int
add_service(const char* service_name)
{
  diet_profile_desc_t* profile = NULL;
  unsigned int pos = 0;


  /* Set profile parameters: */
  profile = diet_profile_desc_alloc(strdup(service_name), 0, 0, 0);

  /* setup scheduler */
  //   set_up_scheduler(profile);

  diet_generic_desc_set(diet_param_desc(profile, pos++), DIET_SCALAR, DIET_INT);

  /* Add service to the service table */
  if (diet_service_table_add(profile, NULL, service)) return 1;

  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);

  std::cout << "Service '" << service_name << "' added!" << std::endl;

  return 0;
}


int checkUsage(int argc, char ** argv) {
  if (argc != 2) {
    usage(argv[0]);
    exit(1);
  }
  return 0;
}

/*
 * MAIN
 */
int
main(int argc, char* argv[]) 
{
  int res;
  std::string service_name = "dyn_add_rem_0";

  checkUsage(argc, argv);

  /* Add service */
  diet_service_table_init(1);
  add_service(service_name.c_str());

  /* Print service table and launch daemon */
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return res;
}

