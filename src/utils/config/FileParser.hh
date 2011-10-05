/****************************************************************************/
/* File parser                                                              */
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


#ifndef _FILE_PARSER_HH_
#define _FILE_PARSER_HH_

#include <map>
#include <string>

#include "FileException.hh"
#include "CommonParser.hh"

/**
 * @author Gael Le Mahec <gael.le.mahec@ens-lyon.fr)
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class FileParser
 * @brief Simple Configuration File Parser
 *
 * support ini-like format, options are stored in a std::map
 */
class FileParser {
private:
  mutable OptionMap options_;
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
  void parseFile(const std::string& path);

  /** 
   * @brief subscript operator
   * @param key option name
   * @return option value
   */
  std::string& operator[](const std::string& key); // setter
  /** 
   * @brief subscript operator
   * @param key option name
   * @return option value
   */
  const std::string& operator[](const std::string& key) const; //getter
    
  /** 
   * @brief return options
   * @return configuration map (key, value)
   */
  const std::map<std::string, std::string>& getConfiguration() const;
};

#endif /* _FILE_PARSER_HH_ */
