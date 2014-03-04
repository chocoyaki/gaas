#ifndef _IAAS_VMS_DEPLOYMENT_HH_
#define _IAAS_VMS_DEPLOYMENT_HH_

#include "IaasInterface.hh"
#include <string>
#include <set>


namespace IaaS {

  class VMsDeployment {
    public:

      VMsDeployment(std::string image_id, int vm_count, const IaaS::pIaasInterface & cloud_interface,
          std::string vm_user, const std::vector<Parameter>& params = std::vector<Parameter>());

      ~VMsDeployment();

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

      bool is_instance_in_error_state(const std::string& id);
      std::set<std::string> get_error_instance_ids();
      void terminate_failed_instances_and_run_others();

      pIaasInterface interf;
      //id of instances
      std::vector<std::string> insts;

      int vm_count;
      std::string image_id;
      std::string vm_user;
      std::vector<Parameter> params;
  };


}

#endif
