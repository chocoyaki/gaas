/*


authors : Adrian Muresan, Lamiel Toch

*/

#ifndef _IAAS_INTERFACE_HH_
#define _IAAS_INTERFACE_HH_

#include <vector>
#include <string>

#include "Instance.hh"
#include "Image.hh"


namespace IaaS {


class IaasInterface {

  public:
    IaasInterface() {};

    /* retrieve all images */
    virtual std::vector<Image*> * get_all_images() = 0;

    /* retrieve all instances */
    virtual std::vector<Instance*> * get_all_instanges() = 0;

    /* launches a specified number of instances from an image */
    virtual std::vector<std::string*> * run_instances(const std::string & image_id, int count) = 0;

    /* terminates a set of instances */
    virtual int terminate_instances(const std::vector<std::string*> & instance_ids) = 0;
	
	/* get an Instance by Selecting Id */
	
	virtual Instance* get_instance_by_id(const std::string& instanceId) = 0;
	
	/* wait that the instance get ready*/
		
	virtual void wait_instance_running(const std::string& instanceId) = 0;
	
    virtual ~IaasInterface() {};
};

}

#endif // _IAAS_INTERFACE_HH_
