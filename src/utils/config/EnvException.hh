/**
* @file EnvException.hh
* 
* @brief  Environment variables parser exceptions 
* 
* @author - Haikel Guemar (haikel.guemar@sysfera.com)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.3  2011/03/07 15:34:55  hguemar
 * fix typo in mail address
 *
 * Revision 1.2  2011/03/03 11:55:52  bdepardo
 * Add missing headers
 *
 ****************************************************************************/


#ifndef _ENV_EXCEPTION_HH_
#define _ENV_EXCEPTION_HH_

#include <exception>
#include <string>

/**
 * @author hguemar
 *
 * @class EnvParserError
 * @brief Base class for all EnvParser exceptions
 */
class EnvParserError : public std::exception {};

/**
 * @author hguemar
 *
 * @class EnvConfigError
 * @brief EnvParser ill-formed configuration
 */
class EnvConfigError : public EnvParserError {
public:
  explicit EnvConfigError(const std::string& msg)
    : diagnostic("Bad Environment configuration") {}

  virtual ~EnvConfigError() throw() {}

  virtual const char *
  what() const throw()   {
    return diagnostic.c_str();
  }
private:
  std::string diagnostic;
};


#endif /* _ENV_EXCEPTION_HH_ */
