#
# Display a little summary
#

# Display a title line
function(disp )
  if (NOT DEFINED ARGV1)
    set(char "X")
  else (NOT DEFINED ARGV1)
    set(char ${ARGV1})
  endif (NOT DEFINED ARGV1)
  
  set(cmpt 0)
  set(resultStr "")
  
  if (DEFINED ARGV0)
    string(LENGTH ${ARGV0} len)
  else (DEFINED ARGV0)
    set(len 0)
  endif (DEFINED ARGV0)
  
  if (${len} GREATER 0)
    math(EXPR len "${len} + 4")
  endif(${len} GREATER 0)
  math(EXPR len "(76  - ${len})/2")
  
  while (${cmpt} LESS ${len})
    set(resultStr ${resultStr}${char})
    math(EXPR cmpt "${cmpt}+1")
  endwhile (${cmpt} LESS ${len})
  
  if (DEFINED ARGV0)
    string(LENGTH ${ARGV0} modulo)
    math(EXPR modulo "${modulo} % 2")
    if (${modulo} EQUAL 0)
      set(resultStr "${resultStr}  ${ARGV0}  ${resultStr}")
    else (${modulo} EQUAL 0)
      set(resultStr "${resultStr}  ${ARGV0}  ${resultStr}${char}")
    endif (${modulo} EQUAL 0)
  else (DEFINED ARGV0)
    set(resultStr "${resultStr}${resultStr}")
  endif (DEFINED ARGV0)
  message(STATUS ${resultStr})
endfunction(disp)

disp()
disp("DIET ${DIET_VERSION}")
disp("Configuration summary")
disp("${DIET_BUILD_VERSION}")
disp()

if (CYGWIN)
  message(STATUS "XXX System name Cygwin on Windows")
elseif (WINDOWS)
  message(STATUS "XXX System name Windows native")
elseif (APPLE)
  message(STATUS "XXX System name Darwin")
elseif (LINUX)
  message(STATUS "XXX System name Linux")
elseif (AIX)
  message(STATUS "XXX System name Aix")
elseif (SUNOS)
  message(STATUS "XXX System name SunOS")
elseif (FREEBSD)
  message(STATUS "XXX System name FreeBSD")
endif (CYGWIN)

message(STATUS "XXX - Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "XXX - C compiler   : ${CMAKE_C_COMPILER}")
if (CMAKE_C_COMPILER_VERSION)
  message(STATUS "XXX    * version   : ${CMAKE_C_COMPILER_VERSION}")
endif (CMAKE_C_COMPILER_VERSION)
message(STATUS "XXX    * options   : ${CMAKE_C_FLAGS}")
message(STATUS "XXX - CXX compiler : ${CMAKE_CXX_COMPILER}")
if (CMAKE_CXX_COMPILER_VERSION)
  message(STATUS "XXX    *  version  : ${CMAKE_CXX_COMPILER_VERSION}")
endif (CMAKE_CXX_COMPILER_VERSION)
message(STATUS "XXX    *  options  : ${CMAKE_CXX_FLAGS}")

message(STATUS "XXX - OmniORB found: ${OMNIORB4_FOUND}")
if (OMNIORB4_FOUND)
  message(STATUS "XXX   * OmniORB version: ${OMNIORB4_VERSION}")
  message(STATUS "XXX   * OmniORB directory: ${OMNIORB4_DIR}")
  message(STATUS "XXX   * OmniORB includes: ${OMNIORB4_INCLUDE_DIR}")
  message(STATUS "XXX   * OmniORB libraries:")
  foreach (lib ${OMNIORB4_LIBRARIES})
    message(STATUS "XXX     - ${lib}")
  endforeach (lib)
endif (OMNIORB4_FOUND)

message(STATUS "XXX - General options: ")
if (DIET_BUILD_LIBRARIES)
  if (BUILD_SHARED_LIBS)
    message(STATUS "XXX   * Dynamics Libraries: ON ")
  else(BUILD_SHARED_LIBS)
    message(STATUS "XXX   * Static Libraries: ON ")
  endif (BUILD_SHARED_LIBS)
endif (DIET_BUILD_LIBRARIES)

message(STATUS "XXX   * Examples: ${DIET_BUILD_EXAMPLES} ")
if (DIET_BUILD_EXAMPLES AND DIET_BUILD_BLAS_EXAMPLES)
  message(STATUS "XXX   * BLAS Examples: ${DIET_BUILD_BLAS_EXAMPLES} ")
endif (DIET_BUILD_EXAMPLES AND DIET_BUILD_BLAS_EXAMPLES)

message(STATUS "XXX - Options set: ")
if (DIET_USE_ALT_BATCH)
  message(STATUS "XXX   * Batch: ${DIET_USE_ALT_BATCH} ")
endif (DIET_USE_ALT_BATCH)
if (DIET_USE_CCS)
  message(STATUS "XXX   * Client scheduling: ${DIET_USE_CCS} ")
endif (DIET_USE_CCS)
if (DIET_USE_CLOUD_EC2)
  message(STATUS "XXX   * CLOUD_EC2: ${DIET_USE_CLOUD_EC2} ")
  message(STATUS "XXX     -- source directory: ${GSOAP_SRC_DIR}")
endif (DIET_USE_CLOUD_EC2)
if (DIET_TRANSFER_PROGRESSION)
  message(STATUS "XXX   * Progression API: ${DIET_TRANSFER_PROGRESSION} ")
endif ()
if (DIET_USE_LOG)
  message(STATUS "XXX   * LogService: ${DIET_USE_LOG} ")
  message(STATUS "XXX     -- LogService library: ${LOG_LIBRARY}")
  message(STATUS "XXX     -- LogService include: ${LOG_INCLUDE_DIR}")
endif (DIET_USE_LOG)
if (DIET_WITH_STATISTICS)
  message(STATUS "XXX   * Statistics: ${DIET_WITH_STATISTICS} ")
endif (DIET_WITH_STATISTICS)
if (DIET_WITH_MULTI_MA)
  message(STATUS "XXX   * Multi-MA: ${DIET_WITH_MULTI_MA} ")
endif (DIET_WITH_MULTI_MA)
if (DIET_USE_WORKFLOW)
  message(STATUS "XXX   * Workflow: ${DIET_USE_WORKFLOW} ")
  message(STATUS "XXX     -- Xerces version: ${XERCES_VERSION_MAJOR}.${XERCES_VERSION_MINOR}.${XERCES_VERSION_REVISION}")
  message(STATUS "XXX     -- Xerces directory: ${XERCES_DIR} ")
  message(STATUS "XXX     -- Xerces includes: ${XERCES_INCLUDE_DIR}")
  message(STATUS "XXX     -- Xerces library: ${XERCES_LIBRARY}")
  message(STATUS "XXX     -- XQilla directory: ${XQILLA_DIR} ")
  message(STATUS "XXX     -- XQilla includes: ${XQILLA_INCLUDE_DIR}")
  message(STATUS "XXX     -- XQilla library: ${XQILLA_LIBRARY}")
endif (DIET_USE_WORKFLOW)
disp()
