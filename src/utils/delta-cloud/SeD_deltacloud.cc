#include "SeD_deltacloud.hh"
#include <stdio.h>











CloudServiceBinary::CloudServiceBinary(const std::string& _local_path, const std::string& _remote_path /*, int _last_in, int _last_out */) : local_path_of_binary(_local_path),
    remote_path_of_binary(_remote_path)/*, last_in(_last_in), last_out(_last_out) */{

}


CloudServiceBinary::CloudServiceBinary(const CloudServiceBinary& binary) {
    local_path_of_binary = binary.local_path_of_binary;
    remote_path_of_binary = binary.remote_path_of_binary;
}


CloudServiceBinary::CloudServiceBinary() {

}





SeDCloudActions::SeDCloudActions(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                          int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& _params) {

    this->image_id = _image_id;
    this->base_url = _base_url;
    this->username = _username;
    this->password = _password;
    this->vm_user = _vm_user;
    this->vm_count = _vm_count;
    this->params = _params;
    this->is_ip_private = _is_ip_private;

    this->vm_instances = NULL;
}


SeDCloudActions::SeDCloudActions() {
     this->vm_instances = NULL;
}

int SeDCloudActions::send_arguments(const std::string& local_path, const std::string& remote_path) {
    vm_instances->rsync_to_vm(0, is_ip_private, local_path, remote_path);
}

int SeDCloudActions::receive_result(const std::string& result_remote_path, const std::string& result_local_path) {
    vm_instances->rsync_from_vm(0, is_ip_private, result_remote_path, result_local_path);
}

int SeDCloudActions::execute_remote_binary(const std::string& remote_path_of_binary, const std::string& args) {
    vm_instances->execute_command_in_vm(0, is_ip_private, remote_path_of_binary + "/exec.sh", args);

}



int SeDCloudAndVMLaunchedActions::perform_action_on_begin_solve(diet_profile_t *pb) {

}

int SeDCloudAndVMLaunchedActions::perform_action_on_end_solve(diet_profile_t *pb) {

}

void SeDCloudAndVMLaunchedActions::perform_action_on_sed_creation() {
    this->vm_instances = new IaaS::VMInstances (this->image_id, this->vm_count, this->base_url, this->username, this->password, this->vm_user, this->params);
}




/*
On sed launch, we copy all binaries for each service
*/
void SeDCloudAndVMLaunchedActions::perform_action_on_sed_launch() {
    vm_instances->wait_all_ssh_connection(this->is_ip_private);

    std::map<std::string, CloudServiceBinary>::const_iterator iter;
    std::map<std::string, CloudServiceBinary>& binaries = cloud_service_binaries;
    for(iter = binaries.begin(); iter != binaries.end(); iter++) {
        std::string name_of_service = iter->first;
        CloudServiceBinary& binary = binaries[name_of_service];

        //TODO : remplacer 0 par l'instance souhaitée dynamiquement
        vm_instances->rsync_to_vm(0, this->is_ip_private, binary.local_path_of_binary, binary.remote_path_of_binary);
    }
}


int SeDCloudAndVMLaunchedActions::perform_action_after_service_table_add(const std::string& name_of_service) {

}




void SeDCloudWithoutVMActions::perform_action_on_sed_creation() {
    //rien pas de creation de vm
}




/*
On sed launch, we copy all binaries for each service
*/
void SeDCloudWithoutVMActions::perform_action_on_sed_launch() {
    //SeDCloud<SeDCloudAndVMLaunchedActions>::get()->vm_instances->wait_all_ssh_connection(this->is_ip_private);

    std::map<std::string, CloudServiceBinary>::const_iterator iter;
    std::map<std::string, CloudServiceBinary>& binaries = cloud_service_binaries;
    for(iter = binaries.begin(); iter != binaries.end(); iter++) {
        std::string name_of_service = iter->first;
        CloudServiceBinary& binary = binaries[name_of_service];

        //TODO : remplacer 0 par l'instance souhaitée dynamiquement
        //SeDCloud<SeDCloudAndVMLaunchedActions>::get()->vm_instances->rsync_to_vm(0, this->is_ip_private, binary.local_path_of_binary, binary.remote_path_of_binary);
        printf("local_path_of_binary=%s remote_path_of_binary=%s\n", binary.local_path_of_binary.c_str(), binary.remote_path_of_binary.c_str() );
        ::rsync_to_vm(binary.local_path_of_binary, binary.remote_path_of_binary, username, address_ip);
    }
}



int SeDCloudWithoutVMActions::send_arguments(const std::string& local_path, const std::string& remote_path) {
    ::rsync_to_vm(local_path, remote_path, username, address_ip);
}

int SeDCloudWithoutVMActions::receive_result(const std::string& result_remote_path, const std::string& result_local_path) {
    ::rsync_from_vm(result_remote_path, result_local_path, username, address_ip);
}

int SeDCloudWithoutVMActions::execute_remote_binary(const std::string& remote_path_of_binary, const std::string& args) {
    printf(">>>>>>>>>>>>>>>>EXECUTE BINARY\n");
    ::execute_command_in_vm(remote_path_of_binary + "/exec.sh", username, address_ip, args);
}
