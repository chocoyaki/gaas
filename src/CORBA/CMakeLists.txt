#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
ADD_SUBDIRECTORY( idl )

IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/utils        # for DIET_data_internal.hh
    ${DIET_BINARY_DIR}/include          # FIXME: must preceed source/include
                                        # to take cmake-generated DIET_config.h
    ${DIET_SOURCE_DIR}/include          # for e.g. DIET_data.h
  )
  
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
  ADD_DEPENDENCIES( CorbaCommon DIET_TARGET_IDL_GENERATED_HEADERS )
  
  TARGET_LINK_LIBRARIES( CorbaCommon
    ${OMNIORB4_LIBRARIES}
  )
  ################### MacOS X and unresolved symbol at link time:
  # The following kludge doesn't imply there is something wrong with MacOS X
  # (well, at least in this case). It is just that by default when linking
  # a shared library with gcc, the undefined symbols are NOT considered 
  # as errors (also reported as warning by default). But on MacOS X when
  # linking a shared library with the native linker undefined symbols 
  # are considered as errors (by default)... Thus we need to override 
  # this default behavior because CorbaCommon has indeed undefined symbols
  # (see the above remark).
  IF( APPLE )
    SET_TARGET_PROPERTIES( CorbaCommon
      PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress" )
  ENDIF( APPLE )
  
  INSTALL_TARGETS(/lib/ CorbaCommon )
ENDIF( DIET_BUILD_LIBRARIES )
