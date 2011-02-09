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

bool getBoolConfigValue(diet::param_type_t key);
int getIntConfigValue(diet::param_type_t key);
unsigned long getULongConfigValue(diet::param_type_t key);
std::string& getStringConfigValue(diet::param_type_t key);
std::string& getAddressConfigValue(diet::param_type_t key);
std::string& getAgentConfigValue(diet::param_type_t key);

#define CONFIG_BOOL(x) getBoolConfigValue((x))
#define CONFIG_INT(x) getIntConfigValue((x))
#define CONFIG_ULONG(x) getULongConfigValue((x))
#define CONFIG_STRING(x) getStringConfigValue((x))
#define CONFIG_ADDRESS(x) getAddressConfigValue((x))
#define CONFIG_AGENT(x) getAgentConfigValue((x))

#endif /* _CONFIGURATION_HH_ */
