#include "SeD_deltacloud.hh"
#include <stdio.h>











CloudServiceBinary::CloudServiceBinary(const std::string& _local_path, const std::string& _remote_path, const std::string& _entry_point_relative_file_path ,
									 const std::string& _remote_path_of_arguments) : local_path_of_binary(_local_path),
    remote_path_of_binary(_remote_path), entry_point_relative_file_path(_entry_point_relative_file_path){

	//if _remote_path_of_arguments == "" we set the defaut location of data to the folder of the binary
	if (_remote_path_of_arguments.compare("") == 0){
		remote_path_of_arguments = _remote_path;
	}
	else {
		remote_path_of_arguments = _remote_path_of_arguments;
	}

}


CloudServiceBinary::CloudServiceBinary(const CloudServiceBinary& binary) {
    local_path_of_binary = binary.local_path_of_binary;
    remote_path_of_binary = binary.remote_path_of_binary;
    entry_point_relative_file_path = binary.entry_point_relative_file_path;
	remote_path_of_arguments = binary.remote_path_of_arguments;
}


CloudServiceBinary::CloudServiceBinary() {

}

int CloudServiceBinary::execute_remote(const std::string& ip, const std::string& user_name, const std::vector<std::string>& args) const {
	 printf(">>>>>>>>>>>>>>>>EXECUTE REMOTE\n");
	 std::string sz_args = "";
	for(int i = 0; i < args.size(); i++) {
		sz_args.append(" " + args[i]);
	}

    ::execute_command_in_vm(remote_path_of_binary + "/" +  entry_point_relative_file_path, user_name, ip, sz_args);
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

int SeDCloudActions::send_arguments(const std::string& local_path, const std::string& remote_path, int vm_index) {
    vm_instances->rsync_to_vm(vm_index, is_ip_private, local_path, remote_path);
}

int SeDCloudActions::receive_result(const std::string& result_remote_path, const std::string& result_local_path, int vm_index) {
    vm_instances->rsync_from_vm(vm_index, is_ip_private, result_remote_path, result_local_path);
}

int SeDCloudActions::execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args, int vm_index) {
    //vm_instances->execute_command_in_vm(vm_index, is_ip_private, remote_path_of_binary + "/exec.sh", args);
    std::string ip = vm_instances->get_ip(vm_index, is_ip_private);
	binary.execute_remote(ip, vm_user, args);
}

int SeDCloudActions::create_remote_directory(const std::string& remote_path, int vm_index) {
	std::string ip = vm_instances->get_ip(vm_index, is_ip_private);
	create_directory_in_vm(remote_path, username, ip);
}

void SeDCloudActions::copy_all_binaries_into_vm(int vm_index) {
    vm_instances->wait_all_ssh_connection(this->is_ip_private);

    std::map<std::string, CloudServiceBinary>::const_iterator iter;
    std::map<std::string, CloudServiceBinary>& binaries = cloud_service_binaries;
    for(iter = binaries.begin(); iter != binaries.end(); iter++) {
        std::string name_of_service = iter->first;
        CloudServiceBinary& binary = binaries[name_of_service];

        //TODO : remplacer 0 par l'instance souhaitée dynamiquement
        vm_instances->rsync_to_vm(vm_index, this->is_ip_private, binary.local_path_of_binary, binary.remote_path_of_binary);
    }
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
    copy_all_binaries_into_vm(0);
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



int SeDCloudWithoutVMActions::send_arguments(const std::string& local_path, const std::string& remote_path, int vm_index) {
    ::rsync_to_vm(local_path, remote_path, username, address_ip);
}

int SeDCloudWithoutVMActions::receive_result(const std::string& result_remote_path, const std::string& result_local_path, int vm_index) {
    ::rsync_from_vm(result_remote_path, result_local_path, username, address_ip);
}

int SeDCloudWithoutVMActions::execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args, int vm_index) {
    printf(">>>>>>>>>>>>>>>>EXECUTE BINARY\n");
    //::execute_command_in_vm(remote_path_of_binary + "/exec.sh", username, address_ip, args);
    binary.execute_remote(address_ip, username, args);
}


int SeDCloudWithoutVMActions::create_remote_directory(const std::string& remote_path, int vm_index) {
	printf(">>>>>>>>>>>>>>>>CREATE DIRECTORY %s\n", remote_path.c_str());
	create_directory_in_vm(remote_path, username, address_ip);
}


void SeDCloudVMLaunchedAtSolveActions::perform_action_on_sed_launch() {
    std::map<std::string, CloudServiceBinary>::const_iterator iter;
    std::map<std::string, CloudServiceBinary>& binaries = cloud_service_binaries;
    for(iter = binaries.begin(); iter != binaries.end(); iter++) {
        std::string name_of_service = iter->first;
        printf("add service %s\n", name_of_service.c_str());
        statistics_on_services.add_service(name_of_service);
    }
}



