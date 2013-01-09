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

class Parameter{
	public:
	
	 std::string name;
	 std::string value;
	
	Parameter(const std::string& n, const std::string& v) : name(n), value(v) {
		
	}
	
	
	Parameter(const Parameter& param) : name(param.name), value(param.value){
		
	}
	
	/*
	Parameter& operator=(const Parameter& param) {
		
	}*/
};


class Instance {
  public:
    const std::string image_id;
    const std::string id;
    std::string private_ip;
    std::string public_ip;
	
    /* ctor */
    Instance(const std::string & _image_id, const std::string & _id,
        const char * _private_ip, const char * _public_ip) :
      image_id(_image_id), id(_id) /*, private_ip(_private_ip), public_ip(_public_ip)*/ {
      
      if (_private_ip == NULL) {
	      private_ip = std::string("???.???.???.???");
      }
      else {
      	private_ip = std::string(_private_ip);
      }
      
      if (_public_ip == NULL){  
	  		public_ip = std::string("???.???.???.???");
	  }
	  else {
	  	public_ip = std::string(_public_ip);
	  }
      
    };
    
    
    
    //bool isPingable();
    //bool isReady();
};

}

#endif // _INSTANCE_HH_


