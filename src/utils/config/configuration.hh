#ifndef _CONFIGURATION_HH_
#define _CONFIGURATION_HH_

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "constants.hh"
#include "CommonParser.hh"
#include "CmdParser.hh"
#include "EnvParser.hh"
#include "FileParser.hh"


#ifndef DIET_VERSION
#define DIET_VERSION "0.0.0"
#endif


// TODO: not thread-safe
extern ConfigMap *configPtr;
// simplify code using global configuration map
#define CONFIGMAP (*configPtr)

template<typename T>
bool getConfigValue(diet::param_type_t, T&);

// TODO: not handled by generic method above
std::string& getAddressConfigValue(diet::param_type_t, bool&);
std::string& getAgentConfigValue(diet::param_type_t, bool&);

#define CONFIG_BOOL(x, y) getConfigValue<bool>((x), (y))
#define CONFIG_INT(x, y) getIntConfigValue<int>((x), (y))
#define CONFIG_ULONG(x, y) getConfigValue<unsigned long>((x), (y))
#define CONFIG_STRING(x, y) getConfigValue<std::string>((x), (y))
#define CONFIG_ADDRESS(x, y) getAddressConfigValue((x), (y))
#define CONFIG_AGENT(x, y) getAgentConfigValue((x), (y))

#endif /* _CONFIGURATION_HH_ */
