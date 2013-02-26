/*
	Authors :
		- Lamiel Toch : lamiel.toch@ens-lyon.fr
*/

#ifndef _SED_DELTACLOUD_HH_
#define _SED_DELTACLOUD_HH_

#include "DIET_data.h"
#include "DIET_server.h"

#include <string>
#include <vector>
#include <map>

#include "Instance.hh"
#include "Tools.hh"

/**
     Function used to allocate a DIET profile descriptors with memory space
     for its argument descriptors. All arguments are DIET_FILE of DIET_CHAR
     @arg If no IN argument, please give -1 for last_in.
     @arg If no INOUT argument, please give last_in for last_inout.
     @arg If no OUT argument, please give last_inout for last_out.

     Once allocation is performed, please use set functions for each descriptor.
     For example, the nth argument is a matrix of doubles:
     \code
     diet_generic_desc_set(diet_param_desc(profile, n), DIET_MATRIX; DIET_DOUBLE);
     \endcode

     @param path name of the service
     @param last_in last_in : \f$-1\f$ + number of input data
     @param last_inout last_inout : \e last_in \f$+\f$ number of inout data
     @param last_out last_out : \e last_inout \f$+\f$ number of out data

     @return the newly allocated diet_profile_desc
  */
  DIET_API_LIB diet_profile_desc_t*
  dietcloud_profile_files_desc_alloc_and_set(const char* path, int last_in,
                          int last_inout, int last_out);

class CloudServiceBinary {
   public:
    std::string local_path_of_binary;
    std::string remote_path_of_binary;

    //const int last_in;
    //const int last_out;

    CloudServiceBinary(const std::string& _local_path, const std::string& _remote_path /*, int last_in, int _last_out */);
    CloudServiceBinary(const CloudServiceBinary& binary);
    CloudServiceBinary();
};


class SeDCloud {
private:
    std::string image_id;
    std::string base_url; // eg : "http://localhost:3001/api";
	std::string username; // eg : "oneadmin";
	std::string password; //eg : "mypassword";
	std::string vm_user; // eg : "opennebula";
	int vm_count; // eg : 1;
	//std::string profile; //eg :  "debian-rc";
	std::vector<IaaS::Parameter> params; //parameters for instantiating a VM with deltacloud
	IaaS::VMInstances* vm_instances;
	bool is_ip_private;
protected:
    SeDCloud(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                          int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& params = std::vector<IaaS::Parameter>());

    ~SeDCloud();
public:
    void addParameter(const std::string& param, const std::string& value);

    static void create(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                          int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& params = std::vector<IaaS::Parameter>());

    static void erase();

    static SeDCloud* get();

    virtual void deployer_on_one_vm(int vm_index, const char* path_to_binary);

    virtual DIET_API_LIB int
        service_table_add(const std::string& name_of_service,
                          int last_in,
                          int last_out,
                         const diet_convertor_t* const cvt,
                         const std::string& local_path_of_binary,
                         const std::string& remote_path_of_binary);

    bool using_private_ip();

protected:
    static int solve(diet_profile_t *pb);
    static SeDCloud* instance;
    static std::map<std::string, CloudServiceBinary> cloud_service_binaries;
 };

std::map<std::string, CloudServiceBinary> SeDCloud::cloud_service_binaries;
SeDCloud* SeDCloud::instance;




#endif
