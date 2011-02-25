/*
 * GTestHook.hpp
 *
 * Created on: 8 august 2010
 * Author: dloureiro
 *
 */

#ifndef GTESTHOOK_HPP_
#define GTESTHOOK_HPP_

#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <gtest/gtest.h>

/* Diet test fixture (aka test context)
 * basically setup omniNames before starting our test 
 * and then cleanup after test has been executed
 */
class F : public ::testing::Test {
protected:
    F() {
	std::cout << "=== Test setup [BEGIN]:  Launching OmniNames ==="
		  << std::endl;

	std::stringstream omniNamesLaunchingString;
    	omniNamesLaunchingString << CMAKE_CURRENT_BINARY_DIR 
				 << "/cfgs/omniNames.launch";
 
	std::cout << "omniNames launching script: "
		  << omniNamesLaunchingString.str()
		  << std::endl;

    	system(omniNamesLaunchingString.str().c_str());

    	std::stringstream omniNamesConfigurationFilePath;
    	omniNamesConfigurationFilePath << CMAKE_CURRENT_BINARY_DIR 
				       << "/cfgs/omniORB4.cfg";
    	setenv("OMNIORB_CONFIG", 
	       const_cast<char*>(omniNamesConfigurationFilePath.str().c_str()),
	       1);

    	std::stringstream orbInitRefString;
    	orbInitRefString << "NameService=corbaname::127.0.0.1:" 
			 << LOCAL_TEST_PORT;
    	setenv("ORBInitRef", 
	       const_cast<char*>(orbInitRefString.str().c_str()),
	       1);
    	setenv("ORBsupportBootstrapAgent", "1", 1);
	std::cout << "=== Test setup [END]:  Launching OmniNames ==="
		  << std::endl;
    }

    ~F() {
	std::cout << "=== Test teardown [BEGIN]: Stopping OmniNames ==="
		  << std::endl;

    	std::stringstream pidFilePath;
    	pidFilePath << CMAKE_CURRENT_BINARY_DIR 
		    << "/cfgs/omni-pidfile.sh" << "\n";

	std::cout << "omniNames stopping script: "
		  << pidFilePath.str()
		  << std::endl;

    	system(pidFilePath.str().c_str());
	
	std::cout << "=== Test teardown [END]: Stopping OmniNames ==="
		  << std::endl;
    }
};

#endif /* BOOSTHOOK_HPP_ */
