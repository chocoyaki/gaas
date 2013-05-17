#include "SeD_deltacloud.hh"
#include <stdio.h>
#include "DIET_uuid.hh"


int create_folder(const char* folder_path) {
	std::string cmd = "";
	cmd.append("mkdir -p ");
	cmd.append(folder_path);
	//printf("CREATE LOCAL DIRECTORY : %s\n", local_results_folder.c_str());
	int env = system(cmd.c_str());

	return env;
}

std::string get_folder_in_dagda_path(const char* folder_name) {
	DagdaImpl *dataManager = DagdaFactory::getDataManager();
	const char* dagda_path = dataManager->getDataPath();



	std::string folder = "";
	folder.append(dagda_path);
	folder.append("/");
	folder.append(folder_name);

	return folder;
}

std::string int2string(int i) {
	char s[512];
	sprintf(s, "%i", i);
	return std::string(s);
}

void append2path(std::string& path, const std::string& add) {
	path.append("/");
	path.append(add);
}

int create_folder_in_dagda_path(const char* folder_name) {
	DagdaImpl *dataManager = DagdaFactory::getDataManager();
	const char* dagda_path = dataManager->getDataPath();


	//the local folder of datas
	std::string local_results_folder = get_folder_in_dagda_path(folder_name);

	int env = create_folder(local_results_folder.c_str());

	return env;
}


int create_folder_in_dagda_path_with_request_id(int reqId) {
	return create_folder_in_dagda_path(int2string(reqId).c_str());
}




CloudServiceBinary::CloudServiceBinary(const std::string& _local_path, const std::string& _remote_path, const std::string& _entry_point_relative_file_path ,
									 const std::string& _remote_path_of_arguments, dietcloud_callback_t _prepocessing,
                         dietcloud_callback_t _postprocessing) : local_path_of_binary(_local_path),
    remote_path_of_binary(_remote_path), entry_point_relative_file_path(_entry_point_relative_file_path){

	//if _remote_path_of_arguments == "" we set the defaut location of data to the folder of the binary
	if (_remote_path_of_arguments.compare("") == 0){
		remote_path_of_arguments = _remote_path;
	}
	else {
		remote_path_of_arguments = _remote_path_of_arguments;
	}

	//default arguments trasnfer Method
	argumentsTransferMethod = filesTransferMethod;

	prepocessing = _prepocessing,
	postprocessing = _postprocessing;

}


CloudServiceBinary::CloudServiceBinary(const CloudServiceBinary& binary) {
    local_path_of_binary = binary.local_path_of_binary;
    remote_path_of_binary = binary.remote_path_of_binary;
    entry_point_relative_file_path = binary.entry_point_relative_file_path;
	remote_path_of_arguments = binary.remote_path_of_arguments;
	argumentsTransferMethod = binary.argumentsTransferMethod;

	prepocessing = binary.prepocessing;
	postprocessing = binary.postprocessing;
}


CloudServiceBinary::CloudServiceBinary() {

}


int CloudServiceBinary::install(const std::string& ip, const std::string& user_name) const {
	if (!isPreinstalled()) {
		return ::rsync_to_vm(local_path_of_binary, remote_path_of_binary, user_name, ip);
	}

	return 0;
}


bool CloudServiceBinary::isPreinstalled() const {
	return local_path_of_binary.compare("") == 0;
}


/*
PreinstalledCloudServiceBinary::PreinstalledCloudServiceBinary(const std::string& _remote_path,
					 const std::string& _entry_point_relative_file_path, const std::string& _remote_path_of_arguments) : CloudServiceBinary("", _remote_path,
						_entry_point_relative_file_path, _remote_path_of_arguments) {

}*/



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


void SeDCloudActions::copy_binary_into_vm(std::string name_of_service, int vm_index) {
	CloudServiceBinary& binary = cloud_service_binaries[name_of_service];

	std::string ip = vm_instances->get_ip(vm_index, is_ip_private);
	binary.install(ip, username);

}




