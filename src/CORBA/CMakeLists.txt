#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.2  2005/04/29 10:36:37  eboix
#*     idl generated files are now taken into account.  --- Injay 2461
#*
#* Revision 1.1  2005/04/28 13:25:10  eboix
#*     Added cmake subdir configuration files. --- Injay 2461
#*
#****************************************************************************#

# PREORDER is here required because above all the things we need the
# makefiles to generate the IDL interface files.
SUBDIRS( PREORDER idl )

INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_BINARY_DIR}/src/CORBA/idl
#CLEANME  ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
   ${DIET_SOURCE_DIR}/src/utils        # for DIET_data_internal.hh
   ${DIET_BINARY_DIR}/include          # FIXME: must preceed source/include
                                       # to take cmake-generated DIET_config.h
   ${DIET_SOURCE_DIR}/include          # for e.g. DIET_data.h
  )


ADD_LIBRARY( CorbaCommon
  ORBMgr.cc
  marshalling.cc )
