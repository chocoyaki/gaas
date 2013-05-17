#include <stdio.h>
#include <stdlib.h>
#include "DIET_client.h"


/* inputs:
 * <config_filename>
 * <wf_xml_file>
 */

int main(int argc, char ** argv) {
  diet_wf_desc_t * profile;

  char * output_dir = NULL;
  char * wf_xml_file;

  if (argc < 3) {
	printf("usage %s cfg xml\n", argv[0]);
	exit(-1);
  }

  if(diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET Initialization failed\n");
    return 1;
  }

  wf_xml_file = argv[2];


  profile = diet_wf_profile_alloc(wf_xml_file, "test-profile", DIET_WF_DAG);


  if(!diet_wf_call(profile)) {
    printf("Workflow successfuly run\n");
    diet_wf_string_get(profile, "node-galaxymaker#out-dir", &output_dir);
    if(output_dir != NULL) {
      printf("Got: %s\n", output_dir);
    } else {
      printf("Did not get a result!\n");
    }
  } else {
    printf("Workflow execution failed\n");
  }

  diet_wf_free(profile);

  return 0;
}
