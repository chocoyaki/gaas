/*
 * @file SeDCloud.cc
 *
 * @brief
 *
 * @author  Lamiel Toch (lamiel.toch@es-lyon.fr) Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include "SeDCloud.hh"
#include "Instance.hh"
#include "DagdaFactory.hh"
#include "DIET_uuid.hh"
#include "Iaas_deltacloud.hh"
#include "boost/filesystem.hpp"
#include "debug.hh"

#include <wait.h>
#include <unistd.h>
#include <libgen.h>


/*
* TODO : create a service like "destroy_CloudX" when linking a SeDCloud to a CloudX...
*
*
*
*
*
*
*
*/


extern char ** environ;

SeDCloud* SeDCloud::instance = NULL;
std::vector<CloudAPIConnection>* SeDCloud::cloud_api_connection_for_vm_destruction = NULL;
CloudAPIConnection* SeDCloud::cloud_api_connection_for_vm_instanciation = NULL;


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
                         const std::string& _installer_relative_path) {



  CloudServiceBinary service_binary(local_path_of_binary,
        remote_path_of_binary, entryPoint, remote_path_of_arguments, prepocessing, postprocessing,
        arguments_transfer_method, _installer_relative_path);

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
    create_folder(local_results_folder.c_str());
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

//  printf("h1\n");
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
  return 0;
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


DIET_API_LIB int SeDCloud::service_homogeneous_vm_instanciation_add(CloudAPIConnection* _cloud_api_connection) {
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
  return 0;
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
  return 0;
}


DIET_API_LIB int SeDCloud::service_cloud_federation_vm_destruction_by_ip_add(std::vector<CloudAPIConnection>* _cloud_api_connection) {
    cloud_api_connection_for_vm_destruction = _cloud_api_connection;

    diet_profile_desc_t* profile;
    profile = diet_profile_desc_alloc("vm_destruction_by_ip", 1, 1, 1);

    diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR); //ips
    diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT); //private or public ips

    diet_service_table_add(profile, NULL, SeDCloud::cloud_federation_vm_destruction_by_ip_solve);
    diet_profile_desc_free(profile);
    return 0;
}


DIET_API_LIB int SeDCloud::service_launch_another_sed_add() {
  diet_profile_desc_t* profile;
  profile = diet_profile_desc_alloc("launch_another_sed", 2, 2, 3);
  //IN
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_STRING, DIET_CHAR); //sed_executable_path
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR); //diet_cfg
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_FILE, DIET_CHAR); //file data
  //OUT
  diet_generic_desc_set(diet_param_desc(profile, 3), DIET_SCALAR, DIET_INT); //result of the launch : 0 : KO, != 0 : success

  diet_service_table_add(profile, NULL, SeDCloud::launch_another_sed_solve);
  diet_profile_desc_free(profile);
  return 0;
}

int SeDCloud::launch_another_sed_solve(diet_profile_t* pb) {
  TRACE_TEXT(TRACE_MAIN_STEPS, "Launching another sed\n")
  char* cfg_path;
  size_t size;
  char* data_file_path;
  char* sed_executable_path;

  diet_string_get(diet_parameter(pb, 0), &sed_executable_path, NULL);
  //diet_file_get(diet_parameter(pb, 1), &cfg_path, NULL, &size);
  diet_file_get(diet_parameter(pb, 2), &data_file_path, NULL, &size);

  std::string data_copy_path = copy_to_tmp_file(data_file_path, ".dat");
  std::cout << "Copying data_file_path " << data_file_path << " to " << data_copy_path << std::endl;
  cfg_path = new char[SeDCloud::instance->config_file.length()+1];
  strcpy(cfg_path, SeDCloud::instance->config_file.c_str());

    boost::uuids::uuid uuid = diet_generate_uuid();
    std::ostringstream name;
    name << "SeDAppli-" << uuid;
    std::ostringstream cfg_copy_path;
    cfg_copy_path << dirname(cfg_path) << "/" << name.str() << ".cfg";
    // change the name in cfg
    std::ifstream cfg(SeDCloud::instance->config_file.c_str());
    std::ofstream tmp(cfg_copy_path.str().c_str());
    std::string line;

    std::cout << SeDCloud::instance->config_file.c_str() << " -> " << cfg_copy_path.str().c_str() << std::endl;
    while(std::getline(cfg, line)) {
        if (line.find("name") != std::string::npos) {
            line = "name = " + name.str();
        }
        std::cout << line << std::endl;
        tmp << line << '\n';
    }

    cfg.close();
    tmp.close();
    int nb_args = pb->last_in + 1;
    char** argv = new char* [nb_args + 2];
    char* sed_path_base_name = basename(sed_executable_path);
    argv[0] = sed_path_base_name;
    argv[1] = const_cast<char*> (cfg_copy_path.str().c_str());
    argv[2] = const_cast<char*> (data_copy_path.c_str());
    argv[3] = NULL;

    pid_t pid = fork();

    if (pid == 0) {
      //in the process child

    int resExec = execve(sed_executable_path, argv, environ);

    //should not arrive here
    printf("erreur #%i execution execv inside children process\n", resExec);
    exit(-1);
  }
  else {
    if (pid > 0) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "fork executing: success\n");
      int* child_status = new int;

      //TODO check if it always does its job
      waitpid(pid, child_status, WNOHANG);


      diet_scalar_set(diet_parameter(pb, 3), child_status, DIET_PERSISTENT_RETURN, DIET_INT);

      return 0;
    }
  }


  return 0;

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


  printf("Instanciation : image='%s', profile='%s', vm_count=%i, is_ip_private=%i\n", vm_image, vm_profile, *vm_count, *is_ip_private);
  printf("Instanciations starting\n");
  IaaS::VMInstances* instances;
  std::vector<std::string> ips;
  instances = new IaaS::VMInstances(vm_image, *vm_count, deltacloud_api_url, deltacloud_user_name, deltacloud_passwd, vm_user, params);
  printf("Instanciations created\n");
  instances->wait_all_instances_running();
  printf("Instanciations running\n");
  instances->wait_all_ssh_connection(*is_ip_private);
  printf("Instanciations ssh ok\n");
  instances->get_ips(ips, *is_ip_private);

  printf("Instanciation almost done\n");
  std::string diet_tmp_ips_file = create_tmp_file(pb, ".txt");
  write_lines(ips, diet_tmp_ips_file);
  diet_file_set(diet_parameter(pb, 9), diet_tmp_ips_file.c_str(), DIET_PERSISTENT_RETURN);

  printf("Instanciation done");



  return 0;
}


