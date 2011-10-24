# do not use UseDoxygen.cmake - too limited -
find_package(Doxygen)
## first: find doxygen
if(NOT DOXYGEN_FOUND)
  message(WARNING "In order to generate doxygen documentation "
    "You need to install doxygen")
endif()
## second: find dot
if(DOXYGEN_DOT_FOUND)
  set(DOXYGEN_HAVE_DOT "YES") # variable used in Doxyfile
else()
  message(WARNING "Dot executable was not found, it is required "
    "to build class diagrams in documentation. It is usually provided "
    "as part of graphviz suite")
  set(DOXYGEN_HAVE_DOT "NO")
endif()

# sets default values for Doxyfile
set(doxygen_FORMATS "HTML;MAN;PDF;RTF")
foreach(loop_var ${doxygen_FORMATS})
  set("GENERATE_${loop_var}" "NO")
endforeach()

include(CMakeParseArguments)
# @brief generate_doxygen: generate doxygen documentation
# @param[optional] OUTPUT (default: ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
# @param[optional] FORMATS (default: html;man allowed:pdf;rtf)
macro(generate_doxygen)
  # parse named arguments
  set(options "")
  set(oneValueArgs OUTPUT_PATH)
  set(multiValueArgs FORMATS)
  cmake_parse_arguments(dox
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  # set doxygen output path
  if(dox_OUTPUT_PATH)
    set(DOXYFILE_OUTPUT_DIR ${dox_OUTPUT_PATH})
  else()
    set(DOXYFILE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
  endif()
  file(MAKE_DIRECTORY ${DOXYFILE_OUTPUT_DIR})
  # set doxygen output formats
  if(NOT dox_FORMATS)
    set(dox_FORMATS "html;man")
  endif()
  foreach(loop_var ${dox_FORMATS})
    string(TOUPPER ${loop_var} format)
    set("GENERATE_${format}" "YES")
  endforeach()
  # configure our Doxyfile
  configure_file(${PROJECT_SOURCE_DIR}/Cmake/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  # add new doxygen target
  add_custom_target(doxygen
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "generate doxygen documentation")
  # we might want doxygen pdf build
  if(GENERATE_PDF)
    find_package(LATEX)
    if(LATEX_COMPILER AND MAKEINDEX_COMPILER)
      add_custom_command(TARGET doxygen
        POST_BUILD
        COMMAND "${CMAKE_BUILD_TOOL}"
        WORKING_DIRECTORY ${DOXYFILE_OUTPUT_DIR}/latex
        COMMENT "Building pdf documentation in ${DOXYFILE_OUTPUT_DIR}/latex")
    else()
      message(WARNING "You need a latex and a makeindex compiler to build "
        "PDF documentation")
    endif()
  endif()
  # add dependency to doc target
  add_dependencies(doc doxygen)
  # ensure that we clean up generated files at clean target
  set_property(DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES ${DOXYFILE_OUTPUT_DIR})
endmacro()