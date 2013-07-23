#include <stdio.h>
#include <stdlib.h>
#include "DIET_client.h"
#include <string>

/* inputs:
 * <config_filename>
 * <wf_xml_file>
 */

int destroy_vms(const char* ips_file_path, int select_private_ip) {
	//destruction of vms

	diet_profile_t* profile_destroy_vm;

	profile_destroy_vm = diet_profile_alloc( "vm_destruction_by_ip", 4, 4, 4);
	diet_string_set(diet_parameter(profile_destroy_vm, 0), "http://localhost:3001/api", DIET_VOLATILE);
	diet_string_set(diet_parameter(profile_destroy_vm, 1), "oneadmin", DIET_VOLATILE);
	diet_string_set(diet_parameter(profile_destroy_vm, 2), "passoneadmin", DIET_VOLATILE);
	diet_file_set(diet_parameter(profile_destroy_vm, 3), ips_file_path, DIET_VOLATILE);
	diet_scalar_set(diet_parameter(profile_destroy_vm, 4), &select_private_ip, DIET_VOLATILE, DIET_INT);
	int env = diet_call(profile_destroy_vm);

	diet_profile_free(profile_destroy_vm);

	return env;
}

int main(int argc, char ** argv) {
  diet_wf_desc_t * profile;
  diet_profile_t* profile_get_tarball_from_vm;
  int env;

  char * master_ip = NULL;
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
   /*

    printf("Workflow successfuly run\n");
    //diet_wf_string_get(profile, "node-galaxymaker#out-dir", &output_dir);
    diet_wf_string_get(profile, "copy-to-machine#ip", &master_ip);




	if(master_ip != NULL) {
      printf("Got: %s\n", master_ip);

	  char* ramses_ips_file_path;
	  char* nfs_ip_file_path;
	  size_t size;



		char* pathTGZ;
		diet_wf_file_get(profile, "node-get-results#file-tgz", &size, &pathTGZ);
		printf("tar.gz is located in %s and its size is %zd\n", pathTGZ, size);

		//DEMO PARIS
		std::string cmd = "/home/lamiel/ramses/demo-paris.sh ";
		cmd.append(pathTGZ);
		env = system(cmd.c_str());



		//destroy nfs vms
		diet_wf_file_get(profile, "node-init-vm-nfs-server#ips", &size, &nfs_ip_file_path);
		env = destroy_vms(nfs_ip_file_path, 0);
		printf("destruction of nfs-server vm : %s\n", env ? "fails" : "success");



		//destroy ramses vms
		diet_wf_file_get(profile, "node-init-vms#ips", &size, &ramses_ips_file_path);
		env = destroy_vms(ramses_ips_file_path, 0);
		printf("destruction of ramses vms : %s\n", env ? "fails" : "success");





    } else {
      printf("Did not get a result!\n");
    }


	*/


  } else {
    printf("Workflow execution failed\n");
  }

  diet_wf_free(profile);


  return 0;
}
