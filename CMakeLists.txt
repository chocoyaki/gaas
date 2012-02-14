cmake_minimum_required(VERSION 2.6)
project(DIET)

set(DIET_MAJOR_VERSION 2)
set(DIET_MINOR_VERSION 8)
set(DIET_REVISION_VERSION 0)

set(DIET_VERSION
  "${DIET_MAJOR_VERSION}.${DIET_MINOR_VERSION}.${DIET_REVISION_VERSION}")

if (NOT WIN32)
  set(CMAKE_THREAD_LIBS_INIT pthread)
endif (NOT WIN32)


add_definitions(-DDIET_VERSION="${DIET_VERSION}")
# Get Date
if(WIN32)
  execute_process(COMMAND "cmd"
    "/C echo %date:~6,4%/%date:~3,2%/%date:~0,2%-%time:~0,2%:%time:~3,2%:%time:~6,2%"
    OUTPUT_VARIABLE BUILD_DATE)
  string(STRIP ${BUILD_DATE} BUILD_DATE) # get rid of newline
elseif(UNIX)
  execute_process(COMMAND "date" "+%Y/%m/%d-%H:%M:%S" 
    OUTPUT_VARIABLE BUILD_DATE)
  string(STRIP ${BUILD_DATE} BUILD_DATE) # get rid of newline
else()
  message(SEND_ERROR "date not implemented")
  set(${BUILD_DATE} 000000)
endif()

set(DIET_BUILD_VERSION ${BUILD_DATE})

# because x86_64 can't link static libraries to shared libraries
# and from 2.5, this is what is done in DIET
# see http://www.mail-archive.com/cross-lfs%40linuxfromscratch.org/msg00411.html
# You will get these messages on x86_64 any place that libtool tries to use a static (.a) library in a .la, and as it says, it can't link. These "recompile with -fPIC" messages fall into three types - 
# (i) recompile the current package with -fPIC
# (ii) fix a broken symlink (I had a dangling symlink for ncurses in my scripts, because of a typo - on x86 libtool couldn't find the .so but took the .a and ran with it, on x86_64 it barfed). 
# (iii) convert a Makefile to use .la instead of .a (very uncommon).

# Note: CMAKE_SYSTEM_PROCESSOR return the equivalent of uname -p, note uname -m

if(UNIX AND NOT WIN32)
  find_program(CMAKE_UNAME uname /bin /usr/bin /usr/local/bin)
  if(CMAKE_UNAME)
    exec_program(uname ARGS -m OUTPUT_VARIABLE SYSTEM_PROC_TYPE)
    set(SYSTEM_PROC_TYPE ${SYSTEM_PROC_TYPE} CACHE INTERNAL 
    	"processor type (i386 and x86_64)")
    if(SYSTEM_PROC_TYPE MATCHES "x86_64|amd64|ppc64")
      # TODO: Should be done only for libraries build
      add_definitions(-fPIC)
    endif()
  endif()
endif()


# --------------------- OPTIONS OF THE PROJECT -----------------------------
# Options dependencies:
#    - DIET_BUILD_BLAS_EXAMPLES => DIET_BUILD_EXAMPLES
#    - DIET_USE_CLOUD => DIET_USE_ALT_BATCH

option(DIET_BUILD_BLAS_EXAMPLES "Build BLAS based DIET examples." OFF)
if(DIET_BUILD_BLAS_EXAMPLES)
  if(NOT DIET_BUILD_EXAMPLES)
    set(DIET_BUILD_EXAMPLES ON CACHE BOOL "Build DIET examples." FORCE)
    message(WARNING "Examples building was activated.")
  endif()
  option(DIET_BUILD_EXAMPLES "Build DIET examples. (Mandatory for BLAS examples building.)" ON)
else()
  option(DIET_BUILD_EXAMPLES "Build DIET examples." OFF)
endif()

option(DIET_USE_CLOUD "Build DIET with Cloud (EC2 compatible) support." OFF)

