/*
 * @file CloudServiceBinary.cc
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include "CloudServiceBinary.hh"

#include "Tools.hh"

#include <string>


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
  for(size_t i = 0; i < args.size(); i++) {
    sz_args.append(" " + args[i]);
  }

  std::string cd;
  if (remote_path_of_binary.compare("") == 0) {
    cd = "";
  }
  else {
    cd = "cd " + remote_path_of_binary + "; ./";
  }

  return ::execute_command_in_vm(cd + entry_point_relative_file_path, vm_user_name, ip, sz_args);
}

