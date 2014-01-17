/**
 * @file test-sed-cloud.cc
 *
 * @brief  Example server for the SeDCloud with a service which instantiates VM
 *
 * @author  Lamiel Toch (lamiel.toch@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SeDCloud.hh"
#include <string>
#include <vector>

#include "Instance.hh"


main(int argc, char *argv[]) {



    if (argc < 2) {
		printf("usage : %s cfg\n", argv[0]);
		exit(0);
	}




  /* Initialize table with maximum 10 service */
  diet_service_table_init(10);

  SeDCloudActionsNULL* actions = new SeDCloudActionsNULL();

  SeDCloud::create(actions);


  SeDCloud::get()->service_homogeneous_vm_instanciation_add();
  SeDCloud::get()->service_rsync_to_vm_add();
  SeDCloud::get()->service_table_add("date", 0, 0, NULL, "", "/bin", "date", "", pathsTransferMethod, NULL, NULL);

  /* Launch the SeD: no return call */
  SeDCloud::launch(argc, argv);

    //SeDCloud::erase();

  /* Dead code */
  return 0;
} /* main */
