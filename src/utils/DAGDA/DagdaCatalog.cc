/***********************************************************/
/* Dagda catalog implementation.                           */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2009/03/25 15:06:41  glemahec
 * Bug correction (data alias)
 *
 * Revision 1.4  2008/11/07 14:32:14  bdepardo
 * Headers correction
 *
 *
 ***********************************************************/
#include "DagdaCatalog.hh"
#include "debug.hh"

using namespace std;

attributes_t MapDagdaCatalog::request(string req) {
  
  return getAttributes(req);
}

int MapDagdaCatalog::insert(string key, attributes_t values) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Create the data alias \"" << key
	     << "\"" << endl);
  if (exists(key)) return 1;
  setAttributes(key, values);
  return 0;
}

bool MapDagdaCatalog::exists(std::string key) {
  bool ret=false;
  std::map<std::string, attributes_t>::const_iterator it;
  
  dbMutex.lock();
  it = database.find(key);
  ret = !(it == database.end());
  dbMutex.unlock();
  return ret;
}

