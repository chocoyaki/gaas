/**
* @file CmdParser.cc
* 
* @brief  Command line parser 
* 
* @author - Haikel Guemar (haikel.guemar@sysfera.com)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.10  2011/03/07 15:34:55  hguemar
 * fix typo in mail address
 *
 * Revision 1.9  2011/03/03 11:55:52  bdepardo
 * Add missing headers
 *
 ****************************************************************************/


#include <cstring>
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>


#include "CmdParser.hh"
#include "CommonParser.hh"

using namespace std;

static CmdConfig nullConfig;

// utils

bool CmdEntry::operator< (const CmdEntry& other) const
{
  return (type < other.type) ? false : true;
}

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class IsGroupedSwitch
 * @brief predicate testing if a switch is grouped or not
 */
class IsGroupedSwitch
{
private:
  const std::string switch_;
public:
  IsGroupedSwitch(const std::string& s) : switch_(s)
  {
  }

  bool operator() (const std::string& str)
  {
    if ((str.length() < 3) ||
        ('-' != str[0]) ||
        ('-' == str[1])) {
      return false;
    }

    if (std::string::npos != str.find(switch_)) {
      return true;
    }

    return false;
  }
};


// public methods
CmdParser::CmdParser(int argc, char *argv[])
  : exec_(argv[0]), hasSwitchs_(false), hasOptions_(false), hasParams_(false),
    enabledHelp_(false), enabledVersion_(false), showVersion_(false)

{
  parseArgs(argc, argv);
}

CmdParser::CmdParser(int argc, char *argv[],
                     CmdConfig& oc = nullConfig)
  : config_(oc), exec_(argv[0]),
    hasSwitchs_(false), hasOptions_(false), hasParams_(false),
    enabledHelp_(false), enabledVersion_(false), showVersion_(false)
{
  parseConfig();
  parseArgs(argc, argv);
}


void CmdParser::setConfig(CmdConfig& oc)
{
  config_ = oc;
  parseConfig();
}

void CmdParser::parse()
{
  if (enabledVersion_) {
    std::cout << version();
    exit(EXIT_SUCCESS);
  }

  if (enabledHelp_) {
    std::cout << help();
    exit(EXIT_SUCCESS);
  }

  try {
    for (CmdConfig::iterator it = config_.begin();
         it != config_.end(); ++it) {
      switch (it->type) {
      case CmdParser::Switch:
        findSwitch(*it);
        break;
      case CmdParser::Option:
        findOption(*it);
        break;
      case CmdParser::Param:
        findParam(*it);
        break;
      }
    }
  } catch (CmdParserError& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << help();
    exit(-1);
  }
}


void CmdParser::enableHelp(bool showVersion)
{
  hasSwitchs_ = true;
  showVersion_ = showVersion;
  config_.push_back(helpEntry);
}

void CmdParser::enableVersion(const std::string& version,
                              const std::string& copyright)
{
  hasSwitchs_ = true;
  version_ = version;
  copyright_ = copyright;
  config_.push_back(versionEntry);
}

