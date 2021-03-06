# When testing diet with ctest we use various commands at the shell level
# e.g. for testing network availability. Note that this process is not
# portable accross WIndoze... (fart happens).
#
# The following variables are set:
# DIET_CTEST_NETWORK_FOUND : set to YES if and only if all the commands
#    required for the networked part of the testing are present. NO otherwise.
# DIET_CTEST_HOSTNAME_COMMAND        <--- hostname
# DIET_CTEST_DNSDOMAINNAME_COMMAND   <--- domainname
# DIET_CTEST_PIDOF_COMMAND           <--- pidof
# DIET_CTEST_MV_COMMAND              <--- mv
# DIET_CTEST_RM_COMMAND              <--- rm
# DIET_CTEST_DATE_COMMAND            <--- date
# DIET_CTEST_NOHUP_COMMAND           <--- nohup

# FIXME: use the IF( COMMAND cmd ) way of things

FIND_PROGRAM( DIET_CTEST_HOSTNAME_COMMAND
  NAMES hostname
  DOC "What is the path to the hostname command."
)

FIND_PROGRAM( DIET_CTEST_DNSDOMAINNAME_COMMAND
  NAMES dnsdomainname
  DOC "What is the path to the dnsdomainname command."
)

FIND_PROGRAM( DIET_CTEST_PIDOF_COMMAND
  NAMES pidof
  PATHS /sbin
  DOC "What is the path to the pidof command."
)

FIND_PROGRAM( DIET_CTEST_MV_COMMAND
  NAMES mv
  PATHS /sbin
  DOC "What is the path to the mv command."
)

FIND_PROGRAM( DIET_CTEST_RM_COMMAND
  NAMES rm
  PATHS /sbin
  DOC "What is the path to the rm command."
)

FIND_PROGRAM( DIET_CTEST_DATE_COMMAND
  NAMES date
  PATHS /sbin
  DOC "What is the path to the date command."
)

FIND_PROGRAM( DIET_CTEST_NOHUP_COMMAND
  NAMES nohup
  PATHS /sbin
  DOC "What is the path to the nohup command."
)

SET( DIET_CTEST_NETWORK_FOUND "NO" )

IF(     DIET_CTEST_HOSTNAME_COMMAND 
    AND DIET_CTEST_DNSDOMAINNAME_COMMAND
    AND DIET_CTEST_PIDOF_COMMAND
    AND DIET_CTEST_MV_COMMAND
    AND DIET_CTEST_RM_COMMAND
    AND DIET_CTEST_DATE_COMMAND
    AND DIET_CTEST_NOHUP_COMMAND
  )
  SET( DIET_CTEST_NETWORK_FOUND "YES" )
  SET( DIET_CTEST_OMNIORB_MAPPER_PORT      "20809" )
  SET( DIET_CTEST_OMNIORB_NAMESERVICE_PORT "20810" )
  EXECUTE_PROCESS(COMMAND
    ${DIET_CTEST_HOSTNAME_COMMAND}
    OUTPUT_VARIABLE DIET_CMAKE_HOSTNAME
  )
  STRING(STRIP "${DIET_CMAKE_HOSTNAME}" DIET_CMAKE_HOSTNAME)
  EXECUTE_PROCESS(COMMAND
    ${DIET_CTEST_DNSDOMAINNAME_COMMAND}
    OUTPUT_VARIABLE DIET_CMAKE_DNSDOMAINNAME
  )
  STRING(STRIP "${DIET_CMAKE_DNSDOMAINNAME}" DIET_CMAKE_DNSDOMAINNAME)
  SET( DIET_CTEST_FQDN_HOSTNAME
    "${DIET_CMAKE_HOSTNAME}"
  )
  MARK_AS_ADVANCED( DIET_CTEST_NETWORK_FOUND )
  MARK_AS_ADVANCED( DIET_CTEST_HOSTNAME_COMMAND )
  MARK_AS_ADVANCED( DIET_CTEST_DNSDOMAINNAME_COMMAND )
  MARK_AS_ADVANCED( DIET_CTEST_PIDOF_COMMAND )
  MARK_AS_ADVANCED( DIET_CTEST_MV_COMMAND )
  MARK_AS_ADVANCED( DIET_CTEST_RM_COMMAND )
  MARK_AS_ADVANCED( DIET_CTEST_DATE_COMMAND )
  MARK_AS_ADVANCED( DIET_CTEST_NOHUP_COMMAND )
ENDIF ( DIET_CTEST_HOSTNAME_COMMAND 
    AND DIET_CTEST_DNSDOMAINNAME_COMMAND
    AND DIET_CTEST_PIDOF_COMMAND
    AND DIET_CTEST_MV_COMMAND
    AND DIET_CTEST_RM_COMMAND
    AND DIET_CTEST_DATE_COMMAND
    AND DIET_CTEST_NOHUP_COMMAND )
