#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
ADD_SUBDIRECTORY( idl )

INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/utils        # for DIET_data_internal.hh
  ${DIET_BINARY_DIR}/include          # FIXME: must preceed source/include
                                      # to take cmake-generated DIET_config.h
  ${DIET_SOURCE_DIR}/include          # for e.g. DIET_data.h
)


ADD_LIBRARY( CorbaCommon
  ORBMgr.cc
  marshalling.cc 
)
ADD_DEPENDENCIES( CorbaCommon DIET_TARGET_IDL_GENERATED_HEADERS )

TARGET_LINK_LIBRARIES( CorbaCommon
  ${OMNIORB4_LIBRARIES}
)
################### MacOS X and unresolved symbol at link time:
# The following kludge shouldn't seen as a problem with MacOS X. It is just
# that by default when linking a shared library with gcc, the undefined
# symbols are NOT reported by default. On MacOS X, undefined symbols at
# (shared) library linking are considered an error by default...
# Indeed CorbaCommon uses some "stuff" defined in src/utils/DIET_data.c or
# in debug.cc: hence CorbaCommon should be linked against UtilsCommon for
# diet not to be dirty. Alas this would create a circular dependency in the
# DEFINITION of the cmake targets which recurses in an directory oriented
# manner (for historical reasons due to cmake'as SUBDIR) since
# UtilsCommon needs the idl genereted headers BUT CorbaCommon needs 
# IDLMA...
IF( APPLE )
  SET_TARGET_PROPERTIES( CorbaCommon
    PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress" )
ENDIF( APPLE )

INSTALL_TARGETS(/lib/ CorbaCommon )
