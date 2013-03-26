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

#include <stdio.h>
#include <set>

/*
The folder must contains an executable and a shell script
which call the executable. The name of this script is "exec.sh".
The parameters are files and are named as follows :
0 for the first parameter
1 for the second parameter
2 for the third and so on.
*/
class CloudServiceBinary {
   public:
    //the local folder path which contains the executable
    std::string local_path_of_binary;

    //the remote folder path which contains the executable (in Virtual Machine)
    std::string remote_path_of_binary;

    CloudServiceBinary(const std::string& _local_path, const std::string& _remote_path /*, int last_in, int _last_out */);
    CloudServiceBinary(const CloudServiceBinary& binary);
    CloudServiceBinary();
};


class ServiceStatistics{
public:
    ServiceStatistics(const std::string _service_name) {
        service_name = _service_name;
        call_number = 0;
    }

    ServiceStatistics(const ServiceStatistics& stats) {
        service_name = stats.service_name;
        call_number = stats.call_number;
    }

    ServiceStatistics(){
        service_name = "";
        call_number = 0;
    }

    void increment_call_number() {
        call_number++;
    }

    int call_count() const {
        return call_number;
    }




    /*
    bool operator< (const ServiceStatistics& other) const {
        return service_name < other.service_name;
    }*/

protected:
    std::string service_name;
    int call_number;
};



class ServiceStatisticsMap{
public:
    ServiceStatisticsMap() {

    }

    void add_service(const std::string& service_name) {
        stats_map[service_name] = ServiceStatistics(service_name);
    }

    bool service_exists(const std::string& service_name) const {
        return stats_map.count(service_name) > 0;
    }

    void increment_call_number(const std::string& service_name) {
        stats_map[service_name].increment_call_number();
    }

    const std::map<std::string, ServiceStatistics>& get() const {
        return stats_map;
    }

    bool one_service_already_called(const std::string& service_name) const {
        const ServiceStatistics stat = stats_map.at(service_name);
        return stat.call_count() > 0;
    }

    //at least one service has already been called
    bool one_service_already_called() const {
        std::map<std::string, ServiceStatistics>::const_iterator iter;

        for(iter = stats_map.begin(); iter != stats_map.end(); iter++){
            const std::string& service_name = iter->first;
            if (one_service_already_called(service_name)) {
                return true;
            }
        }

        return false;
    }


    int call_count(const std::string& service_name) const {
        const ServiceStatistics stat = stats_map.at(service_name);
        return stat.call_count();
    }

protected:
    std::map<std::string, ServiceStatistics> stats_map;


};

class SeDCloudActions {
protected:

    /*******FOR DELTACLOUD********/
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
	ServiceStatisticsMap statistics_on_services;
    void copy_all_binaries_into_vm(int vm_index);
public:
    //execute an action when the the client make a solve
    virtual int perform_action_on_begin_solve(diet_profile_t *pb) = 0;
    virtual int perform_action_on_end_solve(diet_profile_t *pb) = 0;

    virtual void perform_action_on_sed_creation() = 0;

    //at diet_SeD instruction
    virtual void perform_action_on_sed_launch() = 0;

    virtual int perform_action_after_service_table_add(const std::string& name_of_service) = 0;

    virtual int send_arguments(const std::string& local_path, const std::string& remote_path);
    virtual int receive_result(const std::string& result_remote_path, const std::string& result_local_path);
    virtual int execute_remote_binary(const std::string& remote_path_of_binary, const std::string& args);

    SeDCloudActions();

    SeDCloudActions(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                          int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& params = std::vector<IaaS::Parameter>());


    ~SeDCloudActions() {
        if (this->vm_instances != NULL){
            delete this->vm_instances;
            this->vm_instances = NULL;
        }
    }


    bool using_private_ip() {
        return this->is_ip_private;
    }

    static std::map<std::string, CloudServiceBinary> cloud_service_binaries;
};


class SeDCloudAndVMLaunchedActions : public SeDCloudActions {
public:

    virtual int perform_action_on_begin_solve(diet_profile_t *pb);
    virtual int perform_action_on_end_solve(diet_profile_t *pb);

    virtual void perform_action_on_sed_creation();
    virtual void perform_action_on_sed_launch();

    virtual int perform_action_after_service_table_add(const std::string& name_of_service);

    SeDCloudAndVMLaunchedActions() : SeDCloudActions() {}

    SeDCloudAndVMLaunchedActions(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                          int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& _params = std::vector<IaaS::Parameter>()) :
                          SeDCloudActions(_image_id, _base_url, _username, _password, _vm_user, _vm_count, _is_ip_private, _params){

    }



};

class SeDCloudWithoutVMActions : public SeDCloudAndVMLaunchedActions {
protected:
    std::string address_ip;
    std::string username;

public:
    virtual void perform_action_on_sed_creation();
    virtual void perform_action_on_sed_launch();

    virtual int send_arguments(const std::string& local_path, const std::string& remote_path);
    virtual int receive_result(const std::string& result_remote_path, const std::string& result_local_path);
    virtual int execute_remote_binary(const std::string& remote_path_of_binary, const std::string& args);

    SeDCloudWithoutVMActions(const std::string& _address_ip,const std::string& _username) :
                          SeDCloudAndVMLaunchedActions(){
        address_ip = _address_ip;
        username = _username;
    }


};


class SeDCloudVMLaunchedAtSolveActions : public SeDCloudActions {
public:
    virtual int perform_action_on_begin_solve(diet_profile_t *pb) = 0;
    virtual int perform_action_on_end_solve(diet_profile_t *pb) = 0;

