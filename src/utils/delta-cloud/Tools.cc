/*
	Authors :
		- Lamiel Toch : lamiel.toch@ens-lyon.fr
*/

#include "Tools.hh"
#include "Iaas_deltacloud.hh"
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>


#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>

#include "DagdaImpl.hh"
#include "DagdaFactory.hh"

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
}

char* cpp_strdup(const char* src) {
	char* cpy = new char [strlen(src) + 1];
	strcpy(cpy, src);
	return cpy;
}




std::string get_ip_instance_by_id(IaaS::IaasInterface* interf, std::string instance_id, bool is_private_ip) {
    IaaS::Instance* instance = interf->get_instance_by_id(instance_id);

	if (instance == NULL) return "???.???.???.???";

	std::string ip;

	if (is_private_ip) {
		ip = instance->private_ip;
	}
	else {
		ip = instance->public_ip;
	}

	delete instance;

	return ip;
}

std::string IaaS::VMInstances::get_ip(int vm_index, bool is_private_ip) {
	std::string id = get_instance_id(vm_index);
	return get_ip_instance_by_id(interf, id, is_private_ip);
}

std::set<std::string> IaaS::VMInstances::get_error_instance_ids() {
	std::set<std::string> result;

	for(int i = 0; i < insts->size(); i++) {
		std::string& id = *(*insts)[i];

		if (is_instance_in_error_state(id)) {
			result.insert(id);
		}
	}

	return result;
}

bool IaaS::VMInstances::is_instance_in_error_state(const std::string& id) {
	return strcmp(interf->get_instance_state(id).c_str(), "ERROR") == 0;
}


void deleteStringVector(std::vector<std::string*>& v){
	for(int i = 0; i < v.size(); i++) {
		delete v[i];
		v[i] = NULL;
	}
}

int test_ssh_connection(std::string ssh_user, std::string ip) {
	std::string cmd = "ssh -q "  + ssh_user + "@" + ip + " -o StrictHostKeyChecking=no PasswordAuthentication=no 'exit'";
	//std::cout << cmd << std::endl;
	int ret = system(cmd.c_str());

	return ret;
}




int test_ssh_connection_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool is_private_ip){

    std::string ip = get_ip_instance_by_id(interf, instance_id, is_private_ip);

	int ret = ::test_ssh_connection(vm_user, ip);

	return ret;
}

int rsync_to_vm(std::string local_path, std::string remote_path, std::string user, std::string ip) {
    int ret;

    std::string cmd = "rsync -avz -e 'ssh -o StrictHostKeyChecking=no' " + local_path + " " + user + "@" + ip + ":" + remote_path;
    std::cout << cmd << std::endl;
    ret = system(cmd.c_str());

    return ret;
}


int rsync_to_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string local_path, std::string remote_path) {

    std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);

    int ret = ::rsync_to_vm(local_path, remote_path, vm_user, ip);

    return ret;
}


int rsync_from_vm(std::string remote_path, std::string local_path, std::string user, std::string ip) {
    int ret;

    std::string cmd = "rsync -avz -e 'ssh -o StrictHostKeyChecking=no' " + user + "@" + ip + ":" + remote_path + " " + local_path;
    std::cout << cmd << std::endl;
    ret = system(cmd.c_str());

    return ret;
}

int rsync_from_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_path, std::string local_path) {

    std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);

    int ret = ::rsync_from_vm(remote_path, local_path, vm_user, ip);

    return ret;
}




int execute_command_in_vm(const std::string& remote_cmd, std::string vm_user, std::string ip, std::string args) {
    std::string cmd = "ssh "  + vm_user+ "@" + ip + " -o StrictHostKeyChecking=no '" + remote_cmd + " " + args + "'";
	std::cout << cmd << std::endl;
	int ret = system(cmd.c_str());

	return ret;
}


int execute_command_in_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_cmd, std::string args) {
    std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);

    int ret = ::execute_command_in_vm(remote_cmd, vm_user, ip, args);
}



int create_directory_in_vm(const std::string& remote_path, std::string user, std::string ip, std::string args) {
  printf(">>>>>>>>create directory in machine : %s\n", ip.c_str());
  std::string cmd = "ssh "  + user + "@" + ip + " -o StrictHostKeyChecking=no 'mkdir " + args + remote_path + "'";
  int ret = system(cmd.c_str());
  return ret;
}


