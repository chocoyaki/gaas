/*
 * @file ServiceWrapper.cc
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include "ServiceWrapper.hh"
#include "DIET_server.h"

std::ostream& operator <<(std::ostream& stream, const ServiceWrapper& obj) {
  stream << obj.name_of_service << "(" << obj.get_nb_args() << ")\n";
  for(int i = 0; i < obj.get_nb_args(); i++) {
    stream << i << ": " << obj.get_arg(i) << "\n";
  }

  return stream;
}

ServiceWrapper::ServiceWrapper(const ServiceWrapper& wrapper) {
  this->name_of_service = wrapper.name_of_service;
  this->executable_path = wrapper.executable_path;
  this->args = wrapper.args;
  this->preprocessing = wrapper.preprocessing;
  this->postprocessing = wrapper.postprocessing;

  //std::cout << "wrapper=\n" << wrapper << "\n";
  //std::cout << "this=\n" << *this << "\n";
}



static std::map<std::string, ServiceWrapper> service_name_to_executable;

int service_wrapper_solve(diet_profile_t* pb) {
  //SeDCloud::instance->actions->perform_action_on_begin_solve(pb);

  std::string name(pb->pb_name);
  //const std::map<std::string, CloudServiceBinary>& binaries = instance->actions->get_cloud_service_binaries();
  ServiceWrapper& binary = service_name_to_executable[name];
  if (binary.preprocessing != NULL) {
    binary.preprocessing(&binary, pb);
  }

  int nb_args = pb->last_out + 1;
  int last_in = pb->last_in;
  //size_t arg_size;

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

  ServiceWrapper service_wrapper(name_of_service, path_of_binary, 0, prepocessing, postprocessing);
  service_name_to_executable[name_of_service] = service_wrapper;


  diet_profile_desc_t* profile;

  int last_out = last_in + out_types.size();

  profile = diet_profile_desc_alloc(name_of_service.c_str(), last_in, last_in, last_out);

  diet_data_type_t type = DIET_STRING;
  diet_base_type_t base_type = DIET_CHAR;
  for(int i = 0; i <= last_in; i++) {
    diet_generic_desc_set(diet_param_desc(profile, i), type, base_type);
  }

  for(size_t j = 0; j < out_types.size(); j++) {
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
  ServiceWrapper& binary = service_name_to_executable[name];

  //std::cout << "test in solve2 " << binary << "\n";

  if (binary.preprocessing != NULL) {
    binary.preprocessing(&binary, pb);
  }

  int nb_args = binary.get_nb_args();
  int last_in = pb->last_in;
  //size_t arg_size;

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
      default:
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

  //std::cout << "test copy : " << service_name_to_executable[name_of_service] << "\n";

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

  for(size_t j = 0; j < out_types.size(); j++) {
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

  //std::cout << "test copy 2: " << service_name_to_executable[name_of_service] << "\n";

  return 0;

}
