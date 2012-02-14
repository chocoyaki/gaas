# Use OmniORB4 cmake module
#
# defines the following variables:
# IDL_OUTPUT_PATH          - OmniORB4 idl generation path
# IDL_INCLUDE_DIRECTORIES  - OmniORB4 headers path needed for idl processing
# 
set(IDL_OUTPUT_PATH "${PROJECT_BINARY_DIR}/idl")
# TODO: encapsulate this variable in include_directories-like macro
set(IDL_INCLUDE_DIRECTORIES "${PROJECT_BINARY_DIR}/idl")

include(CMakeParseArguments)

# macro processing idl
# TODO: support Python backend
# TODO: allow user to refine generation flags
macro(generate_idl)
  # parse named arguments
  set(options "")
  set(oneValueArgs OUTPUT_PATH OUTPUT)
  set(multiValueArgs INCLUDE_DIR LANG IDLS FLAGS)
  cmake_parse_arguments(omni
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # no idl no cookies
  if(NOT omni_IDLS)
    message(FATAL_ERROR "Please provide idl sources to generate_idl")
  endif()
  # define OUTPUT_PATH
  if(omni_OUTPUT_PATH)
    set(MY_OUTPUT_PATH "${omni_OUTPUT_PATH}")
  else()
    set(MY_OUTPUT_PATH "${IDL_OUTPUT_PATH}")
  endif()
  file(MAKE_DIRECTORY ${MY_OUTPUT_PATH})
  # add include directories
  if(omni_INCLUDE_DIR)
    set(MY_INCLUDE_DIR ${IDL_INCLUDE_DIRECTORIES} ${omni_INCLUDE_DIR})
  else()
    set(MY_INCLUDE_DIR ${IDL_INCLUDE_DIRECTORIES})
  endif()
  # setup flags
  set(MY_FLAGS "")
  if(omni_LANG)
    foreach(loop_var ${omni_LANG})
      if(${loop_var} MATCHES "[cC]\\+\\+")
        # -Wba: generate stubs for TypeCode and Any classes
        # -Wbtp: generate tie skeletons
        list(APPEND MY_FLAGS_BACKEND "-bcxx -Wba -Wbtp")
      elseif(${loop_var} MATCHES "python")
        list(APPEND MY_FLAGS_BACKEND "-bpython")
      endif()
      # fallback on C++
    endforeach()
  endif()
  if(MY_FLAGS_BACKEND)
    list(APPEND MY_FLAGS ${MY_FLAGS_BACKEND})
  else()
    list(APPEND MY_FLAGS "-bcxx -Wba -Wbtp")
  endif()

  list(APPEND MY_FLAGS "-C ${IDL_OUTPUT_PATH}")
  if(MY_INCLUDE_DIR)
    foreach(loop_var ${MY_INCLUDE_DIR})
      list(APPEND MY_FLAGS "-I${loop_var}")
    endforeach()
  endif()

  if(omni_FLAGS)
    list(APPEND MY_FLAGS ${omni_FLAGS})
  endif()
  separate_arguments(MY_FLAGS)
  
  foreach(loop_var ${omni_IDLS})
    get_filename_component(_name ${loop_var} NAME_WE)
    add_custom_command(
    OUTPUT    ${MY_OUTPUT_PATH}/${_name}SK.cc
              ${MY_OUTPUT_PATH}/${_name}DynSK.cc
              ${MY_OUTPUT_PATH}/${_name}.hh
    COMMAND   ${OMNIORB4_IDL_COMPILER} ARGS ${MY_FLAGS}
              ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}
    DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}
    COMMENT   "idl generated code: ${_name}SK.cc, ${_name}DynSK.cc and ${_name}.hh"
    )
    set(${omni_OUTPUT} ${${omni_OUTPUT}} ${MY_OUTPUT_PATH}/${_name}SK.cc ${MY_OUTPUT_PATH}/${_name}DynSK.cc)
  endforeach()
endmacro()
