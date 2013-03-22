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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SeD_deltacloud.hh"
#include <string>
#include <vector>

#include "Instance.hh"


main(int argc, char *argv[]) {
    std::string base_url = "http://localhost:3001/api";
    std::string username = "oneadmin";
    std::string password = "mypassword";
    std::string vm_user = "opennebula";
    int vms_count = 1;
    std::string profile = "debian-rc";
    std::vector<IaaS::Parameter> params;
    bool is_ip_private = false;

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

     SeDCloudVMLaunchedAtFirstSolveActions actions(image_id, base_url, username, password, vm_user,
                          vms_count, is_ip_private, params);

    SeDCloud::create(&actions);


    SeDCloud::get()->service_table_add("matrixAdd", 1, 2, NULL, local_path_of_binary, remote_path_of_binary);

    /* Launch the SeD: no return call */
    SeDCloud::launch(argc, argv);


    //SeDCloud::erase();

  /* Dead code */
  return 0;
} /* main */
