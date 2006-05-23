#
# Ssh server to submit results (used by client)
#
SET( DROP_METHOD "scp" )
SET( DROP_SITE "localhost" )
SET( DROP_LOCATION "ssh-incoming" )
SET( DROP_SITE_USER "Dart" )
FIND_PROGRAM(
  SCPCOMMAND scp
  DOC "Path to scp command, sometimes used for submitting Dart results."
)

#
# Dart server to submit results (used by client)
#
SET (TRIGGER_SITE "http://${DROP_SITE}/Dart/TriggerDart.cgi")

# Project Home Page
SET( PROJECT_URL "http://graal.ens-lyon.fr/DIET/" )

#
# Dart server configuration
#
SET( CVS_WEB_URL "http://${DROP_SITE}/cgi-bin/cvsweb.cgi/dt/" )
SET( CVS_WEB_CVSROOT "dt" )

OPTION( BUILD_DOXYGEN "Build source documentation using doxygen" "On")
SET (DOXYGEN_URL "http://${DROP_SITE}/Insight/Doxygen/html/" )
SET (DOXYGEN_CONFIG "${PROJECT_BINARY_DIR}/doc/Doxyfile" )

# Unused (up to now):
#SET (GNATS_WEB_URL "http://${DROP_SITE}/")

#
# Copy over the testing logo (just to test things around)
#
CONFIGURE_FILE(
  ${DIET_SOURCE_DIR}/doc/ProgrammersGuide/fig/logo_DIET.gif
  ${DIET_BINARY_DIR}/Testing/HTML/TestingResults/Icons/logo_DIET.gif
  COPYONLY 
)
