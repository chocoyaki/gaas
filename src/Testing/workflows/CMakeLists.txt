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

