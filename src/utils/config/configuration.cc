#include "configuration.hh"
#include "constants.hh"
#include <regex.h>
#include <stdexcept>

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

/**
 * @param[in]  param
 * @param[out] value
 * @return param has been set or not
 */
template<>
bool
getConfigValue(diet::param_type_t param, std::string& value) {
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return false;
  } else {
    value = it->second;
  }
  return true;
}


/**
 * @param[in]  param
 * @param[out] value result
 * @return param has been set or not
 * TODO: not handled by generic method
 */
bool
getAddressConfigValue(diet::param_type_t param, std::string& value) {
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return false;
  } else {
    std::string& value = it->second;
    regex_t *preg = new regex_t;
    // check that our address is properly formatted host:port
    // if not return an empty string
    regcomp(preg, "^[-_.a-zA_Z0-9]+?:[0-9]+$", REG_EXTENDED | REG_NOSUB);
    int res = regexec(preg, value.c_str(), 0, 0, 0);
    if( res != 0 ) {
      return false;
    }
    delete preg;
    return true;
  }
}


/**
 * @param[in]  param
 * @param[out] value result
 * @return param has been set or not
 * TODO: not handled by generic method
 */
bool
getAgentConfigValue(diet::param_type_t param, std::string& value) {
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return false;
  } else {
    std::string& value = it->second;
    if( (value == "LA") ||
        (value == "DIET_LOCAL_AGENT") ||
	(value == "MA") ||
	(value == "DIET_MASTER_AGENT") ||
	(value == "MA_DAG") ||
	(value == "DIET_MA_DAG")) {
    } else {
      // FIXME: unknown agent type
      // actually throw an exception but might not be an appropriate behavior
      throw std::runtime_error("Unknown agent type, known types are "
        		       "DIET_LOCAL_AGENT (LA), DIET_MASTER_AGENT (MA)"
        		       "DIET_MA_DAG (MA_DAG)");
    }
  }
  return true;
}
