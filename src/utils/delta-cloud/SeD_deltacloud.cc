/**
 * @file Sed_deltacloud.cc
 *
 * @brief  DIET SeD_deltacloud class implementation
 *
 * @author  Lamiel TOCH (lamiel.toch@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include "SeD_deltacloud.hh"
#include <stdio.h>
#include "DIET_uuid.hh"
#include <fstream>
#include "Iaas_deltacloud.hh"
#include <unistd.h>
#include <libgen.h>

std::string copy_to_tmp_file(const std::string& src, const std::string& ext) {
	char* src_path = strdup(src.c_str());
	char* dir = dirname(src_path);

	std::ostringstream copy_file_path;
	boost::uuids::uuid uuid = diet_generate_uuid();
	copy_file_path << dir << "/" << uuid << ext;
	std::string cmd = "cp " + src + " " + copy_file_path.str().c_str();
	int env = system(cmd.c_str());
	free(src_path);

	if (env) return "";

	return copy_file_path.str();
}

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


std::string create_tmp_file(diet_profile_t* pb, const std::string ext) {
	//create the tmp ip file
	int reqId = pb->dietReqID;
	std::string szReqId = int2string(reqId);
	std::string local_results_folder = get_folder_in_dagda_path(szReqId.c_str());
	int env = create_folder(local_results_folder.c_str());
	std::ostringstream file_path;
	boost::uuids::uuid uuid = diet_generate_uuid();
	file_path << local_results_folder << "/" << uuid << ext;

	FILE* vm_ips_file = fopen(file_path.str().c_str(), "w");
	if (vm_ips_file == NULL) {
		return "";
	}

	fclose(vm_ips_file);


	return file_path.str();

}


int write_lines(const std::vector<std::string>& lines, const std::string& file_path) {
	FILE* file = fopen(file_path.c_str(), "w");

	if (file == NULL) return -1;

	for(int i = 0; i < lines.size(); i++){
		fprintf(file, "%s\n", lines[i].c_str());
	}

	fclose(file);

	return 0;
}

CloudServiceBinary::CloudServiceBinary(const std::string& _local_path, const std::string& _remote_path, const std::string& _entry_point_relative_file_path ,
									 const std::string& _remote_path_of_arguments, dietcloud_callback_t _prepocessing,
                         dietcloud_callback_t _postprocessing, ArgumentsTransferMethod _argsTranferMethod, const std::string& _installer_relative_path) : local_path_of_binary(_local_path),
    remote_path_of_binary(_remote_path), entry_point_relative_file_path(_entry_point_relative_file_path), installer_relative_path(_installer_relative_path){

	//if _remote_path_of_arguments == "" we set the defaut location of data to the folder of the binary
	if (_remote_path_of_arguments.compare("") == 0){
		remote_path_of_arguments = _remote_path;
	}
	else {
		remote_path_of_arguments = _remote_path_of_arguments;
	}


	arguments_transfer_method = _argsTranferMethod;

	prepocessing = _prepocessing,
	postprocessing = _postprocessing;


}


CloudServiceBinary::CloudServiceBinary(const CloudServiceBinary& binary) {
    local_path_of_binary = binary.local_path_of_binary;
    remote_path_of_binary = binary.remote_path_of_binary;
    entry_point_relative_file_path = binary.entry_point_relative_file_path;
	remote_path_of_arguments = binary.remote_path_of_arguments;
	arguments_transfer_method = binary.arguments_transfer_method;

	prepocessing = binary.prepocessing;
	postprocessing = binary.postprocessing;
	installer_relative_path = binary.installer_relative_path;

	name = binary.name;
	last_in = binary.last_in;
	last_out = binary.last_out;
}


CloudServiceBinary::CloudServiceBinary() {

}


int CloudServiceBinary::install(const std::string& ip, const std::string& vm_user_name) const {
	int env = 0;

	if (!is_preinstalled()) {
		env = ::rsync_to_vm(local_path_of_binary, remote_path_of_binary, vm_user_name, ip);

		if (env) return env;

		if (has_installer()) {
			std::string cmd = "cd " + remote_path_of_binary + "; ";
			cmd.append("./" + installer_relative_path);
			env = ::execute_command_in_vm(cmd, "root", ip, "");
		}
	}

	return env;
}


bool CloudServiceBinary::is_preinstalled() const {
	return local_path_of_binary.compare("") == 0;
}

bool CloudServiceBinary::has_installer() const {
	return installer_relative_path.compare("") != 0;
}



int CloudServiceBinary::execute_remote(const std::string& ip, const std::string& vm_user_name, const std::vector<std::string>& args) const {
	 printf(">>>>>>>>>>>>>>>>EXECUTE REMOTE\n");
	 std::string sz_args = "";
	for(int i = 0; i < args.size(); i++) {
		sz_args.append(" " + args[i]);
	}

	std::string cd;
	if (remote_path_of_binary.compare("") == 0) {
		cd = "";
	}
	else {
		cd = "cd " + remote_path_of_binary + "; ./";
	}

    ::execute_command_in_vm(cd + entry_point_relative_file_path, vm_user_name, ip, sz_args);
}


int SeDCloudActions::launch_vms() {
	IaaS::VMInstances* insts = new IaaS::VMInstances (this->image_id, this->vm_count, this->base_url, this->username, this->password, this->vm_user, this->params);

	vm_instances.push_back(insts);

	if (insts == NULL) return -1;


	printf("%i\n", this->is_ip_private);
	insts->wait_all_ssh_connection(this->is_ip_private);
	fill_ips();
	send_vm_ips_to_master();

	int env = perform_action_on_vm_os_ready();

	if (env)  return -1;

	return 0;
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

    //this->vm_instances = NULL;
}


SeDCloudActions::SeDCloudActions() {
     //this->vm_instances = NULL;
}

int SeDCloudActions::send_arguments(const std::string& local_path, const std::string& remote_path) {
    rsync_to_vm(local_path, remote_path, vm_user, master_ip);
}

int SeDCloudActions::receive_result(const std::string& result_remote_path, const std::string& result_local_path) {
    rsync_from_vm(result_remote_path, result_local_path, vm_user, master_ip);
}

int SeDCloudActions::execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args) {
	binary.execute_remote(master_ip, vm_user, args);
}

int SeDCloudActions::create_remote_directory(const std::string& remote_path) {
	create_directory_in_vm(remote_path, vm_user, master_ip);
}


void SeDCloudActions::copy_binary_into_vm(std::string name_of_service, int vm_index) {
	CloudServiceBinary& binary = cloud_service_binaries[name_of_service];

	std::string ip = vm_instances.back()->get_ip(vm_index, is_ip_private);
	binary.install(ip, vm_user);

}


void SeDCloudActions::copy_binary_into_all_vms(std::string name) {
	for(int i = 0; i < vm_count; i++) {
		copy_binary_into_vm(name, i);
	}
}


void SeDCloudActions::copy_all_binaries_into_vm(int vm_index) {
    vm_instances.back()->wait_all_ssh_connection(this->is_ip_private);

    std::map<std::string, CloudServiceBinary>::const_iterator iter;
    std::map<std::string, CloudServiceBinary>& binaries = cloud_service_binaries;
    for(iter = binaries.begin(); iter != binaries.end(); iter++) {
        std::string name_of_service = iter->first;
		copy_binary_into_vm(name_of_service, vm_index);
    }
}

void SeDCloudActions::copy_all_binaries_into_all_vms() {
	for(int i = 0; i < vm_count; i++){
		copy_all_binaries_into_vm(i);
	}
}


int SeDCloudAndVMLaunchedActions::perform_action_on_begin_solve(diet_profile_t *pb) {

}

int SeDCloudAndVMLaunchedActions::perform_action_on_end_solve(diet_profile_t *pb) {

}

void SeDCloudAndVMLaunchedActions::perform_action_on_sed_creation() {
    launch_vms();
}





/*
On sed launch, we copy all binaries for each service
*/
void SeDCloudAndVMLaunchedActions::perform_action_on_sed_launch() {
    copy_all_binaries_into_all_vms();
}


