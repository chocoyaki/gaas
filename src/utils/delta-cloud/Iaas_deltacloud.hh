#ifndef _IAAS_DELTACLOUD_HH_
#define _IAAS_DELTACLOUD_HH_

struct deltacloud_api;


#include <vector>
#include <string>

#include "Instance.hh"
#include "Image.hh"
#include "IaasInterface.hh"


namespace IaaS {


 // using namespace IaaS;


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

      /* retrieve all images */
      virtual std::vector<Image*> * get_all_images();

      /* retrieve all instances */
      virtual std::vector<Instance*> * get_all_instanges();

      /* launches a specified number of instances from an image */
      virtual std::vector<std::string*> * run_instances(const std::string & image_id, int count);

      /* terminates a set of instances */
      virtual int terminate_instances(const std::vector<std::string*> & instance_ids);
	
	  virtual Instance* get_instance_by_id(const std::string& instanceId);
	
	  virtual void wait_instance_running(const std::string& instanceId);
	
	  
	
      virtual ~Iaas_deltacloud() {};
      
      
     protected:
	
	 
	 void get_instance_state(const std::string id, char * state);
   	
     
  };

};

#endif // _IAAS_DELTACLOUD_HH_
