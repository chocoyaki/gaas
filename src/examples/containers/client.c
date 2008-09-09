#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"
#include "DIET_Dagda.h"

int
usage(char* cmd)
{
	printf("Usage : %s <file.cfg> <service_name> <time>\nExample : %s client.cfg service1 100\n",cmd,cmd);
        printf("Warning: file /tmp/logo_diet.jpg MUST EXIST!\n");
	return 1;
}

int
main(int argc, char* argv[])
{
	long sleepTime;
        char *path1 = (char*) "/tmp/logo_diet.jpg";
	long *outsleepTime = NULL;
	char * service_name = NULL;
        if(argc == 4){
		service_name = argv[2];
		sleepTime = (long) atoi(argv[3]);
	} else
		return usage(argv[0]);

	if (diet_initialize(argv[1], argc, argv)) {
	  fprintf(stderr, "DIET initialization failed !\n");
	  return 1;
	}
	diet_profile_t* profile1;
        char * ID1;
        char * ID2;
        char * ID3;
        char * ID4;
	diet_container_t content1, content2;
        long *outsleepTime1 = NULL;
        long *outsleepTime2 = NULL;
        char *path2 = NULL;
        char *path3 = NULL;
        profile1 = diet_profile_alloc(service_name,0,1,1);
        diet_scalar_set(diet_parameter(profile1,0), &sleepTime,DIET_VOLATILE, DIET_LONGINT);
        // set INPUT container
        printf("PUT first element on platform (scalar)\n");
        dagda_put_scalar(&sleepTime, DIET_LONGINT, DIET_PERSISTENT, &ID1);
        printf("PUT second element on platform (file)\n");
        dagda_put_file(path1, DIET_PERSISTENT, &ID2);
        printf("Create CHILD container\n");
        dagda_create_container(&ID3);
        printf("Add container element\n");
        dagda_add_container_element(ID3, ID1, 0);
        printf("Add container element\n");
        dagda_add_container_element(ID3, ID2, 1);
        printf("Create PARENT container\n");
        dagda_create_container(&ID4);
        printf("Add container element\n");
        dagda_add_container_element(ID4, ID3, 0);
        printf("Add PARENT container to profile\n");
        diet_use_data(diet_parameter(profile1,1), ID4);
        printf("Start DIET CALL\n");
	if (diet_call(profile1) ){
		return 1;
	}
        printf("DIET CALL finished\n");
        printf("Get OUTPUT PARENT container\n");
        dagda_get_container(ID4);
        printf("Get PARENT container element list\n");
        dagda_get_container_elements(ID4, &content1);
        printf("Get CHILD container element list\n");
        if (content1.size == 1) {
          dagda_get_container_elements(content1.elt_ids[0], &content2);
          printf("Get CHILD elements\n");
          if (content2.size == 4) {
            dagda_get_scalar(content2.elt_ids[0],&outsleepTime1, NULL);
            dagda_get_file(content2.elt_ids[1],&path2);
            dagda_get_scalar(content2.elt_ids[2],&outsleepTime2, NULL);
            dagda_get_file(content2.elt_ids[3],&path3);
            printf("Container contains: %ld, %s, %ld, %s\n", *outsleepTime1, path2, *outsleepTime2, path3);
          } else {
            printf("ERROR: OUTPUT CHILD container does not contain expected nb of elements\n");
          }
        } else {
          printf("ERROR: OUTPUT PARENT container does not contain expected nb of elements\n");
        }
        printf("Deleting persistent data\n");
        dagda_delete_data(ID4);
        dagda_delete_data(content1.elt_ids[0]);
	diet_profile_free(profile1);
	return 0;
}