    //virtual void perform_action_on_sed_creation();
    virtual void perform_action_on_sed_launch();

    virtual void perform_action_on_sed_creation() {};
    virtual int perform_action_after_service_table_add(const std::string& name_of_service) {};


    SeDCloudVMLaunchedAtSolveActions(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                          int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& _params = std::vector<IaaS::Parameter>()) :
                          SeDCloudActions(_image_id, _base_url, _username, _password, _vm_user, _vm_count, _is_ip_private, _params){

    }

protected:



};


class SeDCloudVMLaunchedAtFirstSolveActions : public SeDCloudVMLaunchedAtSolveActions {
public:

    virtual int perform_action_on_begin_solve(diet_profile_t *pb);
    virtual int perform_action_on_end_solve(diet_profile_t *pb);


    SeDCloudVMLaunchedAtFirstSolveActions(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                          int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& _params = std::vector<IaaS::Parameter>()) :
                          SeDCloudVMLaunchedAtSolveActions(_image_id, _base_url, _username, _password, _vm_user, _vm_count, _is_ip_private, _params) {
    }

};





//the controller which calls actions
class SeDCloud {


protected:
    SeDCloudActions* actions;

    SeDCloud(SeDCloudActions* _actions) {
            instance = NULL;
            actions = _actions;
            actions->perform_action_on_sed_creation();

    }



public:
    //void addParameter(const std::string& param, const std::string& value);

    static void launch(int argc, char* argv[]) {
        SeDCloud::instance->actions->perform_action_on_sed_launch();
        diet_SeD(argv[1], argc, argv);
    }

    static void create(SeDCloudActions* _actions) {
        if (instance == NULL) {
            SeDCloud::instance = new SeDCloud(_actions);
        }
    }

    static void erase() {
        if (instance != NULL) {
            delete instance;
            instance = NULL;
        }
    }

    static SeDCloud* get() {

        return SeDCloud::instance;

    }

    virtual DIET_API_LIB int
        service_table_add(const std::string& name_of_service,
                          int last_in,
                          int last_out,
                         const diet_convertor_t* const cvt,
                         const std::string& local_path_of_binary,
                         const std::string& remote_path_of_binary) {

    diet_profile_desc_t* profile;

    profile = diet_profile_desc_alloc(name_of_service.c_str(), last_in, last_in, last_out);

    for(int i = 0; i <= last_out; i++) {
        diet_generic_desc_set(diet_param_desc(profile, i), DIET_FILE, DIET_CHAR);
    }

    SeDCloudActions::cloud_service_binaries[name_of_service] = CloudServiceBinary(local_path_of_binary, remote_path_of_binary);

    if (diet_service_table_add(profile, cvt, solve)) {
        return 1;
    }


    diet_profile_desc_free(profile);
    diet_print_service_table();

    actions->perform_action_after_service_table_add(name_of_service);

    return 0;
}




protected:
    static int solve(diet_profile_t *pb) {
        SeDCloud::instance->actions->perform_action_on_begin_solve(pb);

        std::string name(pb->pb_name);

        CloudServiceBinary& binary = SeDCloudActions::cloud_service_binaries[name];

        std::string remote_path_of_binary = binary.remote_path_of_binary;
        int nb_args = pb->last_out + 1;
        int last_in = pb->last_in;
        size_t arg_size;
        char* sz_local_path = NULL;
        std::string local_path;
        std::string arg_remote_path;
        std::string arg_local_path;
        char sz_i[512];

        for(int i = 0; i <= last_in; i++) {
            diet_file_get(diet_parameter(pb, i), &sz_local_path, NULL, &arg_size);
            local_path = sz_local_path;

            sprintf(sz_i, "%i", i);
            //a disctinct remote folder for each request
            arg_remote_path = remote_path_of_binary + "/" + sz_i;

            //SeDCloud::instance->vm_instances->rsync_to_vm(0, SeDCloud::instance->using_private_ip(), local_path, arg_remote_path);
            SeDCloud::instance->actions->send_arguments(local_path, arg_remote_path);
        }

        SeDCloud::instance->actions->execute_remote_binary(remote_path_of_binary, "");

        for(int i = last_in + 1 ; i < nb_args; i++) {
            sprintf(sz_i, "/tmp/%i", i);
            arg_local_path = sz_i;

            sprintf(sz_i, "/%i", i);
            arg_remote_path = remote_path_of_binary + sz_i;

            //SeDCloud::instance->vm_instances->rsync_from_vm(0, SeDCloud::instance->using_private_ip(), arg_remote_path, arg_local_path);
            SeDCloud::instance->actions->receive_result(arg_remote_path, arg_local_path);

            if (diet_file_set(diet_parameter(pb, i), arg_local_path.c_str(), DIET_VOLATILE)) {
                printf("diet_file_desc_set error\n");
                return 1;
            }
        }

        for(int i = 0; i <= last_in; i++){
            diet_free_data(diet_parameter(pb, i));
        }


        SeDCloud::instance->actions->perform_action_on_end_solve(pb);
    }


    static SeDCloud* instance;

public:

    //static std::map<std::string, CloudServiceBinary> cloud_service_binaries;

    /*
    void deployServiceBinary(std::string name) {

    }
    */
 };

//template <class Actions>
std::map<std::string, CloudServiceBinary> SeDCloudActions::cloud_service_binaries;

SeDCloud* SeDCloud::instance;




#endif
