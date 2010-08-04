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

ADD_SUBDIRECTORY(utils)
ADD_SUBDIRECTORY(CORBA)
ADD_SUBDIRECTORY(utils/nodes)
ADD_SUBDIRECTORY(client)
IF (DIET_USE_WORKFLOW)
  ADD_SUBDIRECTORY(utils/workflow)
ENDIF (DIET_USE_WORKFLOW)

ADD_SUBDIRECTORY(agent)
ADD_SUBDIRECTORY(SeD)
ADD_SUBDIRECTORY(Testing)


IF( DIET_BUILD_EXAMPLES )
  ADD_SUBDIRECTORY( examples )
ENDIF( DIET_BUILD_EXAMPLES )
