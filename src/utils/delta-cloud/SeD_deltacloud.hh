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


#define HOMOGENEOUS_VM_INSTANCIATION_SERVICE "homogeneous_vm_instanciation"





int create_folder(const char* folder_path);
std::string get_folder_in_dagda_path(const char* folder_name);
std::string int2string(int i);
void append2path(std::string& path, const std::string& add);

std::string create_tmp_file(diet_profile_t* pb, const std::string ext);
int write_lines(const std::vector<std::string>& ips, const std::string& file);

//0 iif succeeds
int create_folder_in_dagda_path(const char* folder_name);
int create_folder_in_dagda_path_with_request_id(int reqId);

std::string copy_to_tmp_file(const std::string& src, const std::string& ext);

typedef int (* dietcloud_callback_t)(diet_profile_t*);

class ServiceWrapper;

typedef int (*dietwrapper_callback_t)(ServiceWrapper*, diet_profile_t*);

/**
	The manner with which arguments are tranfered to vms

*/
enum ArgumentsTransferMethod {
	filesTransferMethod = 0, //files are copied to SedCloud and tranfered to vms
	pathsTransferMethod //files are located in one place (eg on a NFS Server), and vms and SedCloud access to this places
};

enum ServiceWrapperArgumentType {
	dietProfileArgType = 0,
	commandLineArgType
};

/*typedef union {
	int diet_profile_arg;
	char* command_line_arg;
} UServiceWrapperArgument;
*/

typedef struct{
	ServiceWrapperArgumentType arg_type;
	union{
		int diet_profile_arg;
		char* command_line_arg;
	};
} ServiceWrapperArgument;

class ServiceWrapper {

public:
	std::string name_of_service;
	std::string executable_path;
	dietwrapper_callback_t prepocessing;
	dietwrapper_callback_t postprocessing;





	ServiceWrapper(
	const std::string& _name_of_service,
	const std::string& _executable_path,
	dietwrapper_callback_t _prepocessing = NULL,
	dietwrapper_callback_t _postprocessing = NULL,
	int nb_args = 0) {
		name_of_service = _name_of_service;
		executable_path = _executable_path;
		prepocessing = _prepocessing;
		postprocessing = _postprocessing;
		args = std::vector<ServiceWrapperArgument>(nb_args);
	}
	ServiceWrapper() {};



	void set_arg(int index, const std::string& arg) {

		if (args[index].command_line_arg != NULL) {
			free(args[index].command_line_arg);
		}

		args[index].command_line_arg = strdup(arg.c_str());
		args[index].arg_type = commandLineArgType;
	}

	void set_arg(int index, int diet_profile_arg) {

		if (args[index].command_line_arg != NULL) {
			free(args[index].command_line_arg);
			args[index].command_line_arg = NULL;
		}


		args[index].diet_profile_arg = diet_profile_arg;
		args[index].arg_type = dietProfileArgType;
	}

	const ServiceWrapperArgument& get_arg(int index) const {
		return args[index];
	}

	int get_last_diet_in() const {
		int last_in = -1;

		for(int index = 0; index < args.size(); index++){
			if (args[index].arg_type == dietProfileArgType) {
				if (last_in < args[index].diet_profile_arg) {
					last_in = args[index].diet_profile_arg;
				}
			}
		}

		return last_in;
	}

	~ServiceWrapper() {
		std::vector<ServiceWrapperArgument>::iterator iter;
		for(int index = 0; index < args.size(); index++) {
			ServiceWrapperArgument& arg = args[index];
			if (arg.arg_type == commandLineArgType) {
				if(arg.command_line_arg != NULL) {
					free(arg.command_line_arg);
					arg.command_line_arg = NULL;
				}
			}
		}
	}

	const std::vector<ServiceWrapperArgument>& get_args() const {
		return args;
	}

	int get_nb_args() const {
		return args.size();
	}
private:
	//key: the index of the argument in command line; value: the argument
	std::vector<ServiceWrapperArgument> args;
};