const std::string CmdParser::help() const
{
  std::ostringstream help;
  std::ostringstream helpSwitchs;
  std::ostringstream helpOptions;
  std::ostringstream helpParams;

  std::string spc("   ");

  for (CmdConfig::const_iterator it = config_.begin();
       it != config_.end(); ++it)
  {
    switch (it->type) {
    case CmdParser::Param:
      helpParams << spc << it->name << "  "
                 << it->desc
                 << spc << "(option form: -" << it->shortName << ", "
                 << "--" << it->longName << ")";
      (CmdParser::Mandatory == it->flags) ?
        helpParams << " (**Mandatory**)" << endl :
        helpParams << endl;

      break;
    case CmdParser::Switch:
      helpSwitchs << spc << "-" << it->shortName << ", "
                  << "--" << it->longName << "   "
                  << it->desc;
      (CmdParser::Mandatory == it->flags) ?
        helpSwitchs << " (**Mandatory**)" << endl :
        helpSwitchs << endl;
      break;
    case CmdParser::Option:
      helpOptions << spc << "-" << it->shortName << " <value>, "
                  << "--" << it->longName << "=<value>  "
                  << it->desc;
      (CmdParser::Mandatory == it->flags) ?
        helpOptions << " (**Mandatory**)" << endl :
        helpOptions << endl;
      break;
    }
  }

  if (showVersion_ && !version_.empty()) {
    help << version();
  }

  help << "Usage:" << endl
       << spc << exec_;

  if (hasSwitchs_) {
    help << " [switchs]";
  }

  if (hasOptions_) {
    help << " [options]";
  }

  if (hasParams_) {
    help << " parameters ...";
  }

  help << endl << endl;

  if (hasSwitchs_) {
    help << spc << "[switchs]" << endl
         << helpSwitchs.str()
         << endl;
  }

  if (hasOptions_) {
    help << spc << "[options]" << endl
         << helpOptions.str()
         << endl;
  }

  if (hasParams_) {
    help << spc << "[parameters]" << endl
         << helpParams.str()
         << endl;
  }

  return help.str();
}

const std::string CmdParser::version() const
{
  std::ostringstream version;

  if (version_.empty()) {
    return nullString;
  }

  version << exec_ << " - "
          << "version: " << version_ << endl;

  if (copyright_.empty()) {
    version << copyright_ << endl;
  }

  return version.str();
}


// use of operator[] is more idiomatic
std::string& CmdParser::operator[](const std::string& key)
{
  return options_[key];
}


const std::string&
CmdParser::operator[](const std::string& key) const
{
  return options_[key];
}


const std::map<std::string, std::string>&
CmdParser::getConfiguration() const
{
  return options_;
}


// private methods

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class CheckCmdEntry
 * @brief functor testing if two CmdEntry conflicts
 * @internal
 */
class CheckCmdEntry : public std::unary_function<CmdEntry, bool>
{
private:
  const CmdEntry& opt;
public:
  CheckCmdEntry(const CmdEntry& entry) : opt(entry) {}

  bool operator() (const CmdEntry& entry) const
  {
    return ((opt.shortName == entry.shortName) ||
            (opt.longName == entry.longName)) ?
      true : false;
  }
};


// check that our configuration is valid or not
void CmdParser::lint() const
{
  for (CmdConfig::const_iterator it1 = config_.begin();
       it1 != config_.end();) {
    CmdConfig::const_iterator it = it1++;
    CmdConfig::const_iterator end = config_.end();

    CmdConfig::const_iterator  it2 =
      std::find_if (it1, end , CheckCmdEntry(*it));

    if (config_.end() != it2) {
      throw CmdConfigError("");
    }
  }
}


void CmdParser::parseArgs(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    args_.push_back(argv[i]);
    if ((0 == std::strcmp("-h", argv[i])) ||
        (0 == std::strcmp("--help", argv[i]))) {
      enabledHelp_ = true;
    }

    if ((0 == std::strcmp("-V", argv[i])) ||
        (0 == std::strcmp("--version", argv[i]))) {
      enabledVersion_ = true;
    }
  }
}


void CmdParser::parseConfig()
{
  // ensure that we process token in the following order:
  // 1. Options 2. Switchs 3. Parameters
  config_.sort();
  for (CmdConfig::const_iterator it = config_.begin();
       it != config_.end(); ++it) {
    switch (it->type) {
    case CmdParser::Switch:
      hasSwitchs_ = true;
      break;
    case CmdParser::Option:
      hasOptions_ = true;
      break;
    case CmdParser::Param:
      hasParams_ = true;
      break;
    }
  }

  // TODO: not portable (man, windows s*cks
  size_t pos = exec_.find_last_of('/');
  if (std::string::npos != pos) {
    exec_ = exec_.substr(pos+1, std::string::npos);
  }

  lint();
}


