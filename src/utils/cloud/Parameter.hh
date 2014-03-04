/**
 * @file Parameter.hh
 *
 * @brief Class for a parameter <key value>, as <string string>
 *
 * @author  Adrian Muresan : adrian.muresan@ens-lyon.fr
 * 			    Lamiel TOCH (lamiel.toch@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */
#ifndef _IAAS_PARAMETER_HH_
#define _IAAS_PARAMETER_HH_

#include <string>

namespace IaaS {

#define KEYNAME_PARAM "keyname"
#define HARDWARE_PROFILE_ID_PARAM "hwp_id"

  class Parameter{
    public:

      std::string name;
      std::string value;

      Parameter(const std::string& n, const std::string& v) : name(n), value(v) {
      }

      Parameter(const Parameter& param) : name(param.name), value(param.value){
      }

  };
}

#endif
