#ifndef _CONFIGURATION_HH_
#define _CONFIGURATION_HH_

#include <iostream>
#include <sstream>

#include "CommonParser.hh"
#include "CmdParser.hh"
#include "EnvParser.hh"
#include "FileParser.hh"

// TODO: not thread-safe
extern ConfigMap *configPtr;
// simplify code using global configuration map
#define CONFIGMAP (*configPtr)
// memory saver: don't create null string each time,
// we try accessing non-existant key
#define CONFIG(x) getConfigValue((x))

std::string& getConfigValue(const std::string& key);

// default zero_value set to zero
// specialize for other types
template <typename T>
class simple_cast_traits
{
public:
    static const T zero_value = 0;
};

// poor's man lexical_cast
// empty strings are handled by using a traits class
template <typename T, typename S>
T simple_cast(const S& arg)
{
    T output = simple_cast_traits<T>::zero_value;
    std::stringstream buffer;
    buffer << arg;
    buffer >> output;
    
    return output;
}

#endif /* _CONFIGURATION_HH_ */