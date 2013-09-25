/*
 * @file ServiceWrapperArgument.hh
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _SERVICEWRAPPERARGUMENT_HH_
#define _SERVICEWRAPPERARGUMENT_HH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

enum ServiceWrapperArgumentType {
  dietProfileArgType = 0,
  commandLineArgType,
  unknownArgType
};

class ServiceWrapperArgument{
public:
  ServiceWrapperArgumentType arg_type;
  //union{
    int diet_profile_arg;
    char* command_line_arg;
  //};

  ServiceWrapperArgument() {
    command_line_arg = NULL;
    diet_profile_arg = -1;
    arg_type = unknownArgType;
  }

  ServiceWrapperArgument(const ServiceWrapperArgument& obj) {
    if (obj.command_line_arg != NULL){
      this->command_line_arg = strdup(obj.command_line_arg);
    }
    else {
      this->command_line_arg = NULL;
    }

    this->diet_profile_arg = obj.diet_profile_arg;
    this->arg_type = obj.arg_type;
  }

  ~ServiceWrapperArgument() {
    if (command_line_arg != NULL){
      free(command_line_arg);
      command_line_arg = NULL;
    }
  }
};

std::ostream& operator <<(std::ostream& stream, const ServiceWrapperArgument& obj);



#endif /* _SERVICEWRAPPERARGUMENT_HH_ */
