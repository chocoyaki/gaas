#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

# Generates the DIET_CORBA static lib

# Include directories for the library compilation:
#    - OmniORB4 include: try to compile without it! ;)
#    - Binary idl directory: for the generated headers files
#    - Juxmem dir: JuxMem is broken. So, don't know why it is necessary.

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_BINARY_DIR}/src/CORBA/idl
)
if (DIET_USE_JUXMEM)
  include_directories(${JUXMEM_INCLUDE_DIR})
endif (DIET_USE_JUXMEM)

# Generate the code out of the IDL source files

# Define the flags for the IDL compiler
#    -bcxx: C++ Backend
#    -Wba:  Generate stubs for TypeCode and Any classes
#    -Wbtp: Generate 'tie' implementation skeletons
 
set(DIET_IDL_FLAGS "-bcxx -Wba -Wbtp")

# Flags for DIET modules
if (DIET_USE_ALT_BATCH)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_ALT_BATCH")
endif (DIET_USE_ALT_BATCH)

if (DIET_USE_JXTA)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_JXTA")
endif (DIET_USE_JXTA)

if (DIET_USE_WORKFLOW)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_WORKFLOW")
endif (DIET_USE_WORKFLOW)

if (DIET_WITH_MULTI_MA)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_MULTI_MA")
endif (DIET_WITH_MULTI_MA)

if (DIET_USE_DAGDA)
  set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_DAGDA")
endif (DIET_USE_DAGDA)

if (DIET_USE_DYNAMICS)
  set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_DYNAMICS")
endif (DIET_USE_DYNAMICS)

if (DIET_USE_USERSCHED)
  set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DUSERSCHED")
  # Install the files needed for scheduler development.
  # TODO: Create a DIET_scheduler.hpp file
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/response.hh
    ${CMAKE_CURRENT_BINARY_DIR}/common_types.hh
    ${CMAKE_CURRENT_BINARY_DIR}/SeD.hh
    DESTINATION include/scheduler
  )
endif (DIET_USE_USERSCHED)

# Transform DIET_IDL_FLAGS from a string to a string list containing
# the arguments for omniidl
separate_arguments(DIET_IDL_FLAGS)

# Fix the IDL source files to compile
#    - Main parts of DIET
#    - Forwarder's IDLs

set(IDL_SOURCES
  common_types
  Agent
  Callback
  LogComponent
  LocalAgent
  LogTypes
  MasterAgent
  response
  SeD
  AgentFwdr
  CallbackFwdr
  LocalAgentFwdr
  LogComponentFwdr
  MasterAgentFwdr
  SeDFwdr
  Forwarder
)
#    - Workflow's IDLs
if (DIET_USE_WORKFLOW)
  set(IDL_SOURCES
    MaDag
    CltMan
    CltManFwdr
    MaDagFwdr
    WfLogService
    WfLogServiceFwdr
    ${IDL_SOURCES}
  )
endif (DIET_USE_WORKFLOW)
#    - Data manager's IDL
if (DIET_USE_DAGDA)
  set(IDL_SOURCES
    Dagda
    DagdaFwdr
    ${IDL_SOURCES}
  )
else (DIET_USE_DAGDA)
  set(IDL_SOURCES
    DataMgr
    LocMgr
    DataMgrFwdr
    LocMgrFwdr
    ${IDL_SOURCES}
  )
endif (DIET_USE_DAGDA)

# A clean way to call omniidl
#   Output:        Skeletons, dynamic skeletons and headers files
#   Depends:       Produced files depends on the idl files
#   Implicit deps: Scan the idl file for dependencies using CXX scanner

foreach(loop_var ${IDL_SOURCES})
  add_custom_command(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}SK.cc
              ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}DynSK.cc
              ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}.hh
    COMMAND   ${OMNIORB4_IDL_COMPILER} ARGS ${DIET_IDL_FLAGS}
              ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}.idl
    DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}.idl
    IMPLICIT_DEPENDS CXX ${loop_var}.idl
    COMMENT   "idl generated code: ${loop_var}SK.cc, ${loop_var}DynSK.cc and ${loop_var}.hh"
  )
  set(DIET_CORBA_SOURCES ${DIET_CORBA_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}SK.cc
    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}DynSK.cc
  )
  set(IDL_GENERATED_HEADERS
    ${IDL_GENERATED_HEADERS}
		${CMAKE_CURRENT_BINARY_DIR}/${loop_var}.hh
		CACHE INTERNAL "CORBA generated headers"
  )
endforeach(loop_var)

# Create the CORBAFiles target.
add_custom_target(CORBAFiles
  DEPENDS ${DIET_CORBA_SOURCES} ${IDL_GENERATED_HEADERS}
)

# Compile the main static CORBA library
add_library(DIET_CORBA STATIC ${DIET_CORBA_SOURCES})
add_dependencies(DIET_CORBA ${IDL_GENERATED_HEADERS})