void SeDCloudActions::copy_all_binaries_into_vm(int vm_index) {
    vm_instances->wait_all_ssh_connection(this->is_ip_private);

    std::map<std::string, CloudServiceBinary>::const_iterator iter;
    std::map<std::string, CloudServiceBinary>& binaries = cloud_service_binaries;
    for(iter = binaries.begin(); iter != binaries.end(); iter++) {
        std::string name_of_service = iter->first;
		copy_binary_into_vm(name_of_service, vm_index);
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
		const CloudServiceBinary& binary =iter->second;

		binary.install(address_ip, username);
		//copy_binary_into_vm(name_of_service, 0);

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
    //vm_instances->rsync_to_vm(0, is_ip_private, binary.local_path_of_binary, binary.remote_path_of_binary);
	copy_binary_into_vm(name_of_service, 0);


}


int SeDCloudVMLaunchedAtSolveThenDestroyedActions::perform_action_on_end_solve(diet_profile_t *pb) {

    delete this->vm_instances;
    this->vm_instances = NULL;
}



/*if argumentsTranferMethod == pathsTranferMethod
 then one must have lastin+1==lastout, since pathsout are actually inputs, the result is a dummy string
*/
DIET_API_LIB int
        SeDCloud::service_table_add(const std::string& name_of_service,
                          int last_in,
                          int last_out,
                         const diet_convertor_t* const cvt,
                         const std::string& local_path_of_binary,
                         const std::string& remote_path_of_binary,
                         const std::string& entryPoint,
                         const std::string& remote_path_of_arguments,
                         ArgumentsTranferMethod arguments_transfer_method,
                         dietcloud_callback_t prepocessing,
                         dietcloud_callback_t postprocessing) {



	diet_data_type_t type;
	diet_base_type_t base_type;

	switch (arguments_transfer_method) {
		case filesTransferMethod:
			type = DIET_FILE;
			base_type = DIET_CHAR;
			break;
		case pathsTransferMethod:
			type = DIET_STRING;
			base_type = DIET_CHAR;
			last_out = last_in + 1;
			break;
		default:
			printf("WARNING : %i argumentsTranferMethod not supported\n", arguments_transfer_method);
	}


	diet_profile_desc_t* profile;

	profile = diet_profile_desc_alloc(name_of_service.c_str(), last_in, last_in, last_out);

	for(int i = 0; i <= last_out; i++) {
		diet_generic_desc_set(diet_param_desc(profile, i), type, base_type);
	}

	SeDCloudActions::cloud_service_binaries[name_of_service] = CloudServiceBinary(local_path_of_binary, remote_path_of_binary, entryPoint, remote_path_of_arguments, prepocessing, postprocessing);
	SeDCloudActions::cloud_service_binaries[name_of_service].argumentsTransferMethod = arguments_transfer_method;



	if (diet_service_table_add(profile, cvt, solve)) {
		return 1;
	}


	diet_profile_desc_free(profile);
	diet_print_service_table();

	actions->perform_action_after_service_table_add(name_of_service);

	return 0;
}



int SeDCloud::solve(diet_profile_t *pb) {

	SeDCloud::instance->actions->perform_action_on_begin_solve(pb);

	std::string name(pb->pb_name);
	CloudServiceBinary& binary = SeDCloudActions::cloud_service_binaries[name];
	if (binary.prepocessing != NULL) {
		binary.prepocessing(pb);
	}

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%p\n", binary.postprocessing);

	int reqId = pb->dietReqID;
	std::string szReqId = int2string(reqId);




	std::string local_results_folder = get_folder_in_dagda_path(szReqId.c_str());


	if(binary.argumentsTransferMethod == filesTransferMethod) {
		int env = create_folder(local_results_folder.c_str());
	}


	std::string remote_path_of_arguments = binary.remote_path_of_arguments;
	//a disctinct remote folder for each request
	//the remote folder where datas are stored
	std::string remote_request_folder = remote_path_of_arguments + "/" + szReqId;
	if(binary.argumentsTransferMethod == filesTransferMethod) {
		SeDCloud::instance->actions->create_remote_directory(remote_request_folder);
	}


	int nb_args = pb->last_out + 1;
	int last_in = pb->last_in;
	size_t arg_size;
	char* sz_local_path = NULL;
	char* sz_remote_path = NULL;
	std::string local_path;
	std::string arg_remote_path;
	std::string arg_local_path;
	char sz_i[2048];

	std::vector<std::string> arguments_vector;
	for(int i = 0; i < nb_args; i++) {
		sprintf(sz_i, "%i", i);

		if(binary.argumentsTransferMethod == filesTransferMethod) {
			arg_remote_path = remote_request_folder + "/" + sz_i;
			if (i <= last_in) {
				diet_file_get(diet_parameter(pb, i), &sz_local_path, NULL, &arg_size);
				local_path = sz_local_path;
				SeDCloud::instance->actions->send_arguments(local_path, arg_remote_path);
			}
		}
		else if (binary.argumentsTransferMethod == pathsTransferMethod) {
			diet_string_get(diet_parameter(pb, i), &sz_remote_path, NULL);
			arg_remote_path = sz_remote_path;
		}
		else {
			arg_remote_path = "error";
		}

		arguments_vector.push_back(arg_remote_path);

	}

	SeDCloud::instance->actions->execute_remote_binary(binary, arguments_vector);

	for(int i = last_in + 1 ; i < nb_args; i++) {





		if(binary.argumentsTransferMethod == filesTransferMethod) {
			sprintf(sz_i, "%s/%i", local_results_folder.c_str(), i);
			arg_local_path = sz_i;
			arg_remote_path = arguments_vector[i];
			SeDCloud::instance->actions->receive_result(arg_remote_path, arg_local_path);
			if (diet_file_set(diet_parameter(pb, i), arg_local_path.c_str(), DIET_VOLATILE)) {
				printf("diet_file_desc_set error\n");
				return 1;
			}
		}
		else if (binary.argumentsTransferMethod == pathsTransferMethod){

		}
	}

	for(int i = 0; i <= last_in; i++){
		diet_free_data(diet_parameter(pb, i));
	}

	printf("h1\n");
	if (binary.postprocessing != NULL){
		printf("h2\n");
		binary.postprocessing(pb);
	}

	SeDCloud::instance->actions->perform_action_on_end_solve(pb);

	return 0;
}


/**
	params :
		IN:
		0 vm collection name : STRING
		1 vm count : INT
		2 vm image : STRING
		3 vm profile : STRING
		4 delta cloud api url : STRING
		5 deltacloud user name
		6 deltacloud passwd
		7 vm user
		8 take private ips
		OUT:
		9 vm ips : FILE

*/

DIET_API_LIB int SeDCloud::service_homogeneous_vm_instanciation_add() {
	diet_profile_desc_t* profile;

	profile = diet_profile_desc_alloc("homogeneous_vm_instanciation", 3, 3, 4);
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT);
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 4), DIET_FILE, DIET_CHAR);

	diet_service_table_add(profile,  NULL, SeDCloud::homogeneous_vm_instanciation_solve);
}

