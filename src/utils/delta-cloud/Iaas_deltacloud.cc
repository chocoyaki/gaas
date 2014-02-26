#include "deltacloud_config.h"

#include "Iaas_deltacloud.hh"

#include "consts.hh"
#include "Image.hh"

#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#include "Tools.hh"
#endif

#include <libdeltacloud/libdeltacloud.h>
#include <boost/current_function.hpp>

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace IaaS;


bool Iaas_deltacloud::init_api(deltacloud_api * api) {
  char * url = strdup(cloud_connection.base_url.c_str());
  char * username = strdup(cloud_connection.username.c_str());
  char * password = strdup(cloud_connection.password.c_str());
  int res_init = deltacloud_initialize(api, url, username, password);
  free(url);
  free(username);
  free(password);
  if (res_init < 0) {
    cerr<<"Failed to initialize libdeltacloud: "<<deltacloud_get_last_error_string()<<endl;
    return false;
  }
  return true;
}

vector<pImage_t> Iaas_deltacloud::get_all_images() {
  deltacloud_api api;
  if(!init_api(&api)) {
    throw deltacloud_exception(BOOST_CURRENT_FUNCTION, "Deltacloud API is not initialized");
  }
  deltacloud_image * images = NULL;
  if (deltacloud_get_images(&api, &images) < 0) {
    std::string message = deltacloud_get_last_error_string();
    deltacloud_free(&api);
    throw deltacloud_exception(BOOST_CURRENT_FUNCTION, deltacloud_get_last_error_string());
  }

  deltacloud_image * img = images;
  vector<pImage_t> img_arr;

  /* now traverse the list and add the images to the array */
  while(img != NULL) {
    img_arr.push_back(pImage_t(new Image(img->name, img->id)));
    img = img->next;
  }

  deltacloud_free_image_list(&images);
  deltacloud_free(&api);

  return img_arr;
}

vector<pInstance_t> Iaas_deltacloud::get_all_instances() {
  deltacloud_api api;

  deltacloud_instance * instances;
  bool error = false;

  //we do a loop to avoid : error Failed to get expected root element for instances
  do {

    if(!init_api(&api)) {
      throw deltacloud_exception(BOOST_CURRENT_FUNCTION, "Deltacloud API is not initialized");
    }

    if (deltacloud_get_instances(&api, &instances) < 0) {
      cerr<<"Warning!!!Failed to get deltacloud images: "<<deltacloud_get_last_error_string()<<endl;
      deltacloud_free(&api);
      sleep(1);
      //return NULL;
      error = true;
    }
    else {
      error = false;
    }

  }while (error);

  deltacloud_instance * inst = NULL;
  inst = instances;
  vector<pInstance_t> inst_arr;


  while(inst != NULL) {


    Instance* new_instance = new Instance(inst->image_id,
        inst->id,
        inst->private_addresses ? inst->private_addresses->address : NULL,
        inst->public_addresses ? inst->public_addresses->address : NULL);

    inst_arr.push_back(boost::shared_ptr<Instance>(new_instance));
    inst = inst->next;
  }

  deltacloud_free_instance_list(&instances);
  deltacloud_free(&api);

  return inst_arr;
}

void Iaas_deltacloud::get_instance_state(const std::string id, char * state) {
  deltacloud_api api;
  if(!init_api(&api)) {
    sprintf(state, "%s", "ERROR");
    throw deltacloud_exception(BOOST_CURRENT_FUNCTION, "Deltacloud API not initialized.");
  }

  struct deltacloud_instance instance;

  if(deltacloud_get_instance_by_id(&api, id.c_str(),
        &instance) < 0) {
    sprintf(state, "%s", "ERROR");
    throw deltacloud_exception(BOOST_CURRENT_FUNCTION, "Deltacloud cannot retrive instance id");
  }else {
    sprintf(state, "%s", instance.state);
  }

  deltacloud_free(&api);
}


Instance* Iaas_deltacloud::get_instance_by_id(const std::string& instanceId) {
  deltacloud_api api;
  if(!init_api(&api)) {
    throw deltacloud_exception(BOOST_CURRENT_FUNCTION, "Deltacloud API not initialized.");
  }

  struct deltacloud_instance instance;

  if(deltacloud_get_instance_by_id(&api, instanceId.c_str(),
        &instance) < 0) {
    deltacloud_free(&api);
    throw deltacloud_exception(BOOST_CURRENT_FUNCTION, "Deltacloud cannot find instance.");
  }else {

    const char* public_address = NULL;
    const char* private_address = NULL;

    if (instance.private_addresses != NULL) {
      private_address = instance.private_addresses->address;
    }

    if (instance.public_addresses != NULL) {
      public_address = instance.public_addresses->address;
    }

    deltacloud_free(&api);
    return new Instance(instance.image_id, instance.id, private_address, public_address);
  }

}




