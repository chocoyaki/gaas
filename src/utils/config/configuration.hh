/****************************************************************************/
/* Configuration manager                                                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Haikel Guemar (haikel.guemar@sysfera.com)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2011/03/07 15:34:55  hguemar
 * fix typo in mail address
 *
 * Revision 1.7  2011/03/03 11:55:52  bdepardo
 * Add missing headers
 *
 ****************************************************************************/


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


// TODO: not thread-safe
extern ConfigMap *configPtr;
// simplify code using global configuration map
#define CONFIGMAP (*configPtr)

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
  static const std::string zero_value ;
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
getConfigValue(diet::param_type_t param, T& value)
{
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return false;
  } else {
    value = simple_cast<T>(it->second);
    return true;
  }
}

// TODO: not handled by generic method above
bool getAddressConfigValue(diet::param_type_t, std::string&);
bool getAgentConfigValue(diet::param_type_t, std::string&);

#define CONFIG_BOOL(x, y) getConfigValue<bool>((x), (y))
#define CONFIG_INT(x, y) getConfigValue<int>((x), (y))
#define CONFIG_ULONG(x, y) getConfigValue<unsigned long>((x), (y))
#define CONFIG_STRING(x, y) getConfigValue<std::string>((x), (y))
#define CONFIG_ADDRESS(x, y) getAddressConfigValue((x), (y))
#define CONFIG_AGENT(x, y) getAgentConfigValue((x), (y))

#endif /* _CONFIGURATION_HH_ */