int SeDCloud::homogeneous_vm_instanciation_with_one_cloud_api_connection_solve(diet_profile_t* pb) {
  //char* vm_collection_name;
  int* vm_count;
  char* vm_image;
  char* vm_profile;
  const char* deltacloud_api_url;
  const char* deltacloud_user_name;
  const char* deltacloud_passwd;
  char* vm_user;
  //0 if we take the public ip 1 if we take the private ip
  int* is_ip_private;


  deltacloud_api_url = cloud_api_connection_for_vm_instanciation->base_url.c_str();
  deltacloud_user_name = cloud_api_connection_for_vm_instanciation->username.c_str();
  deltacloud_passwd = cloud_api_connection_for_vm_instanciation->password.c_str();

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

  std::string diet_tmp_ips_file = create_tmp_file(pb, ".txt");
  write_lines(ips, diet_tmp_ips_file);


  diet_file_set(diet_parameter(pb, 5), diet_tmp_ips_file.c_str(), DIET_PERSISTENT_RETURN);

  return 0;
}

#ifdef USE_LOG_SERVICE
const DietLogComponent*
SeDCloud::get_log_component() const {
  return DagdaFactory::getSeDDataManager()->getLogComponent();
}
#endif

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


  printf("Instanciation : image='%s', profile='%s', vm_count=%i, keyname=%s\n", vm_image, vm_profile, *vm_count, keyname);

  std::vector<IaaS::Parameter> params;
  params.push_back(IaaS::Parameter(HARDWARE_PROFILE_ID_PARAM, vm_profile));
  params.push_back(IaaS::Parameter(KEYNAME_PARAM, keyname));

  IaaS::VMInstances* instances;
  std::vector<std::string> ips;

  instances = new IaaS::VMInstances(vm_image, *vm_count, deltacloud_api_url, deltacloud_user_name, deltacloud_passwd, vm_user, params);
  //reserved_vms[vm_collection_name] = instances;
  instances->wait_all_instances_running();
  instances->wait_all_ssh_connection(*is_ip_private);
  instances->get_ips(ips, *is_ip_private);

  std::string diet_tmp_ips_file = create_tmp_file(pb, ".txt");
  write_lines(ips, diet_tmp_ips_file);
  diet_file_set(diet_parameter(pb, 10), diet_tmp_ips_file.c_str(), DIET_PERSISTENT_RETURN);

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
  for(size_t i = 0; i < cloud_api_connection_for_vm_destruction->size(); i++) {

    url_api = (*cloud_api_connection_for_vm_destruction)[i].base_url.c_str();
    user_name = (*cloud_api_connection_for_vm_destruction)[i].username.c_str();
    password = (*cloud_api_connection_for_vm_destruction)[i].password.c_str();


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
  return 0;
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
  return 0;
}

int SeDCloud::get_tarball_from_vm_solve(diet_profile_t *pb) {
  char* vm_user;
  char* ips_file_path;
  int* vm_index;
  char* source_file_path;
  //char* out;
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
  return 0;
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
  //char* line;
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