int SeDCloudAndVMLaunchedActions::perform_action_after_service_table_add(const std::string& name_of_service) {

}




void SeDCloudMachinesActions::perform_action_on_sed_creation() {
    //rien pas de creation de vm
}




/*
On sed launch, we copy all binaries for each service
*/
void SeDCloudMachinesActions::perform_action_on_sed_launch() {
    //SeDCloud<SeDCloudAndVMLaunchedActions>::get()->vm_instances->wait_all_ssh_connection(this->is_ip_private);

    std::map<std::string, CloudServiceBinary>::const_iterator iter;
    std::map<std::string, CloudServiceBinary>& binaries = cloud_service_binaries;
    for(iter = binaries.begin(); iter != binaries.end(); iter++) {
        std::string name_of_service = iter->first;
		const CloudServiceBinary& binary = iter->second;

		for(int i = 0; i < ips.size(); i++){
			std::string ip = ips[i];
			binary.install(ip, vm_user);
			//copy_binary_into_vm(name_of_service, 0);
		}
    }


    //fork and wait until ssh connection is broken
    pid_t pid = fork();

    if (pid == 0){
		//in the child
		//std::ostringstream cmd;
		//cmd << "ssh " << vm_user << "@" << get_master_ip() << " exit";

		int env = 0;
		do{
			env = test_ssh_connection(vm_user, get_master_ip());
			sleep(machine_alive_interval);
		}while(env == 0);

		printf("ssh error : lost connection\n");
		printf("Destruction of one SeDCloudMachinesActions\n");

		pid_t parent_pid = getppid();
		kill(parent_pid, SIGTERM);

		exit(0);
    }
    else {
		//in the parent
		//we do nothing
    }


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
        //this->vm_instances = new IaaS::VMInstances (this->image_id, this->vm_count, this->base_url, this->username, this->password, this->vm_user, this->params);
        launch_vms();

        copy_all_binaries_into_all_vms();

    }
    else {
        statistics_on_services.increment_call_number(service_name);
    }


}

int SeDCloudVMLaunchedAtFirstSolveActions::perform_action_on_end_solve(diet_profile_t *pb) {

}


int SeDCloudVMLaunchedAtSolveThenDestroyedActions::perform_action_on_begin_solve(diet_profile_t *pb) {
    //this->vm_instances = new IaaS::VMInstances (this->image_id, this->vm_count, this->base_url, this->username, this->password, this->vm_user, this->params);
    launch_vms();



    std::string name_of_service = pb->pb_name;
    CloudServiceBinary& binary = cloud_service_binaries[name_of_service];
    //vm_instances->rsync_to_vm(0, is_ip_private, binary.local_path_of_binary, binary.remote_path_of_binary);
	copy_binary_into_all_vms(name_of_service);


}


int SeDCloudVMLaunchedAtSolveThenDestroyedActions::perform_action_on_end_solve(diet_profile_t *pb) {

   destroy_vms();
}



diet_profile_desc_t* CloudServiceBinary::to_diet_profile() {
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

	profile = diet_profile_desc_alloc(name.c_str(), last_in, last_in, last_out);

	for(int i = 0; i <= last_out; i++) {
		diet_generic_desc_set(diet_param_desc(profile, i), type, base_type);
	}

	return profile;
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
                         ArgumentsTransferMethod arguments_transfer_method,
                         dietcloud_callback_t prepocessing,
                         dietcloud_callback_t postprocessing,
                         const std::string& remote_path_of_binary_installer) {



	CloudServiceBinary service_binary(local_path_of_binary,
				remote_path_of_binary, entryPoint, remote_path_of_arguments, prepocessing, postprocessing,
				arguments_transfer_method, remote_path_of_binary_installer);

	service_binary.name = name_of_service;
	service_binary.last_in = last_in;
	service_binary.last_out = last_out;

	diet_profile_desc_t* profile = service_binary.to_diet_profile();

	actions->set_cloud_service_binary(name_of_service, service_binary);


	if (diet_service_table_add(profile, cvt, solve)) {
		return 1;
	}


	diet_profile_desc_free(profile);
	//diet_print_service_table();

	actions->perform_action_after_service_table_add(name_of_service);

	return 0;
}



