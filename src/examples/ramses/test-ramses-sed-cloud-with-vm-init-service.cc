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

char* output_dir;

int dummyoutput(diet_profile_t *pb) {
	int last_out = pb->last_out;

	char* path = strdup(output_dir);

	diet_string_set(diet_parameter(pb, last_out), path,  /*DIET_VOLATILE*/  DIET_PERSISTENT_RETURN);
	printf("%d, called\n", last_out);
}

main(int argc, char *argv[]) {



	if (argc < 3) {
		printf("usage : %s cfg output_dir\n", argv[0]);
		exit(0);
	}

	output_dir = argv[2];



	/* Initialize table with maximum 10 service */
	diet_service_table_init(10);

	SedCloudActionsNULL *actions = new SedCloudActionsNULL();



	SeDCloud::create(actions);


	SeDCloud::get()->service_table_add("grafic1", 2, 3, NULL, "", RAMSES_HOME, "call-grafic1", "", pathsTransferMethod, NULL, dummyoutput);
	SeDCloud::get()->service_table_add("ramses3d", 2, 3, NULL, "", RAMSES_HOME, "call-ramses3d", "", pathsTransferMethod, NULL, dummyoutput);
	SeDCloud::get()->service_table_add("halomaker", 1, 2, NULL, "", RAMSES_HOME, "call-halomaker", "", pathsTransferMethod, NULL, dummyoutput);
	SeDCloud::get()->service_table_add("treemaker", 1, 2, NULL, "", RAMSES_HOME, "call-treemaker", "", pathsTransferMethod, NULL, dummyoutput);
	SeDCloud::get()->service_table_add("galaxymaker", 1, 2, NULL, "", RAMSES_HOME, "call-galaxymaker", "", pathsTransferMethod, NULL, dummyoutput);
	SeDCloud::get()->service_homogeneous_vm_instanciation_add();
	SeDCloud::get()->service_rsync_to_vm_add();
	SeDCloud::get()->service_use_vm_add();

	/* Launch the SeD: no return call */
	SeDCloud::launch(argc, argv);

	//SeDCloud::erase();

	/* Dead code */
	return 0;
} /* main */
