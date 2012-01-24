#
# DietConfig.cmake: CMake configuration file for external projects using Diet.
#

# --------------------------------------------------------------------------
# The DIET include path:
set(DIET_INCLUDE_DIR "C:/Program Files (x86)/DIET/include")

# --------------------------------------------------------------------------
# Version number
set(DIET_BUILD_VERSION   "2012/01/24-18:35:06"")
set(DIET_VERSION         "2.8.0")
set(DIET_USE_ALT_BATCH   "OFF")
set(DIET_USE_WORKFLOW    "OFF")
set(DIET_WITH_STATISTICS "OFF")
set(DIET_WITH_MULTI_MA   "OFF")
set(DIET_USE_USERSCHED   "OFF")
set(DIET_USE_CCS         "OFF")

# --------------------------------------------------------------------------
# Path to DIET libraries
if(CYGWIN AND BUILD_SHARED_LIBS)
  # When compiling with gcc in gycwin shared libraries are installed in
  # the bin directory
  set(DIET_LIBRARY_DIR "C:/Program Files (x86)/DIET/bin")
else(CYGWIN AND BUILD_SHARED_LIBS)
  set(DIET_LIBRARY_DIR "C:/Program Files (x86)/DIET/lib")
endif(CYGWIN AND BUILD_SHARED_LIBS)
link_directories(${DIET_LIBRARY_DIR})
# The DIET libraries to link against
set(DIET_CLIENT_LIBRARIES DIET_client DIET_Dagda )
set(DIET_SERVER_LIBRARIES DIET_SeD DIET_client DIET_Dagda )
set(DIET_ADMIN_LIBRARIES DIET_admin ${DIET_SERVER_LIBRARIES})

if(DIET_USE_ALT_BATCH)
  add_definitions(-DHAVE_ALT_BATCH)
endif(DIET_USE_ALT_BATCH)
if(DIET_USE_WORKFLOW)
  add_definitions(-DHAVE_WORKFLOW)
endif(DIET_USE_WORKFLOW)

