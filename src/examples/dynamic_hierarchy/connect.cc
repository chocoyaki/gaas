/**
 * @file connect.cc
 *
 * @brief  Example code for dynamically changing the parent of an element.
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
  if (argc < 4) {
    cout << "Usage: " << argv[0] << " <SED|LA> <element name> <parent name>" <<
    endl;
    return 1;
  }

  dynamic_type_t type;
  char *name = argv[2];
  char *parentName = argv[3];
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
  } else {
    cout << "** " << name << " has been attached to " << parentName << "." <<
    endl;
  }

  return res;
} // main
