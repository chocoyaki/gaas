FILE( GLOB DIET_WF_XML_GLOB 
  "${DIET_SOURCE_DIR}/src/examples/workflow/xml/*.xml"
)
FOREACH( filename ${DIET_WF_XML_GLOB} )
  GET_FILENAME_COMPONENT( filenameNoPath ${filename} NAME )
  SET( filenameDest ${DIET_BINARY_DIR}/examples/workflow/${filenameNoPath} )
  CONFIGURE_FILE(
    ${filename}
    ${filenameDest}
    IMMEDIATE
  )
  INSTALL( FILES ${filenameNoPath} DESTINATION ${EXAMPLE_INSTALL_DIR}/workflow/xml )
ENDFOREACH( filename )

INSTALL( FILES logo_diet.jpg DESTINATION ${EXAMPLE_INSTALL_DIR}/workflow )