if(DIET_USE_CLOUD)
  if(NOT DIET_USE_ALT_BATCH)
    set(DIET_USE_ALT_BATCH ON CACHE BOOL "Batch support" FORCE)
    message(WARNING "Batch was activated for cloud support.")
  endif()
  option(DIET_USE_ALT_BATCH "Build DIET with Batch support (Mandatory for cloud support)." ON)
else()
  option(DIET_USE_ALT_BATCH  "Build DIET with Batch support." OFF)
endif()

option(DIET_USE_WORKFLOW    "Build DIET with workflow support..." OFF)
option(DIET_WITH_STATISTICS "Build DIET with generation of statistics." OFF)
option(DIET_WITH_MULTI_MA   "Build DIET with MULTI-Master-Agent support." OFF)
option(DIET_USE_USERSCHED   "Build DIET with user scheduler support (Experimental)." OFF)
option(DIET_USE_CCS         "Build DIET with client custom scheduling  (Experimental)." OFF)
option(DIET_BUILD_TOOLS	    "Build DIET tools." OFF)

option(DIET_USE_MULTICALL "Build DIET with the multiple async call (In development)" OFF)
mark_as_advanced(DIET_USE_MULTICALL)

option(DIET_BUILD_DOC "Build DIET documentation." OFF)

# --------------------- OPTIONS SET BY DEFAULT  ----------------------------
# Are libraries dynamic libraries as opposed to library archives ?
# Note: this variable is a cmake internal, hence not prefixed with DIET_.
option(BUILD_SHARED_LIBS "Build libraries as shared libraries." ON)

# --------------------- DEPENDENCIES TOWARDS "CLASSIC" C++ HEADERS ---------
# This is just a convenience to advance the detection of any potential missing
# C++ "classic" header to the cmake stage. If we were to forget the following
# tests (and such a header was missing), things would simply break at
# preprocessing stage of the compilation...
# [Those tests are a translation of the AC_CHECK_HEADER(...) that were expressed
# in the configure.ac of the autotools version].
include(${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)
if(WIN32)
  set(DIET_INCLUDES_TO_CHECK
    Winsock2.h
    io.h
    process.h
    time.h
    windows.h
    Shlwapi.h
    assert.h
    iostream
    limits.h
    math.h
    stdlib.h
    string.h
    sys/stat.h
    sys/types.h
    sys/timeb.h
    direct.h)
else()
  set(DIET_INCLUDES_TO_CHECK
    assert.h
    iostream
    limits.h
    math.h
    stdlib.h
    string.h
    unistd.h
    sys/stat.h
    sys/types.h)
endif()

foreach(include_to_check ${DIET_INCLUDES_TO_CHECK})
  check_include_file_cxx(${include_to_check} dummyfound${include_to_check})
  if(NOT dummyfound${include_to_check})
    message(FATAL_ERROR "Missing ${include_to_check} header file.")
  endif()
endforeach()

# --------------------- DEPENDENCIES TOWARDS SYSTEM FUNCTIONS ------------
# This is just a convenience to advance the detection of any potential missing
# external function to the cmake stage. If we were to forget the following
# tests (and such an external function was missing), things would simply break
# at linking stage...
# [Those tests are a translation of the AC_CHECK_FUNCS(...) that were expressed
# in the configure.ac of the autotools version].
include(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
if(WIN32)
  set(DIET_FUNCTIONS_TO_CHECK
    #gethostname #must be retrieved
    #gettimeofday #must be retrieved
    #PathMatchSpec
    #strtok_r
    strdup 
    stricmp
    strpbrk)
else()
  set(DIET_FUNCTIONS_TO_CHECK
    gethostname
    gettimeofday
    strdup
    strtok_r)
endif()
foreach(function_to_check ${DIET_FUNCTIONS_TO_CHECK})
  check_function_exists(${function_to_check} dummyfound${function_to_check})
  if(NOT dummyfound${function_to_check})
    message(FATAL_ERROR "Missing ${function_to_check} function.")
  endif()
endforeach()

#-------------- PLATFORM SPECIFIC COMPILATION FLAGS -----------------------
# Requires CMake >= 2.4.7
# Used within omniORB, when set remove some warnings/errors
# Not tested under SunOS & FreeBSD

string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "AIX" AIX)
string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "Darwin" APPLE)
string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "Linux" LINUX)
string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "SunOS" SUNOS)
string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "FreeBSD" FREEBSD)
string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "CYGWIN" CYGWIN)
string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "Windows" WINDOWS)

