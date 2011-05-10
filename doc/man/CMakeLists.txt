add_custom_target( doc-man )
add_dependencies( doc doc-man )

file( GLOB MAN_PAGES_SRCS 
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.rst" )

# create man output directory
file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/man1" )

foreach( SRC ${MAN_PAGES_SRCS} )
  string( REGEX REPLACE "(.*).rst$" "\\1" BASENAME ${SRC} )
  add_custom_target( "man-${BASENAME}"
    ${RST2MAN_BIN} "${CMAKE_CURRENT_SOURCE_DIR}/${SRC}" "man1/${BASENAME}.1" )
  add_dependencies( doc-man "man-${BASENAME}" ) 
endforeach()

install( DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/man1"
  DESTINATION "${MAN_INSTALL_DIR}" )