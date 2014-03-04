/*


authors : Adrian Muresan, Lamiel Toch

*/

#ifndef _IAAS_INTERFACE_HH_
#define _IAAS_INTERFACE_HH_

#include <vector>
#include <string>

#include "CloudAPIConnection.hh"
#include "Parameter.hh"
#include <boost/shared_ptr.hpp>

namespace IaaS {

  class Instance;
  typedef boost::shared_ptr<Instance> pInstance_t;

  class Image;
  typedef boost::shared_ptr<Image> pImage_t;

  class Parameter;

  class IaasInterface;
  typedef boost::shared_ptr<IaasInterface> pIaasInterface;


  class IaasInterface {

    public:
      IaasInterface() {};

      /* retrieve all images */
      std::vector<pImage_t> get_all_images();

      /* retrieve all instances */
      std::vector<pInstance_t> get_all_instances();

      /* launches a specified number of instances from an image */
      std::vector<std::string> run_instances(const std::string & image_id, int count, const std::vector<Parameter>& params = std::vector<Parameter>());

      /* terminates a set of instances */
      int terminate_instances(const std::vector<std::string> & instance_ids);

      int terminate_instances_by_ips(const std::vector<std::string>& ips, bool select_private_ip = false);

      /* get the instance id from the ip*/
      std::string get_id_from_ip(const std::string& ip, bool select_private_ip = false);

      /* get an Instance by Selecting Id */

      Instance* get_instance_by_id(const std::string& instance_id);

      /* wait that the instance get ready*/

      int wait_instance_running(const std::string& instance_id);

      std::string get_instance_state(const std::string& instance_id);

      virtual ~IaasInterface();

      //virtual IaasInterface * clone() const = 0;

      CloudAPIConnection get_cloud_api_connection() {
        return cloud_connection;
      }

      void set_cloud_api_connection(const CloudAPIConnection& cloud_connection) {
        this->cloud_connection = cloud_connection;
      }

    private:

      virtual std::vector<pImage_t> do_get_all_images() = 0;
      virtual std::vector<pInstance_t> do_get_all_instances() = 0;
      virtual std::vector<std::string> do_run_instances(const std::string & image_id, int count, const std::vector<Parameter>& params) = 0;
      virtual int do_terminate_instances(const std::vector<std::string> & instance_ids) = 0;
      virtual int do_terminate_instances_by_ips(const std::vector<std::string>& ips, bool select_private_ip) = 0;
      virtual std::string do_get_id_from_ip(const std::string& ip, bool select_private_ip) = 0;
      virtual Instance* do_get_instance_by_id(const std::string& instance_id) = 0;
      virtual int do_wait_instance_running(const std::string& instance_id) = 0;
      virtual std::string do_get_instance_state(const std::string& instance_id) = 0;

    protected:
      CloudAPIConnection cloud_connection;
  };
}

#endif // _IAAS_INTERFACE_HH_
