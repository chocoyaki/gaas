/***********************************************************/
/* Dagda catalog implementation.                           */
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
#include "DagdaCatalog.hh"
#include "debug.hh"

using namespace std;

attributes_t MapDagdaCatalog::request(string req) {
  return getAttributes(req);
}

int MapDagdaCatalog::insert(string key, attributes_t values) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Create the data alias \"" << key
    << "\"" << endl);
  setAttributes(key, values);
}

bool MapDagdaCatalog::exists(std::string key) {
  bool ret;
  dbMutex.lock();
  ret = (database.find(key)!=database.end());
  dbMutex.unlock();
  return ret;
}