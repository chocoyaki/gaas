/*
 * @file CloudServiceBinary.hh
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _CLOUDSERVICEBINARY_HH_
#define _CLOUDSERVICEBINARY_HH_

#include "DIET_server.h"

#include <stdio.h>
#include <string>
#include <vector>



typedef int (* dietcloud_callback_t)(diet_profile_t*);

/**
  The manner with which arguments are tranfered to vms

*/
enum ArgumentsTransferMethod {
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




#endif /* _CLOUDSERVICEBINARY_HH_ */
