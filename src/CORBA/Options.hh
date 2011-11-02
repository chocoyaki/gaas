/**
* @file Options.hh
* 
* @brief  DIET forwarder implementation - Executable options 
* 
* @author - Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)  
* 
* @section Licence
*   |LICENSE|                                                                
*/
#ifndef OPTIONS_HH
#define OPTIONS_HH

#include <string>
#include <list>
#include <map>

class Options;

/* Standard configuration class. Used as an abstract class for parameters
 * processing.
 */
class Configuration {
public:
  Configuration();

  explicit Configuration(const std::string& pgName);

  const std::string&
  getPgName() const;

  const std::string&
  getConfigFile() const;

  void
  setConfigFile(const std::string& configFile);

private:
  std::string pgName;
  std::string configFile;
};

/* Callback function type definition. */
typedef void (*optCallback)(const std::string&, Configuration*);

/* Options class. Used to process the users command line parameters. */
/* This class is a generic command line parameters processing tool.
 */
class Options {
public:
  Options(Configuration* config, int argc, char* argv[], char* envp[]=NULL);

  void
  setOptCallback(const std::string& arg, optCallback callBack);

  void
  setEnvCallback(const std::string& arg, optCallback callBack);

  void
  setParamCallback(unsigned int idx, optCallback callBack);

  void
  setFlagCallback(const char flag, optCallback callBack);

  void
  processOptions();

  void
  processEnv();

private:
  Configuration* config;
  std::map<std::string, std::string> arguments;
  std::map<std::string, std::string> environment;
  std::map<unsigned int, std::string> params;
  std::list<std::string> singleArgs;
  std::list<std::string> singleEnvs;
  std::list<char> flags;
  std::map<std::string, optCallback> optCallbacks;
  std::map<std::string, optCallback> envCallbacks;
  std::map<unsigned int, optCallback> paramCallbacks;
  std::map<char, optCallback> flagCallbacks;
};

/* A simple configuration file class.
 * The file must respect the format:
 * <attribute> = <value>
 */
class ConfigFile {
public:
  ConfigFile();
  explicit ConfigFile(const std::string& path);

  void
  parseFile(const std::string& path);

  const std::string&
  getAttr(const std::string& key);

private:
  std::map<std::string, std::string> attributes;
};
#endif