int Iaas_deltacloud::wait_instance_running(const std::string& instanceId) {
  bool ready = false;
  char state[MAX_NAME];
  do{
    get_instance_state(instanceId, state);

    std::cout << "waiting : state " << instanceId << " = " << state << endl;

    if (strcmp(state, "RUNNING") == 0) {
#ifdef USE_LOG_SERVICE
      //the user must call this method to write in the log
      try {
      pInstance_t instance = pInstance_t(get_instance_by_id(std::string(instanceId)));

      DietLogComponent* component = get_log_component();
      if (component != NULL) {
        component->logVMRunning(*instance);
      }
      }
      catch(deltacloud_exception & e) {
        cerr << e.what();
      }
#endif
      ready = true;
    }
    if (strcmp(state, "ERROR") == 0) {
      return -1;
    }
    else {
      sleep(1);
    }

  }while(!ready);

  return 0;
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

vector<string> Iaas_deltacloud::run_instances(const string & image_id, int count, const std::vector<Parameter>& params) {
  deltacloud_api api;
  if(!init_api(&api)) {
    throw deltacloud_exception(BOOST_CURRENT_FUNCTION, "Deltacloud API is not initialized");
  }

  char * instance_id = NULL;
  vector<string> inst_arr;

  int params_count = params.size();
  struct deltacloud_create_parameter* delta_params = create_delta_params(params);

  for(int i_nb = 0; i_nb < count ; ++ i_nb) {
    int env;
    //we make a loop to avoid the error : Could not find instance name after instance creation
    do {
      env = deltacloud_create_instance(&api, image_id.c_str(), delta_params, params_count, &instance_id);
      if(env < 0) {
        cerr<<"[create inst]Failed to instanciate image (" << image_id << ") : err=" << env << " inst#" <<  i_nb + 1 << " " << deltacloud_get_last_error_string()<<endl;
        sleep(1);
      }
      else {

#ifdef USE_LOG_SERVICE
        //TODO: get the image_name
        IaaS::Image image("image_name", image_id);

        //TODO : search the cloud middleware name
        DietLogComponent* diet_log_component = get_log_component();
        if (diet_log_component != NULL) {
          diet_log_component->logVMDeployStart(image, "cloud-middleware", instance_id);
        }
#endif

        inst_arr.push_back(std::string(instance_id));
      }
    } while(env);
  }

  deltacloud_free(&api);

  if (delta_params != NULL) {
    free_delta_params(delta_params, params_count);
  }
  free(instance_id);
  return inst_arr;
}

std::string Iaas_deltacloud::get_id_from_ip(const std::string& ip, bool select_private_ip) {
  std::vector<pInstance_t> instances = get_all_instances();
  std::string instance_id= "";

  bool found = false;
  for(int i = 0; i < instances.size() && (!found); i++) {
    std::string instance_ip = instances[i]->get_ip(select_private_ip);
    if (instance_ip == ip) {
      found = true;
      instance_id = instances[i]->id;
    }
  }

  if (!found) {
    printf("warning : %s not found.\n", ip.c_str());
  }
  return instance_id;
}


int Iaas_deltacloud::terminate_instances(const vector<string> & instance_ids) {
  deltacloud_api api;
  if(!init_api(&api))
    return 2;

  int env = 0;
  /* delete all the instances in the array */
  deltacloud_instance dc_instance;
  for(size_t i_nb = 0; i_nb < instance_ids.size() ; ++ i_nb) {

    if(deltacloud_get_instance_by_id(&api, instance_ids[i_nb].c_str(), &dc_instance) < 0) {
      cerr<<"Failed to get info about instance "<< instance_ids[i_nb] <<endl;
      env = -1;
    } else if(deltacloud_instance_stop(&api, &dc_instance) < 0) {
      cerr<<"Failed to stop instance "<< instance_ids[i_nb]<<endl;
      env = -1;
    } else {

      //OK the instance is stopped
#ifdef USE_LOG_SERVICE
      DietLogComponent* component = get_log_component();
      try {
        if (component != NULL) {
          pInstance_t instance = pInstance_t(get_instance_by_id(instance_ids[i_nb]));
          component->logVMDestroyStart(*instance);
        }
      }
      catch(deltacloud_exception & e) {
        cerr << e.what();
      }
#endif

      if(deltacloud_instance_destroy(&api, &dc_instance) < 0) {
        cerr<<"Failed to destroy instance "<< instance_ids[i_nb]<<endl;
        env = -1;
      }
#ifdef USE_LOG_SERVICE
      else {
        if (component != NULL) {
          pInstance_t instance = pInstance_t(get_instance_by_id(instance_ids[i_nb]));
          component->logVMDestroyEnd(*instance);
        }
      }

#endif


    }

  }

  deltacloud_free(&api);
  return env;
}

int Iaas_deltacloud::terminate_instances_by_ips(const std::vector<std::string>& ips, bool select_private_ip) {
  std::vector<std::string> ids;
  for(int i = 0 ; i < ips.size(); i++) {
    string inst_id = get_id_from_ip(ips[i], select_private_ip);
    ids.push_back(inst_id);
  }

  int env = terminate_instances(ids);

  return env;
}

std::string Iaas_deltacloud::get_instance_state(const std::string& instance_id) {
  std::string res;
  char state[1024];
  get_instance_state(instance_id, state);

  res = state;

  return res;
}

Iaas_deltacloud *
Iaas_deltacloud::clone() const {
  Iaas_deltacloud * the_clone = new Iaas_deltacloud(*this);
  return the_clone;
}