int SeDCloud::solve(diet_profile_t *pb) {

	SeDCloud::instance->actions->perform_action_on_begin_solve(pb);

	std::string name(pb->pb_name);
	const std::map<std::string, CloudServiceBinary>& binaries = instance->actions->get_cloud_service_binaries();
	const CloudServiceBinary& binary = binaries.at(name);
	if (binary.prepocessing != NULL) {
		binary.prepocessing(pb);
	}

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%p\n", binary.postprocessing);

	int reqId = pb->dietReqID;
	std::string szReqId = int2string(reqId);




	std::string local_results_folder = get_folder_in_dagda_path(szReqId.c_str());


	if(binary.arguments_transfer_method == filesTransferMethod) {
		int env = create_folder(local_results_folder.c_str());
	}


	std::string remote_path_of_arguments = binary.remote_path_of_arguments;
	//a disctinct remote folder for each request
	//the remote folder where datas are stored
	std::string remote_request_folder = remote_path_of_arguments + "/" + szReqId;
	if(binary.arguments_transfer_method == filesTransferMethod) {
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

		if(binary.arguments_transfer_method == filesTransferMethod) {
			arg_remote_path = remote_request_folder + "/" + sz_i;
			if (i <= last_in) {
				diet_file_get(diet_parameter(pb, i), &sz_local_path, NULL, &arg_size);
				local_path = sz_local_path;
				SeDCloud::instance->actions->send_arguments(local_path, arg_remote_path);
			}
		}
		else if (binary.arguments_transfer_method == pathsTransferMethod) {
			if (i <= last_in) {
				diet_string_get(diet_parameter(pb, i), &sz_remote_path, NULL);
				arg_remote_path = sz_remote_path;
			}
		}
		else {
			arg_remote_path = "error";
		}

		arguments_vector.push_back(arg_remote_path);

	}

	SeDCloud::instance->actions->execute_remote_binary(binary, arguments_vector);

	for(int i = last_in + 1 ; i < nb_args; i++) {





		if(binary.arguments_transfer_method == filesTransferMethod) {
			sprintf(sz_i, "%s/%i", local_results_folder.c_str(), i);
			arg_local_path = sz_i;
			arg_remote_path = arguments_vector[i];
			SeDCloud::instance->actions->receive_result(arg_remote_path, arg_local_path);
			if (diet_file_set(diet_parameter(pb, i), arg_local_path.c_str(), DIET_VOLATILE)) {
				printf("diet_file_desc_set error\n");
				return 1;
			}
		}
		else if (binary.arguments_transfer_method == pathsTransferMethod){

		}
	}

	for(int i = 0; i <= last_in; i++){
		diet_free_data(diet_parameter(pb, i));
	}

//	printf("h1\n");
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

	profile = diet_profile_desc_alloc("homogeneous_vm_instanciation", 8, 8, 9);
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT);
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 4), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 5), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 6), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 7), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 8), DIET_SCALAR, DIET_INT);
	diet_generic_desc_set(diet_param_desc(profile, 9), DIET_FILE, DIET_CHAR);

	diet_service_table_add(profile,  NULL, SeDCloud::homogeneous_vm_instanciation_solve);

	diet_profile_desc_free(profile);
	//diet_print_service_table();
}

/**
	params :
		IN:
		0 vm count : INT
		1 vm image : STRING
		2 vm profile : STRING
		3 vm user : STRING
		4 take private ips : INT
		OUT:
		5 vm ips : FILE

*/


DIET_API_LIB int SeDCloud::service_homogeneous_vm_instanciation_add(const CloudAPIConnection& _cloud_api_connection) {
	cloud_api_connection_for_vm_instanciation = _cloud_api_connection;

	diet_profile_desc_t* profile;

	profile = diet_profile_desc_alloc("homogeneous_vm_instanciation", 4, 4, 5);
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_INT);
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 4), DIET_SCALAR, DIET_INT);
	diet_generic_desc_set(diet_param_desc(profile, 5), DIET_FILE, DIET_CHAR);

	diet_service_table_add(profile,  NULL, SeDCloud::homogeneous_vm_instanciation_with_one_cloud_api_connection_solve);

	diet_profile_desc_free(profile);
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
		9 keyname
		OUT:
		10 vm ips : FILE

*/

DIET_API_LIB int SeDCloud::service_homogeneous_vm_instanciation_with_keyname_add() {
	diet_profile_desc_t* profile;

	profile = diet_profile_desc_alloc("homogeneous_vm_instanciation", 9, 9, 10);
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT);
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 4), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 5), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 6), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 7), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 8), DIET_SCALAR, DIET_INT);
	diet_generic_desc_set(diet_param_desc(profile, 9), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 10), DIET_FILE, DIET_CHAR);

	diet_service_table_add(profile,  NULL, SeDCloud::homogeneous_vm_instanciation_with_keyname_solve);

	diet_profile_desc_free(profile);
	//diet_print_service_table();
}




DIET_API_LIB int SeDCloud::service_vm_destruction_by_ip_add() {
	diet_profile_desc_t* profile;

	profile = diet_profile_desc_alloc("vm_destruction_by_ip", 4, 4, 4);
	//IN : collection name
	//const std::string & _url_api_base, const std::string & _username, const std::string & _password


	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_STRING, DIET_CHAR); //url_api
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_STRING, DIET_CHAR); //user_name
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_STRING, DIET_CHAR); //password
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_FILE, DIET_CHAR); //ips
	diet_generic_desc_set(diet_param_desc(profile, 4), DIET_SCALAR, DIET_INT); //private or public ips

	diet_service_table_add(profile, NULL, SeDCloud::vm_destruction_by_ip_solve);

	diet_profile_desc_free(profile);
	//diet_print_service_table();
}


