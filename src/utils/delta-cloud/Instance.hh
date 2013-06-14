/**
 * @file Instance.hh
 *
 * @brief  DIET Instance class header
 *
 * @author  Adrian Muresan : adrian.muresan@ens-lyon.fr
 * 			Lamiel TOCH (lamiel.toch@ens-lyon.fr)
 *          Yulin ZHANG (huaxi.zhang@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _INSTANCE_HH_
#define _INSTANCE_HH_


#include <vector>
#include <string>
#include <stdio.h>

namespace IaaS {

#define KEYNAME_PARAM "keyname"
#define HARDWARE_PROFILE_ID_PARAM "hwp_id"


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

	/*Image identity that this instance is instantiated from*/
    std::string image_id;

    /*Identity of the instance*/
    std::string id;

    /*Private ip address of the instance*/
    std::string private_ip;

    /*Public ip address of the instance*/
    std::string public_ip;

    /* Constructor of the instance with instantiated image identity, its identity, private and public ip addresses */
    Instance(const std::string & _image_id, const std::string & _id,
        const char * _private_ip, const char * _public_ip) {

		//printf("hi\n");

	image_id = _image_id;
	id =_id;

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

    std::string get_ip(bool select_private_ip = false) {
		if (select_private_ip == true) {
			return private_ip;
		}
		else {
			return public_ip;
		}
    }

    //bool isPingable();
    //bool isReady();
};

}

#endif // _INSTANCE_HH_


