/**
 * @file server.cc
 *
 * @brief  Server used to dynamically add and remove a service using a library
 *
 * @author  Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>

#include "DIET_server.h"
#include "DIET_Dagda.h"
#include "DynamicServiceMgr.hh"

/* begin function prototypes*/
int
serviceAdd(diet_profile_t *pb);
int
serviceRem(diet_profile_t *pb);
int
add_services();
/* end function prototypes*/

static DynamicServiceMgr *mgr = NULL;

template <typename T>
std::string
toString(T t) {
  std::ostringstream oss;
  oss << t;
  return oss.str();
}


/*
 * SOLVE FUNCTIONS
 */
int
serviceAdd(diet_profile_t *pb) {
  size_t arg_size = 0;
  char *path = NULL;
  int status = 0;
  struct stat buf;

  if (pb->pb_name) {
    std::cout << "## Executing " << pb->pb_name << std::endl;
  } else {
    std::cout << "## ERROR: No name for the service" << std::endl;
    return -1;
  }

  diet_file_get(diet_parameter(pb, 0), &path, NULL, &arg_size);
  std::cerr << "on " << path << " (" << (int) arg_size << ") ";
  if ((status = stat(path, &buf))) {
    return status;
  }
  /* Regular file */
  if (!(buf.st_mode & S_IFREG)) {
    return 2;
  }
  std::cout << "*** Got file: '" << path << "' ***" << std::endl;

  mgr->addServiceMgr(path);
  std::cout << "*** Service added ***" << std::endl;

  /* Print service table */
  diet_print_service_table();

  // /* Unlink file */
  // diet_free_data(diet_parameter(pb, 0));

  return 0;
} // serviceAdd


int
serviceRem(diet_profile_t *pb) {
  char *serviceName = NULL;

  if (pb->pb_name) {
    std::cout << "## Executing " << pb->pb_name << std::endl;
  } else {
    std::cout << "## ERROR: No name for the service" << std::endl;
    return -1;
  }

  diet_string_get(diet_parameter(pb, 0), &serviceName, NULL);

  mgr->removeServiceMgr(serviceName);

  /* Print service table */
  diet_print_service_table();

  // diet_free_data(diet_parameter(pb, 0));

  return 0;
} // serviceRem




// ////////////////////////////////////////////////////////////////////

/*
 * usage function:
 * tell how to launch the SeD
 */
int
usage(char *cmd) {
  std::cerr << "Usage: " << cmd << " <SeD.cfg>" << std::endl;
  return -1;
}



/*
 * add_service function:
 * declare the DIET's service
 */
int
add_services() {
  diet_profile_desc_t *profile = NULL, *profile2 = NULL;
  char add [] = "addDyn";
  char rem [] = "remDyn";


  diet_service_table_init(2);


  /* Set profile parameters: */
  profile = diet_profile_desc_alloc(add, 0, 0, 0);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);

  /* Add service to the service table */
  if (diet_service_table_add(profile, NULL, serviceAdd)) {
    return 1;
  }

  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);

  std::cout << "Service '" << add << "' added!" << std::endl;


  /* Set profile parameters: */
  profile2 = diet_profile_desc_alloc(rem, 0, 0, 0);
  diet_generic_desc_set(diet_param_desc(profile2, 0), DIET_STRING, DIET_CHAR);

  /* Add service to the service table */
  if (diet_service_table_add(profile2, NULL, serviceRem)) {
    return 1;
  }

  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile2);

  std::cout << "Service '" << rem << "' added!" << std::endl;

  return 0;
} // add_services



/*
 * MAIN
 */
int
main(int argc, char *argv[]) {
  int res;

  mgr = new DynamicServiceMgr();

  /* Add service */
  add_services();

  /* Print service table and launch daemon */
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);

  delete mgr;

  return res;
} // main
