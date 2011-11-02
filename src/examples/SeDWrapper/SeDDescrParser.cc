/**
* @file SeDDescrParser.cc
* 
* @brief   Parser of XML files used to convert external service into SeD  
* 
* @author  - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr) 
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.1  2010/04/06 15:02:37  bdepardo
 * Added SeDWrapper example. This example is compiled when workflows are activated.
 *
 */

#include <iostream>
#include <libgen.h>

#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMLSParser.hpp>

#include "DIET_data.h"
#include "DIET_server.h"

#include "SeDDescrParser.hh"
#include "DagWfParser.hh" // for XMLParsingException

#ifndef XTOC
#define XTOC(x) XMLString::transcode(x) // Use iff x is a XMLCh *
#define CTOX(x) XMLString::transcode(x) // Use iff x is a char *
#define XREL(x) XMLString::release(&x)
#endif


/*****************************************************************************/
/*                          CLASS SeDDescrParser                             */
/*****************************************************************************/

SeDDescrParser::SeDDescrParser(const string& fileName)
  : myXmlFileName(fileName), document(NULL) {
}

SeDDescrParser::~SeDDescrParser() {
}

void
SeDDescrParser::parseXml(bool checkValid)
{
  const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

  XMLPlatformUtils::Initialize();
  DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
  DOMLSParser *parser = impl->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
  MyDOMErrorHandler* errHandler = new MyDOMErrorHandler();

  // Validation
  if (parser->getDomConfig()->canSetParameter(XMLUni::fgDOMValidateIfSchema, true)) {
    parser->getDomConfig()->setParameter(XMLUni::fgDOMValidateIfSchema, true);
    parser->getDomConfig()->setParameter(XMLUni::fgXercesValidationErrorAsFatal, true);
  }

  // Error handler
  if (parser->getDomConfig()->canSetParameter(XMLUni::fgDOMErrorHandler, errHandler)) {
    parser->getDomConfig()->setParameter(XMLUni::fgDOMErrorHandler, errHandler);
  }

  // Wrapper
  Wrapper4InputSource * wrapper;
  string errorMsgPfx;

  if (!myXmlFileName.empty()) {
    // INITIALIZE FROM FILE
    errorMsgPfx = "In file " + myXmlFileName + " : ";
    XMLCh* xmlFileName = CTOX(myXmlFileName.c_str());
    LocalFileInputSource * fileBufIS = new LocalFileInputSource(xmlFileName);
    wrapper = new Wrapper4InputSource(fileBufIS);
    //     XREL(xmlFileName);

  } else {
    throw XMLParsingException(XMLParsingException::eUNKNOWN, "Empty XML filename");
  }

  // PARSE
  try {
    this->document = parser->parse((DOMLSInput*) wrapper);
  } catch (...) {
    cerr << errorMsgPfx << "Unexpected exception during XML Parsing";
    throw XMLParsingException(XMLParsingException::eUNKNOWN, "");
  }

  if (document == NULL)
    throw XMLParsingException(XMLParsingException::eFILENOTFOUND,
                              myXmlFileName);

  // Check if DTD was provided
  if (checkValid && !document->getDoctype()) {
    cerr << errorMsgPfx << "XML is not validated (no DTD provided)" << endl
         << "Use <!DOCTYPE workflow SYSTEM \"[DIET_INSTALL_DIR]/etc/FWorkflow.dtd\">"
         << " instruction to provide it";
  }

  DOMNode * root = (DOMNode*)(document->getDocumentElement());
  if (root == NULL)
    throw XMLParsingException(XMLParsingException::eUNKNOWN,
                              "No details available");

  // Parse the root element
  parseRoot(root);

  delete errHandler;
  delete parser;
  XMLPlatformUtils::Terminate();
}

const list<SeDService*>&
SeDDescrParser::getServices()
{
  return myServiceList;
}

/*****************************************************************************/
/*                            CLASS GASWParser                               */
/*****************************************************************************/

GASWParser::GASWParser(const string& fileName)
  : SeDDescrParser(fileName) {
}


string
GASWParser::parseValueURI(const DOMElement *element) {
  string context = DagWfParser::getAttributeValue("context", element);
  string value;
  if (context.empty() || (context == "G5K")) {
    value = DagWfParser::getAttributeValue("value", element);
    string::size_type prefix = value.find("file://");
    if (prefix != string::npos) {
      value = value.substr(7);
    } else {
      cerr << "Invalid URI: " << value << " (must begin with file://)" << endl;
    }
  } // else return empty string
  return value;
}

void
GASWParser::parseRoot(DOMNode* root)
{
  // Check root element
  char * _rootNodeName = XTOC(root->getNodeName());
  if (strcmp (_rootNodeName, "description")) {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "XML for GAS should begin with <description>");
  }
  XREL(_rootNodeName);
  DOMNode * child = root->getFirstChild();
  while ((child != NULL)) {
    // Parse all the <executable> elements
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      const DOMElement * child_elt = (DOMElement*)child;
      char * _childName = XTOC(child_elt->getNodeName());
      string childName(_childName);
      XREL(_childName);

      if (childName == "executable") {

        parseExecutable(child_elt);

      }
      //       } else
      //      throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
      //                "Invalid tag within description element");

    }
    child = child->getNextSibling();
  }
}

