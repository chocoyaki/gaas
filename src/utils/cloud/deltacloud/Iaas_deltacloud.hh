#ifndef _IAAS_DELTACLOUD_HH_
#define _IAAS_DELTACLOUD_HH_

#include "IaasInterface.hh"

#include <vector>
#include <string>
#include <stdexcept>


struct deltacloud_api;
struct deltacloud_create_parameter;

class deltacloud_exception : public std::runtime_error {
  public:
    deltacloud_exception(const std::string & function, const std::string & message) :
      std::runtime_error(std::string("In ") + function + std::string(": ") + message) {}
};
namespace IaaS {

  void convert(const Parameter& src, struct deltacloud_create_parameter& dst);
  struct deltacloud_create_parameter* create_delta_params(const std::vector<Parameter>& params);
  void free_delta_chars_in_param(struct deltacloud_create_parameter& param);
  void free_delta_params(struct deltacloud_create_parameter* params, int params_count);


  class Iaas_deltacloud : public IaasInterface {

    public:

    Iaas_deltacloud(const std::string & _url_api_base, const std::string & _username, const std::string & _password) {
      this->set_cloud_api_connection(CloudAPIConnection(_url_api_base, _username, _password));
    }

    Iaas_deltacloud(const CloudAPIConnection& cloud_connection) {
      this->set_cloud_api_connection(cloud_connection);
    }


    virtual ~Iaas_deltacloud() {};

    private:

    /* retrieve all images */
    virtual std::vector<pImage_t> do_get_all_images();

    /* retrieve all instances */
    virtual std::vector<pInstance_t> do_get_all_instances();


    /* get the instance id from the ip*/
    virtual std::string do_get_id_from_ip(const std::string& ip, bool select_private_ip = false);

    /* launches a specified number of instances from an image */
    virtual std::vector<std::string> do_run_instances(const std::string & image_id, int count, const std::vector<Parameter>& params);

    /* terminates a set of instances */
    virtual int do_terminate_instances(const std::vector<std::string> & instance_ids);

    virtual int do_terminate_instances_by_ips(const std::vector<std::string>& ips, bool select_private_ip = false);

    virtual Instance* do_get_instance_by_id(const std::string& instanceId);

    virtual int do_wait_instance_running(const std::string& instanceId);

    virtual std::string do_get_instance_state(const std::string& instance_id);

//    virtual Iaas_deltacloud * clone() const;

    protected:

    void get_instance_state(const std::string id, char * state);

    private:

    /* initializes the API structure - repetitie stuff */
    bool init_api(deltacloud_api * api);


  };

};

#endif // _IAAS_DELTACLOUD_HH_
