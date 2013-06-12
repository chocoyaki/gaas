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
#include <fstream>
#include <string.h>


char* cpp_strdup(const char* src) {
	char* cpy = new char [strlen(src) + 1];
	strcpy(cpy, src);
	return cpy;
}


std::string get_ip_instance_by_id(IaaS::IaasInterface* interf, std::string instance_id, bool is_private_ip) {
    IaaS::Instance* instance = interf->get_instance_by_id(instance_id);

	if (instance == NULL) return "???.???.???.???";

	std::string ip;

	if (is_private_ip) {
		ip = instance->private_ip;
	}
	else {
		ip = instance->public_ip;
	}

	delete instance;

	return ip;
}

std::string IaaS::VMInstances::get_ip(int vm_index, bool is_private_ip) {
	std::string id = get_instance_id(vm_index);
	return get_ip_instance_by_id(interf, id, is_private_ip);
}


void deleteStringVector(std::vector<std::string*>& v){
	for(int i = 0; i < v.size(); i++) {
		delete v[i];
		v[i] = NULL;
	}
}

int test_ssh_connection(std::string ssh_user, std::string ip) {
	std::string cmd = "ssh "  + ssh_user + "@" + ip + " -o StrictHostKeyChecking=no PasswordAuthentication=no 'ls'";
	std::cout << cmd << std::endl;
	int ret = system(cmd.c_str());

	return ret;
}




int test_ssh_connection_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool is_private_ip){

    std::string ip = get_ip_instance_by_id(interf, instance_id, is_private_ip);

	int ret = ::test_ssh_connection(vm_user, ip);

	return ret;
}

int rsync_to_vm(std::string local_path, std::string remote_path, std::string user, std::string ip) {
    int ret;

    std::string cmd = "rsync -avz -e 'ssh -o StrictHostKeyChecking=no' " + local_path + " " + user + "@" + ip + ":" + remote_path;
    std::cout << cmd << std::endl;
    ret = system(cmd.c_str());

    return ret;
}


int rsync_to_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string local_path, std::string remote_path) {

    std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);

    int ret = ::rsync_to_vm(local_path, remote_path, vm_user, ip);

    return ret;
}


int rsync_from_vm(std::string remote_path, std::string local_path, std::string user, std::string ip) {
    int ret;

    std::string cmd = "rsync -avz -e 'ssh -o StrictHostKeyChecking=no' " + user + "@" + ip + ":" + remote_path + " " + local_path;
    std::cout << cmd << std::endl;
    ret = system(cmd.c_str());

    return ret;
}

int rsync_from_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_path, std::string local_path) {

    std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);

    int ret = ::rsync_from_vm(remote_path, local_path, vm_user, ip);

    return ret;
}




int execute_command_in_vm(const std::string& remote_cmd, std::string vm_user, std::string ip, std::string args) {
    std::string cmd = "ssh "  + vm_user+ "@" + ip + " -o StrictHostKeyChecking=no '" + remote_cmd + " " + args + "'";
	std::cout << cmd << std::endl;
	int ret = system(cmd.c_str());

	return ret;
}


int execute_command_in_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_cmd, std::string args) {
    std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);

    int ret = ::execute_command_in_vm(remote_cmd, vm_user, ip, args);
}



int create_directory_in_vm(const std::string& remote_path, std::string user, std::string ip, std::string args) {
  std::string cmd = "ssh "  + user + "@" + ip + " -o StrictHostKeyChecking=no 'mkdir " + args + remote_path + "'";

  int ret = system(cmd.c_str());
  return ret;
}


int create_directory_in_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_path, std::string args) {
  std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);
  int ret = ::create_directory_in_vm(remote_path, vm_user, ip, args);
}


char* readline(const char* path, int index) {
	int i = 0;
	bool end = false;
	bool found = false;
	std::fstream file;
	file.open(path, std::ios_base::in);

	char* line;
	do {

		try{
			std::string s;
			getline(file, s);
			if (s.compare("") != 0 ){

					if (i >= index) {
						line = cpp_strdup(s.c_str());
						//std::cout << s << std::endl;
						end = true;
						found = true;
					}
			}
			else {
				end = true;
			}
		}
		catch (std::ios_base::failure e) {
			end = true;
		}

		i++;
	} while (!end);

	file.close();
	if (!found) {
		return NULL;
	}


	return line;
}


void readlines(const char* path, std::vector<std::string>& lines) {
	int i = 0;
	bool end = false;
	std::fstream file;
	file.open(path, std::ios_base::in);

	char* line;
	do {

		try{
			std::string s;
			getline(file, s);
			if (s.compare("") != 0 ){
				lines.push_back(s);
			}
			else {
				end = true;
			}
		}
		catch (std::ios_base::failure e) {
			end = true;
		}

		i++;
	} while (!end);
	file.close();
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


int VMInstances::rsync_to_vm(int i, bool private_ip, std::string local_path, std::string remote_path) {

    int ret;

    ret = rsync_to_vm_by_id(interf, vm_user, get_instance_id(i), private_ip, local_path, remote_path);

    return ret;
}

int VMInstances::rsync_from_vm(int i, bool private_ip, std::string remote_path, std::string local_path) {
    int ret;

    ret = rsync_from_vm_by_id(interf, vm_user, get_instance_id(i), private_ip, remote_path, local_path);

    return ret;
}

int VMInstances::execute_command_in_vm(int i, bool private_ip, const std::string& remote_cmd, const std::string& args) {

    //std::string remote_cmd = remote_path + "/exec.sh";
    //std::string args = "";

    /*char arg[16];

    for(int k = 0; k < n; k++) {
        sprintf(arg, "%i", k);
        args = args + " " + std::string(arg);
    }*/

    ::execute_command_in_vm_by_id(interf, vm_user, get_instance_id(i), private_ip, remote_cmd, args);
}


void VMInstances::get_ips(std::vector<std::string>& ips, bool private_ip) {
	for(int i = 0; i < insts->size(); i++) {
		ips.push_back(get_ip(i, private_ip));
	}
}

/*
OpenStackVMInstances::OpenStackVMInstances(std::string image_id, int vm_count, std::string base_url, std::string user_name, std::string password, std::string vm_user, std::string key_name) : VMInstances(image_id, vm_count, base_url, user_name, password, vm_user, std::vector<Parameter>(1, Parameter("keyname", key_name))) {

}*/


}
