/**
 * @file remove.cc
 *
 * @brief  Example for dynamically killing an element and underlying hierachy
 *
 * @author  Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include <iostream>
#include <cstring>
#include <cstdlib>

#include "DIET_admin.h"

using namespace std;

int
main(int argc, char **argv) {
  if (argc < 3) {
    cout << "Usage: " << argv[0] <<
    " <SED|AGENT> <element name> [recursive: 0|1]" << endl;
    return 1;
  }

  dynamic_type_t type;
  char *name = argv[2];
  bool recursive = false;
  int res;

  if (argc == 4) {
    recursive = atoi(argv[3]);
  }

  if (!strcmp(argv[1], "SED")) {
    type = SED;
  } else {
    type = AGENT;
  }
  cout << "Element name: " << name << endl;

  res = diet_remove_from_hierarchy(type, name, recursive);

  if (res) {
    cout << "** A problem occured during the removal of ";
    if (type == SED) {
      cout << "SeD ";
    } else {
      cout << "agent ";
    }
    cout << name << " (error code=" << res << ")." << endl;
  } else {
    cout << "** " << name << " has been removed." << endl;
  }

  return res;
} // main
