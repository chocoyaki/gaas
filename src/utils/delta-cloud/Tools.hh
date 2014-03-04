/*
Authors :
- Lamiel Toch : lamiel.toch@ens-lyon.fr
*/


#ifndef __DELTACLOUD__TOOLS__h
#define __DELTACLOUD__TOOLS__h

#include "IaasInterface.hh"
#include "Instance.hh"
#include "Parameter.hh"
#include "DIET_data.h"
#include <string>
#include <vector>
#include <map>
#include <set>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <string>

#include "deltacloud_config.h"

class ServiceWrapper;
class DietLogComponent;

//using namespace std;
//using namespace xercesc;

void deleteStringVector(std::vector<std::string*>& v);
char* cpp_strdup(const char* src);

std::string get_ip_instance_by_id(const IaaS::pIaasInterface & interf, std::string instance_id, bool is_private_ip);
int read_properties_file(const std::string& path, std::map<std::string, std::string>& results);



char* readline(const char* path, int index);
void readlines(const char* path, std::vector<std::string>& lines);



int create_folder(const char* folder_path);
std::string get_folder_in_dagda_path(const char* folder_name);
std::string int2string(int i);
void append2path(std::string& path, const std::string& add);

std::string create_tmp_file(std::string directory_path, const std::string ext);
std::string create_tmp_file(diet_profile_t* pb, const std::string ext);
int write_lines(const std::vector<std::string>& ips, const std::string& file);

//0 iif succeeds
int create_folder_in_dagda_path(const char* folder_name);
int create_folder_in_dagda_path_with_request_id(int reqId);

std::string copy_to_tmp_file(const std::string& src, const std::string& ext);

#ifdef USE_LOG_SERVICE
// Log Utilities

DietLogComponent*
get_log_component();

typedef struct
{
  ServiceWrapper* serviceWrapper;
  char* vmIP;
  char* vmUserName;
} wrapped_service_log;

extern std::vector<wrapped_service_log> wrappedServicesList;


void
logVMServiceWrapped(
    const ServiceWrapper& serviceWrapper, const char* vmIP,
    const char* vmUserName);


#endif // USE_LOG_SERVICE


/******************* BEGIN : THIS CLASSES ARE REUSABLE**********************/
class XmlDOMErrorHandler : public xercesc::HandlerBase
{
  public:
    void fatal_error(const xercesc::SAXParseException &exc) {
      printf("Parsing error at line %d\n",
          (int)exc.getLineNumber());
      exit(-1);
    }
};

class XmlDOMParser
{
  protected:
    xercesc::XercesDOMParser* m_parser;
    xercesc::ErrorHandler*    m_errHandler;

  public:
    ~XmlDOMParser();

    static XmlDOMParser* get_instance();
    xercesc::DOMDocument* parse(const char* xmlfile);

  private:
    XmlDOMParser();
};


class XmlDOMDocument
{
  protected:
    xercesc::DOMDocument* m_doc;

  public:
    XmlDOMDocument(XmlDOMParser* parser, const char* xmlfile);
    ~XmlDOMDocument();

    std::string get_child_value(const char* parentTag, int parentIndex,
        const char* childTag);

    std::string get_value(const char* tag, int elt_index);
    std::string get_attribute_value(const char* elementTag,
        int elementIndex,
        const char* attributeTag);
    int get_element_count(const char* elementName);

    int add_child_content(const char* parent, int parentIndex, const char* child, std::string child_content);

    void write(const char* path);
  private:
    //XmlDOMDocument();
    //XmlDOMDocument(const XmlDOMDocument&);
};


XmlDOMDocument* read_xml_file(const char* path);

int read_elements_from_xml(const char* path, std::vector<std::string>& list, std::string elt);
std::string read_element_from_xml(const char* path, std::string elt);


/******************* END : THIS CLASSES ARE REUSABLE**********************/

#endif
