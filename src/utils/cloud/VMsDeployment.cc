#include "VMsDeployment.hh"

#include "Instance.hh"
#include "RemoteAccess.hh"
#include <string>
#include <iostream>
#include <stdio.h>

#ifdef USE_LOG_SERVICE
#include "Tools.hh" // For get_log_component().
#include "DietLogComponent.hh"
#endif

#include <string.h>

std::string get_ip_instance_by_id(const IaaS::pIaasInterface & interf, std::string instance_id, bool is_private_ip) {
  IaaS::pInstance_t instance = IaaS::pInstance_t(interf->get_instance_by_id(instance_id));

  if (instance == NULL) return "???.???.???.???";

  std::string ip;

  if (is_private_ip) {
    ip = instance->private_ip;
  }
  else {
    ip = instance->public_ip;
  }

  return ip;
}



int rsync_to_vm_by_id(const IaaS::pIaasInterface & interf, std::string vm_user, std::string instance_id, bool private_ip, std::string local_path, std::string remote_path) {

  std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);

  int ret = ::rsync_to_vm(local_path, remote_path, vm_user, ip);

  return ret;
}



int rsync_from_vm_by_id(const IaaS::pIaasInterface & interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_path, std::string local_path) {

  std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);

  int ret = ::rsync_from_vm(remote_path, local_path, vm_user, ip);

  return ret;
}





int execute_command_in_vm_by_id(const IaaS::pIaasInterface & interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_cmd, std::string args) {
  std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);

  int ret = ::execute_command_in_vm(remote_cmd, vm_user, ip, args);
  return ret;
}




int create_directory_in_vm_by_id(const IaaS::pIaasInterface & interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_path, std::string args) {
  std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);
  int ret = ::create_directory_in_vm(remote_path, vm_user, ip, args);
  return ret;
}
namespace IaaS {
  std::string VMsDeployment::get_ip(int vm_index, bool is_private_ip) {
    std::string id = get_instance_id(vm_index);
    printf("ID DE L'INSTANCE = %s\n",id.c_str());
    if (interf == NULL){
    		printf("INTERFACE NULLE\n");
    }
//    return get_ip_instance_by_id(interf, id, is_private_ip);

    std::string cmd = "";
    cmd = "nova list | grep " + id + " | sed 's/.*=//;s/ .*//'";
    FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "ERROR";
        char buffer[128];
        std::string result = "";
        while(!feof(pipe)) {
        	if(fgets(buffer, 128, pipe) != NULL)
        		result += buffer;
        }
        pclose(pipe);
        return result;
  }

  std::set<std::string> IaaS::VMsDeployment::get_error_instance_ids() {
    std::set<std::string> result;

    for(size_t i = 0; i < insts.size(); i++) {
      std::string& id = insts[i];

      if (is_instance_in_error_state(id)) {
        result.insert(id);
      }
    }

    return result;
  }

  bool VMsDeployment::is_instance_in_error_state(const std::string& id) {
    return strcmp(interf->get_instance_state(id).c_str(), "ERROR") == 0;
  }

  VMsDeployment::VMsDeployment(std::string _image_id, int _vm_count, const IaaS::pIaasInterface & cloud_interface,
      std::string _vm_user, const std::vector<Parameter>& _params) {

    vm_user = _vm_user;
    image_id = _image_id;
    vm_count = _vm_count;
    params = _params;

    interf = cloud_interface;

    insts = interf->run_instances(image_id, vm_count, params);

    //with openstack, error may happen, so we run other replacing instances
    while(get_error_instance_ids().size() > 0){
      terminate_failed_instances_and_run_others();
    };
  }


  VMsDeployment::~VMsDeployment() {
    interf->terminate_instances(insts);

  }