DIET_API_LIB int SeDCloud::service_cloud_federation_vm_destruction_by_ip_add(const std::vector<CloudAPIConnection>& _cloud_api_connection) {
		cloud_api_connection_for_vm_destruction = _cloud_api_connection;

		diet_profile_desc_t* profile;
		profile = diet_profile_desc_alloc("vm_destruction_by_ip", 1, 1, 1);

		diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR); //ips
		diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT); //private or public ips

		diet_service_table_add(profile, NULL, SeDCloud::cloud_federation_vm_destruction_by_ip_solve);
		diet_profile_desc_free(profile);
}


DIET_API_LIB int SeDCloud::service_launch_another_sed_add() {
	diet_profile_desc_t* profile;
	profile = diet_profile_desc_alloc("launch_another_sed", 3, 3, 4);
	//IN
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_STRING, DIET_CHAR); //sed_executable_path
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR); //diet_cfg
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_FILE, DIET_CHAR); //file data
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_STRING, DIET_CHAR); //string data
	//OUT
	diet_generic_desc_set(diet_param_desc(profile, 4), DIET_SCALAR, DIET_INT); //result of the launch : 0 : KO, != 0 : success

	diet_service_table_add(profile, NULL, SeDCloud::launch_another_sed_solve);
	diet_profile_desc_free(profile);
}

int SeDCloud::launch_another_sed_solve(diet_profile_t* pb) {
	char* cfg_path;
	size_t size;
	char* data_file_path;
	char* sed_executable_path;
	char* string_data;

	diet_string_get(diet_parameter(pb, 0), &sed_executable_path, NULL);
	diet_file_get(diet_parameter(pb, 1), &cfg_path, NULL, &size);
	diet_file_get(diet_parameter(pb, 2), &data_file_path, NULL, &size);
	diet_string_get(diet_parameter(pb, 3), &string_data, NULL);

	std::string data_copy_path = copy_to_tmp_file(data_file_path, ".dat");

	pid_t pid = fork();

	if (pid == 0) {
		//in the process child
		int env;

		int nb_args = pb->last_in + 1;
		char** argv = new char* [nb_args + 2];
		char* sed_path_base_name = basename(sed_executable_path);
		argv[0] = sed_path_base_name;
		argv[1] = cfg_path;
		argv[2] = const_cast<char*> (data_copy_path.c_str());
		argv[3] = string_data;
		argv[4] = NULL;


		printf("inside fork\n");
		std::string d_cmd = "cat ";
		d_cmd.append(data_copy_path);
		env = system(d_cmd.c_str());

		env = execv(sed_executable_path, argv);

		//should not arrive here
		printf("erreur #%i execution execv inside children process\n", env);
		exit(-1);
	}
	else {
		if (pid > 0) {
			printf("fork executing: success\n");
			int* child_status = new int;

			waitpid(pid, child_status, WNOHANG);


			diet_scalar_set(diet_parameter(pb, 4), child_status, DIET_PERSISTENT_RETURN, DIET_INT);

			return 0;
		}
	}




}

int SeDCloud::homogeneous_vm_instanciation_solve(diet_profile_t *pb) {
	char* vm_collection_name;
	int* vm_count;
	char* vm_image;
	char* vm_profile;
	char* deltacloud_api_url;
	char* deltacloud_user_name;
	char* deltacloud_passwd;
	char* vm_user;
	//0 if we take the public ip 1 if we take the private ip
	int* is_ip_private;

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

	std::vector<IaaS::Parameter> params;
	params.push_back(IaaS::Parameter(HARDWARE_PROFILE_ID_PARAM, vm_profile));


	printf("instanciation : image='%s', profile='%s', vm_count=%i, is_ip_private=%i\n", vm_image, vm_profile, *vm_count, *is_ip_private);
	IaaS::VMInstances* instances;
	std::vector<std::string> ips;
	instances = new IaaS::VMInstances(vm_image, *vm_count, deltacloud_api_url, deltacloud_user_name, deltacloud_passwd, vm_user, params);
	instances->wait_all_instances_running();
	instances->wait_all_ssh_connection(*is_ip_private);
	instances->get_ips(ips, *is_ip_private);


	std::string diet_tmp_ips_file = create_tmp_file(pb, ".txt");
	write_lines(ips, diet_tmp_ips_file);
	diet_file_set(diet_parameter(pb, 9), diet_tmp_ips_file.c_str(), DIET_PERSISTENT_RETURN);





	return 0;
}


int SeDCloud::homogeneous_vm_instanciation_with_one_cloud_api_connection_solve(diet_profile_t* pb) {
	char* vm_collection_name;
	int* vm_count;
	char* vm_image;
	char* vm_profile;
	const char* deltacloud_api_url;
	const char* deltacloud_user_name;
	const char* deltacloud_passwd;
	char* vm_user;
	//0 if we take the public ip 1 if we take the private ip
	int* is_ip_private;


	deltacloud_api_url = cloud_api_connection_for_vm_instanciation.base_url.c_str();
	deltacloud_user_name = cloud_api_connection_for_vm_instanciation.username.c_str();
	deltacloud_passwd = cloud_api_connection_for_vm_instanciation.password.c_str();

	diet_scalar_get(diet_parameter(pb, 0), &vm_count, NULL);
	diet_string_get(diet_parameter(pb, 1), &vm_image, NULL);
	diet_string_get(diet_parameter(pb, 2), &vm_profile, NULL);
	diet_string_get(diet_parameter(pb, 3), &vm_user, NULL);
	diet_scalar_get(diet_parameter(pb, 4), &is_ip_private, NULL);
	//one creates the vm instances


	printf("url = %s, instanciation : image='%s', profile='%s', vm_count=%i\n", deltacloud_api_url, vm_image, vm_profile, *vm_count);

	std::vector<IaaS::Parameter> params;
	params.push_back(IaaS::Parameter(HARDWARE_PROFILE_ID_PARAM, vm_profile));

	IaaS::VMInstances* instances;
	std::vector<std::string> ips;



	instances = new IaaS::VMInstances(vm_image, *vm_count, deltacloud_api_url, deltacloud_user_name, deltacloud_passwd, vm_user, params);

	instances->wait_all_instances_running();
	instances->wait_all_ssh_connection(*is_ip_private);

	instances->get_ips(ips, *is_ip_private);

	int reqId = pb->dietReqID;
	std::string szReqId = int2string(reqId);
	std::string local_results_folder = get_folder_in_dagda_path(szReqId.c_str());
	int env = create_folder(local_results_folder.c_str());


	std::ostringstream vm_ip_file_path;
	boost::uuids::uuid uuid = diet_generate_uuid();
	vm_ip_file_path << local_results_folder << "-" << uuid << ".txt";






	//write ips in file
	FILE* vm_ips_file = fopen(vm_ip_file_path.str().c_str(), "w");
	if (vm_ips_file == NULL) {
		return -1;
	}
	for(int i = 0; i < *vm_count; i++) {
		fprintf(vm_ips_file, "%s\n", ips[i].c_str());
	}
	fclose(vm_ips_file);

	diet_file_set(diet_parameter(pb, 5), vm_ip_file_path.str().c_str(), DIET_PERSISTENT_RETURN);

	return 0;
}



