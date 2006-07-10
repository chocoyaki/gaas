# Workflow support CMake configuration
# Check for Xerces C++ library needed for XML parsing

MESSAGE("-- Looking for Xerces library")

MARK_AS_ADVANCED( XERCES_HOME )

#SET( XERCES_INC "" CACHE PATH "Xerces C++ include directory." )
#SET( XERCES_LIB "" CACHE PATH "Xerces C++ lib directory." )
	
IF (XERCES_HOME)
   SET (XERCES_INC ${XERCES_HOME}/include)
   SET (XERCES_LIB ${XERCES_HOME}/lib)
ENDIF (XERCES_HOME)

MESSAGE ( "XERCES_INC = " ${XERCES_INC} )
MESSAGE ( "XERCES_LIB = " ${XERCES_LIB} )

MESSAGE("-- Looking for Xerces C++ header")

FOREACH ( file xercesc/util/PlatformUtils.hpp xercesc/parsers/AbstractDOMParser.hpp xercesc/dom/DOMImplementation.hpp xercesc/dom/DOMImplementationLS.hpp xercesc/dom/DOMImplementationRegistry.hpp xercesc/dom/DOMBuilder.hpp xercesc/dom/DOMException.hpp xercesc/dom/DOMDocument.hpp xercesc/dom/DOMNodeList.hpp xercesc/dom/DOMError.hpp xercesc/dom/DOMLocator.hpp xercesc/dom/DOMNamedNodeMap.hpp xercesc/dom/DOMElement.hpp xercesc/dom/DOMAttr.hpp )

   FIND_PATH( XERCES_INCLUDE_DIR ${file}
  		PATHS
		${XERCES_HOME}/include
		${XERCES_INC}
  		/usr/include
  		/usr/local/include
   )

   IF (NOT XERCES_INCLUDE_DIR)
  	MESSAGE(SEND_ERROR ${file} "was not found")
   ENDIF(NOT XERCES_INCLUDE_DIR)
ENDFOREACH (file)

MESSAGE("-- Looking for Xerces C++ library")
FIND_LIBRARY (XERCES_LIBRARY xerces-c
	PATHS 
	${XERCES_HOME}/lib
	${XERCES_LIB}/
	/usr/lib)

IF( XERCES_INCLUDE_DIR )
IF( XERCES_LIBRARY )
  SET (XERCES_FOUND "YES")
ENDIF( XERCES_LIBRARY )
ENDIF( XERCES_INCLUDE_DIR )


IF(NOT XERCES_FOUND )
  MESSAGE("Xerces C++ library was not found. Please provide XERCES_HOME or XERCES_LIB and XERCES_INC:")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DXERCES_LIB:PATH=/usr/local/xerces/lib ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. XERCES_LIB)")
  MESSAGE("    Providing XERCES_LIB (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    Xerces installation...")
  SET( XERCES_LIB "" CACHE PATH "Xerces instal tree." )
ENDIF( NOT XERCES_FOUND )

MESSAGE ("XERCES_LIBRARY = " ${XERCES_LIBRARY})
