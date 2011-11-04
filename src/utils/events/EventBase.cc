/**
 * @file EventBase.cc
 *
 * @brief  Event Base class
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "EventBase.hh"
#include <map>
#include <iostream>

using namespace std;

static const pair<short, string> severity2str_init[] = {
  pair<short, string>(EventBase::TRACE, "TRACE"),
  pair<short, string>(EventBase::DEBUG, "DEBUG"),
  pair<short, string>(EventBase::INFO, "INFO"),
  pair<short, string>(EventBase::NOTICE, "NOTICE"),
  pair<short, string>(EventBase::WARNING, "WARNING"),
  pair<short, string>(EventBase::ERROR, "ERROR"),
  pair<short, string>(EventBase::CRITICAL, "CRITICAL ERROR"),
  pair<short, string>(EventBase::FATAL, "FATAL ERROR")
};

static map<short, string> severity2str(
  severity2str_init, severity2str_init
  + sizeof(severity2str_init) /
  sizeof(severity2str_init[0]));

string
EventBase::toString() const {
  return severity2str[(short) mySeverity];
}

ostream &
operator<<(ostream &out, const EventBase &e) {
  return out << e.toString();
}
