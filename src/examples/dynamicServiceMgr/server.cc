/****************************************************************************/
/* Server used to dynamically add and remove a service using a library      */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Depardon (benjamin.depardon@ens-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/01/14 10:58:09  bdepardo
 * Changes to compile with gcc 4.4
 *
 * Revision 1.1  2008/12/08 15:32:43  bdepardo
 * Added an example to dynamically load a service given a library:
 * the library is sent by the client, and the SeD loads it and uses the new
 * service(s) added.
 *
 *
 ****************************************************************************/

#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>

#include "DIET_server.h"
#include "DIET_Dagda.h"
#include "DynamicServiceMgr.hh"

/* begin function prototypes*/
int serviceAdd(diet_profile_t *pb);
int serviceRem(diet_profile_t *pb);
int add_services();
/* end function prototypes*/

static DynamicServiceMgr *mgr = NULL;

template <typename T>
std::string toString( T t )
{
  std::ostringstream oss;
  oss << t;
  return oss.str();
}


/*
 * SOLVE FUNCTIONS
 */
int
serviceAdd(diet_profile_t* pb)
{
  size_t arg_size  = 0;
  char* path = NULL;
  int status = 0;
  struct stat buf;

  if (pb->pb_name)
    std::cout << "## Executing " << pb->pb_name << std::endl;
  else {
    std::cout << "## ERROR: No name for the service" << std::endl;
    return -1;
  }
    
  diet_file_get(diet_parameter(pb,0), NULL, &arg_size, &path) ;
  std::cerr << "on " << path << " (" << (int) arg_size << ") ";
  if ((status = stat(path, &buf)))
    return status;
  /* Regular file */
  if (!(buf.st_mode & S_IFREG))
    return 2;
  std::cout << "*** Got file: '" << path << "' ***" << std::endl;

  mgr->addServiceMgr(path);
  std::cout << "*** Service added ***" << std::endl;

  /* Print service table */
  diet_print_service_table();

  //   /* Unlink file */
  //   diet_free_data(diet_parameter(pb,0));

  return 0;
}


int
serviceRem(diet_profile_t* pb)
{
  char* serviceName = NULL;

  if (pb->pb_name)
    std::cout << "## Executing " << pb->pb_name << std::endl;
  else {
    std::cout << "## ERROR: No name for the service" << std::endl;
    return -1;
  }
    
  diet_string_get(diet_parameter(pb,0), &serviceName, NULL);
  
  mgr->removeServiceMgr(serviceName);

  /* Print service table */
  diet_print_service_table();

  //   diet_free_data(diet_parameter(pb,0));

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
add_services()
{
  diet_profile_desc_t* profile = NULL, *profile2 = NULL;
  char add [] = "addDyn";
  char rem [] = "remDyn";


  diet_service_table_init(2);


  /* Set profile parameters: */
  profile = diet_profile_desc_alloc(add,0,0,0);
  diet_generic_desc_set(diet_param_desc(profile,0),DIET_FILE, DIET_CHAR);

  /* Add service to the service table */
  if (diet_service_table_add(profile, NULL, serviceAdd )) return 1;

  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);

  std::cout << "Service '" << add << "' added!" << std::endl;


  /* Set profile parameters: */
  profile2 = diet_profile_desc_alloc(rem,0,0,0);
  diet_generic_desc_set(diet_param_desc(profile2,0),DIET_STRING, DIET_CHAR);

  /* Add service to the service table */
  if (diet_service_table_add(profile2, NULL, serviceRem )) return 1;

  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile2);

  std::cout << "Service '" << rem << "' added!" << std::endl;

  return 0;
}



/*
 * MAIN
 */
int
main( int argc, char* argv[]) 
{
  int res;
  
  mgr = new DynamicServiceMgr();

  /* Add service */
  add_services();

  /* Print service table and launch daemon */
  diet_print_service_table();
  res = diet_SeD(argv[1],argc,argv);

  delete mgr;

  return res;
}

