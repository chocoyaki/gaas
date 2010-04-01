#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )

  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/utils        # for DIET_data_internal.hh
    ${DIET_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

  IF( DIET_USE_DAGDA )
    INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/nodes )
    INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA )
  ENDIF( DIET_USE_DAGDA )

IF( DIET_USE_ALT_BATCH )
  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA        # for ORBMgr (for DietLogComponent)
    ${DIET_SOURCE_DIR}/src/utils        # for DIET_data_internal.hh
    ${DIET_SOURCE_DIR}/src/agent        # for e.g. ChildID.hh (for SeDImpl)
    ${DIET_SOURCE_DIR}/src/SeD          # for SeDImpl and batch
  )
  IF( DIET_USE_JUXMEM )
    INCLUDE_DIRECTORIES( ${JUXMEM_INCLUDE_DIR} )
  ENDIF( DIET_USE_JUXMEM )
ENDIF( DIET_USE_ALT_BATCH )
  
  # Note concerning unresolved symbols in CorbaCommon library:
  # CorbaCommon uses some "stuff" defined in src/utils/DIET_data.cc [e.g.
  # matrix_set_desc(...)] or in src/utils/debug.cc. Hence CorbaCommon should
  # be linked against UtilsCommon for all symbols to be resolved (and thus
  # for diet not to be dirty). But in order to build the UtilsCommon library
  # one first needs to generate the idl headers. Note that CorbaCommon
  # also depends on those idl generated headers since it depends on 
  # the library IDLMA (which in turn depends on IDLCommon which itself
  # depends on the idl generated code).
  # The problem is that this clean and classical dependency tree collides
  # with the directory layout that cmake uses for collecting the targets it
  # is in charge of building. Indeed for historical reasons (due to cmake's
  # ADD_SUBDIRECTORY command) cmake uses a depth first recursive algorithm
  # in order to walk the directory tree (in order to collect the
  # CMakeLists.txt files and the target definitions it contains).
  # Indeed for the dependencies to be respected we should build the
  # libraries in the following order:
  #   src/CORBA/idl/IDLCommon
  #   src/utils/UtilsCommon   (depends on IDLCommon)
  #   src/CORBA/CorbaCommon   (depends on IDLCommon AND UtilsCommon)
  # Alas when walking the directory tree, cmake will define these library
  # targets either in the following order:
  #   src/CORBA/idl/IDLCommon (OK, no dependencies)
  #   src/CORBA/CorbaCommon   (Failure: missing UtilsCommon)
  #   src/utils/UtilsCommon   (OK, depends on IDLCommon)
  # or we could also try:
  #   src/utils/UtilsCommon   (Failure: missing IDLCommon)
  #   src/CORBA/idl/IDLCommon (OK, no dependencies)
  #   src/CORBA/CorbaCommon   (OK, depends on UtilsCommon and IDLCommon)
  # We are doomed !
  # A possible solution would be to change the directory layout by promoting
  # idl subdirectory from src/CORBA/idl to src/idl. Cmake targets would
  # now be defined in the following way:
  #   src/idl/IDLCommon      (OK, no dependencies)
  #   src/utils/UtilsCommon  (OK, depends on IDLCommon)
  #   src/CORBA/CorbaCommon  (OK, depends on UtilsCommon and IDLCommon)
  ADD_LIBRARY( CorbaCommon
    ORBMgr.cc
    marshalling.cc 
  )

IF( DIET_USE_ALT_BATCH )
  ADD_DEPENDENCIES( CorbaCommon 
		    DIET_TARGET_IDL_GENERATED_HEADERS 
		    BatchSystem.hh )
  TARGET_LINK_LIBRARIES( CorbaCommon
    UtilsSeDBatch
  )
ELSE( DIET_USE_ALT_BATCH )
  ADD_DEPENDENCIES( CorbaCommon 
		    DIET_TARGET_IDL_GENERATED_HEADERS )
ENDIF( DIET_USE_ALT_BATCH )

  TARGET_LINK_LIBRARIES( CorbaCommon
    UtilsCommon
    ${OMNIORB4_LIBRARIES}
  )

  ## ------------- New : For scheduler load support. ---------------
  # Install the files needed for a scheduler creation.

  IF( DIET_USE_USERSCHED )
    INSTALL( FILES marshalling.hh DESTINATION include/scheduler )
  ENDIF( DIET_USE_USERSCHED )
# ------------------------------------------------------------------
  
  INSTALL( TARGETS CorbaCommon DESTINATION lib )
ENDIF( DIET_BUILD_LIBRARIES )
