/**
 * @file test-sed-cloud.cc
 *
 * @brief  Example server for the SeDCloud
 *
 * @author  Lamiel Toch (lamiel.toch@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include "SeDCloud.hh"
#include "Iaas_deltacloud.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "Instance.hh"


int
main(int argc, char *argv[]) {
  std::string base_url = "http://localhost:3001/api";
  std::string username = "oneadmin";
  std::string password = "passoneadmin";
  IaaS::pIaasInterface interface = IaaS::pIaasInterface(new IaaS::Iaas_deltacloud(base_url, username, password));

  //user name of vm
  std::string vm_user = "root";
  int vms_count = 1;

  //template of occi
  std::string profile = "nfs-server";
  std::vector<IaaS::Parameter> params;
  bool is_ip_private = false;


  /***
   * @cfg: configuration of SedCloud
   * @imageID: image ID
   * @local_path... : the path of a stored application
   * @remote_path : the path that the application should be installed in the VM.
   */
  if (argc < 5) {
    printf("usage : %s cfg imageId local_path_of_binary remote_path_of_binary\n", argv[0]);
    exit(0);
  }

  std::string image_id = argv[2];
  std::string local_path_of_binary = argv[3];
  std::string remote_path_of_binary = argv[4];

  params.push_back(IaaS::Parameter(HARDWARE_PROFILE_ID_PARAM, profile));


  /* Initialize table with maximum 2 service */
  diet_service_table_init(2);

  //pass the parameters to action for initiating the SedCloud
  SeDCloudVMLaunchedAtFirstSolveActions actions(image_id, interface, vm_user,
      vms_count, is_ip_private, params);

  //create a SedCloud using the above action
  SeDCloud::create(&actions);

  // add the service of this SedCloud to the service table of the SedCloud
  SeDCloud::get()->service_table_add("matrixAdd", 1, 2, NULL, local_path_of_binary, remote_path_of_binary);

  /* Launch the SeD: no return call */
  SeDCloud::get()->launch(argc, argv);


  //SeDCloud::erase();

  /* Dead code */
  return 0;
} /* main */
