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
 * Revision 1.6  2011/03/07 15:34:55  hguemar
 * fix typo in mail address
 *
 * Revision 1.5  2011/03/03 11:55:52  bdepardo
 * Add missing headers
 *
 ****************************************************************************/


#include "configuration.hh"
#include "constants.hh"
#include <regex.h>
#include <stdexcept>

const std::string simple_cast_traits<std::string>::zero_value = "";

/**
 * @param[in]  param
 * @param[out] value result
 * @return param has been set or not
 */
template<>
bool
getConfigValue(diet::param_type_t param, std::string& value)
{
  const std::string& key = (diet::params)[param].value;
  ConfigMap::iterator it = configPtr->find(key);
  if (configPtr->end() == it) {
    return false;
  } else {
    value = it->second;
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
