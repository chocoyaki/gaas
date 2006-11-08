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
  INSTALL_FILES( /bin/examples/workflow/xml FILES ${filenameNoPath} )
ENDFOREACH( filename )

INSTALL_FILES( /bin/examples/workflow FILES logo_diet.jpg )