void
GASWParser::parseExecutable(const DOMElement * element)
{
  string name  = DagWfParser::getAttributeValue("name", element);
  SeDService*  service = new SeDService(this, name);
  service->setExecutableName(name);  // use this by default for script name
  myServiceList.push_back(service);

  const DOMNode * child = element->getFirstChild();
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      const DOMElement * child_elt = (DOMElement*) child;
      char *_childName = XTOC(child_elt->getNodeName());
      string childName(_childName);
      XREL(_childName);

      if (childName == "value") {
        string execName = parseValueURI(child_elt);
        if (!execName.empty()) service->setExecutableName(execName);

      } else if (childName == "input") {
        parseInput(child_elt, service);

      } else if (childName == "output") {
        parseOutput(child_elt, service);

      } else if (childName == "sandbox") {
        parseDependency(child_elt, service);

      }
      //       } else
      //         throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
      //                "Invalid tag within executable element");
    }
    child = child->getNextSibling();
  } // end while
}

void
GASWParser::parseInput(const DOMElement * element, SeDService * service)
{
  string name    = DagWfParser::getAttributeValue("name", element);
  string option  = DagWfParser::getAttributeValue("option", element);
  string type    = DagWfParser::getAttributeValue("type", element);
  //   if (type.empty()) throw XMLParsingException(XMLParsingException::eEMPTY_ATTR, "type");
  SeDArgument *in = service->addInput(name, option, type);
}

void
GASWParser::parseOutput(const DOMElement * element, SeDService * service)
{
  string name    = DagWfParser::getAttributeValue("name", element);
  string option  = DagWfParser::getAttributeValue("option", element);
  string type    = DagWfParser::getAttributeValue("type", element);
  //   if (type.empty()) throw XMLParsingException(XMLParsingException::eEMPTY_ATTR, "type");
  SeDArgument *out = service->addOutput(name, option, type);

  const DOMNode * child = element->getFirstChild();
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      const DOMElement * child_elt = (DOMElement*) child;
      char *_childName = XTOC(child_elt->getNodeName());
      string childName(_childName);
      XREL(_childName);

      if (childName == "value") {
        string value    = parseValueURI(child_elt);
        if (!value.empty())  out->setTemplate(value);
      }
    }
    child = child->getNextSibling();
  }
}

void
GASWParser::parseDependency(const DOMElement * element, SeDService * service)
{
  string name    = DagWfParser::getAttributeValue("name", element);
  const DOMNode * child = element->getFirstChild();
  string accessType;
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      const DOMElement * child_elt = (DOMElement*) child;
      char *_childName = XTOC(child_elt->getNodeName());
      string childName(_childName);
      XREL(_childName);

      if (childName == "value") {
        string localPath = parseValueURI(child_elt);
        if (!localPath.empty())
          service->addDependency(name, localPath);
      }
    }
    child = child->getNextSibling();
  }
}


void
GASWParser::evalTemplate(SeDArgument *arg, string& value) {
  string::size_type dirPos = 0;
  string::size_type naPos = 0;
  const string& argTempl = arg->getTemplate();
  const list<SeDArgument*>&  args = arg->getService()->getArgs();
  unsigned int argIndex = 1;  // references start at 1

  // Initialize value with template content
  value = argTempl;

  // Loop for all arguments that are IN && FILE
  for (list<SeDArgument*>::const_iterator argIter = args.begin();
       argIter != args.end();
       ++argIter)
  {
    SeDArgument *currArg = (SeDArgument *) *argIter;
    if (currArg->getIo() == SeDArgument::IN)
    {
      const string& refValue = currArg->getValue();
      string dirRef;
      string naRef;

      if (currArg->getType() == SeDArgument::URI) {
        // Get the directory and base name of the file
        char * pathC1 = strdup(refValue.c_str());
        char * pathC2 = strdup(refValue.c_str());
        dirRef = dirname(pathC1);
        naRef = basename(pathC2);
        free(pathC1);
        free(pathC2);
      } else if (currArg->getType() == SeDArgument::DIR) {
        char * pathC1 = strdup(refValue.c_str());
        dirRef = refValue;
        naRef = basename(pathC1);
      } else {  // argument is scalar
        dirRef = "";
        naRef = refValue;
      }
      // Set templates for directory and base name
      string dirTempl = "$dir" + itoa(argIndex);
      string naTempl = "$na" + itoa(argIndex);
      // Replace $dirX (once)
      if ((dirPos = value.find(dirTempl)) != string::npos) {
        value = value.substr(0, dirPos)
          + dirRef
          + value.substr(dirPos + dirTempl.length(), value.length()-dirPos-dirTempl.length());
      }
      // Replace $naX (once)
      if ((naPos = value.find(naTempl)) != string::npos) {
        value = value.substr(0, naPos)
          + naRef
          + value.substr(naPos + naTempl.length(), value.length()-naPos-naTempl.length());
      }
    }
    ++argIndex;
  }

  // Replace %s by unique id provided by SeDService
  string::size_type idPos = 0;
  if ((idPos = value.find("%s")) != string::npos) {
    value = value.substr(0, idPos)
      + arg->getService()->getReqId()
      + value.substr(idPos + 2, value.length()-idPos-2);
  }

}
