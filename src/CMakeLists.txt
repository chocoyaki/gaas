#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
include(UseOmniORB4)

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${IDL_INCLUDE_DIRECTORIES}
)

# This macro extracts the complete path to the generated header file
# containing the name passed as arg2 and stores it into arg1.
macro(corba_header arg1 arg2)
  foreach(hdr ${IDL_GENERATED_HEADERS})
    string(REGEX MATCH ".*/${arg2}.hh" result ${hdr})
    if (NOT ${result} EQUAL "")
      set(${arg1} ${result})
    endif (NOT ${result} EQUAL "")
  endforeach(hdr)
endmacro(corba_header)

# This macro extracts the complete paths to the generated header files
# containing the names passed as arguments and stores them into arg1
macro(corba_headers arg1)
  foreach (arg ${ARGV})
    corba_header(tmp ${arg})
    set(res ${res} ${tmp})
  endforeach(arg)
  set(${arg1} ${res})
endmacro(corba_headers)


## generate CORBA idl required by almost everyone
set(DIET_IDL_SRCS
  CORBA/idl/common_types.idl
  CORBA/idl/Agent.idl
  CORBA/idl/Callback.idl
  CORBA/idl/LocalAgent.idl
  CORBA/idl/MasterAgent.idl
  CORBA/idl/response.idl
  CORBA/idl/SeD.idl
  CORBA/idl/AgentFwdr.idl
  CORBA/idl/CallbackFwdr.idl
  CORBA/idl/LocalAgentFwdr.idl
  CORBA/idl/MasterAgentFwdr.idl
  CORBA/idl/SeDFwdr.idl
  CORBA/idl/Forwarder.idl
  CORBA/idl/Dagda.idl
  CORBA/idl/DagdaFwdr.idl)

# Flags for DIET modules
if (DIET_USE_ALT_BATCH)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_ALT_BATCH")
endif (DIET_USE_ALT_BATCH)

if (DIET_USE_WORKFLOW)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_WORKFLOW")
   set(DIET_IDL_SRCS ${DIET_IDL_SRCS}
     CORBA/idl/MaDag.idl
     CORBA/idl/CltMan.idl
     CORBA/idl/CltManFwdr.idl
     CORBA/idl/MaDagFwdr.idl
     CORBA/idl/WfLogService.idl
     CORBA/idl/WfLogServiceFwdr.idl)

endif (DIET_USE_WORKFLOW)

if (DIET_WITH_MULTI_MA)
   set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_MULTI_MA")
endif (DIET_WITH_MULTI_MA)

if (DIET_USE_USERSCHED)
  set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DUSERSCHED")
  # Install the files needed for scheduler development.
  # TODO: Create a DIET_scheduler.hpp file
  install(FILES
    ${IDL_INCLUDE_DIRECTORIES}/response.hh
    ${IDL_INCLUDE_DIRECTORIES}/common_types.hh
    ${IDL_INCLUDE_DIRECTORIES}/SeD.hh
    DESTINATION include/scheduler
  )
endif (DIET_USE_USERSCHED)

generate_idl(OUTPUT DIET_IDL_OUTPUT
  IDLS ${DIET_IDL_SRCS}
  FLAGS ${DIET_IDL_FLAGS}
  LANG C++)









add_subdirectory(utils)
add_subdirectory(CORBA)
add_subdirectory(utils/nodes)
add_subdirectory(client)
if (DIET_USE_WORKFLOW)
  add_subdirectory(utils/workflow)
endif (DIET_USE_WORKFLOW)

add_subdirectory(agent)
add_subdirectory(SeD)
add_subdirectory(Testing)

if( DIET_BUILD_EXAMPLES )
  add_subdirectory( examples )
endif( DIET_BUILD_EXAMPLES )
