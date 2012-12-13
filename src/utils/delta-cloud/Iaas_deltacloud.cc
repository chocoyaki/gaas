#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <libdeltacloud/libdeltacloud.h>

#include "consts.hh"
#include "Iaas_deltacloud.hh"
#include "Image.hh"

using namespace std;
using namespace IaaS;

bool Iaas_deltacloud::init_api(deltacloud_api * api) {
  if (deltacloud_initialize(api,
        const_cast<char*>(url_api_base.c_str()),
        const_cast<char*>(username.c_str()),
        const_cast<char*>(password.c_str())) < 0) {
    cerr<<"Failed to initialize libdeltacloud: "<<deltacloud_get_last_error_string()<<endl;
    return false;
  }
  return true;
}

vector<Image*> * Iaas_deltacloud::get_all_images() {
  deltacloud_api api;
  if(!init_api(&api)) {
    return NULL;
  }

  deltacloud_image * images = NULL;
  if (deltacloud_get_images(&api, &images) < 0) {
    cerr<<"Failed to get deltacloud images: "<<deltacloud_get_last_error_string()<<endl;
    deltacloud_free(&api);
    return NULL;
  }

  deltacloud_image * img = NULL;
  img = images;
  vector<Image*> * img_arr = new vector<Image*>();

  /* now traverse the list and add the images to the array */
  while(img != NULL) {
    img_arr->push_back(new Image(img->name, img->id));
    img = img->next;
  }

  deltacloud_free_image_list(&images);
  deltacloud_free(&api);

  return img_arr;
}

vector<Instance*> * Iaas_deltacloud::get_all_instanges() {
  deltacloud_api api;
  if(!init_api(&api)) {
    return NULL;
  }

  deltacloud_instance * instances;
  if (deltacloud_get_instances(&api, &instances) < 0) {
    cerr<<"Failed to get deltacloud images: "<<deltacloud_get_last_error_string()<<endl;
    deltacloud_free(&api);
    return NULL;
  }

  deltacloud_instance * inst = NULL;
  inst = instances;
  vector<Instance*> * inst_arr = new vector<Instance*>();

  while(inst != NULL) {
    inst_arr->push_back(
        new Instance(
          inst->image_id,
          inst->id,
          inst->private_addresses->address,
          inst->public_addresses->address)
        );
    inst = inst->next;
  }

  deltacloud_free_instance_list(&instances);
  deltacloud_free(&api);

  return inst_arr;
}

vector<string*> * Iaas_deltacloud::run_instances(const string & image_id, int count) {
  deltacloud_api api;
  if(!init_api(&api))
    return NULL;

  char * instance_id = new char[MAX_NAME];
  vector<string*> * inst_arr = new vector<string*>();

  for(int i_nb = 0; i_nb < count ; ++ i_nb) {
    if(deltacloud_create_instance(&api, image_id.c_str(), NULL, 0, &instance_id) < 0) {
      cerr<<"Failed to get deltacloud images: "<<deltacloud_get_last_error_string()<<endl;
    }
    inst_arr->push_back(new string(instance_id));
  }
  
  deltacloud_free(&api);
  return inst_arr;
}

int Iaas_deltacloud::terminate_instances(const vector<string> & instance_ids) {
  deltacloud_api api;
  if(!init_api(&api))
    return 2;

  /* delete all the instances in the array */
  deltacloud_instance instance;
  for(int i_nb = 0; i_nb < instance_ids.size() ; ++ i_nb) {
    if(deltacloud_get_instance_by_id(&api, instance_ids[i_nb].c_str(), &instance) < 0) {
      cerr<<"Failed to get info about instance "<<instance_ids[i_nb].c_str()<<endl;
    } else if(deltacloud_instance_stop(&api, &instance) < 0) {
      cerr<<"Failed to stop instance "<<instance_ids[i_nb]<<endl;
    } else if(deltacloud_instance_destroy(&api, &instance) < 0) {
      cerr<<"Failed to destroy instance "<<instance_ids[i_nb]<<endl;
    }
  }
  
  deltacloud_free(&api);
  return 0;
}

