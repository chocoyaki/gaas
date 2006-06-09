// Litte dummy example to demonstrate ctest mechanisms.
#include <iostream>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int TestDummy(int argc, char* argv[])
{
  std::cout << "Kyron was in the dummy test." << std::endl;
  // Do some shit and on success 
  return EXIT_SUCCESS;
  // and on failure
  // return EXIT_FAILURE;
}

