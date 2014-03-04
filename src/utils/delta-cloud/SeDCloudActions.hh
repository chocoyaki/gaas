/*
 * @file SeDCloudActions.hh
 *
 * @brief
 *
 * @author  Lamiel Toch (lamiel.toch@ens-lyon.fr) Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _SEDCLOUDACTIONS_HH_
#define _SEDCLOUDACTIONS_HH_

#include "CloudServiceBinary.hh"
#include "ServiceStatisticsMap.hh"
#include "IaasInterface.hh"
#include "VMsDeployment.hh"
#include "Tools.hh"

#include <boost/shared_ptr.hpp>
#include <string>
#include <list>
#include <vector>

class SeDCloudActions {

  private:
    IaaS::pIaasInterface interface;
    //a list of vm_instances;
    std::list<IaaS::VMsDeployment*> vm_instances;
    void fill_ips();

  protected:
    /*******FOR DELTACLOUD********/
    std::string image_id;

    std::string vm_user; // eg : "root";
    int vm_count; // eg : 1;
    //std::string profile; //eg :  "debian-rc";
    std::vector<IaaS::Parameter> params; //parameters for instantiating a VM with deltacloud

    bool is_ip_private;
    ServiceStatisticsMap statistics_on_services;

    std::vector<std::string> ips;
    std::string master_ip;

    void copy_binary_into_vm(std::string name, int vm_index);
    void copy_binary_into_all_vms(std::string name);
    void copy_all_binaries_into_vm(int vm_index);
    void copy_all_binaries_into_all_vms();
    //launch the vms
    int launch_vms();
    //destroy the vms
    void destroy_vms();
    int send_vm_ips_to_master();
  public:

    void set_credentials(const std::string username, std::string passwd) {
      interface->get_cloud_api_connection().username = username;
      interface->get_cloud_api_connection().password = passwd;
    }

    const std::vector<std::string>& get_ips() const {
      return ips;
    }

    const std::string& get_master_ip() const {
      return master_ip;
    }

    //launch a set of vms
    int launch_vms(const std::string& vm_image, int vm_count, const std::string& vm_user,
        bool is_ip_private, const std::vector<IaaS::Parameter>& params = std::vector<IaaS::Parameter>());

    //execute an action when the the client make a solve
    virtual int perform_action_on_begin_solve(diet_profile_t *pb) = 0;
    virtual int perform_action_on_end_solve(diet_profile_t *pb) = 0;

    virtual void perform_action_on_sed_creation() = 0;

    //at diet_SeD instruction
    virtual void perform_action_on_sed_launch() = 0;

    virtual int perform_action_after_service_table_add(const std::string& name_of_service) = 0;

    //when the OS of the VM is ready and can accept ssh connection
    virtual int perform_action_on_vm_os_ready() { return 0;};

    virtual int send_arguments(const std::string& local_path, const std::string& remote_path);
    virtual int receive_result(const std::string& result_remote_path, const std::string& result_local_path);
    virtual int execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args);
    virtual int create_remote_directory(const std::string& remote_path);

    SeDCloudActions();

    SeDCloudActions(const std::string& _image_id, const IaaS::pIaasInterface & cloud_interface, const std::string& _vm_user,
        int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& params = std::vector<IaaS::Parameter>());


    virtual ~SeDCloudActions();

    bool using_private_ip() {
      return this->is_ip_private;
    }

    const std::map<std::string, CloudServiceBinary>& get_cloud_service_binaries() {
      return cloud_service_binaries;
    }

    void set_cloud_service_binary(const std::string& name, const CloudServiceBinary& binary) {
      cloud_service_binaries[name] = binary;
    }

    void unset_cloud_service_binary(const std::string& name) {
      cloud_service_binaries.erase(name);
    }

    void clone_service_binaries(const SeDCloudActions& src);
  protected:
    std::map<std::string, CloudServiceBinary> cloud_service_binaries;




};


class SeDCloudActionsNULL : public SeDCloudActions {
  public:
    SeDCloudActionsNULL() {};

    virtual int perform_action_on_begin_solve(diet_profile_t *pb) {return 0;};
    virtual int perform_action_on_end_solve(diet_profile_t *pb) {return 0;};
    virtual void perform_action_on_sed_creation() {};
    virtual void perform_action_on_sed_launch() {};
    virtual int perform_action_after_service_table_add(const std::string& name_of_service) { return 0;};
    virtual int send_arguments(const std::string& local_path, const std::string& remote_path) { return 0; };
    virtual int receive_result(const std::string& result_remote_path, const std::string& result_local_path) {return 0;};
    virtual int execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args) {return 0;};
    virtual int create_remote_directory(const std::string& remote_path) {return 0;};

};





class SeDCloudAndVMLaunchedActions : public SeDCloudActions {
  public:

