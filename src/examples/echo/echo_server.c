/**
 * @file scalars_server.c
 *
 * @brief  DIET scalars example: a server for additions of all types of scalars.
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef __WIN32__
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DIET_server.h"

int sendEcho(diet_profile_t *pb) {
	char* name = NULL;
	char result[80] ="Bonjour ";

	diet_string_get(diet_parameter(pb, 0), &name, DIET_VOLATILE);
	printf("Received : %s\n", name);

	strcat(result, name);
	printf("Server : %s\n", result);
	size_t length = diet_string_get_desc(diet_parameter(pb,1))->length;
	printf("Server : %zx\n", length);
	diet_free_data(diet_parameter(pb,0));
	diet_string_get(diet_parameter(pb, 1), &name, DIET_VOLATILE);
	strcpy(name, result);

	diet_string_set(diet_parameter(pb, 1), name, DIET_VOLATILE);
	return 0;
}

int usage(char *cmd) {
	fprintf(stderr, "Usage: %s <file.cfg>\n", cmd);
	return 1;
}

/*
 * MAIN
 */

int main(int argc, char *argv[]) {
	size_t i;
	int res;

	diet_profile_desc_t *profile = NULL;

	if (argc < 2) {
		return usage(argv[0]);
	}

	diet_service_table_init(1);

	profile = diet_profile_desc_alloc("echo", 0, 0, 1);

	diet_generic_desc_set(diet_param_desc(profile, 0),
		DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 1),
		DIET_STRING, DIET_CHAR);

	if (diet_service_table_add(profile, NULL, sendEcho)) {
		return 1;

	}
	diet_profile_desc_free(profile);

	diet_print_service_table();
	res = diet_SeD(argv[1], argc, argv);
	/* Not reached */
	return res;
} /* main */
