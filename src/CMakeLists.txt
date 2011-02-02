#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

# This macro extracts the complete path to the generated header file
# containing the name passed as arg2 and stores it into arg1.
macro(corba_header arg1 arg2)
  foreach(hdr ${IDL_GENERATED_HEADERS})
    string(REGEX MATCH ".*/${arg2}.hh" result ${hdr})
    if (NOT ${result} EQUAL "")
      set(${arg1} ${result})
    endif (NOT ${result} EQUAL "")
  endforeach(hdr)
endmacro(corba_header)

# This macro extracts the complete paths to the generated header files
# containing the names passed as arguments and stores them into arg1
macro(corba_headers arg1)
  foreach (arg ${ARGV})
    corba_header(tmp ${arg})
    set(res ${res} ${tmp})
  endforeach(arg)
  set(${arg1} ${res})
endmacro(corba_headers)

add_subdirectory(CORBA/idl)

add_subdirectory(utils)
add_subdirectory(CORBA)
add_subdirectory(utils/nodes)
add_subdirectory(client)
if (DIET_USE_WORKFLOW)
  add_subdirectory(utils/workflow)
endif (DIET_USE_WORKFLOW)

add_subdirectory(agent)
add_subdirectory(SeD)
add_subdirectory(Testing)

if( DIET_BUILD_EXAMPLES )
  add_subdirectory( examples )
endif( DIET_BUILD_EXAMPLES )
