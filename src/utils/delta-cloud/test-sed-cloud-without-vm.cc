/**
 * @file test-sed-cloud.cc
 *
 * @brief  Example server for the SeDCloud without vm
 *
 * @author  Lamiel Toch (lamiel.toch@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SeDCloud.hh"
#include <string>
#include <vector>

#include "Instance.hh"
#include "VMsDeployment.hh"

int
main(int argc, char *argv[]) {



    if (argc < 6) {
		printf("usage : %s cfg username ip local_path_of_binary remote_path_of_binary\n", argv[0]);
		exit(0);
	}

    std::string username = argv[2];
    std::string ip = argv[3];
	std::string local_path_of_binary = argv[4];
    std::string remote_path_of_binary = argv[5];


  /* Initialize table with maximum 2 service */
  diet_service_table_init(2);

	std::vector<std::string> ips;
	ips.push_back(ip);

  SeDCloudMachinesActions actions(ips, username);




  SeDCloud::create(&actions);


  SeDCloud::get()->service_table_add("matrixAdd", 1, 2, NULL, local_path_of_binary, remote_path_of_binary);



  SeDCloud::get()->launch(argc, argv);




  return 0;
} /* main */