int create_directory_in_vm_by_id(IaaS::IaasInterface* interf, std::string vm_user, std::string instance_id, bool private_ip, std::string remote_path, std::string args) {
  std::string ip = get_ip_instance_by_id(interf, instance_id, private_ip);
  int ret = ::create_directory_in_vm(remote_path, vm_user, ip, args);
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

	char* line;
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



namespace IaaS {

/*Instance * create_one_vm_instance(std::string base_url, std::string user_name, std::string password,
	std::string vm_user, IaasInterface ** interf) {

}
*/




VMInstances::VMInstances(std::string _image_id, int _vm_count, std::string base_url, std::string user_name, std::string password,
	std::string _vm_user, const std::vector<Parameter>& _params) {

	vm_user = _vm_user;
	image_id = _image_id;
	vm_count = _vm_count;
	params = _params;

	interf = new Iaas_deltacloud(base_url, user_name, password);

	insts = interf->run_instances(image_id, vm_count, params);

	//with openstack, error may happen, so we run other replacing instances
	while(get_error_instance_ids().size() > 0){
		terminate_failed_instances_and_run_others();
	};
}


VMInstances::~VMInstances() {
	interf->terminate_instances(*insts);

	delete interf;
	deleteStringVector(*insts);
	delete insts;
}

void VMInstances::terminate_failed_instances_and_run_others() {
	std::set<std::string> failed_instances = get_error_instance_ids();
	int failed_instances_count = failed_instances.size();

	if (failed_instances_count > 0) {
		printf("warning : there are %u failed instances\n", failed_instances_count);
		std::vector<std::string*> v_failed_instances;
		std::set<std::string>::iterator iter;
		for(iter = failed_instances.begin(); iter != failed_instances.end(); iter++) {
			v_failed_instances.push_back(new std::string(*iter));
		}

		//we terminate all failed instances
		interf->terminate_instances(v_failed_instances);

		//we run other instances with the same count
		std::vector<std::string*>* new_insts = interf->run_instances(image_id, failed_instances_count, params);

		wait_all_instances_running();

		int index_new_inst = 0;

		//we search the old instance id places
		for(int i = 0 ; i < insts->size(); i++){
			std::string id = *(*insts)[i];
			if (failed_instances.count(id) > 0) {
				//we add the new instance
				(*insts)[i] = (*new_insts)[index_new_inst];
				index_new_inst++;
			}
		}


		deleteStringVector(v_failed_instances);
		//we do not delete pointers to strings in new_insts
		delete new_insts;

	}
}

void VMInstances::wait_all_instances_running() {
	for(int i = 0; i < insts->size(); i++) {
		std::string instanceId = *(*insts)[i];
		int env = interf->wait_instance_running(instanceId);
	}
}

std::string VMInstances::get_instance_id(int i) {
	return *(*insts)[i];
}


Instance* VMInstances::get_instance(int i) {
	return interf->get_instance_by_id(get_instance_id(i));
}




int VMInstances::test_ssh_connection(int i, bool is_private_ip) {
	bool result = test_ssh_connection_by_id(interf, vm_user, get_instance_id(i), is_private_ip);

#ifdef USE_LOG_SERVICE
	if (result == 0) {
		Instance* instance = get_instance(i);
		DietLogComponent* component = get_log_component();
		component->logVMOSReady(*instance);
		delete instance;
	}
#endif


	return result;
}

int VMInstances::test_all_ssh_connection(bool private_ips) {
	for(int i = 0; i < insts->size(); i++) {
		int ret = test_ssh_connection(i, private_ips);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}


void VMInstances::wait_all_ssh_connection(bool private_ips) {
	int ret;

	do{
		ret = test_all_ssh_connection(private_ips);
		sleep(1);
	} while (ret != 0);
}


int VMInstances::rsync_to_vm(int i, bool private_ip, std::string local_path, std::string remote_path) {

    int ret;

    ret = rsync_to_vm_by_id(interf, vm_user, get_instance_id(i), private_ip, local_path, remote_path);

    return ret;
}

int VMInstances::rsync_from_vm(int i, bool private_ip, std::string remote_path, std::string local_path) {
    int ret;

    ret = rsync_from_vm_by_id(interf, vm_user, get_instance_id(i), private_ip, remote_path, local_path);

    return ret;
}

int VMInstances::execute_command_in_vm(int i, bool private_ip, const std::string& remote_cmd, const std::string& args) {

    //std::string remote_cmd = remote_path + "/exec.sh";
    //std::string args = "";

    /*char arg[16];

    for(int k = 0; k < n; k++) {
        sprintf(arg, "%i", k);
        args = args + " " + std::string(arg);
    }*/

    ::execute_command_in_vm_by_id(interf, vm_user, get_instance_id(i), private_ip, remote_cmd, args);
}


void VMInstances::get_ips(std::vector<std::string>& ips, bool private_ip) {
	for(int i = 0; i < insts->size(); i++) {
		ips.push_back(get_ip(i, private_ip));
	}
}

/*
OpenStackVMInstances::OpenStackVMInstances(std::string image_id, int vm_count, std::string base_url, std::string user_name, std::string password, std::string vm_user, std::string key_name) : VMInstances(image_id, vm_count, base_url, user_name, password, vm_user, std::vector<Parameter>(1, Parameter("keyname", key_name))) {

}*/


}




#ifdef USE_LOG_SERVICE


DietLogComponent*
get_log_component() {
  return DagdaFactory::getSeDDataManager()->getLogComponent();
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
