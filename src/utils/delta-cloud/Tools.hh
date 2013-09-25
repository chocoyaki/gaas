/*
	Authors :
		- Lamiel Toch : lamiel.toch@ens-lyon.fr
*/


#ifndef __DELTACLOUD__TOOLS__h
#define __DELTACLOUD__TOOLS__h

#include "IaasInterface.hh"
#include "Instance.hh"
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


//using namespace std;
//using namespace xercesc;

void deleteStringVector(std::vector<std::string*>& v);
char* cpp_strdup(const char* src);

std::string get_ip_instance_by_id(IaaS::IaasInterface* interf, std::string instance_id, bool is_private_ip);
int read_properties_file(const std::string& path, std::map<std::string, std::string>& results);


/*

return 0 iif success
*/
int test_ssh_connection(std::string ssh_user, std::string ip);
int test_ssh_connection_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip = true);
int rsync_to_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string local_path, std::string remote_path);

int rsync_to_vm(std::string local_path, std::string remote_path, std::string user, std::string ip);
int rsync_from_vm(std::string remote_path, std::string local_path, std::string user, std::string ip);


int execute_command_in_vm(const std::string& remote_cmd, std::string user, std::string ip, std::string args);
int execute_command_in_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_cmd, std::string args);

int create_directory_in_vm(const std::string& remote_path, std::string user, std::string ip, std::string args = " -p ");
int create_directory_in_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_path, std::string args = " -p ");


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
void
logVMServiceWrapped(
    const ServiceWrapper& serviceWrapper, const char* vmIP,
    const char* vmUserName);
#endif // USE_LOG_SERVICE


namespace IaaS {






class VMInstances {
	public:

	/*
	create VM instances

	example :  {
		string base_url = "http://localhost:3001/api";
		string username = "admin+admin";
		string password = "admin";
		string vm_user = "cirros";
	*/
	VMInstances(std::string image_id, int vm_count, std::string base_url, std::string user_name, std::string password,
	std::string vm_user, const std::vector<Parameter>& params = std::vector<Parameter>());

	~VMInstances();

	void wait_all_instances_running();
	std::string get_instance_id(int i);
	Instance* get_instance(int i);

	void get_ips(std::vector<std::string>& ips, bool private_ip);


	//connection to vm index i with private ip address by default, public address otherwise
	int test_ssh_connection(int i, bool private_ip = true);
	int test_all_ssh_connection(bool private_ips = true);
	void wait_all_ssh_connection(bool private_ips = true);
	std::string get_ip(int vm_index, bool private_ip = true);

    int rsync_from_vm(int i, bool private_ip, std::string remote_path, std::string local_path);
    int rsync_to_vm(int i, bool private_ip, std::string local_path, std::string remote_path);
    //command : remote_path/exec.sh 1 2 3 4 ...
    int execute_command_in_vm(int i, bool private_ip, const std::string& remote_cmd, const std::string& args);


	protected:

	bool is_instance_in_error_state(const std::string& id);
	std::set<std::string> get_error_instance_ids();
	void terminate_failed_instances_and_run_others();

	IaasInterface* interf;
	//id of instances
	std::vector<std::string*>* insts;

	int vm_count;
	std::string image_id;
	std::string vm_user;
	std::vector<Parameter> params;
};


//std::vector<std::string*>* create_one_vm_instance(int vm_count, std::string base_url, std::string user_name, std::string password,
//	std::string vm_user, IaasInterface ** interf);


/*
class OpenStackVMInstances : public VMInstances{
	public:
		OpenStackVMInstances(std::string image_id, int vm_count, std::string base_url, std::string user_name, std::string password,
	std::string vm_user, std::string key_name);
};
*/

}

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
