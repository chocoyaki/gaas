#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_USE_JXTA )
  ADD_CUSTOM_COMMAND(
    OUTPUT    ${DIET_BINARY_DIR}/src/examples/JXTA/JXTAClient.class
    COMMAND   ${JAVA_COMPILE}
    ARGS      -classpath ${DIET_SOURCE_DIR}/src/lib/jxta.jar
              -d ${DIET_BINARY_DIR}/src/examples/JXTA
              ${DIET_SOURCE_DIR}/src/examples/JXTA/JXTAClient.java
    DEPENDS   ${DIET_SOURCE_DIR}/src/examples/JXTA/JXTAClient.java
    COMMENT   "java compile"
  )
  ADD_CUSTOM_TARGET( JXTAClientNameDummy ALL echo
   DEPENDS   ${DIET_BINARY_DIR}/src/examples/JXTA/JXTAClient.class
   )

ENDIF( DIET_USE_JXTA )

