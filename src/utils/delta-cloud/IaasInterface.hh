/*


authors : Adrian Muresan, Lamiel Toch

*/

#ifndef _IAAS_INTERFACE_HH_
#define _IAAS_INTERFACE_HH_

#include <vector>
#include <string>

#include "Instance.hh"
#include "Image.hh"
#include "CloudAPIConnection.hh"
#include "Parameter.hh"

namespace IaaS {


  class IaasInterface {

    public:
      IaasInterface() {};

      /* retrieve all images */
      virtual std::vector<Image*> * get_all_images() = 0;

      /* retrieve all instances */
      virtual std::vector<Instance*> * get_all_instances() = 0;

      /* launches a specified number of instances from an image */
      virtual std::vector<std::string*> * run_instances(const std::string & image_id, int count, const std::vector<Parameter>& params = std::vector<Parameter>()) = 0;

      /* terminates a set of instances */
      virtual int terminate_instances(const std::vector<std::string*> & instance_ids) = 0;

      virtual int terminate_instances_by_ips(const std::vector<std::string>& ips, bool select_private_ip = false) = 0;

      /* get the instance id from the ip*/
      virtual std::string get_id_from_ip(const std::string& ip, bool select_private_ip = false) = 0;

      /* get an Instance by Selecting Id */

      virtual Instance* get_instance_by_id(const std::string& instanceId) = 0;

      /* wait that the instance get ready*/

      virtual int wait_instance_running(const std::string& instanceId) = 0;

      virtual ~IaasInterface() {};

      virtual std::string get_instance_state(const std::string& instance_id) = 0;

      virtual IaasInterface * clone() const = 0;

      CloudAPIConnection get_cloud_api_connection() {
        return cloud_connection;
      }

      void set_cloud_api_connection(const CloudAPIConnection& cloud_connection) {
        this->cloud_connection = cloud_connection;
      }

    protected:
      CloudAPIConnection cloud_connection;
  };
}

#endif // _IAAS_INTERFACE_HH_
