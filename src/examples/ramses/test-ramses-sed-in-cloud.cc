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

#include <boost/lexical_cast.hpp>
#include "DIET_uuid.hh"
#include "SeD_deltacloud.hh"
#include <string>
#include <vector>

#include "Instance.hh"


#define RAMSES_HOME "/home/adi/scenario"


int ouput_same_as_input(ServiceWrapper* sw, diet_profile_t *pb) {
	int last_out = pb->last_out;

	char* run_dir;
	diet_string_get(diet_parameter(pb, last_out - 1), &run_dir, NULL);

	//output-dir
	char* path = strdup(run_dir);

	diet_string_set(diet_parameter(pb, last_out), path, DIET_PERSISTENT_RETURN);
	//printf("%d, called\n", last_out);
}


int add_tar_arg(ServiceWrapper* sw, diet_profile_t* pb) {
	sw->set_arg(0, "cvzf");

	boost::uuids::uuid uuid = diet_generate_uuid();
	std::string sz_uuid = "/tmp/";
	sz_uuid.append(boost::lexical_cast<std::string>(uuid));
	sz_uuid.append(".tar.gz");

	sw->set_arg(1, sz_uuid);
	sw->set_arg(2, 0);
}

int send_tar(ServiceWrapper* sw, diet_profile_t *pb) {
	const char* path_out = sw->get_arg(1).command_line_arg;

	diet_file_set(diet_parameter(pb, 1), path_out, DIET_PERSISTENT_RETURN);

}


main(int argc, char *argv[]) {



	if (argc < 2) {
		printf("usage : %s cfg\n", argv[0]);
		exit(0);
	}


	/* Initialize table with maximum 20 service */
	diet_service_table_init(20);

	std::vector<std::pair<diet_data_type_t, diet_base_type_t> > out_types;
	out_types.push_back(std::pair<diet_data_type_t, diet_base_type_t>(DIET_STRING, DIET_CHAR));





	service_wrapper_table_add("grafic1", 2, out_types, RAMSES_HOME "/call-grafic1", NULL, ouput_same_as_input);
	service_wrapper_table_add("ramses3d", 2, out_types, RAMSES_HOME "/call-ramses3d", NULL, ouput_same_as_input);
	service_wrapper_table_add("halomaker", 1, out_types, RAMSES_HOME "/call-halomaker", NULL, ouput_same_as_input);
	service_wrapper_table_add("treemaker", 1, out_types, RAMSES_HOME "/call-treemaker", NULL, ouput_same_as_input);
	service_wrapper_table_add("galaxymaker", 1, out_types, RAMSES_HOME "/call-galaxymaker", NULL, ouput_same_as_input);

	std::vector<std::pair<diet_data_type_t, diet_base_type_t> > no_out;
	service_wrapper_table_add("ls", 0, no_out, "ls", NULL, NULL);

	std::vector<std::pair<diet_data_type_t, diet_base_type_t> > tar_out;
	tar_out.push_back(std::pair<diet_data_type_t, diet_base_type_t> (DIET_FILE, DIET_CHAR));
	ServiceWrapper serviceWrapperTAR("tar", "tar", add_tar_arg, send_tar, 3);
	service_wrapper_table_add(serviceWrapperTAR, tar_out);

	diet_print_service_table();

	int res = diet_SeD(argv[1], argc, argv);



	/* Dead code */
	return 0;
} /* main */
