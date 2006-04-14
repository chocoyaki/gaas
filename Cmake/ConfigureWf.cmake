# Workflow support CMake configuration
# Check for Qt4 library needed for XML parsing

MESSAGE("-- Looking for QT library")
MARK_AS_ADVANCED( QT_INC )
MARK_AS_ADVANCED( QT_LIB )
SET( QT_INC "" CACHE PATH "Qt4 include directory." )
SET( QT_LIB "" CACHE PATH "Qt4 lib directory." )

MESSAGE("-- Looking for QDomDocument header")
FIND_PATH( QT_INCLUDE_DIR Qt/QDomDocument
  PATHS
  ${QT_INC}
  /usr/include
  /usr/local/include
)

IF(QT_INCLUDE_DIR)
  MESSAGE("-- Looking for QString header")
  FIND_PATH( QT_INCLUDE_DIR Qt/QString
    PATHS
    ${QT_INC}
    /usr/include
    /usr/local/include
  )
ENDIF(QT_INCLUDE_DIR)

IF (NOT QT_INCLUDE_DIR)
  MESSAGE("-- QDomDocument or QString was not found")
ENDIF (NOT QT_INCLUDE_DIR)

MESSAGE("-- Looking for QtXml library")
FIND_LIBRARY (QTXML_LIBRARY QtXml 
	PATHS 
	${QT_LIB}/
	/usr/lib)

IF( QT_INCLUDE_DIR )
IF( QTXML_LIBRARY )
  SET (QTXML_FOUND "YES")
ENDIF( QTXML_LIBRARY )
ENDIF( QT_INCLUDE_DIR )


IF( NOT QTXML_FOUND )
  MESSAGE("QtXml library was not found. Please provide QT_LIB and QT_INC:")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DQT_LIB:PATH=/usr/lib/qt4 ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. QT_LIB)")
  MESSAGE("    Providing QTXML_LIB (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    Qt4 installation...")
  SET( QT_LIB "" CACHE PATH "Qt4 instal tree." )
ENDIF( NOT QTXML_FOUND )
