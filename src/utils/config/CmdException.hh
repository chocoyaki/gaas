/**
* @file CmdException.hh
* 
* @brief  Command line exceptions 
* 
* @author  Haikel Guemar (haikel.guemar@sysfera.com)
* 
* @section Licence
*   |LICENCE|                                                                
*/



#ifndef _CMD_EXCEPTION_HH_
#define _CMD_EXCEPTION_HH_

#include <exception>
#include <sstream>
#include <string>

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class CmdParserError
 * @brief Base class for all CmdParser exceptions
 */
class CmdParserError : public std::exception {
};

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class CmdConfigError
 * @brief CmdParser ill-formed configuration
 */
class CmdConfigError : public CmdParserError {
public:
  explicit CmdConfigError(const std::string& msg)
    : diagnostic("Bad Options configuration") {
  }

  virtual ~CmdConfigError() throw() {
  }

  virtual const char *
  what() const throw() {
    return diagnostic.c_str();
  }

private:
  std::string diagnostic;
};


/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class OptionNotFoundError
 * @brief Missing mandatory option
 */
class OptionNotFoundError : public CmdParserError {
public:
  explicit OptionNotFoundError(const std::string& msg)
    : diagnostic("Option not found: ") {
    diagnostic.append(msg);
  }

  virtual ~OptionNotFoundError() throw() {
  }

  virtual const char *
  what() const throw() {
    return diagnostic.c_str();
  }

private:
  std::string diagnostic;
};

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class OptionNoArgumentsError
 * @brief Missing option parameter
 */
class OptionNoArgumentsError : public CmdParserError {
public:
  explicit OptionNoArgumentsError(const std::string& name) {
    std::ostringstream ss(diagnostic);
    ss << "Option " << name << "has no argument.";
  }

  virtual ~OptionNoArgumentsError() throw() {
  }

  virtual const char *
  what() const throw() {
    return diagnostic.c_str();
  }

private:
  std::string diagnostic;
};


/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class ParameterNotFoundError
 * @brief Missing parameter
 */
class ParameterNotFoundError : public CmdParserError {
public:
  explicit ParameterNotFoundError(const std::string& msg)
    : diagnostic("Missing parameter: ") {
    diagnostic.append(msg);
  }

  virtual ~ParameterNotFoundError() throw() {
  }

  virtual const char *
  what() const throw() {
    return diagnostic.c_str();
  }

private:
  std::string diagnostic;
};

#endif  /* _OPTION_EXCEPTION_HH_ */
