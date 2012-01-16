# To find if the diet log service module is installed 
# And to set some environemment variables
#
# Variables set :
# LOG_LIB_DIR: Directory containing all the lib
# LOG_INCLUDE_DIR: Directory containing the files to include

set(libName "LogServiceComponentBase")
mark_as_advanced(libName)

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
if(WIN32)  
  find_library(LOG_LIBRARY_UTILS  "LogForwarderUtils" 
    PATHS ${LOGSERVICE_DIR}/lib 
    DOC "The log service library")
  find_library(LOG_LIBRARY_BASE "LogServiceComponentBase" 
    PATHS ${LOGSERVICE_DIR}/lib 
    DOC "The log service library") 
  find_library(LOG_LIBRARY_TBASE "LogServiceToolBase" 
    PATHS ${LOGSERVICE_DIR}/lib 
    DOC "The log service library") 
  find_library(LOG_LIBRARY_CORBA "LogCorba" 
    PATHS ${LOGSERVICE_DIR}/lib 
    DOC "The log service library") 
  set(LOG_LIBRARY
    ${LOG_LIBRARY_UTILS} ${LOG_LIBRARY_BASE}
    ${LOG_LIBRARY_TBASE} ${LOG_LIBRARY_CORBA})  
else()
  find_library(LOG_COMPONENT_LIBRARY ${libName})
  find_library(LOG_UTILS_LIBRARY LogForwarderUtils)
  set(LOG_LIBRARY ${LOG_COMPONENT_LIBRARY})
  set(LOG_LIBRARIES ${LOG_COMPONENT_LIBRARY} ${LOG_UTILS_LIBRARY})
endif()
  
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LOGSERVICE DEFAULT_MSG LOG_INCLUDE_DIR LOG_LIBRARY)

if(NOT LOGSERVICE_FOUND)
  message(STATUS
    "Log not found on this machine. DIET will not be able to use the log system.")
  set(LOGSERVICE_DIR "" CACHE PATH "Root of log service tree installation.")
endif()

