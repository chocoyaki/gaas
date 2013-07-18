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


	service_wrapper_table_add("grafic1", 2, 3, RAMSES_HOME "/call-grafic1", NULL, ouput_same_as_input);
	service_wrapper_table_add("ramses3d", 2, 3, RAMSES_HOME "/call-ramses3d", NULL, ouput_same_as_input);
	service_wrapper_table_add("halomaker", 1, 2, RAMSES_HOME "/call-halomaker", NULL, ouput_same_as_input);
	service_wrapper_table_add("treemaker", 1, 2, RAMSES_HOME "/call-treemaker", NULL, ouput_same_as_input);
	service_wrapper_table_add("galaxymaker", 1, 2, RAMSES_HOME "/call-galaxymaker", NULL, ouput_same_as_input);



	int res = diet_SeD(argv[1], argc, argv);



	/* Dead code */
	return 0;
} /* main */
