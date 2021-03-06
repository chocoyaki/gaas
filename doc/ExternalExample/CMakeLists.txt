PROJECT( DIETSIMPLEEXAMPLE )

cmake_minimum_required(VERSION 2.6)

# This example needs the FindDiet.cmake package detection script. We placed
# this script in the cmake sub-directory:
SET( CMAKE_MODULE_PATH ${DIETSIMPLEEXAMPLE_SOURCE_DIR}/cmake )

# Try to automaticaly detect a DIET installation...
FIND_PACKAGE( Diet )

# ...and on failure provide the user with some hints:
IF( NOT DIET_FOUND )
  IF( DIET_DIR ) 
    MESSAGE( "The provided DIET_DIR parameter seems NOT correct." )
  ELSE( DIET_DIR ) 
    MESSAGE("Could NOT find any DIET installation among the well known paths.")
    MESSAGE("If your DIET installation is in a non canonical place, please provide DIET_DIR:")
    MESSAGE("  - through the GUI when working with ccmake, ")
    MESSAGE("  - as a command line argument when working with cmake e.g. ")
    MESSAGE("    cmake .. -DDIET_DIR:PATH=/home/<your_login_name>/local/diet ")
  ENDIF( DIET_DIR ) 
ENDIF( NOT DIET_FOUND )

# On success use the information we just recovered: 
INCLUDE_DIRECTORIES( ${DIET_INCLUDE_DIR} )
LINK_DIRECTORIES( ${DIET_LIBRARY_DIR} )

### Define a simple server...
ADD_EXECUTABLE( simple_server simple_server.c )
TARGET_LINK_LIBRARIES( simple_server ${DIET_SERVER_LIBRARIES} )
INSTALL( TARGETS simple_server DESTINATION bin )

### ... and it's associated simple client.
ADD_EXECUTABLE( simple_client simple_client.c )
TARGET_LINK_LIBRARIES( simple_client ${DIET_CLIENT_LIBRARIES} )
INSTALL( TARGETS simple_client DESTINATION bin )
