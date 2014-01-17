#include <stdio.h>
#include <stdlib.h>
#include "DIET_client.h"
#include <string>
#include <time.h>
#include <string.h>

/* inputs:
 * <config_filename>
 * <wf_xml_file>
 */



int destroy_vms(const char* ips_file_path, int select_private_ip) {
  //destruction of vms

  diet_profile_t* profile_destroy_vm;

  profile_destroy_vm = diet_profile_alloc( "vm_destruction_by_ip", 1, 1, 1);
  diet_file_set(diet_parameter(profile_destroy_vm, 0), ips_file_path, DIET_VOLATILE);
  diet_scalar_set(diet_parameter(profile_destroy_vm, 1), &select_private_ip, DIET_VOLATILE, DIET_INT);
  int env = diet_call(profile_destroy_vm);

  diet_profile_free(profile_destroy_vm);

  return env;
}

//with OPENSTACK

int main(int argc, char ** argv) {
  time_t time_start = time(NULL);
  time_t time_alloc_end;
  time_t time_end;

  diet_wf_desc_t * bootstrap_profile;
  diet_wf_desc_t * ramses_profile;

  diet_profile_t* profile_get_tarball_from_vm;
  int env;

  char * master_ip = NULL;
  char * wf_xml_file; //bootstrap
  char * ramses_dag; //the ramses dag grafic1->ramses3d->halo->tree->galaxy

  if (argc < 5) {
    printf("usage %s cfg bootstrap-file-path.xml ramses-dag.xml expe-file-path\n", argv[0]);
    exit(-1);
  }

  if(diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET Initialization failed\n");
    return 1;
  }

  wf_xml_file = argv[2];
  ramses_dag = argv[3];
  char* expe_file_path = argv[4];

  bootstrap_profile = diet_wf_profile_alloc(wf_xml_file, "bootstrap", DIET_WF_DAG);



  int result_diet_wf_call_bootstap = diet_wf_call(bootstrap_profile);

  if(result_diet_wf_call_bootstap == 0) {
    printf("bootstrap Workflow successfuly run\n");

    time_alloc_end = time(NULL);
  } else {
    printf("BOOTSTRAP - Workflow execution failed\n");
    exit(-1);
  }


  ramses_profile = diet_wf_profile_alloc(ramses_dag, "ramses", DIET_WF_DAG);
  int result_diet_wf_call_ramses = diet_wf_call(ramses_profile);

  if(result_diet_wf_call_ramses == 0) {
    printf("ramses Workflow successfuly run\n");
    time_end = time(NULL);
  } else {
    printf("RAMSES - Workflow execution failed\n");
    exit(-1);
  }


  time_t alloc_duration = time_alloc_end - time_start;
  time_t ramses_runtime = time_end - time_alloc_end;

  FILE* expe_file = fopen(expe_file_path, "w+");
  fprintf(expe_file, "%ld %ld", alloc_duration, ramses_runtime);
  fclose(expe_file);


  //DESTROY VMs

  if (result_diet_wf_call_bootstap == 0) {

    char* ramses_ips_file_path;
    char* nfs_ip_file_path;
    size_t size;


    //destroy nfs vms
    diet_wf_file_get(bootstrap_profile, "node-init-vm-nfs-server#ips", &size, &nfs_ip_file_path);
    env = destroy_vms(nfs_ip_file_path, 1);
    printf("destruction of nfs-server vm : %s\n", env ? "fails" : "success");



    //destroy ramses vms
    diet_wf_file_get(bootstrap_profile, "node-init-vms#ips", &size, &ramses_ips_file_path);
    env = destroy_vms(ramses_ips_file_path, 1);
    printf("destruction of ramses vms : %s\n", env ? "fails" : "success");
  }





  diet_wf_free(bootstrap_profile);
  diet_wf_free(ramses_profile);

  int w = 15;
  printf("sleeping %d s to unsure SedCloudMachinesActions are destroyed\n", w);
  sleep(w);

  return 0;
}
