/**
*  @file  simple_server.c
*  @brief  A Simple Client example
*  @author
*  @section Licence 
*    |LICENCE|
*/ 
#include <stdio.h>
#include "DIET_server.h"

/* SOLVE FUNCTION */
int service(diet_profile_t* pb)
{
   printf("### Entering service function.\n");
   int res = 0;
   long *sleepTime    = NULL;
   long *outsleepTime = NULL;
   diet_scalar_get(diet_parameter(pb, 0),
                    &sleepTime,
                    NULL);
   diet_scalar_get(diet_parameter(pb, 1),
                    &outsleepTime,
                    NULL);
   *outsleepTime = *sleepTime;
   printf("Time to Sleep =%ld ms", *outsleepTime);
   usleep(*outsleepTime * 1000);
   diet_scalar_desc_set(diet_parameter(pb, 1),  
                         outsleepTime);
   printf("### Exiting service function.\n");
   return res;
}

int usage(char* cmd)
{
   printf("Usage : %s <file.cfg> <service_name>\n", cmd);
   printf("\texample: %s SeD.cfg service1\n", cmd);
   return 1;
}

int main(int argc, char* argv[])
{
   if (argc < 3)
   {
      return usage(argv[0]);
   }

   diet_service_table_init(argc - 2);
   int i;
   for (i = 0; i < argc - 2; i++)
   {
      char *service_name = NULL;
      diet_profile_desc_t *profile = NULL;
      service_name = argv[ 2 + i ];
      profile = diet_profile_desc_alloc(service_name, 0, 0, 1);
      diet_generic_desc_set(diet_param_desc(profile, 0),
                             DIET_SCALAR,
                             DIET_LONGINT);
      diet_generic_desc_set(diet_param_desc(profile, 1),
                             DIET_SCALAR, 
                             DIET_LONGINT);
      diet_service_table_add(profile, NULL, service);
      diet_profile_desc_free(profile);
   }
   diet_print_service_table();
   int res = diet_SeD(argv[1], argc, argv);
   return res;
}

