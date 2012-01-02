set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# set documentation install dir
set(DOC_INSTALL_DIR "share/doc/${PROJECT_NAME}"
  CACHE PATH "Directory where doc will be installed")

# set man pages install dir
set(MAN_INSTALL_DIR "share/man"
  CACHE PATH "Directory where man pages will be installed")
# create doc target if non-existant
add_custom_target(doc ALL)

# cleanup
set_property(DIRECTORY APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES doc)

##############################################################################
# Options
##############################################################################
option(DIET_BUILD_USERMANUAL "Build DIET User Manual." ON)
option(DIET_BUILD_USERFAQ "Build DIET User FAQ." ON)
option(DIET_BUILD_PROGRAMMERGUIDE "Build DIET Programmer Guide." OFF)
mark_as_advanced(DIET_BUILD_PROGRAMMERGUIDE)
option(DIET_BUILD_MANPAGE "Build DIET Manpages." ON)
option(DIET_BUILD_DOXYGEN "Build DIET Doxygen." OFF)

##############################################################################
# Doxygen is required for building API doc
# no Doxygen, no API doc
#
##############################################################################
if(DIET_BUILD_DOXYGEN)
  include(UseDoxygen)
  generate_doxygen(FORMATS ${DOXYGEN_FORMATS})
endif()


#############################################################################
# Guides require latex, transfig and ImageMagick to be installed
#
#############################################################################
find_package(LATEX)
find_package(Transfig)
find_package(ImageMagick)

set(ENABLE_GUIDE ON)
if(NOT PDFLATEX_COMPILER)
  message(WARNING " Latex installation was not found.")
  set(ENABLE_GUIDE OFF)
endif()

if(NOT TRANSFIG_FOUND)
  message(WARNING " Transfig installation was not found. Please provide FIG2DEV.")
  set(ENABLE_GUIDE OFF)
endif()

if(NOT IMAGEMAGICK_CONVERT_EXECUTABLE)
  message("Convert ImageMagick utility was not found.")
  message("Please provide IMAGEMAGICK_CONVERT_EXECUTABLE.")
  set(ENABLE_GUIDE OFF)
endif()

if(ENABLE_GUIDE)
  include(cmake/UseLATEX.cmake)
  if(DIET_BUILD_PROGRAMMERGUIDE)
    add_subdirectory(ProgrammersGuide)
  endif()

  if(DIET_BUILD_USERMANUAL)
    # UsersManual correctly built but cmake may raise an error
    # due to latex number of warnings
    add_subdirectory(UsersManual)
  endif()

  if(DIET_BUILD_USERFAQ)
    add_subdirectory(UsersFAQ)
  endif()
else()
  message(WARNING " Guides will not be built due to missing requirements.")
endif()


#############################################################################
# man pages generation
# we need rst2man for the generation
#
#############################################################################
if(DIET_BUILD_MANPAGE)
  find_program(RST2MAN_BIN rst2man)
  if(RST2MAN_BIN)
    add_subdirectory(man)
  else()
    message(WARNING " Man pages will not be built due to missing requirements.")
    message(WARNING " Please install rst2man (usually included in python-docutils package)")
  endif()
endif()
