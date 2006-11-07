# Workflow support CMake configuration
# Check for Xerces C++ library needed for XML parsing


	
#IF (XERCES_HOME)
#   SET (XERCES_INCLUDE_DIR ${XERCES_HOME}/include)
#   SET (XERCES_LIB ${XERCES_HOME}/lib)
#ELSE (XERCES_HOME)
#ENDIF (XERCES_HOME)

FOREACH ( file xercesc/util/PlatformUtils.hpp xercesc/parsers/AbstractDOMParser.hpp xercesc/dom/DOMImplementation.hpp xercesc/dom/DOMImplementationLS.hpp xercesc/dom/DOMImplementationRegistry.hpp xercesc/dom/DOMBuilder.hpp xercesc/dom/DOMException.hpp xercesc/dom/DOMDocument.hpp xercesc/dom/DOMNodeList.hpp xercesc/dom/DOMError.hpp xercesc/dom/DOMLocator.hpp xercesc/dom/DOMNamedNodeMap.hpp xercesc/dom/DOMElement.hpp xercesc/dom/DOMAttr.hpp )

   FIND_PATH( XERCES_INCLUDE_DIR ${file}
  		PATHS
		${XERCES_HOME}/include
  		/usr/include
  		/usr/local/include
   )

#   IF (NOT XERCES_INCLUDE_DIR)
#  	MESSAGE(SEND_ERROR ${file} "was not found")
#   ENDIF(NOT XERCES_INCLUDE_DIR)
ENDFOREACH (file)

#MESSAGE("-- Looking for Xerces C++ library")

FIND_LIBRARY (XERCES_LIBRARY xerces-c
	PATHS 
	${XERCES_HOME}/lib
	/usr/lib)

IF( XERCES_INCLUDE_DIR )
IF( XERCES_LIBRARY )
  SET (XERCES_FOUND "YES")
  MARK_AS_ADVANCED ( XERCES_HOME )
#  MARK_AS_ADVANCED ( XERCES_INCLUDE_DIR )
ENDIF( XERCES_LIBRARY )
ENDIF( XERCES_INCLUDE_DIR )


IF(NOT XERCES_FOUND )
  MESSAGE("Xerces C++ library was not found. Please provide XERCES_HOME ")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DXERCES_HOME:PATH=/usr/local/xerces ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. XERCES_HOME)")
  MESSAGE("    Providing XERCES_HOME (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    Xerces installation...")
  SET( XERCES_HOME "" CACHE PATH "Root of Xerces install directory." )
ENDIF( NOT XERCES_FOUND )
