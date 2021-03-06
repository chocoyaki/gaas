/**
 * @file Instance.hh
 *
 * @brief  DIET Instance class header
 *
 * @author  Adrian Muresan : adrian.muresan@ens-lyon.fr
 * 			    Lamiel TOCH (lamiel.toch@ens-lyon.fr)
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
          const char * _private_ip, const char * _public_ip);

      std::string get_ip(bool select_private_ip = false);
      //bool isPingable();
      //bool isReady();
  };

}

#endif // _INSTANCE_HH_


