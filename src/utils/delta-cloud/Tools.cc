/*
Authors :
- Lamiel Toch : lamiel.toch@ens-lyon.fr
*/

#include "Tools.hh"
#include "Iaas_deltacloud.hh"
#include "SeDCloud.hh"
#include "ServiceWrapper.hh"
#include "DIET_server.h"
#include "DIET_uuid.hh"
#include "DagdaFactory.hh"

#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <libgen.h>


#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>

int read_properties_file(const std::string& path, std::map<std::string, std::string>& results) {
  std::ifstream file(path.c_str());
  std::string line;

  while(std::getline(file, line))
  {
    std::vector<std::string> tokens;

    if(line.length() == 0 || line.find("//") == 0)
      continue;

    boost::split(tokens, line, boost::is_any_of("="));

    if(tokens.size() == 2)
      results[tokens[0]] = tokens[1];
  }
  return 0;
}

char* cpp_strdup(const char* src) {
  char* cpy = new char [strlen(src) + 1];
  strcpy(cpy, src);
  return cpy;
}







char* readline(const char* path, int index) {
  int i = 0;
  bool end = false;
  bool found = false;
  std::fstream file;
  file.open(path, std::ios_base::in);

  char* line;
  do {

    try{
      std::string s;
      getline(file, s);
      if (s.compare("") != 0 ){

        if (i >= index) {
          line = cpp_strdup(s.c_str());
          //std::cout << s << std::endl;
          end = true;
          found = true;
        }
      }
      else {
        end = true;
      }
    }
    catch (std::ios_base::failure e) {
      end = true;
    }

    i++;
  } while (!end);

  file.close();
  if (!found) {
    return NULL;
  }


  return line;
}


void readlines(const char* path, std::vector<std::string>& lines) {
  int i = 0;
  bool end = false;
  std::fstream file;
  file.open(path, std::ios_base::in);

  do {

    try{
      std::string s;
      getline(file, s);
      if (s.compare("") != 0 ){
        lines.push_back(s);
      }
      else {
        end = true;
      }
    }
    catch (std::ios_base::failure e) {
      end = true;
    }

    i++;
  } while (!end);
  file.close();
}



std::string copy_to_tmp_file(const std::string& src, const std::string& ext) {
  char* src_path = strdup(src.c_str());
  char* dir = dirname(src_path);

  std::ostringstream copy_file_path;
  boost::uuids::uuid uuid = diet_generate_uuid();
  copy_file_path << dir << "/" << uuid << ext;
  std::string cmd = "cp " + src + " " + copy_file_path.str().c_str();
  int env = system(cmd.c_str());
  free(src_path);

  if (env) return "";

  return copy_file_path.str();
}

int create_folder(const char* folder_path) {
  std::string cmd = "";
  cmd.append("mkdir -p ");
  cmd.append(folder_path);
  //printf("CREATE LOCAL DIRECTORY : %s\n", local_results_folder.c_str());
  int env = system(cmd.c_str());

  return env;
}

std::string get_folder_in_dagda_path(const char* folder_name) {
  DagdaImpl *dataManager = DagdaFactory::getDataManager();
  const char* dagda_path = dataManager->getDataPath();



  std::string folder = "";
  folder.append(dagda_path);
  folder.append("/");
  folder.append(folder_name);

  return folder;
}

std::string int2string(int i) {
  char s[512];
  sprintf(s, "%i", i);
  return std::string(s);
}

void append2path(std::string& path, const std::string& add) {
  path.append("/");
  path.append(add);
}

int create_folder_in_dagda_path(const char* folder_name) {
  /*DagdaImpl *dataManager = DagdaFactory::getDataManager();
    const char* dagda_path = dataManager->getDataPath();
    */

  //the local folder of datas
  std::string local_results_folder = get_folder_in_dagda_path(folder_name);

  int env = create_folder(local_results_folder.c_str());

  return env;
}


int create_folder_in_dagda_path_with_request_id(int reqId) {
  return create_folder_in_dagda_path(int2string(reqId).c_str());
}


std::string create_tmp_file(std::string directory_path, const std::string ext) {

  std::ostringstream file_path;
  boost::uuids::uuid uuid = diet_generate_uuid();
  file_path << directory_path << "/" << uuid << ext;

  FILE* tmp_file = fopen(file_path.str().c_str(), "w");
  if (tmp_file == NULL) {
    return "";
  }

  fclose(tmp_file);


  return file_path.str();
}


std::string create_tmp_file(diet_profile_t* pb, const std::string ext) {
  //create the tmp ip file
  int reqId = pb->dietReqID;
  std::string szReqId = int2string(reqId);
  std::string local_results_folder = get_folder_in_dagda_path(szReqId.c_str());
  int env = create_folder(local_results_folder.c_str());

  if (env) return "";

  return create_tmp_file(local_results_folder, ext);

}


