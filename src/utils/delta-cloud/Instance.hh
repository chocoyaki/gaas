/*
	Authors :
		- Adrian Muresan : adrian.muresan@ens-lyon.fr
		- Lamiel Toch : lamiel.toch@ens-lyon.fr		
*/


#ifndef _INSTANCE_HH_
#define _INSTANCE_HH_


#include <vector>
#include <string>


namespace IaaS {



class Instance {
  public:
    const std::string image_id;
    const std::string id;
    const std::string private_ip;
    const std::string public_ip;
	
    /* ctor */
    Instance(const std::string & _image_id, const std::string & _id,
        const std::string & _private_ip, const std::string & _public_ip) :
      image_id(_image_id), id(_id), private_ip(_private_ip), public_ip(_public_ip) {
    };
    
    bool isPingable();
    bool isReady();
};

}

#endif // _INSTANCE_HH_


