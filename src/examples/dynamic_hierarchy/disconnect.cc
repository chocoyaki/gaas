/**
 * @file disconnect.cc
 *
 * @brief  Example code for dynamically disconnecting an element from its parent.
 *
 * @author  Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include <iostream>
#include <cstring>

#include "DIET_admin.h"

using namespace std;

int
main(int argc, char **argv) {
  if (argc < 3) {
    cout << "Usage: " << argv[0] << " <SED|LA> <element name>" << endl;
    return 1;
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
    cout << name << " (error code=" << res << ")." << endl;
  } else {
    cout << "** " << name << " has been disconnected from the hierarchy." <<
    endl;
  }

  return res;
} // main
