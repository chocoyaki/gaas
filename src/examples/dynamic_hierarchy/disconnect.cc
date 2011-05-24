/****************************************************************************/
/* Dynamic hierarchy management                                             */
/* Example code for dynamically disconnecting an element from its parent.   */
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
  if (argc < 3) {
    cout << "Usage: " << argv[0] << " <SED|LA> <element name>" << endl;
    exit(1);
  }
  
  dynamic_type_t type;
  char *name = argv[2];
  int res;
  cout << "Element name: " << name << endl;
	
  if (!strcmp(argv[1], "SED")) {
    type = SED;
  } else {
    type = LA;
  }

  res = diet_disconnect_from_hierarchy(type, name);

  if (res) {
    cout << "** A problem occured while disconnecting ";
    if (type == SED) {
      cout << "SeD ";
    } else {
      cout << "LA ";
    }
    cout << name << "." << endl;

    return res;
  }

  cout << "** " << name << " has been disconnected from the hierarchy." << endl;
  return 0;
}
