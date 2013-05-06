/**
 * @file test-sed-cloud.cc
 *
 * @brief  Example server for the SeDCloud without vm : testing ramses
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


#define RAMSES_HOME "/home/adi/scenario"

static char* run_dir;

int dummyoutput(diet_profile_t *pb) {
	int last_out = pb->last_out;
	diet_string_set(diet_parameter(pb, last_out), run_dir, DIET_PERSISTENT_RETURN);

}

main(int argc, char *argv[]) {



	if (argc < 5) {
		printf("usage : %s cfg username ip run_dir\n", argv[0]);
		exit(0);
	}

	std::string username = argv[2];
	std::string ip = argv[3];
	std::string run_dir = argv[4];



	/* Initialize table with maximum 2 service */
	diet_service_table_init(2);

	SeDCloudWithoutVMActions actions(ip, username);


	SeDCloud::create(&actions);


	SeDCloud::get()->service_table_add("grafic1", 2, 3, NULL, "", RAMSES_HOME, "call-grafic1", pathsTransferMethod, NULL, dummyoutput);

	/* Launch the SeD: no return call */
	SeDCloud::launch(argc, argv);

	//SeDCloud::erase();

	/* Dead code */
	return 0;
} /* main */