int SeDCloud::homogeneous_vm_instanciation_with_keyname_solve(diet_profile_t *pb) {
	char* vm_collection_name;
	int* vm_count;
	char* vm_image;
	char* vm_profile;
	char* deltacloud_api_url;
	char* deltacloud_user_name;
	char* deltacloud_passwd;
	char* vm_user;
	//0 if we take the public ip 1 if we take the private ip
	int* is_ip_private;
	char* keyname;

	diet_string_get(diet_parameter(pb, 0), &vm_collection_name, NULL);
	diet_scalar_get(diet_parameter(pb, 1), &vm_count, NULL);
	diet_string_get(diet_parameter(pb, 2), &vm_image, NULL);
	diet_string_get(diet_parameter(pb, 3), &vm_profile, NULL);
	diet_string_get(diet_parameter(pb, 4), &deltacloud_api_url, NULL);
	diet_string_get(diet_parameter(pb, 5), &deltacloud_user_name, NULL);
	diet_string_get(diet_parameter(pb, 6), &deltacloud_passwd, NULL);
	diet_string_get(diet_parameter(pb, 7), &vm_user, NULL);
	diet_scalar_get(diet_parameter(pb, 8), &is_ip_private, NULL);
	diet_string_get(diet_parameter(pb, 9), &keyname, NULL);
	//one creates the vm instances


	printf("instanciation : image='%s', profile='%s', vm_count=%i, keyname=%s\n", vm_image, vm_profile, *vm_count, keyname);

	std::vector<IaaS::Parameter> params;
	params.push_back(IaaS::Parameter(HARDWARE_PROFILE_ID_PARAM, vm_profile));
	params.push_back(IaaS::Parameter(KEYNAME_PARAM, keyname));

	IaaS::VMInstances* instances;
	std::vector<std::string> ips;


	//if (reserved_vms.count(vm_collection_name) == 0 ) {
		instances = new IaaS::VMInstances(vm_image, *vm_count, deltacloud_api_url, deltacloud_user_name, deltacloud_passwd, vm_user, params);
		//reserved_vms[vm_collection_name] = instances;
		instances->wait_all_instances_running();
		instances->wait_all_ssh_connection(*is_ip_private);
		instances->get_ips(ips, *is_ip_private);

		pid_t pid = fork();

		if (pid == 0) {
			//je suis le fils

			SeDCloudMachinesActions* actions = new SeDCloudMachinesActions(ips, vm_user);
			actions->clone_service_binaries(instance->get_actions());
			instance->setActions(actions);



		}
		else {
			//je suis le pere

			int reqId = pb->dietReqID;
			std::string szReqId = int2string(reqId);
			std::string local_results_folder = get_folder_in_dagda_path(szReqId.c_str());
			int env = create_folder(local_results_folder.c_str());


			std::ostringstream vm_ip_file_path;
			boost::uuids::uuid uuid = diet_generate_uuid();
			vm_ip_file_path << local_results_folder << "-" << uuid << ".txt";






			//write ips in file
			FILE* vm_ips_file = fopen(vm_ip_file_path.str().c_str(), "w");
			if (vm_ips_file == NULL) {
				return -1;
			}
			for(int i = 0; i < *vm_count; i++) {
				fprintf(vm_ips_file, "%s\n", ips[i].c_str());
			}
			fclose(vm_ips_file);

			diet_file_set(diet_parameter(pb, 10), vm_ip_file_path.str().c_str(), DIET_PERSISTENT_RETURN);




			for(int i=0; i < 10; i++) {
				diet_free_data(diet_parameter(pb, i));
			}
		}







	return 0;
}



















//useless if there is a cloud federation
int SeDCloud::vm_destruction_by_ip_solve(diet_profile_t *pb) {
	char* path;
	size_t size;
	char* url_api;
	char* user_name;
	char* password;
	int* select_private_ips;

	diet_string_get(diet_parameter(pb, 0), &url_api, NULL);
	diet_string_get(diet_parameter(pb, 1), &user_name, NULL);
	diet_string_get(diet_parameter(pb, 2), &password, NULL);
	diet_file_get(diet_parameter(pb, 3), &path, NULL, &size);
	diet_scalar_get(diet_parameter(pb, 4), &select_private_ips, NULL);

	IaaS::IaasInterface* interface = new IaaS::Iaas_deltacloud(url_api, user_name, password);

	std::vector<std::string> ips;
	readlines(path, ips);
	interface->terminate_instances_by_ips(ips, *select_private_ips);


	for(int i=0; i <= 4; i++) {
		diet_free_data(diet_parameter(pb, i));
	}

	delete interface;


	//TODO : check if vms are really freed
	return 0;
}


