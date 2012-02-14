#
# Finds the gSOAP package installation or build tree
#
# Produces
#   GSOAP_FOUND         - boolean variable to tell if the package was found
#   GSOAP_LIBRARIES     - path of the gsoapssl++ library
#   GSOAP_INCLUDE_DIRS  - include directory of the gSOAP installation
#


find_library(GSOAP_LIBRARY NAMES gsoapssl
  PATHS ${GSOAP_SRC_DIR}/gsoap)
find_path(GSOAP_INCLUDE_DIR stdsoap2.h
  PATHS ${GSOAP_SRC_DIR}/gsoap)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GSOAP DEFAULT_MSG GSOAP_LIBRARY GSOAP_INCLUDE_DIR)

if(GSOAP_FOUND)
  set(GSOAP_LIBRARIES ${GSOAP_LIBRARY})
  set(GSOAP_INCLUDE_DIRS ${GSOAP_INCLUDE_DIR})
  mark_as_advanced(GSOAP_SRC_DIR)
  mark_as_advanced(GSOAP_LIBRARY)
  mark_as_advanced(GSOAP_LIBRARIES)
  mark_as_advanced(GSOAP_INCLUDE_DIR)
  mark_as_advanced(GSOAP_INCLUDE_DIRS)
endif(GSOAP_FOUND)

