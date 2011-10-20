#include <stdio.h>
#include <stdlib.h>  // for atoi
#include "DIET_client.h"

int usage(char* cmd)
{
   printf("Usage : %s <file.cfg> <service_name> <time>\n", cmd);
   printf("Example : %s client.cfg service1 100\n", cmd);
   return 1;
}

int main(int argc, char* argv[])
{
   long sleepTime;
   long *outsleepTime = NULL;
   char *service_name = NULL;
   if (argc == 4)
   {
      service_name = argv[2];
      sleepTime = (long) atoi(argv[3]);
   } else {
      return usage(argv[0]);
   }

   if (diet_initialize(argv[1], argc, argv))
   {
      printf("DIET initialization failed !\n");
      return 1;
   } 
   diet_profile_t* profile = diet_profile_alloc(service_name, 0, 0, 1);
   diet_scalar_set(diet_parameter(profile, 0),
                    &sleepTime,
                    DIET_VOLATILE,
                    DIET_LONGINT);
   diet_scalar_set(diet_parameter(profile, 1),
                    NULL,
                    DIET_VOLATILE,
                    DIET_LONGINT);
   if (diet_call(profile))
   {
      return 1;
   }
   diet_scalar_get(diet_parameter(profile, 1),
                    &outsleepTime,
                    NULL);
   printf("OUTSLEEPTIME with service1 = %ld \n", *outsleepTime);
   diet_profile_free(profile);
   return 0;
}
