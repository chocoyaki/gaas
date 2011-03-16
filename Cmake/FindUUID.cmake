#
# Find UUID library for DAGDA
# If your UUID libs are not found, try to install libuuid and libuuid-devel.
# Or try to compile libuuid with -fPIC option, use this command line:
# $ CFLAGS="-fPIC" ./configure --prefix=<install-dir>
#   --disable-testio-debug --disable-libblkid --disable-debugfs --disable-imager
#   --disable-resizer --enable-libuuid --disable-nls
#   --disable-e2initrd-helper --enable-elf-shlibs

#

# Standard UNIX system libraries directories
set(SYS_LIB_DIRS
  /lib
  /usr/lib
  /usr/local/lib
  /opt/lib
  /opt/local/lib
  ${CMAKE_SYSTEM_LIBRARY_PATH}
  )
# Standard UNIX system include directories
set(SYS_INCLUDE_DIRS
  /include
  /usr/include
  /usr/local/include
  /opt/include
  /opt/local/include
  ${CMAKE_SYSTEM_INCLUDE_PATH}
  )

# Specific paths for Mac OS X
if (APPLE)
  set(SYS_LIB_DIRS
    ${SYS_LIB_DIRS}
    /Developer/usr/lib
    )
  set(SYS_INCLUDE_DIRS
    ${SYS_INCLUDE_DIRS}
    /Developer/usr/include
    )
endif (APPLE)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  # Add the 64 bits system libraries directories
  set(SYS_LIB_DIRS
    /lib64
    /usr/lib64
    /usr/local/lib64
    /opt/lib64
    /opt/local/lib64
    ${SYS_LIB_DIRS}
    )
endif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
set(SYS_LIB_DIRS
  ${UUID_PATH}/lib
  ${SYS_LIB_DIRS}
  )
set(SYS_INCLUDE_DIRS
  ${UUID_PATH}/include
  ${SYS_INCLUDE_DIRS}
  )

# Advance data ID depending on the uuid lib.
foreach (dir ${SYS_INCLUDE_DIRS})
  set(UUID_DIRS ${UUID_DIRS} ${dir}/uuid ${dir})
endforeach (dir)

# Search for uuid.h
find_path(UUID_INCLUDE_DIR
  NAMES uuid.h
  PATHS ${UUID_DIRS}
  NO_DEFAULT_PATH
  )

if (UUID_INCLUDE_DIR)
  # This library is not needed on Mac OS X.
  if (NOT APPLE)
    find_library(UUID_LIB
      NAMES
      ${CMAKE_SHARED_LIBRARY_PREFIX}uuid${CMAKE_SHARED_LIBRARY_SUFFIX}
      PATHS ${SYS_LIB_DIRS}
      NO_DEFAULT_PATH
      )
    if (UUID_LIB)
      set(UUID_FOUND TRUE)
      add_definitions(-DHAVE_ADVANCED_UUID)
    endif (UUID_LIB)
  else (NOT APPLE)
    set(UUID_FOUND TRUE)
    add_definitions(-DHAVE_ADVANCED_UUID)
  endif (NOT APPLE)
else (UUID_INCLUDE_DIR)
  set(UUID_FOUND FALSE)
endif (UUID_INCLUDE_DIR)

if (UUID_FOUND)
  if (NOT APPLE)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
      set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Cmake)
      configure_file(
        ${SRC_DIR}/libuuid_test.cc.in
	${CMAKE_BINARY_DIR}/Cmake/libuuid_test.cc
	)
      try_compile(
	VALID_LIB
	${CMAKE_BINARY_DIR}/Cmake
	${CMAKE_BINARY_DIR}/Cmake/libuuid_test.cc
	CMAKE_FLAGS -DCXXFLAGS:string="-shared -fPIC"
	-DINCLUDE_DIRECTORIES:string="${UUID_INCLUDE_DIR}"
	-DLIBRARY_DIRECTORIES:string="${SYS_LIB_DIRS}"
	-DLINK_LIBRARIES:string="uuid"
	OUTPUT_VARIABLE err
	)
      if (NOT VALID_LIB)
	message(WARNING "Library ${UUID_LIB} cannot be used for DIET compilation.
Try to recompile this library using -fPIC C flag or set UUID_PATH to a valid library path.
(You can find libuuid source code here: http://e2fsprogs.sourceforge.net)")
	set(UUID_FOUND FALSE)
	unset(UUID_LIB CACHE)
      endif (NOT VALID_LIB)
    endif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  endif (NOT APPLE)
endif (UUID_FOUND)
