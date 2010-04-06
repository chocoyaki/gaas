#****************************************************************************#
#* GAS SeD
#****************************************************************************#

FIND_PATH(BOOST_INCLUDE_DIR boost/filesystem.hpp /usr/local/include /usr/include)
FIND_LIBRARY(BOOST_LIBRARY boost_filesystem PATHS /usr/local/lib /usr/lib)

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include
                     ${DIET_SOURCE_DIR}/src/utils/DAGDA
                     ${DIET_SOURCE_DIR}/src/utils/workflow
                     ${BOOST_INCLUDE_DIR}
		     ${XERCES_INCLUDE_DIR} )

ADD_EXECUTABLE( GASWServer
  SeDDescrParser.hh
  SeDDescrParser.cc
  SeDService.hh
  SeDService.cc
  GASWServer.cc )

TARGET_LINK_LIBRARIES( GASWServer
  DIET_SeD
  UtilsWf
  ${XERCES_LIBRARY}
  ${BOOST_LIBRARY}
)

INSTALL( TARGETS GASWServer DESTINATION bin/examples/SeDWrapper )
ADD_SUBDIRECTORY( xml )
