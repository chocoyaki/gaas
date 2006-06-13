# Caveat emptor: according to the thread 
#    http://public.kitware.com/pipermail/cmake/2006-April/008795.html
# cmake's current (2.2.x) abilities to define a Gui/Command-line overridable
# build mode (or customized built-in mode default compiler flags or an extended 
# build mode) are limited.
#
# FOR THE TIME BEING ALL THE DEFAULTS AND EXTENSIONS SET HERE ARE NOT
# OVERRIDABLE EITHER FROM THE COMMAND LINE NOR FROM THE GUIs.
# Hence if you need to change some of those defaults you'ell get to
# edit this file and manage things manually... Sorry for that !
#
################# OFFER AN EXTRA MAINTAINER BUILD MODE #######################
# The extra maintainer build type is a build mode for which the compilers
# and linkers go paranoid and report about most of the warning they are aware
# of.
# Note: this Maintainer mode is only defined for GCC compiler collection.
SET( CMAKE_BUILD_TYPE_DOCSTRING
  "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." )

IF( CMAKE_COMPILER_IS_GNUCC AND CMAKE_COMPILER_IS_GNUCXX )
  # -Wold-style-cast generates too many warnings due to omniORB includes
  # -pedantic generates ISO C++ unsupported 'long long' errors for omniORB
  # includes
  SET( CMAKE_CXX_FLAGS_MAINTAINER
    "-Wall -Wabi -Wstrict-null-sentinel -Woverloaded-virtual"
    CACHE STRING 
    "Flags used by the C++ compiler during maintainer builds."
    FORCE
  )
  SET( CMAKE_C_FLAGS_MAINTAINER
    "-Wall -pedantic"
    CACHE STRING 
    "Flags used by the C compiler during maintainer builds."
    FORCE
  )
  SET( CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    "-Wl,--unresolved-symbols=report-all,--warn-unresolved-symbols,--warn-once"
    CACHE STRING 
    "Flags used for linking binaries during maintainer builds."
    FORCE
  )
  SET( CMAKE_SHARED_LINKER_FLAGS_MAINTAINER
    "-Wl,--unresolved-symbols=report-all,--warn-unresolved-symbols,--warn-once"
    CACHE STRING 
    "Flags used by the shared libraries linker during maintainer builds."
    FORCE
  )
  SET( CMAKE_MODULE_LINKER_FLAGS_MAINTAINER
    "-dummy_option_to_see_what_happens"
    CACHE STRING 
    "What the hack is a module anyhow (Apple notion?)..."
    FORCE
  )
  MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_MAINTAINER
    CMAKE_C_FLAGS_MAINTAINER
    CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    CMAKE_SHARED_LINKER_FLAGS_MAINTAINER
    CMAKE_MODULE_LINKER_FLAGS_MAINTAINER
  )
  SET( CMAKE_BUILD_TYPE_DOCSTRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Maintainer." )
ENDIF( CMAKE_COMPILER_IS_GNUCC AND CMAKE_COMPILER_IS_GNUCXX )

################# MODIFY THE DEFAULT OF DEBUG MODE #######################
# When using GCC compiler collection, the Debug build-in mode type (refer
# to CMAKE_BUILD_TYPE) is modified to be defined as the Maintainer mode
# with an additional -g flag.
IF( CMAKE_COMPILER_IS_GNUCC AND CMAKE_COMPILER_IS_GNUCXX )
  SET( CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG_INIT} ${CMAKE_CXX_FLAGS_MAINTAINER}"
    CACHE STRING 
    "Flags used by the C++ compiler during debug builds."
    FORCE
  )
  SET( CMAKE_C_FLAGS_DEBUG
    "${CMAKE_C_FLAGS_DEBUG_INIT} ${CMAKE_C_FLAGS_MAINTAINER}"
    CACHE STRING 
    "Flags used by the C compiler during debug builds."
    FORCE
  )
  SET( CMAKE_EXE_LINKER_FLAGS_DEBUG
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT} ${CMAKE_EXE_LINKER_FLAGS_MAINTAINER}"
    CACHE STRING 
    "Flags used for linking binaries during debug builds."
    FORCE
  )
  SET( CMAKE_SHARED_LINKER_FLAGS_DEBUG
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT} ${CMAKE_SHARED_LINKER_FLAGS_MAINTAINER}"
    CACHE STRING 
    "Flags used by the shared libraries linker during debug builds."
    FORCE
  )
  SET( CMAKE_MODULE_LINKER_FLAGS_DEBUG
    "${CMAKE_MODULE_LINKER_FLAGS_DEBUG_INIT} ${CMAKE_MODULE_LINKER_FLAGS_MAINTAINER}"
    CACHE STRING 
    "What the hack is a module anyhow (Apple notion?)..."
    FORCE
  )
ENDIF( CMAKE_COMPILER_IS_GNUCC AND CMAKE_COMPILER_IS_GNUCXX )

############################################################################
# Set the default build mode:
#  - to Maintainer when DIET_MAINTAINER_MODE is set.
#  - by default to RelWithDebInfo [compile c++ and c code with
#    optimizations and debug info i.e.e roughly speaking "-O2 -g" on Un*x]:
IF( NOT CMAKE_BUILD_TYPE )
  IF( DIET_MAINTAINER_MODE )
    SET( CMAKE_BUILD_TYPE
      Maintainer CACHE STRING
      ${CMAKE_BUILD_TYPE_DOCSTRING}
      FORCE
    )
  ELSE( DIET_MAINTAINER_MODE )
    SET( CMAKE_BUILD_TYPE
      RelWithDebInfo CACHE STRING
      ${CMAKE_BUILD_TYPE_DOCSTRING}
      FORCE
    )
  ENDIF( DIET_MAINTAINER_MODE )
ENDIF(NOT CMAKE_BUILD_TYPE)
