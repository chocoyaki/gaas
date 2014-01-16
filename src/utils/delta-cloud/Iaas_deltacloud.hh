#ifndef _IAAS_DELTACLOUD_HH_
#define _IAAS_DELTACLOUD_HH_

struct deltacloud_api;


#include <vector>
#include <string>

#include "Instance.hh"
#include "Image.hh"
#include "IaasInterface.hh"

namespace IaaS {

  void convert(const Parameter& src, struct deltacloud_create_parameter& dst);
  struct deltacloud_create_parameter* create_delta_params(const std::vector<Parameter>& params);
  void free_delta_chars_in_param(struct deltacloud_create_parameter& param);
  void free_delta_params(struct deltacloud_create_parameter* params, int params_count);

  class Iaas_deltacloud : public IaasInterface {

    /* config parameters */
    std::string url_api_base;
    std::string username;
    std::string password;

    /* initializes the API structure - repetitie stuff */
    bool init_api(deltacloud_api * api);

    public:

    Iaas_deltacloud(const std::string & _url_api_base, const std::string & _username, const std::string & _password)
      : url_api_base(_url_api_base), username(_username), password(_password) {
      }

    virtual ~Iaas_deltacloud() {};

    /* retrieve all images */
    virtual std::vector<Image*> * get_all_images();

    /* retrieve all instances */
    virtual std::vector<Instance*> * get_all_instances();


    /* get the instance id from the ip*/
    virtual std::string get_id_from_ip(const std::string& ip, bool select_private_ip = false);

    /* launches a specified number of instances from an image */
    virtual std::vector<std::string*> * run_instances(const std::string & image_id, int count, const std::vector<Parameter>& params);

    /* terminates a set of instances */
    virtual int terminate_instances(const std::vector<std::string*> & instance_ids);

    virtual int terminate_instances_by_ips(const std::vector<std::string>& ips, bool select_private_ip = false);

    virtual Instance* get_instance_by_id(const std::string& instanceId);

    virtual int wait_instance_running(const std::string& instanceId);

    virtual std::string get_instance_state(const std::string& instance_id);

    protected:

    void get_instance_state(const std::string id, char * state);

  };

};

#endif // _IAAS_DELTACLOUD_HH_
