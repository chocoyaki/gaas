# When testing diet with use various utilities

# DIET_CMAKE_HOSTNAME
# DIET_CMAKE_FQDN_HOSTNAME

# FIXME: use the IF( COMMAND cmd ) way of things

FIND_PROGRAM( DIET_HOSTNAME_COMMAND
  NAMES hostname
  DOC "What is the path to the hostname command."
)

FIND_PROGRAM( DIET_DNSDOMAINNAME_COMMAND
  NAMES dnsdomainname
  DOC "What is the path to the dnsdomainname command."
)

FIND_PROGRAM( DIET_PIDOF_COMMAND
  NAMES pidof
  PATHS /sbin
  DOC "What is the path to the pidof command."
)

SET( DIET_CMAKE_NETWORK_FOUND "NO" )

IF ( DIET_HOSTNAME_COMMAND )
IF ( DIET_DNSDOMAINNAME_COMMAND )
IF ( DIET_PIDOF_COMMAND )
  SET( DIET_CMAKE_NETWORK_FOUND "YES" )
  EXEC_PROGRAM(
    ${DIET_HOSTNAME_COMMAND}
    OUTPUT_VARIABLE DIET_CMAKE_HOSTNAME
  )
  EXEC_PROGRAM(
    ${DIET_DNSDOMAINNAME_COMMAND}
    OUTPUT_VARIABLE DIET_CMAKE_DNSDOMAINNAME
  )
  SET( DIET_CMAKE_FQDN_HOSTNAME
    "${DIET_CMAKE_HOSTNAME}.${DIET_CMAKE_DNSDOMAINNAME}"
  )
  MARK_AS_ADVANCED( DIET_CMAKE_NETWORK_FOUND )
  MARK_AS_ADVANCED( DIET_HOSTNAME_COMMAND )
  MARK_AS_ADVANCED( DIET_DNSDOMAINNAME_COMMAND )
  MARK_AS_ADVANCED( DIET_PIDOF_COMMAND )
ENDIF ( DIET_PIDOF_COMMAND )
ENDIF ( DIET_DNSDOMAINNAME_COMMAND )
ENDIF ( DIET_HOSTNAME_COMMAND )