//usefull if there is a cloud federation
int SeDCloud::cloud_federation_vm_destruction_by_ip_solve(diet_profile_t *pb) {
	char* path;
	size_t size;
	const char* url_api;
	const char* user_name;
	const char* password;
	int* select_private_ips;

	diet_file_get(diet_parameter(pb, 0), &path, NULL, &size);
	diet_scalar_get(diet_parameter(pb, 1), &select_private_ips, NULL);

	std::vector<std::string> ips;
	readlines(path, ips);
	int env = -1;
	for(int i = 0; i < cloud_api_connection_for_vm_destruction.size(); i++) {

		url_api = cloud_api_connection_for_vm_destruction[i].base_url.c_str();
		user_name = cloud_api_connection_for_vm_destruction[i].username.c_str();
		password = cloud_api_connection_for_vm_destruction[i].password.c_str();


		IaaS::IaasInterface* interface = new IaaS::Iaas_deltacloud(url_api, user_name, password);
		env = interface->terminate_instances_by_ips(ips, *select_private_ips);
		delete interface;

		if (env == 0) break;
	}

	//TODO : check if vms are really freed
	return 0;
}


/*
	<node id="copy-to-machine" path="rsync_to_vm" >
		<in name="file_to_copy" type="DIET_FILE" />
		<in name="destination_folder" type="DIET_STRING" />
		<in name="vm_user" type="DIET_STRING" />
		<in name="ips" type="DIET_FILE" />
		<in name="vm_index" type="DIET_INT" value="0" />
		<out name="ip" type="DIET_STRING" />
	</node>
*/
DIET_API_LIB int SeDCloud::service_rsync_to_vm_add() {
	diet_profile_desc_t* profile;

	profile = diet_profile_desc_alloc("rsync_to_vm", 4, 4, 5);
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_FILE, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 4), DIET_SCALAR, DIET_INT);
	diet_generic_desc_set(diet_param_desc(profile, 5), DIET_STRING, DIET_CHAR);

	diet_service_table_add(profile,  NULL, SeDCloud::rsync_to_vm_solve);

	diet_profile_desc_free(profile);
	//diet_print_service_table();
}


/**
	IN:
		0 : vm_user
		1 : vm ips file
		2 : vm index
		3 : remote path
	OUT:
		4 : file.tgz
*/
DIET_API_LIB int SeDCloud::service_get_tarball_from_vm_add() {
	diet_profile_desc_t* profile;

	profile = diet_profile_desc_alloc("get_tarball_from_vm", 3, 3, 4);
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_SCALAR, DIET_INT);
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 4), DIET_FILE, DIET_CHAR);

	diet_service_table_add(profile,  NULL, SeDCloud::get_tarball_from_vm_solve);

	diet_profile_desc_free(profile);
	//diet_print_service_table();
}

int SeDCloud::get_tarball_from_vm_solve(diet_profile_t *pb) {
	char* vm_user;
	char* ips_file_path;
	int* vm_index;
	char* source_file_path;
	char* out;
	size_t size;

	diet_string_get(diet_parameter(pb, 0), &vm_user, NULL);
	diet_file_get(diet_parameter(pb, 1), &ips_file_path, NULL, &size);
	diet_scalar_get(diet_parameter(pb, 2), &vm_index, NULL);
	diet_string_get(diet_parameter(pb, 3), &source_file_path, NULL);

	char* c_ip = readline(ips_file_path, *vm_index);

	if (c_ip == NULL) {
		return -1;
	}

	printf(">>>>>>>>>>>>>>get_tarball_from_vm solve\n");

	printf(">>>>>>>>>>>>>>make tarball\n");

	std::ostringstream remote_tarball_path;
	boost::uuids::uuid uuid = diet_generate_uuid();
	remote_tarball_path << "/tmp/tarball"<< uuid << ".tar.gz";

	std::string remote_cmd = "tar cvzf ";
	remote_cmd.append(remote_tarball_path.str() + " ");
	remote_cmd.append(source_file_path);


	int env = ::execute_command_in_vm(remote_cmd, vm_user, c_ip, "");

	if (env) return -1;

	int reqId = pb->dietReqID;
	std::string szReqId = int2string(reqId);
	std::string local_results_folder = get_folder_in_dagda_path(szReqId.c_str());
	env = create_folder(local_results_folder.c_str());

	if (env) return -1;

	std::ostringstream out_path;
	uuid = diet_generate_uuid();
	out_path << local_results_folder << "/" << uuid << ".tar.gz";

	env = rsync_from_vm(remote_tarball_path.str(), out_path.str().c_str(), vm_user, c_ip);

	if (env) return -1;

	diet_file_set(diet_parameter(pb, 4), out_path.str().c_str(), DIET_PERSISTENT_RETURN);


	for(int i=0; i < 4; i++) {
		diet_free_data(diet_parameter(pb, i));
	}

	return 0;
}




int SeDCloud::rsync_to_vm_solve(diet_profile_t *pb) {
	char* ips;
	size_t out_size;
	int* index;
	char* to_copy;
	char* destination;
	char* vm_user;

	diet_file_get(diet_parameter(pb, 0), &to_copy, NULL, &out_size);
	diet_string_get(diet_parameter(pb, 1), &destination, NULL);
	diet_string_get(diet_parameter(pb, 2), &vm_user, NULL);
	diet_file_get(diet_parameter(pb, 3), &ips, NULL, &out_size);
	diet_scalar_get(diet_parameter(pb, 4), &index, NULL);




	char* c_ip = readline(ips, *index);

	if (c_ip == NULL) {
		return -1;
	}

	printf(">>>>>>>>>>>>>>rsync solve\n");

	//rsync
	::rsync_to_vm(to_copy, destination, vm_user, c_ip);


	diet_string_set(diet_parameter(pb, 5), c_ip, DIET_PERSISTENT_RETURN);

	for(int i=0; i < 5; i++) {
		diet_free_data(diet_parameter(pb, i));
	}
	//free(c_ip);

	printf(">>>>>>>>>>>>>>rsync solve: OK\n");

	return 0;
}




