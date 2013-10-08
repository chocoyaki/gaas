/*
 * @file SeDCloud.hh
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _SEDCLOUD_HH_
#define _SEDCLOUD_HH_

#include "DIET_server.h"
#include "SeDCloudActions.hh"
#include "CloudAPIConnection.hh"

#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif

#include <stdio.h>

typedef int (* dietcloud_callback_t)(diet_profile_t*);

//the controller which calls actions
class SeDCloud {


protected:
    SeDCloudActions* actions;

#ifdef USE_LOG_SERVICE
    const DietLogComponent* get_log_component() const;
#endif

    SeDCloud(SeDCloudActions* _actions) {
            instance = NULL;
            actions = _actions;
            actions->perform_action_on_sed_creation();
    }


public:
    //void addParameter(const std::string& param, const std::string& value);


    static void launch(int argc, char* argv[]) {
      // Assuming config file is the first arg
      // We need the name to launch seds from SeDCloud
      SeDCloud::instance->config_file = std::string(argv[1]);

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



//  SeDCloudActions& get_actions() const{
//    return *actions;
//  }

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
                         const std::string& _installer_relative_path = ""
                         ) ;


  DIET_API_LIB int service_homogeneous_vm_instanciation_with_keyname_add();

  //add a service which allows to instantiate homogeneous vms
  //only for test
  DIET_API_LIB int service_homogeneous_vm_instanciation_add();

  //add a service which allows to instantiate homogeneous vms
  //this sedCloud is linked to only one Cloud API
  DIET_API_LIB int service_homogeneous_vm_instanciation_add(CloudAPIConnection* cloud_api_connection);


  //add a service which allows to destroy homogeneous vms
  //only for test
  DIET_API_LIB int service_vm_destruction_by_ip_add();


  DIET_API_LIB int service_cloud_federation_vm_destruction_by_ip_add(std::vector<CloudAPIConnection>* cloud_api_connection);


  DIET_API_LIB int service_rsync_to_vm_add();
  DIET_API_LIB int service_get_tarball_from_vm_add();

  DIET_API_LIB int service_mount_nfs_add();

  DIET_API_LIB int service_launch_another_sed_add();
protected:
    static int solve(diet_profile_t *pb);
  static std::vector<CloudAPIConnection>* cloud_api_connection_for_vm_destruction;
  static CloudAPIConnection* cloud_api_connection_for_vm_instanciation;

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

  std::string config_file;

  //TODO to link to user or group
  //static std::map<std::string, IaaS::VMInstances*> reserved_vms;


public:

    //static std::map<std::string, CloudServiceBinary> cloud_service_binaries;

    /*
    void deployServiceBinary(std::string name) {

    }
    */
 };

#endif /* _SEDCLOUD_HH_ */
