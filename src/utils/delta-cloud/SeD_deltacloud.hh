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

#include "Instance.hh"

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

class SeDCloud {
private:
    std::string base_url; // eg : "http://localhost:3001/api";
	std::string username; // eg : "oneadmin";
	std::string password; //eg : "mypassword";
	std::string vm_user; // eg : "opennebula";
	int vm_count; // eg : 1;
	//std::string profile; //eg :  "debian-rc";
	std::vector<IaaS::Parameter> params; //parameters for instantiating a VM with deltacloud
public:
    SeDCloud(std::string _base_url, std::string _username, std::string _password, std::string _vm_user,
                          int _vm_count, const std::vector<IaaS::Parameter>& params = std::vector<IaaS::Parameter>());

    void addParameter(std::string param, std::string value) {
        params.push_back(IaaS::Parameter(param, value));
    }


    virtual void deployer_on_one_vm(const char* path_to_binary);

    virtual DIET_API_LIB int
        service_table_add(const diet_profile_desc_t* const profile,
                         const diet_convertor_t* const cvt,
                         diet_solve_t solve_func);

 };







#endif
