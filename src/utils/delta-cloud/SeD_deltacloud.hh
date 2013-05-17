/**
 * @file Sed_deltacloud.hh
 *
 * @brief  DIET SeD_deltacloud class header
 *
 * @author  Lamiel TOCH (lamiel.toch@ens-lyon.fr)
 *          Yulin ZHANG (huaxi.zhang@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
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

#include "DagdaImpl.hh"
#include "DagdaFactory.hh"


int create_folder(const char* folder_path);
std::string get_folder_in_dagda_path(const char* folder_name);
std::string int2string(int i);
void append2path(std::string& path, const std::string& add);

//0 iif succeeds
int create_folder_in_dagda_path(const char* folder_name);
int create_folder_in_dagda_path_with_request_id(int reqId);

typedef int (* dietcloud_callback_t)(diet_profile_t*);

/**
	The manner with which arguments are tranfered to vms

*/
enum ArgumentsTranferMethod {
	filesTransferMethod = 0, //files are copied to SedCloud and tranfered to vms
	pathsTransferMethod //files are located in one place (eg on a NFS Server), and vms and SedCloud access to this places
};

/*
The folder must contains an executable and a shell script
which call the executable. The name of this script is "exec.sh".
The parameters are files and are named as follows :
0 for the first parameter
1 for the second parameter
2 for the third and so on.
This executable script is created to launch a service. The parameters are the parameters of the service.
*/
class CloudServiceBinary {
   public:
    //the local folder path which contains the executable
    //if this attribute is set to "" then it is a preinstalled service binary
    std::string local_path_of_binary;

    //the remote folder path which contains the executable (in Virtual Machine)
    std::string remote_path_of_binary;

    //the relative path of the binary (relative to remote_path_of_binary)
    std::string entry_point_relative_file_path;

	//the remote folder which contains the data as parameters
	std::string remote_path_of_arguments;

	dietcloud_callback_t prepocessing;
	dietcloud_callback_t postprocessing;

    /*Constructor specified by given local_path and remote_path*/
    CloudServiceBinary(const std::string& _local_path, const std::string& _remote_path,
					 const std::string& _entry_point_relative_file_path="exec.sh", const std::string& _remote_path_of_arguments = "", dietcloud_callback_t _prepocessing = NULL,
                         dietcloud_callback_t _postprocessing = NULL);
    CloudServiceBinary(const CloudServiceBinary& binary);
    CloudServiceBinary();


	ArgumentsTranferMethod argumentsTransferMethod;

	int install(const std::string& ip, const std::string& user_name) const;
    int execute_remote(const std::string& ip, const std::string& user_name, const std::vector<std::string>& args) const;

    bool isPreinstalled() const;




};

/*
	PreinstalledCloudServiceBinary : this is a service which does not need to be deployed on the vm
*/
/*class PreinstalledCloudServiceBinary : public CloudServiceBinary {
	public:

	PreinstalledCloudServiceBinary(const std::string& _remote_path,
					 const std::string& _entry_point_relative_file_path="exec.sh", const std::string& _remote_path_of_arguments = "");
};*/



/*********************************************************************************************/
/* A ServiceStatistics registers one service with how many times it has been called***********/
/*********************************************************************************************/
class ServiceStatistics{
public:
	/*Constructor to initiate a service with 0 time request*/
    ServiceStatistics(const std::string _service_name) {
        service_name = _service_name;
        call_number = 0;
    }

    /*Copy constructor*/
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
    void copy_binary_into_vm(std::string name, int vm_index);
public:
    //execute an action when the the client make a solve
    virtual int perform_action_on_begin_solve(diet_profile_t *pb) = 0;
    virtual int perform_action_on_end_solve(diet_profile_t *pb) = 0;

    virtual void perform_action_on_sed_creation() = 0;

    //at diet_SeD instruction
    virtual void perform_action_on_sed_launch() = 0;

    virtual int perform_action_after_service_table_add(const std::string& name_of_service) = 0;

	virtual int send_arguments(const std::string& local_path, const std::string& remote_path, int vm_index = 0);
    virtual int receive_result(const std::string& result_remote_path, const std::string& result_local_path, int vm_index = 0);
	virtual int execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args, int vm_index = 0);
	virtual int create_remote_directory(const std::string& remote_path, int vm_index = 0);

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


class SedCloudActionsNULL : public SeDCloudActions {
public:
	SedCloudActionsNULL() {};

	virtual int perform_action_on_begin_solve(diet_profile_t *pb) {return 0;};
    virtual int perform_action_on_end_solve(diet_profile_t *pb) {return 0;};
	virtual void perform_action_on_sed_creation() {};
	virtual void perform_action_on_sed_launch() {};
	virtual int perform_action_after_service_table_add(const std::string& name_of_service) { return 0;};
	virtual int send_arguments(const std::string& local_path, const std::string& remote_path, int vm_index = 0) { return 0; };
    virtual int receive_result(const std::string& result_remote_path, const std::string& result_local_path, int vm_index = 0) {return 0;};
	virtual int execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args, int vm_index = 0) {return 0;};
	virtual int create_remote_directory(const std::string& remote_path, int vm_index = 0) {return 0;};

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

	//vm_index not used here because we use only one pm
    virtual int send_arguments(const std::string& local_path, const std::string& remote_path, int vm_index);
    virtual int receive_result(const std::string& result_remote_path, const std::string& result_local_path, int vm_index);
    virtual int execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args, int vm_index);
	virtual int create_remote_directory(const std::string& remote_path, int vm_index);

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


class SeDCloudVMLaunchedAtSolveThenDestroyedActions : public SeDCloudVMLaunchedAtSolveActions {
public:
    virtual int perform_action_on_begin_solve(diet_profile_t *pb);
    virtual int perform_action_on_end_solve(diet_profile_t *pb);


    SeDCloudVMLaunchedAtSolveThenDestroyedActions(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
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

        if (_actions != NULL) {
			if (instance == NULL) {
				SeDCloud::instance = new SeDCloud(_actions);
			}
        }
    }

    static void erase() {
        if (instance != NULL) {
            delete instance;
            instance = NULL;
        }

        //std::map<std::string, SeDCloudActions*>::iterator iter;
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
                         const std::string& remote_path_of_binary,
                         const std::string& entryPoint = "exec.sh",
                         const std::string& remote_path_of_arguments = "",
                         ArgumentsTranferMethod argumentsTransferMethod = filesTransferMethod,
                         dietcloud_callback_t prepocessing = NULL,
                         dietcloud_callback_t postprocessing = NULL
                         ) ;

	//add a service which allows to instantiate homogeneous vms
	DIET_API_LIB int service_homogeneous_vm_instanciation_add();


protected:
    static int solve(diet_profile_t *pb);

    static SeDCloud* instance;

	//solve the service which allows to instantiate homogeneous vms
	static int homogeneous_vm_instanciation_solve(diet_profile_t *pb);
	static std::map<std::string, IaaS::VMInstances*> reserved_vms;

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
std::map<std::string, IaaS::VMInstances*> SeDCloud::reserved_vms;



#endif
