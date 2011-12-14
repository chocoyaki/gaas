# To find if the diet log service module is installed 
# And to set some environemment variables
#
# Variables set :
# LOG_LIB_DIR: Directory containing all the lib
# LOG_INCLUDE_DIR: Directory containing the files to include

set (libName "LogForwarderUtils")
MARK_AS_ADVANCED (libName)

if(BUILD_SHARED_LIBS)
  set(PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
  set(SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
  set(PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
  set(SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

find_path(LOG_INCLUDE_DIR LogORBMgr.hh
  PATHS ${LOGSERVICE_DIR}/include 
  DOC "Directory containing the log service include files")
find_library(LOG_LIBRARY  ${PREFIX}${libName}${SUFFIX}
  PATHS ${LOGSERVICE_DIR}/lib 
  DOC "The log service library")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LOGSERVICE DEFAULT_MSG LOG_INCLUDE_DIR LOG_LIBRARY)


if (NOT LOGSERVICE_FOUND)
  MESSAGE("Log not found on this machine. DIET will not be able to use the log system.")
  SET(LOGSERVICE_DIR "" CACHE PATH "Root of log service tree installation".)
endif (NOT LOGSERVICE_FOUND)

