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


int ouput_same_as_input(diet_profile_t *pb) {
	int last_out = pb->last_out;

	char* run_dir;
	diet_string_get(diet_parameter(pb, last_out - 1), &run_dir, NULL);

	//output-dir
	char* path = strdup(run_dir);

	diet_string_set(diet_parameter(pb, last_out), path, DIET_PERSISTENT_RETURN);
	//printf("%d, called\n", last_out);
}






main(int argc, char *argv[]) {
	printf("launch %s...\n", argv[0]);


	if (argc < 4) {
		printf("usage : %s cfg ips_file_path user_name\n", argv[0]);
		exit(0);
	}


	printf("argument number: Ok\n");

	std::string d_cmd = "cat ";
	d_cmd.append(argv[2]);
	int env = system(d_cmd.c_str());

	/* Initialize table with maximum 20 service */
	diet_service_table_init(20);

	SeDCloudMachinesActions *actions = new SeDCloudMachinesActions(argv[2], argv[3]);


	printf("creation of SeDCloud\n");
	SeDCloud::create(actions);


	SeDCloud::get()->service_table_add("grafic1", 2, 3, NULL, "", RAMSES_HOME, "call-grafic1", "", pathsTransferMethod, NULL, ouput_same_as_input);
	SeDCloud::get()->service_table_add("ramses3d", 2, 3, NULL, "", RAMSES_HOME, "call-ramses3d", "", pathsTransferMethod, NULL, ouput_same_as_input);
	SeDCloud::get()->service_table_add("halomaker", 1, 2, NULL, "", RAMSES_HOME, "call-halomaker", "", pathsTransferMethod, NULL, ouput_same_as_input);
	SeDCloud::get()->service_table_add("treemaker", 1, 2, NULL, "", RAMSES_HOME, "call-treemaker", "", pathsTransferMethod, NULL, ouput_same_as_input);
	SeDCloud::get()->service_table_add("galaxymaker", 1, 2, NULL, "", RAMSES_HOME, "call-galaxymaker", "", pathsTransferMethod, NULL, ouput_same_as_input);
	SeDCloud::get()->service_rsync_to_vm_add();
	SeDCloud::get()->service_mount_nfs_add();
	SeDCloud::get()->service_get_tarball_from_vm_add();

	diet_print_service_table();
	/* Launch the SeD: no return call */
	SeDCloud::launch(argc, argv);

	/* Dead code */
	return 0;
}
