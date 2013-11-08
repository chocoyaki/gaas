include_directories(
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src/Testing/third-party/process
  common
)

set( Boost_USE_MULTITHREADED ON )
link_directories ( ${Boost_LIBRARY_DIRS} )

# common stuff: unit test framework entry point
# fixtures etc ...
add_subdirectory( common )
# each test suite should be put in its own directory
add_subdirectory( allocation )
add_subdirectory( client )
add_subdirectory( data )
add_subdirectory( initialization )
add_subdirectory( workflow )
add_subdirectory( serviceTable )
add_subdirectory( dynamicHierarchy )
add_subdirectory( GRPC )
if(DIET_USE_USERSCHED)
add_subdirectory( sched )
endif()

#add_subdirectory( initialization2 )
#add_subdirectory( sample )