/** SERVICE which mount a nfs shared folder
	IN
	0 file ips ips[0] machine  ip where shared folder is located
	1 file ips machines ip under which one mounts the shared folder
	2 shared folder path
	3 destination folder path
	OUT
	4 same as arg 1
*/
DIET_API_LIB int SeDCloud::service_mount_nfs_add() {
	diet_profile_desc_t* profile;

	profile = diet_profile_desc_alloc("mount_nfs", 3, 3, 4);
	diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 2), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 3), DIET_STRING, DIET_CHAR);
	diet_generic_desc_set(diet_param_desc(profile, 4), DIET_FILE, DIET_CHAR);

	diet_service_table_add(profile,  NULL, SeDCloud::mount_nfs_solve);

	diet_profile_desc_free(profile);
	//diet_print_service_table();
}

/**
	IN
	0 file ips ips[0] machine  ip where shared folder is located
	1 file ips machines ip under which one mounts the shared folder
	2 shared folder path
	3 destination folder path
	OUT
	4 same as arg 1
*/
int SeDCloud::mount_nfs_solve(diet_profile_t *pb) {
	char* source_ips;
	char* destination_ips;
	char* shared_folder;
	char* mount_point_folder;
	size_t out_size;


	diet_file_get(diet_parameter(pb, 0), &source_ips, NULL, &out_size);
	diet_file_get(diet_parameter(pb, 1), &destination_ips, NULL, &out_size);
	diet_string_get(diet_parameter(pb, 2), &shared_folder, NULL);
	diet_string_get(diet_parameter(pb, 3), &mount_point_folder, NULL);

	//we get the first ip of source_ips
	char* c_ip = readline(source_ips, 0);

	printf("nfs server : %s\n", c_ip);

	//we retrieve the ips of destination_ips
	//and we mount the folder under each machine
	std::fstream file;
	file.open(destination_ips, std::ios_base::in);
	bool end = false;
	char* line;
	do {
		std::string s;


		try{
			getline(file, s);
			if (s.compare("") != 0 ){
				std::string cmd = "ssh root@" + s;
				cmd.append(" 'mount -t nfs ");
				cmd.append(c_ip);
				cmd.append(":");
				cmd.append(shared_folder);
				cmd.append(" ");
				cmd.append(mount_point_folder);
				cmd.append("'");
				printf("mount nfs... : %s\n", cmd.c_str());
				int env = system(cmd.c_str());

				if (env) return -1;
			}
			else {
				end = true;
			}
		}
		catch (std::ios_base::failure e) {
			end = true;
		}
	} while (!end);



	char* out = strdup(destination_ips);

	diet_file_set(diet_parameter(pb, 4), out, DIET_PERSISTENT_RETURN);

	for(int i=0; i < 4; i++) {
		diet_free_data(diet_parameter(pb, i));
	}

	return 0;
}


//for measuring time
int time_solve(diet_profile_t *pb) {
	int last_out = pb->last_out;

	time_t seconds = time(NULL);
	long* results = new long(seconds);

	diet_scalar_set(diet_parameter(pb, last_out), results, DIET_PERSISTENT_RETURN, DIET_LONGINT);

	return 0;
}


void SeDCloudActions::clone_service_binaries(const SeDCloudActions& src) {
	cloud_service_binaries = src.cloud_service_binaries;
}


static std::map<std::string, ServiceWrapper> service_name_to_executable;

int service_wrapper_solve(diet_profile_t* pb) {
	//SeDCloud::instance->actions->perform_action_on_begin_solve(pb);

	std::string name(pb->pb_name);
	//const std::map<std::string, CloudServiceBinary>& binaries = instance->actions->get_cloud_service_binaries();
	ServiceWrapper& binary = service_name_to_executable.at(name);
	if (binary.prepocessing != NULL) {
		binary.prepocessing(&binary, pb);
	}

	int nb_args = pb->last_out + 1;
	int last_in = pb->last_in;
	size_t arg_size;

	const std::string& path_of_executable = binary.executable_path;

	std::string cmd = path_of_executable;
	for(int i = 0; i < nb_args; i++) {
		if (i <= last_in) {
				char* sz;
				diet_string_get(diet_parameter(pb, i), &sz, NULL);
				cmd.append(" ");
				cmd.append(sz);
		}
	}

	printf("EXECUTION of %s\n", cmd.c_str());
	int env = system(cmd.c_str());



	if (binary.postprocessing != NULL){
		binary.postprocessing(&binary, pb);
	}



	return env;
}

DIET_API_LIB int
        service_wrapper_table_add(const std::string& name_of_service,
                          int last_in,
                          const std::vector<std::pair<diet_data_type_t, diet_base_type_t> >& out_types,
                         const std::string& path_of_binary,
                         dietwrapper_callback_t prepocessing,
                         dietwrapper_callback_t postprocessing
                         ) {



	/*switch (arguments_transfer_method) {
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
	}*/

	ServiceWrapper service_wrapper(name_of_service, path_of_binary, prepocessing, postprocessing);
	service_name_to_executable[name_of_service] = service_wrapper;


	diet_profile_desc_t* profile;

	int last_out = last_in + out_types.size();

	profile = diet_profile_desc_alloc(name_of_service.c_str(), last_in, last_in, last_out);

	diet_data_type_t type = DIET_STRING;
	diet_base_type_t base_type = DIET_CHAR;
	for(int i = 0; i <= last_in; i++) {
		diet_generic_desc_set(diet_param_desc(profile, i), type, base_type);
	}

	for(int j = 0; j < out_types.size(); j++) {
		const std::pair<diet_data_type_t, diet_base_type_t>& type_pair = out_types[j];
		type = type_pair.first;
		base_type = type_pair.second;
		diet_generic_desc_set(diet_param_desc(profile, j + last_in + 1), type, base_type);
	}


	if (diet_service_table_add(profile, NULL, service_wrapper_solve)) {
		return 1;
	}


	diet_profile_desc_free(profile);
	//diet_print_service_table();



	return 0;

}



