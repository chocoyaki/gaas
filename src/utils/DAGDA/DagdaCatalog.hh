/***********************************************************/
/* Dagda catalog implementation header.                    */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id:
/* $Log
/*														   */
/***********************************************************/
#include <map>
#include <list>
#include <string>
#include <omniORB4/CORBA.h>

typedef std::list<std::string> attributes_t;

class DagdaCatalog {
private:
public:
  virtual attributes_t request(std::string req) = 0;
  virtual int insert(std::string key, attributes_t values) = 0;
  virtual bool exists(std::string key) = 0;
};

class MapDagdaCatalog : public DagdaCatalog {
private:
  omni_mutex dbMutex;
  std::map<std::string, attributes_t> database;
public:
  virtual attributes_t request(std::string req);
  virtual int insert(std::string key, attributes_t values);
  virtual bool exists(std::string key);
protected:
  attributes_t getAttributes(std::string key) {
    attributes_t ret;
	dbMutex.lock();
    ret = database[key];
	dbMutex.unlock();
	return ret;
  }
  void setAttributes(std::string key, attributes_t values) {
    dbMutex.lock();
    database[key] = values;
	dbMutex.unlock();
  }
};
