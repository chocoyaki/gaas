/**
* @file configuration.hh
*
* @brief  Configuration manager
*
* @author  Haikel Guemar (haikel.guemar@sysfera.com)
*
* @section Licence
*   |LICENCE|                                                            */


#ifndef _CONFIGURATION_HH_
#define _CONFIGURATION_HH_

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "constants.hh"
#include "CommonParser.hh"
#include "CmdParser.hh"
#include "EnvParser.hh"
#include "FileParser.hh"


#ifndef DIET_VERSION
#define DIET_VERSION "0.0.0"
#endif
#ifdef __WIN32__
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif

// TODO: not thread-safe

// simplify code using global configuration map
#define CONFIGMAP (* GetConfigPtr())

/**
 * @class simple_cast_trait
 * @brief traits class used by simple cast that sets zero_value
 * by default to 0, specialize for each type that requires.
 */
template <typename T>
class simple_cast_traits {
public:
  static const T zero_value = 0;
};

template <>
class simple_cast_traits<std::string> {
public:
  static const std::string zero_value;
};

/**
 * @brief poor's man lexical_cast
 * empty strings are handled by using a traits class
 * @param arg argument
 * @return properly casted argument
 */
template <typename T, typename S>
T simple_cast(const S& arg) {
  T output = simple_cast_traits<T>::zero_value;
  std::stringstream buffer;
  buffer << arg;
  buffer >> output;

  return output;
}

/**
 * @param[in]  param
 * @param[out] value result
 * @return param has been set or not
 */

template<typename T>
bool
DIET_API_LIB getConfigValue(diet::param_type_t param, T& value) {
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = GetConfigPtr()->find(key);
  if (GetConfigPtr()->end() == it) {
    return false;
  } else {
    value = simple_cast<T>(it->second);
    return true;
  }
}

// TODO: not handled by generic method above
template<>
DIET_API_LIB bool
getConfigValue(diet::param_type_t param, std::string& value);

DIET_API_LIB bool
getAddressConfigValue(diet::param_type_t, std::string&);

DIET_API_LIB bool
getAgentConfigValue(diet::param_type_t, std::string&);

#define CONFIG_BOOL(x, y) getConfigValue<bool>((x), (y))
#define CONFIG_INT(x, y) getConfigValue<int>((x), (y))
#define CONFIG_ULONG(x, y) getConfigValue<unsigned long>((x), (y))
#define CONFIG_STRING(x, y) getConfigValue<std::string>((x), (y))
#define CONFIG_ADDRESS(x, y) getAddressConfigValue((x), (y))
#define CONFIG_AGENT(x, y) getAgentConfigValue((x), (y))

#endif /* _CONFIGURATION_HH_ */
