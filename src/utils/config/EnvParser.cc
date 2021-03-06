/**
* @file EnvParser.cc
* 
* @brief  Environment variables parser 
* 
* @author  Haikel Guemar (haikel.guemar@sysfera.com)
* 
* @section Licence
*   |LICENSE|                                                                
*/



#include "EnvParser.hh"
#include "CommonParser.hh"

#include <algorithm>
#include <functional>


// utils

/**
 * @author hguemar <hguemar@sysfera.com>
 *
 * @class CheckEnvEntry
 * @brief functor testing if two environment entries conflicts
 * @internal
 */
class CheckEnvEntry : public std::unary_function<std::string, bool>
{
private:
  const std::pair<std::string, std::string>& s1;
public:
  CheckEnvEntry(const std::pair<std::string, std::string>& entry)
    : s1(entry) {}

  bool operator() (const std::pair<std::string, std::string>& s2) const
  {
    return ((s1.first == s2.first) || (s1.second == s2.second))
      ? true : false;
  }
};


// public methods

EnvParser::EnvParser(char *envp[], ConfigMap& config) :
  config_(config)
{
  parseEnv(envp);
}

void EnvParser::parse()
{
  Splitter split('=');
  std::string key, value;

  for (ConfigMap::iterator it1 = config_.begin();
       it1 != config_.end(); ++it1) {
    Env::iterator it2 =
      std::find_if (env_.begin(), env_.end(), StartsWith(it1->second));
    if (env_.end() != it2) {
      split.reset(*it2);

      std::string& token = split();
      key.swap(token);
      token = split();
      value.swap(token);

      debug << "Environment variable " << key
            << " : " << value << "\n";

      options_[key] = value;

      env_.erase(it2);
    }
  }
}

// use of operator[] is more idiomatic
std::string& EnvParser::operator[](const std::string& key)
{
  return options_[key];
}


const std::string&
EnvParser::operator[](const std::string& key) const
{
  return options_[key];
}


const std::map<std::string, std::string>&
EnvParser::getConfiguration() const
{
  return options_;
}

// private methods
void EnvParser::lint() const
{
  for (ConfigMap::const_iterator it1 = config_.begin();
       it1 != config_.end();) {
    ConfigMap::const_iterator it = it1++;
    ConfigMap::const_iterator it2 =
      std::find_if (it1, config_.end(), CheckEnvEntry(*it));

    if (config_.end() != it2) {
      throw EnvConfigError("");
    }
  }
}


void EnvParser::parseEnv(char *envp[])
{
  int i = 0;
  while (0 != envp[i]) {
    env_.push_back(envp[i]);
    i++;
  }
}


