#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <libdeltacloud/libdeltacloud.h>

#include "consts.hh"
#include "Iaas_deltacloud.hh"
#include "Image.hh"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

vector<Instance*> * Iaas_deltacloud::get_all_instances() {
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

void Iaas_deltacloud::get_instance_state(const std::string id, char * state) {
  deltacloud_api api;
  if(!init_api(&api)) {
    return;
  }
  
  struct deltacloud_instance instance;
  
	if(deltacloud_get_instance_by_id(&api, id.c_str(),
				  &instance) < 0) {
		cerr<<"Failed to get instance: " << id << endl;
		sprintf(state, "%s", "ERROR");
	}else {
		sprintf(state, "%s", instance.state);
	}
				  
  
}


Instance* Iaas_deltacloud::get_instance_by_id(const std::string& instanceId) {
	deltacloud_api api;
	if(!init_api(&api)) {
    	return NULL;
  	}
  	
  	struct deltacloud_instance instance;
  	
  	if(deltacloud_get_instance_by_id(&api, instanceId.c_str(),
				  &instance) < 0) {
		cerr<<"Failed to get instance: " << instanceId << endl;
	}else {
		
		const char* public_address = NULL;
		const char* private_address = NULL;
		
		if (instance.private_addresses != NULL) {
			private_address = instance.private_addresses->address;
		}
		
		if (instance.public_addresses != NULL) {
			public_address = instance.public_addresses->address;
		}
		
		return new Instance(instance.image_id, instance.id, private_address, public_address);
	}
  	
}




void Iaas_deltacloud::wait_instance_running(const std::string& instanceId) {
 	bool ready = false;
 	char state[MAX_NAME];
 	do{
 		get_instance_state(instanceId, state);
 		
 		std::cout << "waiting : state " << instanceId << " = " << state << endl;
 		
 		if (strcmp(state, "RUNNING") == 0) {
 			ready = true;
 		}
 		else {
 			sleep(1);
 		}
 		
 	}while(!ready);
 }


void IaaS::convert(const Parameter& src, struct deltacloud_create_parameter& dst) {
	dst.name = new char [src.name.length() + 1];
	strcpy(dst.name, src.name.c_str());
	
	dst.value = new char[src.value.length() + 1];
	strcpy(dst.value, src.value.c_str());
}

void IaaS::free_delta_chars_in_param(struct deltacloud_create_parameter& param) {
	delete [] param.name;
	
	param.name = NULL;
	
	delete [] param.value;
	
	param.value = NULL;
}

void IaaS::free_delta_params(struct deltacloud_create_parameter* params, int params_count) {
	for(int i = 0; i < params_count; i++) {
		free_delta_chars_in_param(params[i]);
	}
	
	delete [] params;
}

struct deltacloud_create_parameter* IaaS::create_delta_params(const std::vector<Parameter>& params) {
  struct deltacloud_create_parameter* delta_params = NULL;
  if (params.size() > 0) {
  	int params_count = params.size();
  	delta_params = new struct deltacloud_create_parameter [params_count];
  	for(int i = 0; i < params_count; i++) {
  		convert(params[i], delta_params[i]);
  	}
  }
  
  
  return delta_params;
}

vector<string*> * Iaas_deltacloud::run_instances(const string & image_id, int count, const std::vector<Parameter>& params) {
  deltacloud_api api;
  if(!init_api(&api))
    return NULL;

  char * instance_id = new char[MAX_NAME];
  vector<string*> * inst_arr = new vector<string*>();

  
  int params_count = params.size();
  struct deltacloud_create_parameter* delta_params = create_delta_params(params);
  

  for(int i_nb = 0; i_nb < count ; ++ i_nb) {
    if(deltacloud_create_instance(&api, image_id.c_str(), delta_params, params_count, &instance_id) < 0) {
      cerr<<"Failed to get deltacloud images: "<<deltacloud_get_last_error_string()<<endl;
    }
    inst_arr->push_back(new string(instance_id));
  }
  
  deltacloud_free(&api);
  
  if (delta_params != NULL) {
  	free_delta_params(delta_params, params_count);
  }
  return inst_arr;
}

int Iaas_deltacloud::terminate_instances(const vector<string*> & instance_ids) {
  deltacloud_api api;
  if(!init_api(&api))
    return 2;

  /* delete all the instances in the array */
  deltacloud_instance instance;
  for(int i_nb = 0; i_nb < instance_ids.size() ; ++ i_nb) {
    if(deltacloud_get_instance_by_id(&api, instance_ids[i_nb]->c_str(), &instance) < 0) {
      cerr<<"Failed to get info about instance "<< instance_ids[i_nb] <<endl;
    } else if(deltacloud_instance_stop(&api, &instance) < 0) {
      cerr<<"Failed to stop instance "<< *instance_ids[i_nb]<<endl;
    } else if(deltacloud_instance_destroy(&api, &instance) < 0) {
      cerr<<"Failed to destroy instance "<< *instance_ids[i_nb]<<endl;
    }
  }
  
  deltacloud_free(&api);
  return 0;
}