if(CYGWIN)
  add_definitions(-D__cygwin__)

  # Workflow compilation on Windows inside the Cygwin environment
  # is not permitted until the good compilation of the libraries of
  # XQilla and Xerces are available.
  # For instance it is not possible so we have to wait for the 
  # problem resolution
  # TODO : solve the problem.
  set(DIET_USE_WORKFLOW OFF CACHE BOOL "Unable to compile workflow on Windows with Cygwin. " FORCE)
elseif(WINDOWS)
  message(STATUS "XXX System name Windows native")
  # __WIN32__ is also used by omniorb for the good use of omnithread.h
  add_definitions(-D__WIN32__)
  # definitions to add for the compilation of something using omniORB under windows
  # with native compilation
  # these definitions are taken from the omniORB user's guide
  #
  # These definitions prevent the fatal error compilation with VS9 looking like  
  # omniORB-4.1.4\include\omniORB4/CORBA_sysdep_trad.h(609) : fatal error C1189: #error :  "The byte order of this platform is unknown"
  add_definitions(-D__x86__ -D__NT__ -D__OSVERSION__=4)
  # definition added to prevent some warnings due to strcpy being deprecated on windows
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(AIX)
  add_definitions(-D__aix__)
  # AIX doesn't like mixing static and shared libraries	 
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build libraries as shared libraries." FORCE)
elseif(APPLE)
  add_definitions(-D__darwin__)
  if(NOT BUILD_SHARED_LIBS)
    message("Cannot do a static build on Apple platforms")
  endif()
  set(BUILD_SHARED_LIBS ON CACHE BOOL "Build libraries as shared libraries." FORCE)
elseif(LINUX)
  add_definitions(-D__linux__)
elseif(SUNOS)
  add_definitions(-D__sunos__)
elseif(FREEBSD)
  add_definitions(-D__freebsd__)
endif()

# --------------------- DEPENDENCIES TOWARDS EXTERNAL PACKAGES -------------
# Path to additional modules (i.e. used by FIND_PACKAGE commands making
# reference to non CMake defined "standard" modules):
set(CMAKE_MODULE_PATH ${DIET_SOURCE_DIR}/cmake)

# Boost is mandatory
find_package(Boost 1.46.0 COMPONENTS filesystem regex system thread REQUIRED)
# Boost Filesystem API v2 is deprecated will be removed after Boost 1.48 release
add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
if(Boost_FOUND)
  if(WIN32)
    link_directories(${Boost_LIBRARY_DIRS})
  endif()
    set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS}
      ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()


# ssh is mandatory for the forwarders
find_package(SSH)
if(SSH_FOUND)
  add_definitions(-DSSH_BINARY="${SSH_BINARY}")
  add_definitions(-DSCP_BINARY="${SCP_BINARY}")
endif()

# OmniORB is mandatory:
find_package(OmniORB4 REQUIRED)

# --------------------------- Batch support ------------------------
if(DIET_USE_ALT_BATCH)
  add_definitions(-DHAVE_ALT_BATCH)
endif()

# --------------------------- Cloud support ------------------------
if(DIET_USE_CLOUD)
  add_definitions(-DHAVE_CLOUD)
  set(GSOAP_SRC_DIR "~" CACHE PATH "Root of gSOAP source tree.")
  find_package(GSOAP)
  find_package(OpenSSL)
  find_package(ZLIB)
  set(GSOAP_INCLUDE_DIRS ${GSOAP_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})
  set(GSOAP_LIBRARIES ${GSOAP_LIBRARIES}
    ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES})
