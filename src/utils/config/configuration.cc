#include "configuration.hh"
#include "constants.hh"
#include <regex.h>
#include <stdexcept>

// default zero_value set to zero
// specialize for other types
template <typename T>
class simple_cast_traits {
public:
  static const T zero_value = 0;
};


// poor's man lexical_cast
// empty strings are handled by using a traits class
template <typename T, typename S>
T simple_cast(const S& arg) {
  T output = simple_cast_traits<T>::zero_value;
  std::stringstream buffer;
  buffer << arg;
  buffer >> output;

  return output;
}


bool
getBoolConfigValue(diet::param_type_t param)
{
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return false;
  } else {
    return simple_cast<bool>(it->second);
  }
}

int
getIntConfigValue(diet::param_type_t param)
{
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return -1;
  } else {
    return simple_cast<int>(it->second);
  }
}

unsigned long
getULongConfigValue(diet::param_type_t param)
{
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return 0;
  } else {
    return simple_cast<int>(it->second);
  }
}

std::string&
getStringConfigValue(diet::param_type_t param) {
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return nullString;
  } else {
    return it->second;
  }
}

std::string&
getAddressConfigValue(diet::param_type_t param) {
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return nullString;
  } else {
    std::string& value = it->second;
    regex_t *preg = new regex_t;
    // check that our address is properly formatted host:port
    // if not return an empty string
    regcomp(preg, "^[-_.a-zA_Z0-9]+?:[0-9]+$", REG_EXTENDED | REG_NOSUB);
    int res = regexec(preg, value.c_str(), 0, 0, 0);
    if( res != 0 ) {
      return nullString;
    }
    delete preg;
    return value;
  }
}


std::string&
getAgentConfigValue(diet::param_type_t param) {
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return nullString;
  } else {
    std::string& value = it->second;
    if( (value == "LA") ||
	(value == "DIET_LOCAL_AGENT") ||
	(value == "MA") ||
	(value == "DIET_MASTER_AGENT") ||
	(value == "MA_DAG") ||
	(value == "DIET_MA_DAG")) {
      return value;
    } else {
      throw std::runtime_error("Unknown agent type, known types are "
			       "DIET_LOCAL_AGENT (LA), DIET_MASTER_AGENT (MA)"
			       "DIET_MA_DAG (MA_DAG)");
    }
  }
}
