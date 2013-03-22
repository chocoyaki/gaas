/**
 * @file SeDDescrParser.hh
 *
 * @brief   Parser of XML files used to convert external service into SeD
 *
 * @author   Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _SEDDESCRPARSER_HH_
#define _SEDDESCRPARSER_HH_

// Standard library headers
#include <list>
#include <string>

// Xerces headers
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMAttr.hpp>

// Diet headers
#include "WfUtils.hh"
#include "SeDService.hh"

XERCES_CPP_NAMESPACE_USE


/*****************************************************************************/
/*                          CLASS SeDDescrParser                             */
/*****************************************************************************/


class SeDDescrParser {
public:
  explicit
  SeDDescrParser(const std::string &fileName);

  virtual ~SeDDescrParser();

  void
  parseXml(bool checkValid = false);

  virtual void
  evalTemplate(SeDArgument *arg, string &value) = 0;

  const list<SeDService *> &
  getServices();

protected:
  virtual void
  parseRoot(DOMNode *root) = 0;

  /**
   * XML File name
   */
  std::string myXmlFileName;

  /**
   * Xml document
   */
  DOMDocument *document;

  /**
   * Generated objects storage
   */
  std::list<SeDService *>  myServiceList;
};


/*****************************************************************************/
/*                            CLASS GASWParser                               */
/*****************************************************************************/

class GASWParser : public SeDDescrParser {
public:
  explicit
  GASWParser(const std::string &fileName);

  virtual void
  evalTemplate(SeDArgument *arg, std::string &value);

protected:
  virtual std::string
  parseValueURI(const DOMElement *element);

  virtual void
  parseRoot(DOMNode *root);

  virtual void
  parseExecutable(const DOMElement *element);

  virtual void
  parseInput(const DOMElement *element, SeDService *service);

  virtual void
  parseOutput(const DOMElement *element, SeDService *service);

  virtual void
  parseDependency(const DOMElement *element, SeDService *service);
};

#endif /* ifndef _SEDDESCRPARSER_HH_ */