int SeDCloudVMLaunchedAtFirstSolveActions::perform_action_on_begin_solve(diet_profile_t *pb){
    std::string service_name = pb->pb_name;

    printf("call solve for service %s\n", service_name.c_str());



    if (!statistics_on_services.one_service_already_called()) {
        statistics_on_services.increment_call_number(service_name);
        this->vm_instances = new IaaS::VMInstances (this->image_id, this->vm_count, this->base_url, this->username, this->password, this->vm_user, this->params);
        vm_instances->wait_all_ssh_connection(this->is_ip_private);
        copy_all_binaries_into_vm(0);

    }
    else {
        statistics_on_services.increment_call_number(service_name);
    }


}

int SeDCloudVMLaunchedAtFirstSolveActions::perform_action_on_end_solve(diet_profile_t *pb) {

}


int SeDCloudVMLaunchedAtSolveThenDestroyedActions::perform_action_on_begin_solve(diet_profile_t *pb) {
    this->vm_instances = new IaaS::VMInstances (this->image_id, this->vm_count, this->base_url, this->username, this->password, this->vm_user, this->params);
    vm_instances->wait_all_ssh_connection(this->is_ip_private);


    std::string name_of_service = pb->pb_name;
    CloudServiceBinary& binary = cloud_service_binaries[name_of_service];
    vm_instances->rsync_to_vm(0, is_ip_private, binary.local_path_of_binary, binary.remote_path_of_binary);

}


int SeDCloudVMLaunchedAtSolveThenDestroyedActions::perform_action_on_end_solve(diet_profile_t *pb) {

    delete this->vm_instances;
    this->vm_instances = NULL;
}



int SeDCloud::solve(diet_profile_t *pb) {
	int reqId = pb->dietReqID;

	std::ostringstream string_stream;
	string_stream << reqId;
	std::string szReqId = string_stream.str();



	DagdaImpl *dataManager = DagdaFactory::getDataManager();
	const char* dagda_path = dataManager->getDataPath();
	//the local folder of datas
	std::string local_results_folder = "";
	local_results_folder.append(dagda_path);
	local_results_folder.append("/");
	local_results_folder.append(szReqId);
	std::string cmd = "mkdir -p " + local_results_folder;
	printf("CREATE LOCAL DIRECTORY : %s\n", local_results_folder.c_str());
	system(cmd.c_str());

	SeDCloud::instance->actions->perform_action_on_begin_solve(pb);

	std::string name(pb->pb_name);

	CloudServiceBinary& binary = SeDCloudActions::cloud_service_binaries[name];

	std::string remote_path_of_binary = binary.remote_path_of_binary;
	std::string remote_path_of_arguments = binary.remote_path_of_arguments;

	//a disctinct remote folder for each request
	//the remote folder where datas are stored
	std::string remote_request_folder = remote_path_of_arguments + "/" + szReqId;
	SeDCloud::instance->actions->create_remote_directory(remote_request_folder);



	int nb_args = pb->last_out + 1;
	int last_in = pb->last_in;
	size_t arg_size;
	char* sz_local_path = NULL;
	std::string local_path;
	std::string arg_remote_path;
	std::string arg_local_path;
	char sz_i[2048];

	std::vector<std::string> arguments_vector;
	for(int i = 0; i < nb_args; i++) {
		sprintf(sz_i, "%i", i);


		arg_remote_path = remote_request_folder + "/" + sz_i;
		//printf("arg_remote_path = %s\n", arg_remote_path.c_str());
		//printf("szReqId = %s\n", szReqId.c_str());
		arguments_vector.push_back(arg_remote_path);

		if (i <= last_in) {
			diet_file_get(diet_parameter(pb, i), &sz_local_path, NULL, &arg_size);
			local_path = sz_local_path;
			SeDCloud::instance->actions->send_arguments(local_path, arg_remote_path);
		}



	}

	SeDCloud::instance->actions->execute_remote_binary(binary, arguments_vector);

	for(int i = last_in + 1 ; i < nb_args; i++) {



		sprintf(sz_i, "%s/%i", local_results_folder.c_str(), i);
		arg_local_path = sz_i;


		arg_remote_path = arguments_vector[i];

		//SeDCloud::instance->vm_instances->rsync_from_vm(0, SeDCloud::instance->using_private_ip(), arg_remote_path, arg_local_path);
		SeDCloud::instance->actions->receive_result(arg_remote_path, arg_local_path);

		if (diet_file_set(diet_parameter(pb, i), arg_local_path.c_str(), DIET_VOLATILE)) {
			printf("diet_file_desc_set error\n");
			return 1;
		}
	}

	for(int i = 0; i <= last_in; i++){
		diet_free_data(diet_parameter(pb, i));
	}


	SeDCloud::instance->actions->perform_action_on_end_solve(pb);
}
