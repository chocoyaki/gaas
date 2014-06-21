/**
 * @file test-sed-cloud-with-vm-destructor-service.cc
 *
 * @brief  Example server for the SeDCloud with a service which destroys VM
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

int
main(int argc, char *argv[]) {



    if (argc < 5) {
		printf("usage : %s cfg delta_cloud_url usr mdp\n", argv[0]);
		exit(0);
	}

    printf("In progress!\n");

	char* url = argv[2];
	char* usr = argv[3];
	char* mdp = argv[4];

//	Déclaration d'un ensemble de connections au serveur deltacloud (url,user,password)
	std::vector<CloudAPIConnection> ctx;
	CloudAPIConnection deltacloud_api(url, usr, mdp);
	ctx.push_back(deltacloud_api);


	/* Initialize table with maximum 10 service */
	diet_service_table_init(10);

//	Déclaration d'une ensemble d'actions disponibles pour le SeD
	SeDCloudActionsNULL* actions = new SeDCloudActionsNULL();

// Création d'une instance unique (Pattern Singleton)
	SeDCloud::create(actions);

// get() retourne l'instance
// Ajout du service de création de VM (lié à une instance particulière de deltacloud)
	SeDCloud::get()->service_homogeneous_vm_instanciation_add(&deltacloud_api);


	//SeDCloud::get()->service_cloud_federation_vm_destruction_by_ip_add(ctx);

//	Affichage de la liste des services déclarés
	diet_print_service_table();

	/* Launch the SeD: no return call */
//	Lit le fichier de confguration
//	Execute une fonction liée au démarrage du SeD (actions->perform_action_on_sed_launch();)
//	Exécute dietSeD
	SeDCloud::get()->launch(argc, argv);

	/* Dead code */
	return 0;
} /* main */
