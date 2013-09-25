/*
 * @file SeDCloudActions.cc
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include "SeDCloudActions.hh"

#include "DIET_data.h"
#include "DIET_uuid.hh"

#include <signal.h>
#include <unistd.h>
#include <sstream>

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
    return rsync_to_vm(local_path, remote_path, vm_user, master_ip);
}

int SeDCloudActions::receive_result(const std::string& result_remote_path, const std::string& result_local_path) {
  return rsync_from_vm(result_remote_path, result_local_path, vm_user, master_ip);
}

int SeDCloudActions::execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args) {
  return binary.execute_remote(master_ip, vm_user, args);
}

int SeDCloudActions::create_remote_directory(const std::string& remote_path) {
  return create_directory_in_vm(remote_path, vm_user, master_ip);
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
  return 0;
}

int SeDCloudAndVMLaunchedActions::perform_action_on_end_solve(diet_profile_t *pb) {
 return 0;
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
  return 0;
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

    for(size_t i = 0; i < ips.size(); i++){
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
      if(env == 0){
        sleep(machine_alive_interval);
      }
    }while(env == 0);

    printf("ssh : lost connection\n");
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
        int res_launch = launch_vms();

        copy_all_binaries_into_all_vms();
        return res_launch;
    }
    else {
        statistics_on_services.increment_call_number(service_name);
    }
    return 0;

}

int SeDCloudVMLaunchedAtFirstSolveActions::perform_action_on_end_solve(diet_profile_t *pb) {
  return 0;
}


int SeDCloudVMLaunchedAtSolveThenDestroyedActions::perform_action_on_begin_solve(diet_profile_t *pb) {
    //this->vm_instances = new IaaS::VMInstances (this->image_id, this->vm_count, this->base_url, this->username, this->password, this->vm_user, this->params);
    int res_launch =  launch_vms();



    std::string name_of_service = pb->pb_name;
   // CloudServiceBinary& binary = cloud_service_binaries[name_of_service];
    //vm_instances->rsync_to_vm(0, is_ip_private, binary.local_path_of_binary, binary.remote_path_of_binary);
  copy_binary_into_all_vms(name_of_service);

  return res_launch;
}


int SeDCloudVMLaunchedAtSolveThenDestroyedActions::perform_action_on_end_solve(diet_profile_t *pb) {

   destroy_vms();
   return 0;
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
  return 0;
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

  return SeDCloudActions::launch_vms();
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
