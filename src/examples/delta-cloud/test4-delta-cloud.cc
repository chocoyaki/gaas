#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Image.hh"
#include "Instance.hh"
#include "IaasInterface.hh"
#include "Iaas_deltacloud.hh"
#include "Tools.hh"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

using namespace std;
using namespace IaaS;




int main(int argc, const char *argv[]) {
	string base_url = "http://localhost:3001/api";
	string username = "oneadmin";
	string password = "mypassword";
	string vm_user = "opennebula";
	int vms_count = 1;
	string profile = "debian-rc";
	std::vector<Parameter> params;
	
	
	if (argc < 2) {
		printf("usage : %s imageId\n", argv[0]);
		exit(0);
	}

	string image_id = argv[1];

	params.push_back(Parameter(HARDWARE_PROFILE_ID_PARAM, profile));
	
  IaaS::IaasInterface * interface = new IaaS::Iaas_deltacloud(base_url, username, password);
	VMInstances* vm_instances = new VMInstances(image_id, vms_count, interface,
	vm_user, params);
	
	vm_instances->wait_all_instances_running();
	
	vm_instances->wait_all_ssh_connection(false);
	
	printf("SSH : OK\n");
	sleep(5);
	printf("destruction of VM instances\n");
	delete vm_instances;
	
	return 0;
}