int SeDCloud::homogeneous_vm_instanciation_solve(diet_profile_t *pb) {
	char* vm_collection_name;
	int vm_count;
	char* vm_image;
	char* vm_profile;
	char* deltacloud_api_url;
	char* deltacloud_user_name;
	char* deltacloud_passwd;
	char* vm_user;
	//0 if we take the public ip 1 if we take the private ip
	int is_ip_private;

	diet_string_get(diet_parameter(pb, 0), &vm_collection_name, NULL);
	diet_scalar_get(diet_parameter(pb, 1), &vm_count, NULL);
	diet_string_get(diet_parameter(pb, 2), &vm_image, NULL);
	diet_string_get(diet_parameter(pb, 3), &vm_profile, NULL);
	diet_string_get(diet_parameter(pb, 4), &deltacloud_api_url, NULL);
	diet_string_get(diet_parameter(pb, 5), &deltacloud_user_name, NULL);
	diet_string_get(diet_parameter(pb, 6), &deltacloud_passwd, NULL);
	diet_string_get(diet_parameter(pb, 7), &vm_user, NULL);
	diet_scalar_get(diet_parameter(pb, 8), &is_ip_private, NULL);
	//one creates the vm instances
	//SeDCloudActions* actions = new SeDCloudActions();

	std::vector<IaaS::Parameter> params;
	params.push_back(IaaS::Parameter(HARDWARE_PROFILE_ID_PARAM, vm_profile));

	IaaS::VMInstances* instances = new IaaS::VMInstances(vm_image, vm_count, deltacloud_api_url, deltacloud_user_name, deltacloud_passwd, vm_user, params);
	reserved_vms[vm_collection_name] = instances;
	instances->wait_all_instances_running();
	instances->wait_all_ssh_connection(is_ip_private);

	int reqId = pb->dietReqID;
	std::string szReqId = int2string(reqId);
	std::string local_results_folder = get_folder_in_dagda_path(szReqId.c_str());
	int env = create_folder(local_results_folder.c_str());


	std::ostringstream vm_ip_file_path;
	boost::uuids::uuid uuid = diet_generate_uuid();

	vm_ip_file_path << local_results_folder << "-" << uuid << ".txt";

	std::vector<std::string> ips;
	instances->get_ips(ips, is_ip_private);

	//write ips in file
	FILE* vm_ips_file = fopen(vm_ip_file_path.str().c_str(), "w");
	if (vm_ips_file == NULL) {
		return -1;
	}
	for(int i = 0; i < vm_count; i++) {
		fprintf(vm_ips_file, "%s\n", ips[i].c_str());
	}
	fclose(vm_ips_file);

	diet_file_set(diet_parameter(pb, 9), vm_ip_file_path.str().c_str(), DIET_PERSISTENT_RETURN);




	for(int i=0; i < 8; i++) {
		diet_free_data(diet_parameter(pb, i));
	}

	return 0;
}
