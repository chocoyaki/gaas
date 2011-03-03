/****************************************************************************/
/* Command line exceptions                                                  */
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


#ifndef _CMD_EXCEPTION_HH_
#define _CMD_EXCEPTION_HH_

#include <exception>
#include <string>
#include <sstream>

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class CmdParserError
 * @brief Base class for all CmdParser exceptions
 */
class CmdParserError : public std::exception {};

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class CmdConfigError
 * @brief CmdParser ill-formed configuration
 */
class CmdConfigError : public CmdParserError
{
private:
    std::string diagnostic;
public:
    CmdConfigError(const std::string& msg)
	: diagnostic("Bad Options configuration") {}
    
    virtual ~CmdConfigError() throw() {}

    virtual const char *what() const throw()
    {
	return diagnostic.c_str();
    }
};

    
/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class OptionNotFoundError
 * @brief Missing mandatory option
 */
class OptionNotFoundError : public CmdParserError
{ 
private:
    std::string diagnostic;
public:
    OptionNotFoundError(const std::string& msg) 
	: diagnostic("Option not found: ") 
    {
	diagnostic.append(msg);
    }
    
    virtual ~OptionNotFoundError() throw() {}
    
    virtual const char *what() const throw()
    {
	return diagnostic.c_str();
    }
};

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class OptionNoArgumentsError
 * @brief Missing option parameter
 */
class OptionNoArgumentsError : public CmdParserError
{ 
    std::string diagnostic;
public:
    OptionNoArgumentsError(const std::string& name)
    {
	std::ostringstream ss(diagnostic);
	ss << "Option " << name << "has no argument.";
    }
    
    virtual ~OptionNoArgumentsError() throw() {};
    
    virtual const char *what() const throw()
    {
	return diagnostic.c_str();
    }
};


/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class ParameterNotFoundError
 * @brief Missing parameter
 */
class ParameterNotFoundError : public CmdParserError
{ 
    std::string diagnostic;
public:
    ParameterNotFoundError(const std::string& msg) 
	: diagnostic("Missing parameter: ")
    {
	diagnostic.append(msg);
    }
    
    virtual ~ParameterNotFoundError() throw() {};
    
    virtual const char *what() const throw()
    {
	return diagnostic.c_str();
    }
};

#endif /* _OPTION_EXCEPTION_HH_ */
