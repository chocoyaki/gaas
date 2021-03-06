/**
* @file EnvParser.hh
* 
* @brief  Environment variables parser 
* 
* @author  Haikel Guemar (haikel.guemar@sysfera.com)
* 
* @section Licence
*   |LICENSE|                                                                
*/



#ifndef _ENV_PARSER_HH_
#define _ENV_PARSER_HH_



#include <map>
#include <string>
#include <vector>

#include "CommonParser.hh"
#include "EnvException.hh"

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class EnvParser
 * @brief Environment variables parser
 *
 * environment variables parser, relevant options are stored in a std::map
 */
class EnvParser {
public:
  /**
   * @brief constructor
   * @param envp environment variables array
   * @param config parser configuration
   */
  EnvParser(char *envp[], ConfigMap& config);
  /**
   * @brief do the parsing
   */
  void
  parse();

  /**
   * @brief subscript operator returning option value
   * @param key option name
   * @return  option value (switchs return "true") or empty string
   */
  std::string&
  operator[](const std::string& key);  // setter
  /**
   * @brief const subscript operator returning option value
   * @param key option name
   * @return  option value (switchs return "true") or empty string
   */
  const std::string&
  operator[](const std::string& key) const;  // getter

  /**
   * @brief return configuration
   * @return configuration map (option name, value)
   */
  const std::map<std::string, std::string>&
  getConfiguration() const;

private:
  void
  lint() const;
  void
  parseEnv(char *envp[]);

  // option name, environment name
  typedef std::vector<std::string> Env;
  Env env_;  // *raw* environments
  ConfigMap config_;  // parser configuration
  mutable OptionMap options_;  // our options
};

#endif /* _ENV_PARSER_HH_ */