endif()


# --------------------------- CORI support ------------------------
# CORI (COllector of Resource Information) is heavily dependant on
# system calls for probing ressources:
include(${DIET_SOURCE_DIR}/cmake/ConfigureCORI.cmake)

# --------------------------- Multi MA support ------------------------
# So called MULTI-Master-Agent
if(DIET_WITH_MULTI_MA)
  add_definitions(-DHAVE_MULTI_MA)
endif()

# --------------------------- Statistic support ------------------------
if(DIET_WITH_STATISTICS)
  # Enable generation of the statistics log:
  add_definitions(-DHAVE_STATISTICS)
endif()

if(DIET_BUILD_BLAS_EXAMPLES)
  # BLAS (Basic Linear Algebric Subroutines) examples subdir requires...BLAS !
  find_package(BLAS)
  if(BUILD_SHARED_LIBS)
    set(BLAS_LIBRARIES ${BLAS_LIBRARIES_sh})
  else()
    set(BLAS_LIBRARIES ${BLAS_LIBRARIES_st})
  endif()
endif()

# --------------------------- Workflow support ------------------------
if(DIET_USE_WORKFLOW)
  find_package(Xerces)
  if(XERCES_FOUND)
    if(XERCES_VERSION LESS 30)
      message("Diet Workflow support requires at least version 3.0 of Xerces.")
      message("Workflow support will be disabled")
      set(DIET_USE_WORKFLOW OFF)
    else()
      find_package(XQilla)
      if(XQILLA_FOUND)
        add_definitions(-DHAVE_WORKFLOW)
	if(BUILD_SHARED_LIBS AND NOT AIX)
	  # Link DIET to Xerces and XQilla shared libs
	  if(WIN32)
	    set(XERCES_LIBRARY ${XERCES_LIBRARY_st})
	    set(XQILLA_LIBRARY ${XQILLA_LIBRARY_st})
	  else()
	    set(XERCES_LIBRARY ${XERCES_LIBRARY_sh})
	    set(XQILLA_LIBRARY ${XQILLA_LIBRARY_sh})
	  endif()
	else()
	  # Link DIET to Xerces and XQilla static libs
	  set(XERCES_LIBRARY ${XERCES_LIBRARY_st})
	  set(XQILLA_LIBRARY ${XQILLA_LIBRARY_st})
	endif()
        set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS}
          ${XERCES_LIBRARY}
          ${XQILLA_LIBRARY})
      else()
        message("Diet Workflow support requires XQilla library.")
        message("Workflow support will be disabled")
        set(DIET_USE_WORKFLOW OFF)
      endif()
    endif()
  else(XERCES_FOUND)
    set(DIET_USE_WORKFLOW OFF CACHE BOOL "Build DIET with workflow support...")
  endif()
endif()

# --------------- For users agents scheduler support ------------------
if(DIET_USE_USERSCHED)
  add_definitions(-DUSERSCHED)
endif(DIET_USE_USERSCHED)

# --------------- Dagda as data manager support ---------------
if(DIET_TRANSFER_PROGRESSION)
  add_definitions(-DDAGDA_PROGRESSION)
endif(DIET_TRANSFER_PROGRESSION)

#-------------------- DIET log service use -------------------
option(DIET_USE_LOG "Allow Diet to send log message to log service " OFF)

if(DIET_USE_LOG)
  find_package (LOGSERVICE)
  if(LOGSERVICE_FOUND)
    add_definitions(-DUSE_LOG_SERVICE)
  else(LOGSERVICE_FOUND)
  set(LOGSERVICE_DIR "" CACHE PATH "Root of log service install tree.")
  endif(LOGSERVICE_FOUND)
  