int write_lines(const std::vector<std::string>& lines, const std::string& file_path) {
  FILE* file = fopen(file_path.c_str(), "w");

  if (file == NULL) return -1;

  for(size_t i = 0; i < lines.size(); i++){
    fprintf(file, "%s\n", lines[i].c_str());
  }

  fclose(file);

  return 0;
}






#ifdef USE_LOG_SERVICE

DietLogComponent *
get_log_component() {
  if (DagdaFactory::getSeDDataManager() == NULL) {
    return NULL;
  }
  return DagdaFactory::getSeDDataManager()->getLogComponent();
}

std::vector<wrapped_service_log> wrappedServicesList;

void
logVMServiceWrapped(const ServiceWrapper& serviceWrapper, const char* vmIP,
    const char* vmUserName) {
  wrapped_service_log entry ;
  entry.serviceWrapper = new ServiceWrapper(serviceWrapper);

  entry.vmIP = (char*) malloc(sizeof(char) *(strlen(vmIP)+1));
  strcpy(entry.vmIP, vmIP);

  entry.vmUserName = (char*) malloc(sizeof(char) *(strlen(vmUserName)+1));
  strcpy(entry.vmUserName, vmUserName);

  wrappedServicesList.push_back(entry);
}

#endif










/******************* BEGIN : THIS CLASSES ARE REUSABLE**********************/


static XmlDOMParser* domParser;

XmlDOMParser* XmlDOMParser::get_instance()
{
  if (domParser == NULL) {
    domParser = new XmlDOMParser();
  }
  return domParser;
}


XmlDOMParser::XmlDOMParser() : m_parser(NULL), m_errHandler(NULL)
{
  xercesc::XMLPlatformUtils::Initialize();
  m_parser = new xercesc::XercesDOMParser();
  m_errHandler = (xercesc::ErrorHandler*) new XmlDOMErrorHandler();
  m_parser->setErrorHandler(m_errHandler);
}

XmlDOMParser::~XmlDOMParser()
{
  if (m_parser) {
    delete m_parser;
    xercesc::XMLPlatformUtils::Terminate();
    domParser = NULL;
  }
}


xercesc::DOMDocument* XmlDOMParser::parse(const char* xmlfile)
{
  m_parser->parse(xmlfile);
  return m_parser->adoptDocument();
}


XmlDOMDocument::XmlDOMDocument(XmlDOMParser* parser,
    const char* xmlfile) : m_doc(NULL)
{
  m_doc = parser->parse(xmlfile);
}


XmlDOMDocument::~XmlDOMDocument()
{
  if (m_doc) m_doc->release();

  m_doc = NULL;
}

std::string XmlDOMDocument::get_child_value(const char* parentTag,
    int parentIndex,
    const char* childTag)
{
  XMLCh* temp = xercesc::XMLString::transcode(parentTag);
  xercesc::DOMNodeList* list = m_doc->getElementsByTagName(temp);
  xercesc::XMLString::release(&temp);

  xercesc::DOMElement* parent =
    dynamic_cast<xercesc::DOMElement*>(list->item(parentIndex));
  xercesc::DOMElement* child =
    dynamic_cast<xercesc::DOMElement*>(parent->getElementsByTagName(
          xercesc::XMLString::transcode(childTag))->item(0));
  std::string value;
  if (child) {
    char* temp2 = xercesc::XMLString::transcode(child->getTextContent());
    value = temp2;
    xercesc::XMLString::release(&temp2);
  }
  else {
    value = "";
  }
  return value;
}

std::string XmlDOMDocument::get_value(const char* tag, int elt_index) {
  XMLCh* temp = xercesc::XMLString::transcode(tag);
  xercesc::DOMNodeList* list = m_doc->getElementsByTagName(temp);
  xercesc::XMLString::release(&temp);

  xercesc::DOMElement* node =
    dynamic_cast<xercesc::DOMElement*>(list->item(elt_index));

  std::string value;
  if (node) {
    char* temp2 = xercesc::XMLString::transcode(node->getTextContent());
    value = temp2;
    xercesc::XMLString::release(&temp2);
  }
  else {
    return "";
  }

  return value;
}

