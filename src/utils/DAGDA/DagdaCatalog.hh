/**
 * @file DagdaCatalog.hh
 *
 * @brief  Dagda catalog implementation header.
 *
 * @author  Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _DAGDACATALOG_HH_
#define _DAGDACATALOG_HH_

#include <list>
#include <map>
#include <string>
#include <omniORB4/CORBA.h>
#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif
typedef std::list<std::string> attributes_t;

class DagdaCatalog {
public:
  virtual ~DagdaCatalog() {
  }

  virtual attributes_t
  request(std::string req) = 0;

  virtual int
  insert(std::string key, attributes_t values) = 0;

  virtual bool
  exists(std::string key) = 0;
};

class DIET_API_LIB MapDagdaCatalog : public DagdaCatalog {
public:
  virtual ~MapDagdaCatalog() {
  }

  virtual attributes_t
  request(std::string req);

  virtual int
  insert(std::string key, attributes_t values);

  virtual bool
  exists(std::string key);

protected:
  attributes_t
  getAttributes(std::string key) {
    attributes_t ret;
    if (!exists(key)) {
      return ret;
    }
    dbMutex.lock();
    ret = database[key];
    dbMutex.unlock();
    return ret;
  }

  void
  setAttributes(std::string key, attributes_t values) {
    dbMutex.lock();
    database[key] = values;
    dbMutex.unlock();
  }

private:
  omni_mutex dbMutex;
  std::map<std::string, attributes_t> database;
};

#endif  // _DAGDACATALOG_HH_
