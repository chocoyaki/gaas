// Litte dummy example to demonstrate ctest mechanisms.
#include <iostream>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

int
TestDummy(int argc, char *argv[]) {
  std::cout << "Kyron was in the dummy test." << std::endl;
  // Do some shit and on success
  return EXIT_SUCCESS;
  // and on failure
  // return EXIT_FAILURE;
}