    virtual int perform_action_on_begin_solve(diet_profile_t *pb);
    virtual int perform_action_on_end_solve(diet_profile_t *pb);

    virtual void perform_action_on_sed_creation();
    virtual void perform_action_on_sed_launch();

    virtual int perform_action_after_service_table_add(const std::string& name_of_service);

    SeDCloudAndVMLaunchedActions() : SeDCloudActions() {}

    SeDCloudAndVMLaunchedActions(const std::string& _image_id, const IaaS::pIaasInterface & interface, const std::string& _vm_user,
        int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& _params = std::vector<IaaS::Parameter>()) :
      SeDCloudActions(_image_id, interface, _vm_user, _vm_count, _is_ip_private, _params){

      }



};

#define MACHINE_ALIVE_INTERVAL 2

class SeDCloudMachinesActions : public SeDCloudAndVMLaunchedActions {
  protected:
    int machine_alive_interval;

  public:
    virtual void perform_action_on_sed_creation();
    virtual void perform_action_on_sed_launch();

    SeDCloudMachinesActions(const std::vector<std::string>& _ips, const std::string& _username) {
      ips = _ips;
      master_ip = ips[0];
      vm_user = _username;
      machine_alive_interval = MACHINE_ALIVE_INTERVAL;
    }

    SeDCloudMachinesActions(std::string ip_file_path,  const std::string& _username) {
      readlines(ip_file_path.c_str(), ips);
      master_ip = ips[0];
      vm_user = _username;
      machine_alive_interval = MACHINE_ALIVE_INTERVAL;
    }


    void set_machine_alive_interval(int x) {
      machine_alive_interval = x;
    }
};


class SeDCloudVMLaunchedAtSolveActions : public SeDCloudActions {
  public:
    virtual int perform_action_on_begin_solve(diet_profile_t *pb) = 0;
    virtual int perform_action_on_end_solve(diet_profile_t *pb) = 0;

    //virtual void perform_action_on_sed_creation();
    virtual void perform_action_on_sed_launch();

    virtual void perform_action_on_sed_creation() {};
    virtual int perform_action_after_service_table_add(const std::string& name_of_service) { return 0;};


    SeDCloudVMLaunchedAtSolveActions(const std::string& _image_id, const IaaS::pIaasInterface & interface, const std::string& _vm_user,
        int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& _params = std::vector<IaaS::Parameter>()) :
      SeDCloudActions(_image_id, interface, _vm_user, _vm_count, _is_ip_private, _params) {

      }

  protected:



};


class SeDCloudVMLaunchedAtFirstSolveActions : public SeDCloudVMLaunchedAtSolveActions {
  public:

    virtual int perform_action_on_begin_solve(diet_profile_t *pb);
    virtual int perform_action_on_end_solve(diet_profile_t *pb);


    SeDCloudVMLaunchedAtFirstSolveActions(const std::string& _image_id, const IaaS::pIaasInterface & interface, const std::string& _vm_user,
        int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& _params = std::vector<IaaS::Parameter>()) :
      SeDCloudVMLaunchedAtSolveActions(_image_id, interface, _vm_user, _vm_count, _is_ip_private, _params) {
      }

};


class SeDCloudVMLaunchedAtSolveThenDestroyedActions : public SeDCloudVMLaunchedAtSolveActions {
  public:
    virtual int perform_action_on_begin_solve(diet_profile_t *pb);
    virtual int perform_action_on_end_solve(diet_profile_t *pb);


    SeDCloudVMLaunchedAtSolveThenDestroyedActions(const std::string& _image_id, const IaaS::pIaasInterface & interface, const std::string& _vm_user,
        int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& _params = std::vector<IaaS::Parameter>()) :
      SeDCloudVMLaunchedAtSolveActions(_image_id, interface, _vm_user, _vm_count, _is_ip_private, _params) {
      }
};

//execute a programm after VMs are launched
/*
   class SeDCloudVMLaunchedThenExecProgramActions : public SeDCloudActionsNULL {
   private:
   std::string program_path;
   std::vector<std::string> program_args;

   public:

//SeDCloudVMLaunchedThenExecProgramActions() {};

SeDCloudVMLaunchedThenExecProgramActions(const std::string& program_path, const std::vector<std::string>& program_args) {
this->program_path = program_path;
this->program_args = program_args;
}

void set_program_path(const std::string& program_path) {
this->program_path = program_path;
}

void set_program_args(const std::vector<std::string>& program_args) {
this->program_args = program_args;
}

void set_program_arg(int index, const std::string& arg) {
program_args[index] = arg;
}

const std::string& get_program_arg(int index) const{
return program_args[index];
}

void push_back_arg(const std::string arg){
program_args.push_back(arg);
}

//when the OS of the VM is ready and can accept ssh connection
virtual int perform_action_on_vm_os_ready();
};*/



#endif /* _SEDCLOUDACTIONS_HH_ */
