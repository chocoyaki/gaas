#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

# When OMNIORB was found generate the code out of the IDL source files and
# build the ad-hoc libraries.
IF( OMNIORB4_FOUND )

  # Define the flags for the IDL compiler
  SET( DIET_IDL_FLAGS "-bcxx -Wba -Wbtp")
  IF( DIET_USE_JXTA )
     SET( DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_JXTA" )
  ENDIF( DIET_USE_JXTA )
  SEPARATE_ARGUMENTS( DIET_IDL_FLAGS )

  SET( IDL_SOURCES
    Agent
    Callback
    common_types
    DataMgr
    LogComponent
    LocalAgent
    LocMgr
    LogTypes
    MasterAgent
    response
    SeD )
  
  FOREACH( loop_var ${IDL_SOURCES} )

    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}SK.cc
      COMMAND   ${OMNIORB4_IDL_COMPILER}
      ARGS      ${DIET_IDL_FLAGS}
                ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}.idl
      DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}.idl
      COMMENT   "idl generated code"
    )

    # Dirty trick to avoid executing twice the IDL compiler which produces
    # multiple outputs ${loop_var}SK.cc, ${loop_var}DynSK.cc and
    # ${loop_var}.hh [refer to
    # http://public.kitware.com/pipermail/cmake/2004-August/005392.html ]
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}DynSK.cc
      COMMAND   echo
      ARGS      -n
      DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}SK.cc
      COMMENT   "idl generated code"
    )
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}.hh
      COMMAND   echo
      ARGS      -n
      DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}SK.cc
      COMMENT   "idl generated code"
    )
  ENDFOREACH(loop_var)
  
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA/idl     # Current directory
  )
  
  ADD_LIBRARY( IDLAgent 
    AgentSK.cc
    AgentDynSK.cc
    DataMgrSK.cc
    DataMgrDynSK.cc
    LogComponentDynSK.cc
    LogComponentSK.cc
    LocMgrSK.cc
    LocMgrDynSK.cc
    LogTypesSK.cc
    LogTypesDynSK.cc 
  )
  INSTALL_TARGETS(/lib/ IDLAgent )
  
  ADD_LIBRARY( IDLCommon 
    CallbackDynSK.cc
    CallbackSK.cc
    common_typesSK.cc
    common_typesDynSK.cc
    responseDynSK.cc
    responseSK.cc
    SeDSK.cc
    SeDDynSK.cc
  )
  INSTALL_TARGETS(/lib/ IDLCommon )
  
  ADD_LIBRARY( IDLLA
    LocalAgentSK.cc
    LocalAgentDynSK.cc
   )
  INSTALL_TARGETS(/lib/ IDLLA )
  
  ADD_LIBRARY( IDLMA
    MasterAgentSK.cc
    MasterAgentDynSK.cc
  )
  INSTALL_TARGETS(/lib/ IDLMA )
  
ENDIF( OMNIORB4_FOUND )
