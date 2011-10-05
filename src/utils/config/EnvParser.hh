/****************************************************************************/
/* Environment variables parser                                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Haikel Guemar (haikel.guemar@sysfera.com)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2011/03/07 15:34:55  hguemar
 * fix typo in mail address
 *
 * Revision 1.3  2011/03/03 11:55:52  bdepardo
 * Add missing headers
 *
 ****************************************************************************/


#ifndef _ENV_PARSER_HH_
#define _ENV_PARSER_HH_

/**
 * @file   EnvParser.hh
 * @author hguemar <hguemar@sysfera.com>
 * @date   Mon Jan 31 09:17:06 2011
 * 
 * @brief  Environment variables parser
 */

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
class EnvParser
{
private:
  // option name, environment name
  typedef std::vector<std::string> Env;
  Env env_; // *raw* environments
  ConfigMap config_; // parser configuration
  mutable OptionMap options_; // our options
public:
  /** 
   * @brief constructor
   * @param envp environment variables array
   * @param config parser configuration 
   */    
  EnvParser(char *envp[],
	    ConfigMap& config);
  /** 
   * @brief do the parsing
   */
  void parse();

  /** 
   * @brief subscript operator returning option value
   * @param key option name
   * @return  option value (switchs return "true") or empty string 
   */    
  std::string& operator[](const std::string& key); // setter
  /** 
   * @brief const subscript operator returning option value
   * @param key option name
   * @return  option value (switchs return "true") or empty string 
   */    
  const std::string& operator[](const std::string& key) const; //getter

  /** 
   * @brief return configuration
   * @return configuration map (option name, value)
   */
  const std::map<std::string, std::string>& getConfiguration() const;

private:
  void lint() const;
  void parseEnv(char *envp[]);
};

#endif /* _ENV_PARSER_HH_ */
