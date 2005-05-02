#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.4  2005/05/02 15:34:54  eboix
#*     - Cmake/FindOmniORB.cmake now exposes the OMNIORB4_DIR (no need to
#*       toggle to advanced mode anymore) and also hides away (to advanced mode)
#*       the PATHs (include, lib, omniidl) when found.
#*       Tested on FC2 and graal.ens-lyon.fr: libCOS4.so and libDynamicCOS4.so
#*       seemed to be allways required...
#*     - src/examples/cfgs/CMakeLists.txt now generates all the test configuration
#*       files.   --- Injay 2461
#*
#* Revision 1.3  2005/04/29 12:05:22  eboix
#*      Clean up.  --- Injay 2461
#*
#* Revision 1.2  2005/04/29 10:36:37  eboix
#*     idl generated files are now taken into account.  --- Injay 2461
#*
#* Revision 1.1  2005/04/28 13:25:10  eboix
#*     Added cmake subdir configuration files. --- Injay 2461
#*
#****************************************************************************#

# When OMNIORB was found generate the code out of the IDL source files and
# build the ad-hoc libraries.
IF( OMNIORB4_FOUND )
  
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
  
  SET( TARGET_IDL_DIR  ${DIET_BINARY_DIR}/src/CORBA/idl )
  
  FOREACH( loop_var ${IDL_SOURCES} )
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${TARGET_IDL_DIR}/${loop_var}SK.cc
      COMMAND   ${OMNIORB4_IDL_COMPILER}
      ARGS      -bcxx -Wba -Wbtp 
                ${DIET_SOURCE_DIR}/src/CORBA/idl/${loop_var}.idl
      DEPENDS   ${DIET_SOURCE_DIR}/src/CORBA/idl/${loop_var}.idl
      COMMENT   "idl generated code"
    )
    # Dirty trick to avoid executing twice the IDL compiler which produces
    # multiple outputs ${loop_var}SK.cc, ${loop_var}DynSK.cc and
    # ${loop_var}.hh [refer to
    # http://public.kitware.com/pipermail/cmake/2004-August/005392.html ]
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${TARGET_IDL_DIR}/${loop_var}DynSK.cc
      COMMAND   echo
      ARGS      -n
      DEPENDS   ${TARGET_IDL_DIR}/${loop_var}SK.cc
      COMMENT   "idl generated code"
    )
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${TARGET_IDL_DIR}/${loop_var}.hh
      COMMAND   echo
      ARGS      -n
      DEPENDS   ${TARGET_IDL_DIR}/${loop_var}SK.cc
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
    LogTypesDynSK.cc )
  
  ADD_LIBRARY( IDLCommon 
    CallbackDynSK.cc
    CallbackSK.cc
    common_typesSK.cc
    common_typesDynSK.cc
    responseDynSK.cc
    responseSK.cc
    SeDSK.cc
    SeDDynSK.cc )
  
  ADD_LIBRARY( IDLLA
    LocalAgentSK.cc
    LocalAgentDynSK.cc )
  
  ADD_LIBRARY( IDLMA
    MasterAgentSK.cc
    MasterAgentDynSK.cc )
  
ENDIF( OMNIORB4_FOUND )
