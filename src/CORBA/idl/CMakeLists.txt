#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

# Generates the DIET_CORBA static lib

# Generate the code out of the IDL source files

# Define the flags for the IDL compiler
#    -bcxx: C++ Backend
#    -Wba:  Generate stubs for TypeCode and Any classes
#    -Wbtp: Generate 'tie' implementation skeletons
 
# Flags for DIET modules
if (DIET_USE_ALT_BATCH)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_ALT_BATCH")
endif (DIET_USE_ALT_BATCH)

if (DIET_USE_WORKFLOW)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_WORKFLOW")
endif (DIET_USE_WORKFLOW)

if (DIET_WITH_MULTI_MA)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_MULTI_MA")
endif (DIET_WITH_MULTI_MA)

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

# Fix the IDL source files to compile
#    - Main parts of DIET
#    - Forwarder's IDLs

set(DIET_IDL_SRCS
  common_types.idl
  Agent.idl
  Callback.idl
  LocalAgent.idl
  MasterAgent.idl
  response.idl
  SeD.idl
  AgentFwdr.idl
  CallbackFwdr.idl
  LocalAgentFwdr.idl
  MasterAgentFwdr.idl
  SeDFwdr.idl
  Forwarder.idl
)
#    - Workflow's IDLs
if (DIET_USE_WORKFLOW)
  set(DIET_IDL_SRCS
    MaDag.idl
    CltMan.idl
    CltManFwdr.idl
    MaDagFwdr.idl
    WfLogService.idl
    WfLogServiceFwdr.idl
    ${DIET_IDL_SRCS}
  )
endif (DIET_USE_WORKFLOW)
#    - Data manager's IDL
set(DIET_IDL_SRCS
  Dagda.idl
  DagdaFwdr.idl
  ${DIET_IDL_SRCS}
  )

generate_idl(OUTPUT DIET_IDL_OUTPUT
  IDLS ${DIET_IDL_SRCS}
  FLAGS ${DIET_IDL_FLAGS}
  LANG C++)

# Compile the main static CORBA library
add_library(DIET_CORBA STATIC ${DIET_IDL_OUTPUT})