DIET_API_LIB int
        service_wrapper_table_add(const std::string& name_of_service,
							int last_in,
							const std::vector<std::pair<diet_data_type_t, diet_base_type_t> >& out_types,
							const std::string& path_of_binary,
							dietwrapper_callback_t prepocessing = NULL,
							dietwrapper_callback_t postprocessing = NULL
                         );

DIET_API_LIB int
        service_wrapper_table_add(const ServiceWrapper& service_wrapper,
							const std::vector<std::pair<diet_data_type_t, diet_base_type_t> >& out_types
                         );





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

	//the relative path of the installer of the binary
	std::string installer_relative_path;

	std::string name;
	int last_in;
	int last_out;

    /*Constructor specified by given local_path and remote_path*/
    CloudServiceBinary(const std::string& _local_path, const std::string& _remote_path,
					 const std::string& _entry_point_relative_file_path="exec.sh", const std::string& _remote_path_of_arguments = "", dietcloud_callback_t _prepocessing = NULL,
                         dietcloud_callback_t _postprocessing = NULL, ArgumentsTransferMethod argsTranferMethod = filesTransferMethod, const std::string& _installer_relative_path = "");
    CloudServiceBinary(const CloudServiceBinary& binary);
    CloudServiceBinary();


	ArgumentsTransferMethod arguments_transfer_method;

	int install(const std::string& ip, const std::string& vm_user_name) const;
    int execute_remote(const std::string& ip, const std::string& vm_user_name, const std::vector<std::string>& args) const;

    bool is_preinstalled() const;

	bool has_installer() const;

	diet_profile_desc_t* to_diet_profile();

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

class CloudAPIConnection{
public:
	CloudAPIConnection(const std::string& url, const std::string& usr, const std::string& passwd){
		username = usr;
		password = passwd;
		base_url = url;
	}

	CloudAPIConnection() {};
	~CloudAPIConnection(){};


	std::string username; // eg : "oneadmin";
	std::string password; //eg : "mypassword";
    std::string base_url; // eg : "http://localhost:3001/api";
} ;




class SeDCloudActions {

private:
    /*******FOR DELTACLOUD********/
    std::string username; // eg : "oneadmin";
	std::string password; //eg : "mypassword";
	//a list of vm_instances;
	std::list<IaaS::VMInstances*> vm_instances;
	void fill_ips();

protected:
    /*******FOR DELTACLOUD********/
	std::string base_url; // eg : "http://localhost:3001/api";
    std::string image_id;

	std::string vm_user; // eg : "root";
	int vm_count; // eg : 1;
	//std::string profile; //eg :  "debian-rc";
	std::vector<IaaS::Parameter> params; //parameters for instantiating a VM with deltacloud

	bool is_ip_private;
	ServiceStatisticsMap statistics_on_services;

	std::vector<std::string> ips;
    std::string master_ip;

	void copy_binary_into_vm(std::string name, int vm_index);
	void copy_binary_into_all_vms(std::string name);
	void copy_all_binaries_into_vm(int vm_index);
	void copy_all_binaries_into_all_vms();
	//launch the vms
	int launch_vms();
	//destroy the vms
    void destroy_vms();
    int send_vm_ips_to_master();
public:

	void set_credentials(const std::string username, std::string passwd) {
		this->username = username;
		this->password = passwd;
	}

	const std::vector<std::string>& get_ips() const {
		return ips;
	}

	const std::string& get_master_ip() const {
		return master_ip;
	}

	//launch a set of vms
	int launch_vms(const std::string& vm_image, int vm_count, const std::string& deltacloud_api_url,
		const std::string& deltacloud_user_name, const std::string& deltacloud_passwd, const std::string& vm_user,
		bool is_ip_private, const std::vector<IaaS::Parameter>& params = std::vector<IaaS::Parameter>());

    //execute an action when the the client make a solve
    virtual int perform_action_on_begin_solve(diet_profile_t *pb) = 0;
    virtual int perform_action_on_end_solve(diet_profile_t *pb) = 0;

