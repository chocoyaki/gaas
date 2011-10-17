/****************************************************************************/
/* Event Base class                                                         */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/11/24 15:38:05  bdepardo
 * New line at the end of the file to prevent warnings
 *
 * Revision 1.1  2010/07/20 09:13:57  bisnard
 * Adding event dispatcher
 *
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

static map<short, string> severity2str(severity2str_init, severity2str_init
                                      + sizeof(severity2str_init)/sizeof(severity2str_init[0]));

string
EventBase::toString() const {
  return severity2str[(short) mySeverity];
}

ostream&
operator << (ostream& out, const EventBase& e) {
  return out << e.toString();
}

