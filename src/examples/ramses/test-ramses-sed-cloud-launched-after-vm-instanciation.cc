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
#include <iostream>
#include <unistd.h>

#include "SeD_deltacloud.hh"
#include <string>
#include <vector>

#include "Instance.hh"
#include "Tools.hh"










static std::string RAMSES_WRAPPERS_HOME;


int output_same_as_input(ServiceWrapper* sw, diet_profile_t *pb) {
	int last_out = pb->last_out;

	char* run_dir;
	diet_string_get(diet_parameter(pb, last_out - 1), &run_dir, NULL);

	//output-dir
	char* path = strdup(run_dir);

	diet_string_set(diet_parameter(pb, last_out), path, DIET_PERSISTENT_RETURN);
	//printf("%d, called\n", last_out);

	return 0;
}


//int set_common_arguments(ServiceWrapper* sw, diet_profile_t* pb) {
//	sw->set_arg(0, username);
//	sw->set_arg(1, ip);
//}

void add_common_services(const std::string& username, const std::string& ip) {
	std::vector<std::pair<diet_data_type_t, diet_base_type_t> > out_types;
	out_types.push_back(std::pair<diet_data_type_t, diet_base_type_t>(DIET_STRING, DIET_CHAR));

	ServiceWrapper grafic1_service_wrapper("grafic1", RAMSES_WRAPPERS_HOME + "/grafic1-wrapper", 4, NULL, output_same_as_input);
	grafic1_service_wrapper.set_arg(0, username);
	grafic1_service_wrapper.set_arg(1, ip);
	grafic1_service_wrapper.set_arg(2, 0); //level
	grafic1_service_wrapper.set_arg(3, 1); //working_dir
	service_wrapper_table_add(grafic1_service_wrapper, out_types);

#ifdef USE_LOG_SERVICE
	logVMServiceWrapped(grafic1_service_wrapper, ip.c_str(), username.c_str());
#endif

	ServiceWrapper halo_maker_service_wrapper("halomaker", RAMSES_WRAPPERS_HOME + "/2-args-wrapper", 4, NULL, output_same_as_input);
	halo_maker_service_wrapper.set_arg(0, username);
	halo_maker_service_wrapper.set_arg(1, ip);
	halo_maker_service_wrapper.set_arg(2, 0); //working_dir
	halo_maker_service_wrapper.set_arg(3, "./call-halomaker");
	service_wrapper_table_add(halo_maker_service_wrapper, out_types);

#ifdef USE_LOG_SERVICE
	logVMServiceWrapped(halo_maker_service_wrapper, ip.c_str(), username.c_str());
#endif

	ServiceWrapper tree_maker_service_wrapper("treemaker", RAMSES_WRAPPERS_HOME + "/2-args-wrapper", 4, NULL, output_same_as_input);
	tree_maker_service_wrapper.set_arg(0, username);
	tree_maker_service_wrapper.set_arg(1, ip);
	tree_maker_service_wrapper.set_arg(2, 0); //working_dir
	tree_maker_service_wrapper.set_arg(3, "./call-treemaker");
	service_wrapper_table_add(tree_maker_service_wrapper, out_types);

#ifdef USE_LOG_SERVICE
	logVMServiceWrapped(tree_maker_service_wrapper, ip.c_str(), username.c_str());
#endif

	ServiceWrapper galaxy_maker_service_wrapper("galaxymaker", RAMSES_WRAPPERS_HOME + "/2-args-wrapper", 4, NULL, output_same_as_input);
	galaxy_maker_service_wrapper.set_arg(0, username);
	galaxy_maker_service_wrapper.set_arg(1, ip);
	galaxy_maker_service_wrapper.set_arg(2, 0); //working_dir
	galaxy_maker_service_wrapper.set_arg(3, "./call-galaxymaker");
	service_wrapper_table_add(galaxy_maker_service_wrapper, out_types);

#ifdef USE_LOG_SERVICE
	logVMServiceWrapped(galaxy_maker_service_wrapper, ip.c_str(), username.c_str());
#endif


	diet_print_service_table();

	//std::cout << "<<<<<<<<<<<<<<<<<<<" << grafic1_service_wrapper << "\n";
}

int
main(int argc, char *argv[]) {
	printf("launch %s...\n", argv[0]);

	const char* config_xml;
	const char* username;
	if (argc < 3) {
		printf("usage : %s cfg config.xml\n", argv[0]);
		exit(0);
	}
	config_xml = argv[2];

	printf("argument number: Ok\n");
	/* Initialize table with maximum 20 service */
	diet_service_table_init(20);


	std::vector<std::string> ips;
	read_elements_from_xml(config_xml, ips, "ip");
	RAMSES_WRAPPERS_HOME = read_element_from_xml(config_xml, "wrappers_dir");
	std::cout<< "RAMSES WRAPPERS PATH=" << RAMSES_WRAPPERS_HOME << "\n";
	username = strdup(read_element_from_xml(config_xml, "user_name").c_str());
	std::cout<<"username=" << username << "\n";

	for(int i = 1; i < ips.size(); i++) {
		pid_t pid = fork();
		if (pid == 0) {
			//child process
			//we create a SedCloudMachinesActions
			std::string ip = ips[i];

			std::vector<std::string> one_ip;
			one_ip.push_back(ip);
			SeDCloudMachinesActions* machine_action = new SeDCloudMachinesActions(one_ip, username);
			printf("creation of SeDCloud\n");
			SeDCloud::create(machine_action);
			add_common_services(username, ip);

			diet_print_service_table();
			SeDCloud::launch(argc, argv);
			//dead code in the child

			exit(-1);
		}

	}

	SeDCloudMachinesActions *actions = new SeDCloudMachinesActions(ips, username);
	SeDCloud::create(actions);
	add_common_services(username, ips[0]);

	std::string ip_file_path = create_tmp_file("/tmp", ".txt");
	write_lines(ips, ip_file_path);
	std::vector<std::pair<diet_data_type_t, diet_base_type_t> > out_types;
	out_types.push_back(std::pair<diet_data_type_t, diet_base_type_t>(DIET_STRING, DIET_CHAR));
	ServiceWrapper ramses3d_service_wrapper("ramses3d", RAMSES_WRAPPERS_HOME + "/ramses3d-wrapper", 5, NULL, output_same_as_input);
	ramses3d_service_wrapper.set_arg(0, username);
	ramses3d_service_wrapper.set_arg(1, ips[0]);
	ramses3d_service_wrapper.set_arg(2, ip_file_path);
	ramses3d_service_wrapper.set_arg(3, 0); //number of processes
	ramses3d_service_wrapper.set_arg(4, 1); //working_dir
	service_wrapper_table_add(ramses3d_service_wrapper, out_types);

#ifdef USE_LOG_SERVICE
	logVMServiceWrapped(ramses3d_service_wrapper, ips[0].c_str(), username);
#endif


	diet_print_service_table();
	/* Launch the SeD: no return call */
	SeDCloud::launch(argc, argv);



	/* Dead code */
	return 0;
}
