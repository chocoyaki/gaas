/**
 * @file Sed_deltacloud.cc
 *
 * @brief  DIET SeD_deltacloud class implementation
 *
 * @author  Lamiel TOCH (lamiel.toch@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include "SeD_deltacloud.hh"
#include <stdio.h>
#include "DIET_uuid.hh"
#include <fstream>
#include "Iaas_deltacloud.hh"
#include <libgen.h>

int add_seq_in_data_xml_solve(diet_profile_t *pb) {
	char* xml_in_path;
	char* lines_file_path;
	size_t size;
	char* tag_name;


	diet_file_get(diet_parameter(pb, 0), &xml_in_path, NULL, &size);
	diet_file_get(diet_parameter(pb, 1), &lines_file_path, NULL, &size);
	diet_string_get(diet_parameter(pb, 2), &tag_name, NULL);

	XmlDOMDocument* xml_in = read_xml_file(xml_in_path);

	if (xml_in == NULL) return -1;
	if (xml_in->get_element_count("data") <= 0) {
		std::cout << "ERROR: the XML input file must contains <data> ... </data>\n";
		return -1;
	}
	std::vector<std::string> lines;
	readlines(lines_file_path, lines);

	for(size_t i = 0; i < lines.size(); i++){
		xml_in->add_child_content("data", 0, tag_name, lines[i]);
	}

	std::string diet_tmp_file = create_tmp_file(pb, ".xml");
	xml_in->write(diet_tmp_file.c_str());
	diet_file_set(diet_parameter(pb, 3), diet_tmp_file.c_str(), DIET_PERSISTENT_RETURN);

	return 0;
}


void service_add_seq_in_data_xml_add() {
	diet_profile_desc_t* profile;
	profile = diet_profile_desc_alloc("add_seq_in_data_xml", 2, 2, 3);
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR); //the  data XML file
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR); //the lines file
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_STRING, DIET_CHAR); //the xml tag name around each line
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_FILE, DIET_CHAR); //the  resulting data xml file
	diet_service_table_add(profile, NULL, add_seq_in_data_xml_solve);

	diet_profile_desc_free(profile);
}


void service_time_solve_add() {
	diet_profile_desc_t* profile;
	profile = diet_profile_desc_alloc("time", 0, 0, 1);
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_STRING, DIET_CHAR); //dummy input
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_LONGINT); //time in seconds since 01/01/1970
	diet_service_table_add(profile, NULL, time_solve);

	diet_profile_desc_free(profile);
}


