#include "IaasInterface.hh"

namespace IaaS {
std::vector<pImage_t>
IaasInterface::get_all_images() {
  return do_get_all_images();
}

/* retrieve all instances */
std::vector<pInstance_t>
IaasInterface::get_all_instances() {
  return do_get_all_instances();
}

/* launches a specified number of instances from an image */
std::vector<std::string>
IaasInterface::run_instances(const std::string & image_id, int count,
    const std::vector<Parameter>& params) {
  return do_run_instances(image_id, count, params);
}

/* terminates a set of instances */
int
IaasInterface::terminate_instances(const std::vector<std::string> & instance_ids){
  return do_terminate_instances(instance_ids);
}

int
IaasInterface::terminate_instances_by_ips(const std::vector<std::string>& ips, bool select_private_ip) {
  return do_terminate_instances_by_ips(ips, select_private_ip);
}

/* get the instance id from the ip*/
std::string
IaasInterface::get_id_from_ip(const std::string& ip, bool select_private_ip) {
  return do_get_id_from_ip(ip, select_private_ip);
}

/* get an Instance by Selecting Id */
Instance*
IaasInterface::get_instance_by_id(const std::string& instance_id) {
  return do_get_instance_by_id(instance_id);
}

/* wait that the instance get ready*/
int
IaasInterface::wait_instance_running(const std::string& instance_id) {
  return do_wait_instance_running(instance_id);
}

std::string
IaasInterface::get_instance_state(const std::string& instance_id) {
  return do_get_instance_state(instance_id);
}

IaasInterface::~IaasInterface() {}
}
