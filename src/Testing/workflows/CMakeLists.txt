if (DIET_USE_WORKFLOW) 
  file( TO_NATIVE_PATH "${PROJECT_SOURCE_DIR}/src/utils/workflow/FWorkflow.dtd" WF_DTD_NATIVE_PATH )
  
  
  file( GLOB XML_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.xml.in" )
  foreach( INPUT ${XML_SRCS} )
    string( REGEX REPLACE "(.*).in$" "\\1" OUTPUT ${INPUT} )
    configure_file( ${INPUT} ${WF_XML_DIR}/${OUTPUT} @ONLY )
  endforeach()

  file( GLOB XML_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.xml" )
  foreach( INPUT ${XML_SRCS} )
    configure_file( ${INPUT} ${WF_XML_DIR}/${INPUT} @ONLY )
  endforeach()
endif (DIET_USE_WORKFLOW) 


# configuring inout_sink.xml.in with all data transfer types
set( WF_PARAM_TYPE "DIET_CHAR" )
set( WF_SERVICE_NAME "CADD")
configure_file( inout_sink.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_SHORT" )
set( WF_SERVICE_NAME "BADD")
configure_file( inout_sink.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_INT" )
set( WF_SERVICE_NAME "IADD")
configure_file( inout_sink.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_LONGINT" )
set( WF_SERVICE_NAME "LADD")
configure_file( inout_sink.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_FLOAT" )
set( WF_SERVICE_NAME "FADD")
configure_file( inout_sink.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_DOUBLE" )
set( WF_SERVICE_NAME "DADD")
configure_file( inout_sink.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )


# configuring inout_sink_vectors.xml.in with all data transfer types
set( WF_PARAM_TYPE "DIET_CHAR" )
set( WF_SERVICE_NAME "CVADD")
configure_file( inout_sink_vectors.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_SHORT" )
set( WF_SERVICE_NAME "BVADD")
configure_file( inout_sink_vectors.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_INT" )
set( WF_SERVICE_NAME "IVADD")
configure_file( inout_sink_vectors.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_LONGINT" )
set( WF_SERVICE_NAME "LVADD")
configure_file( inout_sink_vectors.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_FLOAT" )
set( WF_SERVICE_NAME "FVADD")
configure_file( inout_sink_vectors.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

set( WF_PARAM_TYPE "DIET_DOUBLE" )
set( WF_SERVICE_NAME "DVADD")
configure_file( inout_sink_vectors.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )


# configuring inout_sink.xml.in with all data transfer types
set( WF_PARAM_TYPE "DIET_STRING" )
set( WF_SERVICE_NAME "SPRINT")
configure_file( inout_sink.xml.in ${WF_XML_DIR}/inout_sink_${WF_SERVICE_NAME}.xml @ONLY )

# configuring inout_sink.xml.in with all data transfer types
configure_file( inout_sink_PSPRINT.xml.in ${WF_XML_DIR}/inout_sink_PSPRINT.xml @ONLY )
