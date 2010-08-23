/****************************************************************************/
/* Example client for the Cloud Batch System                                */
/*                                                                          */
/* Author(s):                                                               */
/*    - Adrian Muresan (adrian.muresan@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/08/23 09:44:28  amuresan
 * Fixed cmake conflicting target problem
 *
 * Revision 1.2  2010/06/14 21:27:24  amuresan
 * Added header to cloud example files and client config file.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DIET_server.h"

void make_perf(diet_profile_t *pb)
{
      pb->walltime = 125 ;
      pb->nbprocs = 2 ; 
      pb->nbprocess = pb->nbprocs ;
}

int solve_cloud(diet_profile_t *pb)
{
    FILE * fi;
    char * result = (char*)malloc(9000 * sizeof(char));

  char*aux;
  char*script =
      "#!/bin/bash\n\n"
      "rm rez\n"
      "for h in $DIET_CLOUD_VMS\n"
      "do\n"
      "ssh root@$h -i $KEY 'ls -l /' >> rez\n"
      "done";

    aux = (char*)malloc(500*sizeof(char));
    sprintf(aux, "%s\0", script);

  /* Call performance prediction or not, but fields are to be fullfilled */ 
  make_perf(pb) ; 
         
  /* Submission */ 
  int res = diet_submit_parallel(pb, 
          NULL,
          aux) ;

  printf("submitted script '%s' and got result %d\n", aux, res);
  result[0] = '\0';

    /* Collect the result */
    fi = fopen("rez", "r");
    while(!feof(fi))
    {
	fgets(aux, 500, fi);
        strcat(result, aux);
    }
    if(fi != NULL)
        fclose(fi);

  /* Set OUT arguments */
    diet_string_set(diet_parameter(pb, 0), result, DIET_VOLATILE);

  return 0;
}
int main(int argc, char* argv[])
{
  diet_profile_desc_t *profile;
  diet_set_server_status( BATCH ) ;
  
  /* Initialize table with maximum 1 service */
  diet_service_table_init(1);
  
  /* Define smprod profile */
  profile = diet_profile_desc_alloc("cloud-demo", -1, -1, 0);
  diet_profile_desc_set_parallel(profile);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_STRING, DIET_CHAR);
  
  /* Add the service (the profile descriptor is deep copied) */
  diet_service_table_add(profile, NULL, solve_cloud);
  
  /* Free the profile descriptor, since it was deep copied. */
  diet_profile_desc_free(profile);
  
  /* Launch the SeD: no return call */
  diet_SeD("./SeD_cloud.cfg", argc, argv);
  
  /* Dead code */
  return 0;
}