endif(DIET_USE_LOG)
# --------------- DIET custom client scheduling support  ------------------
if(DIET_USE_CCS)
  add_definitions(-DHAVE_CCS)
endif(DIET_USE_CCS)

# --------------- DIET multiple async call support  ------------------
if(DIET_USE_MULTICALL)
  add_definitions(-DHAVE_MULTICALL)
endif(DIET_USE_MULTICALL)

# --------------- OFFER BUILD MODE EXTENSIONS AND DEFAULTS  ----------------
include(${DIET_SOURCE_DIR}/cmake/DietDefaultBuildTypes.cmake)

# --------------- HANDLE SPECIFICITIES OF THE C COMPILER -------------------
include(${DIET_SOURCE_DIR}/cmake/ConfigureCCompiler.cmake)

#---------------- COMPILER SPECIFIC COMPILATION FLAGS ----------------------

if(CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
  # Get version
  execute_process(COMMAND ${CMAKE_C_COMPILER} "-dumpversion" 
    OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} "-dumpversion"
    OUTPUT_VARIABLE CMAKE_CXX_COMPILER_VERSION 
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif(CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)


# ------------------- CTEST SUPPORT ----------------------------------------
if(BUILD_TESTING)
  include(${DIET_SOURCE_DIR}/cmake/FindTestingUtils.cmake)
  # Caveat emptor:
  # 1/ Excerpt from docs about ENABLE_TESTING:
  #    [...] ctest expects to find a test file in the build directory root.
  #    Therefore, this command should be in the source directory root.
  # 2/ ENABLE_TESTING() has to be run prior to any ADD_TEST() command (or
  #    the ADD_TEST are simply ignored). Since the Testing directory is
  #    within the src subdir, the ENABLE_TESTING() must occur PRIOR to
  #    the ADD_SUBDIRECTORY(src) command (see below).
  # 3/ Dart2 support also requires the ENABLE_TESTING() to be present
  #    and invoked prior to the INCLUDE(CTest) command (see below).
  enable_testing()

  if(NOT DIET_BUILD_EXAMPLES)
    set(DIET_BUILD_EXAMPLES ON CACHE BOOL "Build DIET examples." FORCE)
  endif()


  # For comments on the following CXX_TEST_PATH refer to
  #   http://public.kitware.com/pipermail/cmake/2002-January/002643.html
  if(RUNTIME_OUTPUT_DIRECTORY)
    set(CXX_TEST_PATH ${RUNTIME_OUTPUT_DIRECTORY})
  else()
    set(CXX_TEST_PATH .)
  endif()
  
  if(NOT WIN32)
  set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage"
    CACHE STRING 
    "Flags used by the C++ compiler during debug builds."
    FORCE)
  set(CMAKE_C_FLAGS_DEBUG
    "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage"
    CACHE STRING 
    "Flags used by the C compiler during debug builds."
    FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -lgcov"
    CACHE STRING 
    "Flags used for linking binaries during debug builds."
    FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -lgcov"
    CACHE STRING 
    "Flags used by the shared libraries linker during debug builds."
    FORCE)
  set(CMAKE_MODULE_LINKER_FLAGS_DEBUG
    "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -lgcov"
    CACHE STRING 
    "What the hack is a module anyhow (Apple notion?)..."
    FORCE)
  endif()
endif()

if(CYGWIN)
  if(BUILD_SHARED_LIBS)  
    set(DIET_INSTALL_LIBS_DIR bin)
  else()
    set(DIET_INSTALL_LIBS_DIR lib)
  endif()
else()
  set(DIET_INSTALL_LIBS_DIR lib)
endif(CYGWIN)

# installation customization hooks
# manage lib/lib64 install issue
set(LIB_SUFFIX "" CACHE STRING "Define suffix of lib directory name (32/64)")
set(LIB_INSTALL_DIR "lib${LIB_SUFFIX}"
  CACHE INTERNAL "Directory where libraries will be installed")
set(BIN_INSTALL_DIR "bin"
  CACHE INTERNAL "Directory where binaries will be installed")