void CmdParser::findSwitch(const CmdEntry& entry)
{
  bool doErase = true;
  // search short form
  std::string s("-");
  s.append(entry.shortName);

  Args::iterator it = std::find(args_.begin(), args_.end(), s);

  // then long form
  if (args_.end() == it) {
    s.assign("--");
    s.append(entry.longName);
    s.append(" ");

    it = std::find(args_.begin(), args_.end(), s);
  }

  // check grouped switch
  if (args_.end() == it) {
    it = std::find_if (args_.begin(), args_.end(),
                      IsGroupedSwitch(entry.shortName));
    doErase = false;
  }

  if (args_.end() != it) {
    debug << "switch: " << "'" << s << "' found" << endl;
    options_[entry.name] = "true";
    if (doErase) {
      args_.erase(it);
    }
    return;
  }

  if (CmdParser::Mandatory == entry.flags) {
    throw OptionNotFoundError(entry.longName);
  }
}

void CmdParser::findOption(const CmdEntry& entry)
{
  const std::string& value = findOption_(entry);

  if (!value.empty()) {
    options_[entry.name] = value;
    return;
  }

  // if mandatory option not found throw exception !
  if (CmdParser::Mandatory == entry.flags) {
    throw OptionNotFoundError(entry.longName);
  }
}

void CmdParser::findParam(const CmdEntry& entry)
{
  // check that our parameter hasn't been provided in
  // optional form
  const std::string& value = findOption_(entry);

  if (!value.empty()) {
    options_[entry.name] = value;
    return;
  }

  std::string s("-");
  // skip unknown options
  Args::iterator it = std::find_if (args_.begin(), args_.end(),
                                   std::not1(StartsWith("-")));

  if (args_.end() != it) {
    debug << "Parameter: " << "'" << entry.longName << "'"
          << " value: " << "'" << *it << "'"
          << " found" << endl;

    options_[entry.name] = *it;
    args_.erase(it);
    return;
  }

  if (CmdParser::Mandatory == entry.flags) {
    throw ParameterNotFoundError(entry.longName);
  }
}

const std::string
CmdParser::findOption_(const CmdEntry& entry)
{
  std::string typeString;
  switch (entry.type) {
  case CmdParser::Option:
    typeString.assign("Option : ");
    break;
  case CmdParser::Param:
    typeString.assign("Param : ");
    break;
    // it should never NEVER happen !
  default:
    throw std::runtime_error("private API was fed wrong parameters");
    break;
  }


  std::string s("-");
  s.append(entry.shortName);

  Args::iterator it1 = std::find(args_.begin(), args_.end(), s);

  // found our option
  if (args_.end() != it1) {
    Args::iterator it2 = ++it1;
    const std::string value = *it2;
    // our option is followed by another switch/option
    // or the last one
    if ((args_.end() == it2) || StartsWith("-")(value)) {
      return nullString;
    }

    debug << typeString << "'" << s << "'"
          << " value : " << "'" << value << "'"
          << " found" << endl;

    // erase both option and value from args
    args_.erase(it1, ++it2);
    // get the hell out of here
    return value;
  }

  s.assign("--");
  s.append(entry.longName);
  s.append("=");
  it1 = std::find_if (args_.begin(), args_.end(), StartsWith(s));

  if (args_.end() != it1) {
    std::string key, value;
    Splitter split(*it1, '=');

    // get option key
    std::string& token = split();
    key.swap(token);

    // no value => **error**
    if (!split.hasNext()) {
      throw OptionNoArgumentsError(entry.longName);
    }

    // get option value
    token = split();
    value.swap(token);

    debug << typeString << "'" << key << "'"
          << " value : " << "'" << value << "'"
          << " found" << endl;


    args_.erase(it1);
    return value;
  }

  if (CmdParser::Param == entry.type) {
    s.assign("--");
    s.append(entry.longName);
    it1 =std::find_if (args_.begin(), args_.end(), StartsWith(s));

    if (args_.end() != it1) {
      throw ParameterNotFoundError(entry.longName);
    }
  }
  return nullString;
}