std::string XmlDOMDocument::get_attribute_value(const char* elementTag,
    int elementIndex,
    const char* attributeTag)
{
  XMLCh* temp = xercesc::XMLString::transcode(elementTag);
  xercesc::DOMNodeList* list = m_doc->getElementsByTagName(temp);
  xercesc::XMLString::release(&temp);

  xercesc::DOMElement* element =
    dynamic_cast<xercesc::DOMElement*>(list->item(elementIndex));
  temp = xercesc::XMLString::transcode(attributeTag);
  char* temp2 = xercesc::XMLString::transcode(element->getAttribute(temp));

  std::string value = temp2;
  xercesc::XMLString::release(&temp);
  xercesc::XMLString::release(&temp2);
  return value;
}


int XmlDOMDocument::get_element_count(const char* elementName)
{
  xercesc::DOMNodeList* list =
    m_doc->getElementsByTagName(xercesc::XMLString::transcode(elementName));
  return (int)list->getLength();
}


XmlDOMDocument* read_xml_file(const char* path) {
  XmlDOMParser* parser = XmlDOMParser::get_instance();

  if (parser) {
    XmlDOMDocument* doc = new XmlDOMDocument(parser, path);
    return doc;
  }


  return NULL;
}


int read_elements_from_xml(const char* path, std::vector<std::string>& list, std::string elt){
  std::string value;
  XmlDOMParser* parser = XmlDOMParser::get_instance();
  if (parser) {
    XmlDOMDocument* doc = new XmlDOMDocument(parser, path);
    if (doc) {
      for (int i = 0; i < doc->get_element_count(elt.c_str()); i++) {
        value = doc->get_value(elt.c_str(), i);
        list.push_back(value);
      }
      delete doc;
    }
    else {
      return -1;
    }

    delete parser;
  }
  else {
    return -1;
  }

  return 0;
}

std::string read_element_from_xml(const char* path, std::string elt) {
  std::vector<std::string> list;
  std::string result;
  int env = read_elements_from_xml(path, list, elt);
  if (env) return "";
  if (list.size() <= 0) return "";
  result = list[0];

  return result;
}

int XmlDOMDocument::add_child_content(const char* parent, int parentIndex, const char* child, std::string child_content) {
  XMLCh* temp = xercesc::XMLString::transcode(parent);
  xercesc::DOMNodeList* list = m_doc->getElementsByTagName(temp);
  xercesc::XMLString::release(&temp);

  xercesc::DOMElement* node =
    dynamic_cast<xercesc::DOMElement*>(list->item(parentIndex));

  temp = xercesc::XMLString::transcode(child);
  xercesc::DOMElement* child_element = m_doc->createElement(temp);
  xercesc::XMLString::release(&temp);

  temp = xercesc::XMLString::transcode(child_content.c_str());
  child_element->setTextContent(temp);
  xercesc::XMLString::release(&temp);

  node->appendChild(child_element);
  return 0;
}


void dom_to_xml_file(xercesc::DOMDocument* pDOMDocument, const char* path )
{
  const XMLCh gLS[] = { xercesc::chLatin_L, xercesc::chLatin_S, xercesc::chNull };
  xercesc::DOMImplementation *pImplement = NULL;
  xercesc::DOMLSSerializer *pSerializer = NULL; // @DOMWriter
  xercesc::LocalFileFormatTarget *pTarget = NULL;

  //Return the first registered implementation that has the desired features. In this case, we are after
  //a DOM implementation that has the LS feature... or Load/Save.
  pImplement = xercesc::DOMImplementationRegistry::getDOMImplementation(gLS);

  //From the DOMImplementation, create a DOMWriter.
  //DOMWriters are used to serialize a DOM tree [back] into an XML document.

  pSerializer = ((xercesc::DOMImplementationLS*)pImplement)->createLSSerializer(); //@createDOMWriter();

  //This line is optional. It just sets a feature of the Serializer to make the output
  //more human-readable by inserting line-feeds, without actually inserting any new elements/nodes
  //into the DOM tree. (There are many different features to set.) Comment it out and see the difference.

  //pSerializer->setFeature(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true); //
  xercesc::DOMLSOutput *pOutput = ((xercesc::DOMImplementationLS*)pImplement)->createLSOutput();
  xercesc::DOMConfiguration *pConfiguration = pSerializer->getDomConfig();

  // Have a nice output
  if (pConfiguration->canSetParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true))
    pConfiguration->setParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true);

  pTarget = new xercesc::LocalFileFormatTarget(path);
  pOutput->setByteStream(pTarget);

  // @pSerializer->write(pDOMDocument->getDocumentElement(), pOutput); // missing header "<xml ...>" if used
  pSerializer->write(pDOMDocument, pOutput);

  delete pTarget;
  pOutput->release();
  pSerializer->release();
}


void XmlDOMDocument::write(const char* path) {
  dom_to_xml_file(m_doc, path);
}




/*******************END : THIS CLASSES ARE REUSABLE**********************/
