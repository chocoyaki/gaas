/**
* @file FileParser.hh
* 
* @brief  File parser  
* 
* @author  Haikel Guemar (haikel.guemar@sysfera.com)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#ifndef _FILE_PARSER_HH_
#define _FILE_PARSER_HH_

#include <map>
#include <string>

#include "FileException.hh"
#include "CommonParser.hh"

#ifdef __WIN32__
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif

/**
 * @author Gael Le Mahec <gael.le.mahec@ens-lyon.fr)
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class FileParser
 * @brief Simple Configuration File Parser
 *
 * support ini-like format, options are stored in a std::map
 */
class DIET_API_LIB FileParser {
public:
  /**
   * @brief default constructor
   */
  FileParser();
  /**
   * @brief constructor
   * @param path path to config file
   */
  explicit FileParser(const std::string& path);
  /**
   * @brief parse configuration file
   * @param path path to config file
   * @exception FileOpenError
   */
  void
  parseFile(const std::string& path);

  /**
   * @brief subscript operator
   * @param key option name
   * @return option value
   */
  std::string&
  operator[](const std::string& key);  // setter
  /**
   * @brief subscript operator
   * @param key option name
   * @return option value
   */
  const std::string&
  operator[](const std::string& key) const;  // getter

  /**
   * @brief return options
   * @return configuration map (key, value)
   */
  const std::map<std::string, std::string>&
  getConfiguration() const;

private:
  mutable OptionMap options_;
};

#endif /* _FILE_PARSER_HH_ */
