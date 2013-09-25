/*
 * @file ServiceWrapper.hh
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _SERVICEWRAPPER_HH_
#define _SERVICEWRAPPER_HH_

#ifdef __WIN32__
   #define DIET_API_LIB __declspec(dllexport)
#else /* __WIN32__ */
   #define DIET_API_LIB
#endif /* __WIN32__ */

#include "ServiceWrapperArgument.hh"
#include "DIET_server.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <map>

class ServiceWrapper;

typedef int (*dietwrapper_callback_t)(ServiceWrapper*, diet_profile_t*);

class ServiceWrapper {

public:
  std::string name_of_service;
  std::string executable_path;
  dietwrapper_callback_t preprocessing;
  dietwrapper_callback_t postprocessing;


  ServiceWrapper(const ServiceWrapper& wrapper);

  ServiceWrapper(
  const std::string& _name_of_service,
  const std::string& _executable_path,
  int nb_args,
  dietwrapper_callback_t _preprocessing = NULL,
  dietwrapper_callback_t _postprocessing = NULL) {
    name_of_service = _name_of_service;
    executable_path = _executable_path;
    preprocessing = _preprocessing;
    postprocessing = _postprocessing;
    args = std::vector<ServiceWrapperArgument>(nb_args);
  }

  ServiceWrapper() {
    name_of_service = "service_name_not_set";
    executable_path = "????";
    preprocessing = NULL;
    postprocessing = NULL;
    args = std::vector<ServiceWrapperArgument>(0);

  };



  void set_arg(int index, const std::string& arg) {

    if (args[index].command_line_arg != NULL) {
      free(args[index].command_line_arg);
    }

    args[index].command_line_arg = strdup(arg.c_str());
    args[index].arg_type = commandLineArgType;
  }

  void set_arg(int index, int diet_profile_arg) {

    if (args[index].command_line_arg != NULL) {
      free(args[index].command_line_arg);
      args[index].command_line_arg = NULL;
    }


    args[index].diet_profile_arg = diet_profile_arg;
    args[index].arg_type = dietProfileArgType;
  }

  const ServiceWrapperArgument& get_arg(int index) const {
    return args[index];
  }

  int get_last_diet_in() const {
    int last_in = -1;

    for(size_t index = 0; index < args.size(); index++){
      if (args[index].arg_type == dietProfileArgType) {
        if (last_in < args[index].diet_profile_arg) {
          last_in = args[index].diet_profile_arg;
        }
      }
    }

    return last_in;
  }

  ~ServiceWrapper() {
    std::vector<ServiceWrapperArgument>::iterator iter;
    for(size_t index = 0; index < args.size(); index++) {
      ServiceWrapperArgument& arg = args[index];
      if (arg.arg_type == commandLineArgType) {
        if(arg.command_line_arg != NULL) {
          free(arg.command_line_arg);
          arg.command_line_arg = NULL;
        }
      }
    }
  }

  const std::vector<ServiceWrapperArgument>& get_args() const {
    return args;
  }

  int get_nb_args() const {
    return args.size();
  }
private:
  //key: the index of the argument in command line; value: the argument
  std::vector<ServiceWrapperArgument> args;
};


std::ostream& operator <<(std::ostream& stream, const ServiceWrapper& obj);

DIET_API_LIB int
        service_wrapper_table_add(const std::string& name_of_service,
              int last_in,
              const std::vector<std::pair<diet_data_type_t, diet_base_type_t> >& out_types,
              const std::string& path_of_binary,
              dietwrapper_callback_t prepocessing = NULL,
              dietwrapper_callback_t postprocessing = NULL
                         );

DIET_API_LIB int
        service_wrapper_table_add(const ServiceWrapper& service_wrapper,
              const std::vector<std::pair<diet_data_type_t, diet_base_type_t> >& out_types
                         );




#endif /* _SERVICEWRAPPER_HH_ */
