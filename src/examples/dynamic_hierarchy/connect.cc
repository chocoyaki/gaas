/****************************************************************************/
/* Dynamic hierarchy management                                             */
/* Example code for dynamically changing the parent of an element.          */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (Benjamin.Depardon@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/01/14 11:06:37  bdepardo
 * Compiles with gcc 4.4
 *
 * Revision 1.1  2009/10/26 09:23:51  bdepardo
 * Examples to dynamically manage the hierarchy.
 *
 *****************************************************************************/

#include <iostream>

#include "DIET_admin.h"

using namespace std;

int
main (int argc, char** argv) {
  if (argc < 4) {
    cout << "Usage: " << argv[0] << " <SED|LA> <element name> <parent name>" << endl;
    exit(1);
  }
  
  dynamic_type_t type;
  char * name = argv[2];
  char * parentName = argv[3];
  int res;

  if (!strcmp(argv[1], "SED")) {
    type = SED;
  } else {
    type = LA;
  }
  cout << "Element name: " << name << endl;
  cout << "Parent name: " << parentName << endl;
	

  res = diet_change_parent(type, name, parentName);

  if (res) {
    cout << "** A problem occured while changing parent of ";
    if (type == SED) {
      cout << "SeD ";
    } else {
      cout << "LA ";
    }
    cout << name << " (error code=" << res << ")." << endl;

    return res;
  }

  cout << "** " << name << " has been attached to " << parentName << "." << endl;
  return res;
}
