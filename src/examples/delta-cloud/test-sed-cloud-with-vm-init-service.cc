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

#include "SeD_deltacloud.hh"
#include <string>
#include <vector>

#include "Instance.hh"


main(int argc, char *argv[]) {



    if (argc < 2) {
		printf("usage : %s cfg\n", argv[0]);
		exit(0);
	}




  /* Initialize table with maximum 2 service */
  diet_service_table_init(2);

  SedCloudActionsNULL actions;

  SeDCloud::create(&actions);


  SeDCloud::get()->service_homogeneous_vm_instanciation_add();
  SeDCloud::get()->service_rsync_to_vm_add();

  /* Launch the SeD: no return call */
  SeDCloud::launch(argc, argv);

    //SeDCloud::erase();

  /* Dead code */
  return 0;
} /* main */
