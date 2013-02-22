#include "SeD_deltacloud.hh"



DIET_API_LIB diet_profile_desc_t*
  dietcloud_profile_files_desc_alloc_and_set(const char* path, int last_in, int last_inout, int last_out) {
    diet_profile_desc_t* profile;

    profile = diet_profile_desc_alloc(path, last_in, last_inout, last_out);

    for(int i = 0; i <= last_out; i++) {
        diet_generic_desc_set(diet_param_desc(profile, i), DIET_FILE, DIET_CHAR);
    }

    return profile;
  }



SeDCloud::SeDCloud(std::string _base_url, std::string _username, std::string _password, std::string _vm_user,
                          int _vm_count, const std::vector<IaaS::Parameter>& params) {
        base_url = _base_url;
        username = _username;
        password = _password;
        vm_user = _vm_user;
        vm_count = _vm_count;
}




DIET_API_LIB int
        SeDCloud::service_table_add(const diet_profile_desc_t* const profile,
                         const diet_convertor_t* const cvt,
                         diet_solve_t solve_func) {



}


void SeDCloud::deployer_on_one_vm(const char* path_to_binary) {

}