    virtual void perform_action_on_sed_creation() = 0;

    //at diet_SeD instruction
    virtual void perform_action_on_sed_launch() = 0;

    virtual int perform_action_after_service_table_add(const std::string& name_of_service) = 0;

	//when the OS of the VM is ready and can accept ssh connection
	virtual int perform_action_on_vm_os_ready() { return 0;};

	virtual int send_arguments(const std::string& local_path, const std::string& remote_path);
    virtual int receive_result(const std::string& result_remote_path, const std::string& result_local_path);
	virtual int execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args);
	virtual int create_remote_directory(const std::string& remote_path);

    SeDCloudActions();

    SeDCloudActions(const std::string& _image_id, const std::string& _base_url, const std::string& _username, const std::string& _password, const std::string& _vm_user,
                          int _vm_count, bool _is_ip_private, const std::vector<IaaS::Parameter>& params = std::vector<IaaS::Parameter>());


    ~SeDCloudActions() {
		destroy_vms();
    }


    bool using_private_ip() {
        return this->is_ip_private;
    }

	const std::map<std::string, CloudServiceBinary>& get_cloud_service_binaries() {
		return cloud_service_binaries;
	}

	void set_cloud_service_binary(const std::string& name, const CloudServiceBinary& binary) {
		cloud_service_binaries[name] = binary;
	}

	void unset_cloud_service_binary(const std::string& name) {
		cloud_service_binaries.erase(name);
	}

	void clone_service_binaries(const SeDCloudActions& src);
protected:
    std::map<std::string, CloudServiceBinary> cloud_service_binaries;




};


class SeDCloudActionsNULL : public SeDCloudActions {
public:
	SeDCloudActionsNULL() {};

	virtual int perform_action_on_begin_solve(diet_profile_t *pb) {return 0;};
    virtual int perform_action_on_end_solve(diet_profile_t *pb) {return 0;};
	virtual void perform_action_on_sed_creation() {};
	virtual void perform_action_on_sed_launch() {};
	virtual int perform_action_after_service_table_add(const std::string& name_of_service) { return 0;};
	virtual int send_arguments(const std::string& local_path, const std::string& remote_path) { return 0; };
    virtual int receive_result(const std::string& result_remote_path, const std::string& result_local_path) {return 0;};
	virtual int execute_remote_binary(const CloudServiceBinary& binary, const std::vector<std::string>& args) {return 0;};
	virtual int create_remote_directory(const std::string& remote_path) {return 0;};

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

#define MACHINE_ALIVE_INTERVAL 5

class SeDCloudMachinesActions : public SeDCloudAndVMLaunchedActions {
protected:
	int machine_alive_interval;

public:
    virtual void perform_action_on_sed_creation();
    virtual void perform_action_on_sed_launch();

	SeDCloudMachinesActions(const std::vector<std::string>& _ips, const std::string& _username) {
		ips = _ips;
		master_ip = ips[0];
		vm_user = _username;
		machine_alive_interval = MACHINE_ALIVE_INTERVAL;
	}

