/*
	Authors :
		- Lamiel Toch : lamiel.toch@ens-lyon.fr
*/


#ifndef __DELTACLOUD__TOOLS__h
#define __DELTACLOUD__TOOLS__h

#include "IaasInterface.hh"
#include "Instance.hh"
#include <string>
#include <vector>


void deleteStringVector(std::vector<std::string*>& v);


std::string get_ip_instance_by_id(IaaS::IaasInterface* interf, std::string instance_id, bool is_private_ip);

/*

return 0 iif success
*/
int test_ssh_connection(std::string ssh_user, std::string ip);
int test_ssh_connection_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip = true);
int rsync_to_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string local_path, std::string remote_path);

int rsync_to_vm(std::string local_path, std::string remote_path, std::string user, std::string ip);
int rsync_from_vm(std::string remote_path, std::string local_path, std::string user, std::string ip);


int execute_command_in_vm(const std::string& remote_cmd, std::string user, std::string ip, std::string args);
int execute_command_in_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_cmd, std::string args);

int create_directory_in_vm(const std::string& remote_path, std::string user, std::string ip, std::string args = " -p ");
int create_directory_in_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_path, std::string args = " -p ");


char* readline(const char* path, int index);
void readlines(const char* path, std::vector<std::string>& lines);

namespace IaaS {






class VMInstances {
	public:

	/*
	create VM instances

	example :  {
		string base_url = "http://localhost:3001/api";
		string username = "admin+admin";
		string password = "admin";
		string vm_user = "cirros";
	*/
	VMInstances(std::string image_id, int vm_count, std::string base_url, std::string user_name, std::string password,
	std::string vm_user, const std::vector<Parameter>& params = std::vector<Parameter>());

	~VMInstances();

	void wait_all_instances_running();
	std::string get_instance_id(int i);
	Instance* get_instance(int i);

	void get_ips(std::vector<std::string>& ips, bool private_ip);


	//connection to vm index i with private ip address by default, public address otherwise
	int test_ssh_connection(int i, bool private_ip = true);
	int test_all_ssh_connection(bool private_ips = true);
	void wait_all_ssh_connection(bool private_ips = true);
	std::string get_ip(int vm_index, bool private_ip = true);

    int rsync_from_vm(int i, bool private_ip, std::string remote_path, std::string local_path);
    int rsync_to_vm(int i, bool private_ip, std::string local_path, std::string remote_path);
    //command : remote_path/exec.sh 1 2 3 4 ...
    int execute_command_in_vm(int i, bool private_ip, const std::string& remote_cmd, const std::string& args);
	protected:

	IaasInterface* interf;
	//id of instances
	std::vector<std::string*>* insts;

	std::string vm_user;
};


//std::vector<std::string*>* create_one_vm_instance(int vm_count, std::string base_url, std::string user_name, std::string password,
//	std::string vm_user, IaasInterface ** interf);


class OpenStackVMInstances : public VMInstances{
	public:
		OpenStackVMInstances(std::string image_id, int vm_count, std::string base_url, std::string user_name, std::string password,
	std::string vm_user, std::string key_name);
};


}


#endif
