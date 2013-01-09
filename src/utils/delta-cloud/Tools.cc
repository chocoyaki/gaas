/*
	Authors :
		- Lamiel Toch : lamiel.toch@ens-lyon.fr		
*/

#include "Tools.hh"
#include "Iaas_deltacloud.hh"
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

void deleteStringVector(std::vector<std::string*>& v){
	for(int i = 0; i < v.size(); i++) {
		delete v[i];
		v[i] = NULL;
	}
}

int test_ssh_connection(std::string ssh_user, std::string ip) {
	std::string cmd = "ssh "  + ssh_user + "@" + ip + " -o StrictHostKeyChecking=no 'ls'";
	std::cout << cmd << std::endl;
	int ret = system(cmd.c_str());
		
	return ret;	
}


int test_ssh_connection_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool is_private_ip){
	IaaS::Instance* instance = interf->get_instance_by_id(instance_id);
	
	std::string ip;
	
	if (is_private_ip) {
		ip = instance->private_ip;
	}
	else {
		ip = instance->public_ip;
	}
	
	int ret = ::test_ssh_connection(vm_user, ip);
	
	delete instance;
	
	return ret;
}

namespace IaaS {

/*Instance * create_one_vm_instance(std::string base_url, std::string user_name, std::string password,
	std::string vm_user, IaasInterface ** interf) {
		
}
*/




VMInstances::VMInstances(std::string image_id, int vm_count, std::string base_url, std::string user_name, std::string password,
	std::string _vm_user, const std::vector<Parameter>& params) {
	
	vm_user = _vm_user;
	
	interf = new Iaas_deltacloud(base_url, user_name, password);
	
	insts = interf->run_instances(image_id, vm_count, params);
}


VMInstances::~VMInstances() {
	interf->terminate_instances(*insts);
	
	delete interf;
	deleteStringVector(*insts);
	delete insts;
}

void VMInstances::wait_all_instances_running() {
	for(int i = 0; i < insts->size(); i++) {
		std::string instanceId = *(*insts)[i];
		interf->wait_instance_running(instanceId);
	}
}

std::string VMInstances::get_instance_id(int i) {
	return *(*insts)[i];
}


Instance* VMInstances::get_instance(int i) {
	return interf->get_instance_by_id(get_instance_id(i));
}




int VMInstances::test_ssh_connection(int i, bool is_private_ip) {
	return test_ssh_connection_by_id(interf, vm_user, get_instance_id(i), is_private_ip);
}

int VMInstances::test_all_ssh_connection(bool private_ips) {
	for(int i = 0; i < insts->size(); i++) {
		int ret = test_ssh_connection(i, private_ips);
		if (ret != 0) {
			return ret;
		}
	}
	
	return 0;
}


void VMInstances::wait_all_ssh_connection(bool private_ips) {
	int ret;
	
	do{
		ret = test_all_ssh_connection(private_ips);
		sleep(1);
	} while (ret != 0);
}


OpenStackVMInstances::OpenStackVMInstances(std::string image_id, int vm_count, std::string base_url, std::string user_name, std::string password, std::string vm_user, std::string key_name) : VMInstances(image_id, vm_count, base_url, user_name, password, vm_user, std::vector<Parameter>(1, Parameter("keyname", key_name))) {
	
}


}