set(INCLUDE_INSTALL_DIR "include"
  CACHE INTERNAL "Directory where include files will be installed")
set(ETC_INSTALL_DIR "etc"
  CACHE INTERNAL "Directory where configuration files will be installed")
set(DATA_INSTALL_DIR "share/diet"
  CACHE INTERNAL "Directory where data files will be installed")
set(CMAKE_MOD_INSTALL_DIR "share/cmake/Modules"
  CACHE INTERNAL "Directory where cmake modules will be installed")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

# hack to avoid installing configuration stuff in /usr/etc
if(${CMAKE_INSTALL_PREFIX} MATCHES "^/usr")
  set(ETC_INSTALL_DIR "/etc")
endif()

# --------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(cmake)
if(DIET_BUILD_DOC)
  add_subdirectory(doc)
endif()

# --------------------------------------------------------------------------
include(${DIET_SOURCE_DIR}/cmake/DisplaySummary.cmake)

if(NOT WIN32)
  # --------------------------------------------------------------------------
  # Configuration of CPack to generate packages
  set(CPACK_PACKAGE_NAME "DIET")
  set(CPACK_PACKAGE_VERSION_MAJOR ${DIET_MAJOR_VERSION})
  set(CPACK_PACKAGE_VERSION_MINOR ${DIET_MINOR_VERSION})
  set(CPACK_PACKAGE_VERSION_REVISION ${DIET_REVISION_VERSION})
  set(CPACK_PACKAGE_VERSION_PATCH ${DIET_REVISION_VERSION})
  # set(CPACK_PACKAGE_DESCRIPTION_FILE) # TODO
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "DIET (Distributed Interactive Engineering Toolbox) is a middleware designed for high-performance computing in a heterogeneous and distributed environment (workstations, clusters, grids, clouds).")
  set(CPACK_PACKAGE_FILE_NAME "diet-${DIET_VERSION}")
  set(CPACK_RESOURCE_FILE_LICENSE ${DIET_SOURCE_DIR}/LICENCE_eng.txt)
  set(CPACK_RESOURCE_FILE_README ${DIET_SOURCE_DIR}/README.txt)
  # set(CPACK_RESOURCE_FILE_WELCOME) # not necessary, CPack generates a default welcome screen
  
  # CPack generators: do not use this macro, but change the comments whenever a
  # new generator is supported. Also push your modifications in the INSTALL.txt
  # file.
  # set(CPACK_GENERATOR 
  #   # CPACK_BINARY_BUNDLE     # currently not supported
  #   # CPACK_BINARY_DEB        # do not use, it does not generate proper debian packages
  #   CPACK_BINARY_DRAGNDROP    # OK, but not really interesting 
  #   # CPACK_BINARY_NSIS       # not tested yet
  #   CPACK_BINARY_OSXX11       # do not use, creates a .app, but it does not mean much for us
  #   CPACK_BINARY_PACKAGEMAKER # OK
  #   # CPACK_BINARY_RPM        # do not use, it does not generate proper rpm packages
  #   CPACK_BINARY_STGZ         # OK
  #   CPACK_BINARY_TBZ2         # OK
  #   CPACK_BINARY_TGZ          # OK
  #   CPACK_SOURCE_TBZ2         # currently not supported
  #   CPACK_SOURCE_TGZ          # currently not supported
  #   CPACK_SOURCE_TZ           # currently not supported
  #   CPACK_SOURCE_ZIP          # currently not supported
  #)
  
  # CPack Mac OS X specific variables
  set(CPACK_BUNDLE_NAME "DIET v${DIET_VERSION}")
  # set(CPACK_BUNDLE_PLIST) # TODO
  # set(CPACK_BUNDLE_ICON) # TODO
  # set(CPACK_DMG_BACKGROUND_IMAGE) # TODO
  
  # Finally include CPack
  include(CPack)
endif()

## eof - CMakeLists.txt
