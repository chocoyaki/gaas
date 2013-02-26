#include "SeD_deltacloud.hh"
#include <stdio.h>


DIET_API_LIB diet_profile_desc_t*
  dietcloud_profile_files_desc_alloc_and_set(const char* path, int last_in, int last_inout, int last_out) {
    diet_profile_desc_t* profile;

    profile = diet_profile_desc_alloc(path, last_in, last_inout, last_out);

    for(int i = 0; i <= last_out; i++) {
        diet_generic_desc_set(diet_param_desc(profile, i), DIET_FILE, DIET_CHAR);
    }

    return profile;
  }



SeDCloud::SeDCloud(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                          int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& _params) {
        image_id = _image_id;
        base_url = _base_url;
        username = _username;
        password = _password;
        vm_user = _vm_user;
        vm_count = _vm_count;
        params = _params;
        is_ip_private = _is_ip_private;

        vm_instances = new IaaS::VMInstances (image_id, vm_count, base_url, username, password, vm_user, params);
}




DIET_API_LIB int
        SeDCloud::service_table_add(const std::string& name_of_service,
                          int last_in,
                          int last_out,
                         const diet_convertor_t* const cvt,
                         const std::string& local_path_of_binary,
                         const std::string& remote_path_of_binary) {

    diet_profile_desc_t* profile;

    profile = diet_profile_desc_alloc(name_of_service.c_str(), last_in, last_in, last_out);

    for(int i = 0; i <= last_out; i++) {
        diet_generic_desc_set(diet_param_desc(profile, i), DIET_FILE, DIET_CHAR);
    }


    if (diet_service_table_add(profile, cvt, solve)) {
        return 1;
    }


    SeDCloud::instance->vm_instances->wait_all_ssh_connection(is_ip_private);
    //TODO : remplacer 0 par l'instance souhaitée dynamiquement
    SeDCloud::instance->vm_instances->rsync_to_vm(0, is_ip_private, local_path_of_binary, remote_path_of_binary);


    SeDCloud::cloud_service_binaries[name_of_service] = CloudServiceBinary(local_path_of_binary, remote_path_of_binary);


    diet_profile_desc_free(profile);
    diet_print_service_table();

    return 0;
}


void SeDCloud::deployer_on_one_vm(int vm_index, const char* path_to_binary) {

}


void SeDCloud::addParameter(const std::string& param, const std::string& value) {
        params.push_back(IaaS::Parameter(param, value));
}


int SeDCloud::solve(diet_profile_t *pb) {
    std::string name(pb->pb_name);

    CloudServiceBinary& binary = SeDCloud::cloud_service_binaries[name];

    std::string remote_path_of_binary = binary.remote_path_of_binary;
    int nb_args = pb->last_out + 1;
    int last_in = pb->last_in;
    size_t arg_size;
    char* sz_local_path = NULL;
    std::string local_path;
    std::string arg_remote_path;
    std::string arg_local_path;
    char sz_i[512];

    for(int i = 0; i <= last_in; i++) {
        diet_file_get(diet_parameter(pb, i), &sz_local_path, NULL, &arg_size);
        local_path = sz_local_path;

        sprintf(sz_i, "%i", i);
        //a disctinct remote folder for each request
        arg_remote_path = remote_path_of_binary + "/" + sz_i;

        SeDCloud::instance->vm_instances->rsync_to_vm(0, SeDCloud::instance->using_private_ip(), local_path, arg_remote_path);
    }

    SeDCloud::instance->vm_instances->execute_command_in_vm(0, SeDCloud::instance->using_private_ip(), remote_path_of_binary);

    for(int i = last_in + 1 ; i < nb_args; i++) {
        sprintf(sz_i, "/tmp/%i", i);
        arg_local_path = sz_i;

        sprintf(sz_i, "/%i", i);
        arg_remote_path = remote_path_of_binary + sz_i;

        SeDCloud::instance->vm_instances->rsync_from_vm(0, SeDCloud::instance->using_private_ip(), arg_remote_path, arg_local_path);


        if (diet_file_set(diet_parameter(pb, i), arg_local_path.c_str(), DIET_VOLATILE)) {
            printf("diet_file_desc_set error\n");
            return 1;
        }
    }

    for(int i = 0; i <= last_in; i++){
        diet_free_data(diet_parameter(pb, i));
    }
}



void SeDCloud::create(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                        int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& params) {
    SeDCloud::instance = new SeDCloud(_image_id, _base_url, _username, _password, _vm_user, _vm_count, _is_ip_private, params);

}

SeDCloud::~SeDCloud() {
    if (vm_instances != NULL){
        delete vm_instances;
        vm_instances = NULL;
    }
}

void SeDCloud::erase() {
    if (instance != NULL) {
        delete instance;
        instance = NULL;
    }
}


SeDCloud* SeDCloud::get() {
    return SeDCloud::instance;
}


CloudServiceBinary::CloudServiceBinary(const std::string& _local_path, const std::string& _remote_path /*, int _last_in, int _last_out */) : local_path_of_binary(_local_path),
    remote_path_of_binary(_remote_path)/*, last_in(_last_in), last_out(_last_out) */{

}

CloudServiceBinary::CloudServiceBinary(const CloudServiceBinary& binary) {
    local_path_of_binary = binary.local_path_of_binary;
    remote_path_of_binary = binary.remote_path_of_binary;
}


CloudServiceBinary::CloudServiceBinary() {

}


bool SeDCloud::using_private_ip() {
    return is_ip_private;
}
