/**
 * @file Instance.cc
 *
 * @brief  Instance.cc for delta-cloud
 *
 * @author  Lamiel Toch (lamiel.toch@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */




#include "Instance.hh"

using namespace IaaS;

Instance::Instance(const std::string & _image_id, const std::string & _id,
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

std::string
Instance::get_ip(bool select_private_ip) {
  if (select_private_ip == true) {
    return private_ip;
  }
  else {
    return public_ip;
  }
}

/*
bool Instance::isPingable() {
	return false;
}


bool Instance::isReady() {
	return false;
}

*/
