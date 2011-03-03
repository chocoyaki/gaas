/****************************************************************************/
/* Environment variables parser exceptions                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Haikel Guemar (haikel.guemard@sysfera.com)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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
class EnvConfigError : public EnvParserError 
{
private:
    std::string diagnostic;
public:
    EnvConfigError(const std::string& msg)
	: diagnostic("Bad Environment configuration") {}

    virtual ~EnvConfigError() throw() {}
    
    virtual const char *what() const throw()
    {
	return diagnostic.c_str();
    }
};


#endif /* _ENV_EXCEPTION_HH_ */