int service_wrapper_solve2(diet_profile_t* pb) {
	//SeDCloud::instance->actions->perform_action_on_begin_solve(pb);

	std::string name(pb->pb_name);
	//const std::map<std::string, CloudServiceBinary>& binaries = instance->actions->get_cloud_service_binaries();
	ServiceWrapper& binary = service_name_to_executable.at(name);
	if (binary.prepocessing != NULL) {
		binary.prepocessing(&binary, pb);
	}

	int nb_args = binary.get_nb_args();
	int last_in = pb->last_in;
	size_t arg_size;

	const std::string& path_of_executable = binary.executable_path;

	std::string cmd = path_of_executable;
	const std::vector<ServiceWrapperArgument>& service_wrapper_args = binary.get_args();
	for(int i = 0; i < nb_args; i++) {

		switch(service_wrapper_args[i].arg_type){

			case dietProfileArgType:
				if (service_wrapper_args[i].diet_profile_arg <= last_in) {
						char* sz;
						diet_string_get(diet_parameter(pb, service_wrapper_args[i].diet_profile_arg), &sz, NULL);
						cmd.append(" ");
						cmd.append(sz);
				}
				break;
			case commandLineArgType:
				cmd.append(" ");
				cmd.append(service_wrapper_args[i].command_line_arg);
				break;
		}

	}

	printf("EXECUTION of %s\n", cmd.c_str());
	int env = system(cmd.c_str());



	if (binary.postprocessing != NULL){
		binary.postprocessing(&binary, pb);
	}



	return env;
}


DIET_API_LIB int
        service_wrapper_table_add(const ServiceWrapper& service_wrapper,
							const std::vector<std::pair<diet_data_type_t, diet_base_type_t> >& out_types) {

	std::string name_of_service = service_wrapper.name_of_service;
	service_name_to_executable[name_of_service] = service_wrapper;

	//std::map<int, ServiceWrapperArgument> args = service_wrapper.getArgs();

	int last_in = service_wrapper.get_last_diet_in();

	diet_profile_desc_t* profile;

	int last_out = last_in + out_types.size();

	profile = diet_profile_desc_alloc(name_of_service.c_str(), last_in, last_in, last_out);

	diet_data_type_t type = DIET_STRING;
	diet_base_type_t base_type = DIET_CHAR;
	for(int i = 0; i <= last_in; i++) {
		diet_generic_desc_set(diet_param_desc(profile, i), type, base_type);
	}

	for(int j = 0; j < out_types.size(); j++) {
		const std::pair<diet_data_type_t, diet_base_type_t>& type_pair = out_types[j];
		type = type_pair.first;
		base_type = type_pair.second;
		diet_generic_desc_set(diet_param_desc(profile, j + last_in + 1), type, base_type);
	}


	if (diet_service_table_add(profile, NULL, service_wrapper_solve2)) {
		return 1;
	}


	diet_profile_desc_free(profile);
	//diet_print_service_table();



	return 0;

}


int SeDCloudActions::send_vm_ips_to_master() {
	std::ostringstream os_vm_ip_file_path;
	boost::uuids::uuid uuid = diet_generate_uuid();
	os_vm_ip_file_path << "/tmp/" << uuid << ".txt";
	const char* local_path = os_vm_ip_file_path.str().c_str();

	//write ips in file
	FILE* vm_ips_file = fopen(local_path, "w");
	if (vm_ips_file == NULL) {
		return -1;
	}
	for(int i = 0; i < vm_count; i++) {
		fprintf(vm_ips_file, "%s\n", ips[i].c_str());
	}
	fclose(vm_ips_file);

	//copy the ips file to the master vm at /home/vm_user/nodes.txt

	rsync_to_vm(local_path, "nodes.txt", vm_user, master_ip);


	remove(local_path);

}


void SeDCloudActions::destroy_vms() {
	std::list<IaaS::VMInstances*>::iterator iter;

	for(iter = vm_instances.begin(); iter != vm_instances.end(); iter++) {
		IaaS::VMInstances* elt = *iter;
		if (elt != NULL){
			delete elt;
			*iter = NULL;
		}
	}
}


void SeDCloudActions::fill_ips() {
	ips = std::vector<std::string>();
	vm_instances.back()->get_ips(ips, is_ip_private);
	master_ip = ips[0];
}


int SeDCloudActions::launch_vms(const std::string& vm_image, int vm_count, const std::string& deltacloud_api_url,
		const std::string& deltacloud_user_name, const std::string& deltacloud_passwd, const std::string& vm_user, bool is_ip_private,
		const std::vector<IaaS::Parameter>& params) {
	this->image_id = vm_image;
	this->vm_count = vm_count;
	this->base_url = deltacloud_api_url;
	set_credentials(deltacloud_user_name, deltacloud_passwd);
	this->vm_user = vm_user;
	this->is_ip_private = is_ip_private;
	this->params = params;

	SeDCloudActions::launch_vms();
}


/*
int SeDCloudVMLaunchedThenExecProgramActions::perform_action_on_vm_os_ready() {
	pid_t pid = fork();

	if (pid == 0) {
		//in the process child
		int env;

		int nb_args = program_args.size();
		char** argv = new char* [nb_args + 2];


		argv[0] = basename(strdup(program_path.c_str()));
		printf("execv : %s ", argv[0]);
		for(int i = 0; i < nb_args; i++) {

			argv[i + 1 ] = strdup(program_args[i].c_str());
			printf(" %s", argv[i + 1]);
		}
		argv[nb_args + 1] = NULL;
		printf("\n");

		printf("inside fork\n");
		std::string d_cmd = "cat ";
		d_cmd.append(program_args[1].c_str());
		printf("%s\n", d_cmd.c_str());
		env = system(d_cmd.c_str());


		//char* d_argv[] = {"ls", "-l", NULL};
		//execv("/bin/ls", d_argv);
		env = execv(program_path.c_str(), argv);

		//should not arrive here
		printf("erreur #%i execution execv inside children process\n", env);
		return -1;

	}
	else {
		if (pid > 0) {
			printf("fork executing: success\n");
			return 0;
		}
	}

}*/