  void VMsDeployment::terminate_failed_instances_and_run_others() {
    std::set<std::string> failed_instances = get_error_instance_ids();
    int failed_instances_count = failed_instances.size();

    if (failed_instances_count > 0) {
      printf("warning : there are %u failed instances\n", failed_instances_count);
      std::vector<std::string> v_failed_instances;
      std::set<std::string>::iterator iter;
      for(iter = failed_instances.begin(); iter != failed_instances.end(); iter++) {
        v_failed_instances.push_back(std::string(*iter));
      }

      //we terminate all failed instances
      interf->terminate_instances(v_failed_instances);

      //we run other instances with the same count
      std::vector<std::string> new_insts = interf->run_instances(image_id, failed_instances_count, params);

      wait_all_instances_running();

      int index_new_inst = 0;

      //we search the old instance id places
      for(size_t i = 0 ; i < insts.size(); i++){
        std::string id = insts[i];
        if (failed_instances.count(id) > 0) {
          //we add the new instance
          insts[i] = new_insts[index_new_inst];
          index_new_inst++;
        }
      }
    }
  }

  void VMsDeployment::wait_all_instances_running() {
    for(size_t i = 0; i < insts.size(); i++) {
      interf->wait_instance_running(insts[i]);
    }
  }

  std::string VMsDeployment::get_instance_id(int i) {
    return insts[i];
  }


  Instance* VMsDeployment::get_instance(int i) {
    return interf->get_instance_by_id(get_instance_id(i));
  }


  int test_ssh_connection(std::string ssh_user, std::string ip) {
    std::string cmd = "ssh -q "  + ssh_user + "@" + ip + " -o StrictHostKeyChecking=no PasswordAuthentication=no 'exit'";
    //std::cout << cmd << std::endl;
    int ret = system(cmd.c_str());

    return ret;
  }




  int test_ssh_connection_by_id(const IaaS::pIaasInterface & interf, std::string vm_user, std::string instance_id, bool is_private_ip){

    std::string ip = get_ip_instance_by_id(interf, instance_id, is_private_ip);

    int ret = test_ssh_connection(vm_user, ip);

    return ret;
  }
  int VMsDeployment::test_ssh_connection(int i, bool is_private_ip) {
    bool result = test_ssh_connection_by_id(interf, vm_user, get_instance_id(i), is_private_ip);

#ifdef USE_LOG_SERVICE
    if (result == 0) {
      Instance* instance = get_instance(i);
      DietLogComponent* component = get_log_component();
      if (component != NULL) {
        component->logVMOSReady(*instance);
      }
      delete instance;
    }
#endif


    return result;
  }

  int VMsDeployment::test_all_ssh_connection(bool private_ips) {
    for(size_t i = 0; i < insts.size(); i++) {
      int ret = test_ssh_connection(i, private_ips);
      if (ret != 0) {
        return ret;
      }
    }

    return 0;
  }


  void VMsDeployment::wait_all_ssh_connection(bool private_ips) {
    int ret;

    do{
      ret = test_all_ssh_connection(private_ips);
      sleep(1);
    } while (ret != 0);
  }


  int VMsDeployment::rsync_to_vm(int i, bool private_ip, std::string local_path, std::string remote_path) {

    int ret;

    ret = rsync_to_vm_by_id(interf, vm_user, get_instance_id(i), private_ip, local_path, remote_path);

    return ret;
  }

  int VMsDeployment::rsync_from_vm(int i, bool private_ip, std::string remote_path, std::string local_path) {
    int ret;

    ret = rsync_from_vm_by_id(interf, vm_user, get_instance_id(i), private_ip, remote_path, local_path);

    return ret;
  }

  int VMsDeployment::execute_command_in_vm(int i, bool private_ip, const std::string& remote_cmd, const std::string& args) {
    return ::execute_command_in_vm_by_id(interf, vm_user, get_instance_id(i), private_ip, remote_cmd, args);
  }


  void VMsDeployment::get_ips(std::vector<std::string>& ips, bool private_ip) {
    for(size_t i = 0; i < insts.size(); i++) {
      ips.push_back(get_ip(i, private_ip));
    }
  }


}
