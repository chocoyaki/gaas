/*
 * @file ServiceWrapperArgument.cc
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include "ServiceWrapperArgument.hh"


std::ostream& operator <<(std::ostream& stream, const ServiceWrapperArgument& obj)
{
    if (obj.arg_type == dietProfileArgType){
    stream<< "diet_param(" << obj.diet_profile_arg << ")";
    }
    else {
    stream<< obj.command_line_arg;
    }
    return stream;
}
