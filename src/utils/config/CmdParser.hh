/****************************************************************************/
/* Command line parser                                                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Haikel Guemar (haikel.guemar@sysfera.com)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2011/03/07 15:34:55  hguemar
 * fix typo in mail address
 *
 * Revision 1.4  2011/03/03 11:55:52  bdepardo
 * Add missing headers
 *
 ****************************************************************************/


#ifndef _CMD_PARSER_HH_
#define _CMD_PARSER_HH_
/**
 * @file   CmdParser.hh
 * @author hguemar <hguemar@sysfera.com>
 * @date   Mon Jan 31 12:24:17 2011
 *
 * @brief  Command line parser
 */

#include <functional>
#include <list>
#include <map>
#include <sstream>
#include <string>

#include "CommonParser.hh"
#include "CmdException.hh"

struct CmdEntry;
class Splitter;

// use a priority queue to properly parse switchs/options
typedef std::list<CmdEntry> CmdConfig;
typedef std::list<std::string> Args;

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class CmdParser
 * @brief Command line parser
 *
 * Command line arguments parser, relevant options are stored in a std::map.
 * Currently, we don't support multiple switchs/options
 * switches/options must have short and long names
 * params always come last, switches/options can be mixed
 */
class CmdParser
{
private:
  CmdConfig config_;
  mutable OptionMap options_;
  Args args_;

  std::string version_;
  std::string copyright_;
  std::string exec_;

  bool hasSwitchs_;
  bool hasOptions_;
  bool hasParams_;
  bool enabledHelp_;
  bool enabledVersion_;
  bool showVersion_;

public:
  /**
   * @enum Type
   */
  typedef enum {
    Param,                      /**< parameter */
    Switch,                     /**< switch (option without any parameter) */
    Option                      /**< option (option with parameter) */
  } Type;

  /**
   * @enum Flags
   */
  typedef enum {
    Optional = 0,               /**< optional argument */
    Mandatory           /**< mandatory argument */
  } Flags;

  /**
   * @brief constructor
   * @param argc command line argc
   * @param argv command line argv
   */
  CmdParser(int argc, char *argv[]);
  /**
   * @brief constructor
   * @param argc command line argc
   * @param argv command line argv
   * @param oc  CmdParser configuration
   */
  CmdParser(int argc, char *argv[],
            CmdConfig& oc);

  /**
   * @brief sets command line configuration object
   * @param oc CmdParser configuration
   */
  void
  setConfig(CmdConfig& oc);

  /**
   * @brief do the parsing
   */
  void
  parse();

  /**
   * @brief enable generated help switch
   * @param showVersion display version string as preamble (false by default)
   */
  void
  enableHelp(bool showVersion=false);

  /**
   * @brief enable generated version switch
   * @param version version string
   * @param copyright copyright string (optional)
   */
  void
  enableVersion(const std::string& version,
                const std::string& copyright = nullString);

  /**
   * @brief get command line help string
   * @return generated help string
   */
  const std::string
  help() const;
  /**
   * @brief get command line version string
   * @return generated version string
   */
  const std::string
  version() const;

  /**
   * @brief subscript operator
   * @param key option name
   * @return option value
   */
  std::string&
  operator[](const std::string& key); // setter
  /** y
   * @brief subscript operator
   * @param key option name
   * @return option value
   */
  const std::string&
  operator[](const std::string& key) const; //getter

  /**
   * @brief return options
   * @return configuration map (key, value)
   */
  const std::map<std::string, std::string>&
  getConfiguration() const;

private:
  void
  lint() const;
  void
  parseArgs(int argc, char *argv[]);
  void
  parseConfig();
  void
  findSwitch(const CmdEntry& entry);
  void
  findOption(const CmdEntry& entry);
  void
  findParam(const CmdEntry& entry);
  // used by both findOption and findParam
  const std::string
  findOption_(const CmdEntry& entry);
};

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @struct CmdEntry
 * @brief represent a command line entry
 *
 * store command line option configuration
 */
struct CmdEntry
{
  CmdParser::Type type; /**< command-line argument type */
  CmdParser::Flags flags;       /**< command-line argument flag */
  std::string name;             /**< option name */
  std::string longName; /**< command-line argument long name */
  std::string shortName;        /**< command-line argument short name */
  std::string desc;             /**< command-line argument description */

  // required for sorting entries
  bool
  operator< (const CmdEntry& other) const;
};

const CmdEntry helpEntry = {CmdParser::Switch,
                            CmdParser::Optional,
                            "help", "help",
                            "h", "Display this help and exit"};

const CmdEntry versionEntry = {CmdParser::Switch,
                               CmdParser::Optional,
                               "version", "version", "V",
                               "Display version and exit"};

#endif /* _CMD_PARSER_HH_ */