	SeDCloudMachinesActions(std::string ip_file_path,  const std::string& _username) {
		readlines(ip_file_path.c_str(), ips);
		master_ip = ips[0];
		vm_user = _username;
		machine_alive_interval = MACHINE_ALIVE_INTERVAL;
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

//execute a programm after VMs are launched
/*
class SeDCloudVMLaunchedThenExecProgramActions : public SeDCloudActionsNULL {
private:
		std::string program_path;
		std::vector<std::string> program_args;

public:

	//SeDCloudVMLaunchedThenExecProgramActions() {};

	SeDCloudVMLaunchedThenExecProgramActions(const std::string& program_path, const std::vector<std::string>& program_args) {
		this->program_path = program_path;
		this->program_args = program_args;
	}

	void set_program_path(const std::string& program_path) {
		this->program_path = program_path;
	}

	void set_program_args(const std::vector<std::string>& program_args) {
		this->program_args = program_args;
	}

	void set_program_arg(int index, const std::string& arg) {
		program_args[index] = arg;
	}

	const std::string& get_program_arg(int index) const{
		return program_args[index];
	}

	void push_back_arg(const std::string arg){
		program_args.push_back(arg);
	}

	//when the OS of the VM is ready and can accept ssh connection
	virtual int perform_action_on_vm_os_ready();
};*/


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

    /*static void erase() {
        if (instance != NULL) {
            delete instance;
            instance = NULL;
        }

        //std::map<std::string, SeDCloudActions*>::iterator iter;
    }*/

    static SeDCloud* get() {

        return SeDCloud::instance;

    }

	//warining : make sure that actions is destroyable (allocated by new)
	void setActions(SeDCloudActions* _actions) {
		if (actions != NULL) {
			delete actions;
		}
		actions = _actions;
	}

	SeDCloudActions& get_actions() const{
		return *actions;
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
                         ArgumentsTransferMethod arguments_transfer_method = filesTransferMethod,
                         dietcloud_callback_t prepocessing = NULL,
                         dietcloud_callback_t postprocessing = NULL,
                         const std::string& remote_path_of_binary_installer = ""
                         ) ;


	DIET_API_LIB int service_homogeneous_vm_instanciation_with_keyname_add();

	//add a service which allows to instantiate homogeneous vms
	//only for test
	DIET_API_LIB int service_homogeneous_vm_instanciation_add();

	//add a service which allows to instantiate homogeneous vms
	//this sedCloud is linked to only one Cloud API
	DIET_API_LIB int service_homogeneous_vm_instanciation_add(const CloudAPIConnection& cloud_api_connection);


	//add a service which allows to destroy homogeneous vms
	//only for test
	DIET_API_LIB int service_vm_destruction_by_ip_add();


	DIET_API_LIB int service_cloud_federation_vm_destruction_by_ip_add(const std::vector<CloudAPIConnection>& cloud_api_connection);


	DIET_API_LIB int service_rsync_to_vm_add();
	DIET_API_LIB int service_get_tarball_from_vm_add();

	DIET_API_LIB int service_mount_nfs_add();

	DIET_API_LIB int service_launch_another_sed_add();
protected:
    static int solve(diet_profile_t *pb);
	static std::vector<CloudAPIConnection> cloud_api_connection_for_vm_destruction;
	static CloudAPIConnection cloud_api_connection_for_vm_instanciation;

    static SeDCloud* instance;

	//solve the service which allows to instantiate homogeneous vms
	static int homogeneous_vm_instanciation_solve(diet_profile_t *pb);
	static int homogeneous_vm_instanciation_with_one_cloud_api_connection_solve(diet_profile_t* pb);
	static int vm_destruction_by_ip_solve(diet_profile_t *pb);
	static int cloud_federation_vm_destruction_by_ip_solve(diet_profile_t *pb);
	static int rsync_to_vm_solve(diet_profile_t *pb);
	static int get_tarball_from_vm_solve(diet_profile_t* pb);
	static int mount_nfs_solve(diet_profile_t *pb);
	static int launch_another_sed_solve(diet_profile_t* pb);

	static int homogeneous_vm_instanciation_with_keyname_solve(diet_profile_t *pb);

	//TODO to link to user or group
	//static std::map<std::string, IaaS::VMInstances*> reserved_vms;


public:

    //static std::map<std::string, CloudServiceBinary> cloud_service_binaries;

    /*
    void deployServiceBinary(std::string name) {

    }
    */
 };






//template <class Actions>
//std::map<std::string, CloudServiceBinary> SeDCloudActions::cloud_service_binaries;

SeDCloud* SeDCloud::instance;
//std::map<std::string, IaaS::VMInstances*> SeDCloud::reserved_vms;
std::vector<CloudAPIConnection> SeDCloud::cloud_api_connection_for_vm_destruction;
CloudAPIConnection SeDCloud::cloud_api_connection_for_vm_instanciation;

int time_solve(diet_profile_t *pb);





#endif
