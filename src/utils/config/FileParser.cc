/****************************************************************************/
/* File parser                                                              */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Haikel Guemar (haikel.guemar@sysfera.com)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2011/05/09 21:09:03  bdepardo
 * More robust configuration file parsing: check if the path points to a
 * directory or a file
 *
 * Revision 1.5  2011/03/07 15:34:55  hguemar
 * fix typo in mail address
 *
 * Revision 1.4  2011/03/03 11:55:52  bdepardo
 * Add missing headers
 *
 ****************************************************************************/


#include "FileParser.hh"

#include <cctype>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <dirent.h> // for opendir

#include "CommonParser.hh"


FileParser::FileParser() {
}

FileParser::FileParser(const std::string& path) {
  parseFile(path);
}

void
FileParser::parseFile(const std::string& path) {
  DIR *dp = opendir(path.c_str());
  if(dp != NULL) {
    closedir(dp);
    throw FileOpenError(path);
  }

  std::ifstream file(path.c_str());

  unsigned int l = 0;

  if (!file.is_open()) {
    throw FileOpenError(path);
  }

  Splitter split('=');

  while (!file.eof()) {
    char buffer[1024];
    std::string line, key, value;
    std::istringstream is;
    std::string::size_type pos;

    l++;
    file.getline(buffer, 1024);
    line = buffer;
    /* Remove comments. */
    pos = line.find('#');
    if (std::string::npos != pos) {
      line = line.substr(0, pos);
    }

    /* Remove white spaces.
       use remove then erase idiom
       use ::isspace so we don't get the std::locale plagued C++ variant*/
    line.erase(
      std::remove_if(line.begin(), line.end(), ::isspace),
      line.end());

    /* Empty line => continue. */
    if (line.empty()) {
      continue;
    }

    /* Cut the line on '=' character. */
    split.reset(line);

    std::string& token = split();
    key.swap(token);

    if (!split.hasNext()) {
      debug << "Warning : " << key
            << " has no value! (line " << l << ")\n";
    }

    token = split();
    value.swap(token);

    if (split.hasNext()) {
      debug << "Warning : " << key
            << " has multiple values! "
            << split()
            << " (line " << l << ")\n";
    }


    std::map<std::string, std::string>::iterator it = options_.find(key);
    if (options_.end() == it) {
      options_[key] = value;
    } else {
      debug << "Warning : " << key
            << " already defined! \n"
            << "old value : " << it->second << "\n"
            << "new value : " << value << " (line " << l << ")\n";
    }
  }
}

// use of operator[] is more idiomatic
std::string&
FileParser::operator[](const std::string& key) {
  return options_[key];
}


const std::string&
FileParser::operator[](const std::string& key) const {
  return options_[key];
}


const std::map<std::string, std::string>&
FileParser::getConfiguration() const {
  return options_;
}

