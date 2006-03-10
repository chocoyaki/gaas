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
# Force the idl compiler invocation
ADD_DEPENDENCIES( CorbaCommon IDLAgent IDLAgent IDLLA IDLMA )
INSTALL_TARGETS(/lib/ CorbaCommon )
