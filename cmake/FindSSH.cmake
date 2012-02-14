# 
# Find SSH binary
# 
# References: for info on SSH, please refer to http://fr.wikipedia.org/wiki/Secure_Shell
#
# SSH is needed by the forwarder
#
# The following variables are set:
# SSH_FOUND         - When false, don't try to use SSH
# SSH_DIR           - (optional) Suggested installation directory to search
# SSH_BINARY        - binary of SSH
#
# SSH_DIR can be used to make it simpler to find the binary
# Just set SSH_DIR to point to your specific installation directory.
#

FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
      "FindSSH.cmake: Looking for SSH & SCP binary.\n\n" )

  # Search for ssh binary
  FIND_PROGRAM(
    SSH_BINARY 
    NAMES ssh ssh.exe
    PATHS 
    ${SSH_DIR}
    ${SSH_DIR}/bin
  )
  
    # Search for scp binary
  FIND_PROGRAM(
    SCP_BINARY 
    NAMES scp scp.exe
    PATHS 
    ${SSH_DIR}
    ${SSH_DIR}/bin
  )

SET( SSH_FOUND FALSE )
  

IF( SSH_BINARY )
  SET( SSH_FOUND TRUE )
  MARK_AS_ADVANCED ( SSH_DIR )
  MARK_AS_ADVANCED ( SSH_BINARY )
  MARK_AS_ADVANCED ( SCP_BINARY )
ENDIF( SSH_BINARY )


IF(NOT SSH_FOUND )
  MESSAGE("SSH binary was not found. Please provide SSH_DIR ")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DSSH_DIR:PATH=C:\openssh ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. SSH_DIR)")
  MESSAGE("    Providing SSH_DIR (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    SSH installation...")
  SET( SSH_DIR "" CACHE PATH "Root of SSH install directory." )
ENDIF( NOT SSH_FOUND )



