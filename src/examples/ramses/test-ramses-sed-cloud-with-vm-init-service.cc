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



	if (argc < 2) {
		printf("usage : %s cfg\n", argv[0]);
		exit(0);
	}


	/* Initialize table with maximum 20 service */
	diet_service_table_init(20);




	SeDCloudActionsNULL *actions = new SeDCloudActionsNULL();



	SeDCloud::create(actions);



	SeDCloud::get()->service_homogeneous_vm_instanciation_add();
	SeDCloud::get()->service_homogeneous_vm_instanciation_with_keyname_add();
	SeDCloud::get()->service_launch_another_sed_add();
	SeDCloud::get()->service_rsync_to_vm_add();
	SeDCloud::get()->service_mount_nfs_add();
	SeDCloud::get()->service_get_tarball_from_vm_add();


	//for experiments
	service_time_solve_add();
	service_add_seq_in_data_xml_add();

	diet_print_service_table();

	//SeDCloud::get()->service_use_vm_add();
	//SeDCloud::get()->service_table_add("mount-nfs", 0, 1, NULL, "", RAMSES_HOME, "mount-nfs", "", pathsTransferMethod, NULL, dummyoutput);

	/* Launch the SeD: no return call */
	SeDCloud::launch(argc, argv);

	//SeDCloud::erase();

	/* Dead code */
	return 0;
} /* main */
