#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Image.hh"
#include "Instance.hh"
#include "IaasInterface.hh"
#include "Iaas_deltacloud.hh"

using namespace std;
using namespace IaaS;

void print_str(const string * str) {
  cout<<*str<<endl;
}

void print_instance(Instance * ins) {
  cout<<ins->id<<" "<<" "<<ins->image_id<<endl;
}

void print_image(Image * img) {
  cout<<img->id<<" "<<img->name<<endl;
}

void test_images(IaasInterface * interf) {
  vector<Image*> * images = interf->get_all_images();
  cout<<"Images "<<images->size()<<endl;
  for_each(images->begin(), images->end(), print_image);
  cout<<endl;
}

void test_instances(IaasInterface * interf) {
  vector<Instance*> * instances = interf->get_all_instanges();
  cout<<"Instances "<<instances->size()<<endl;
  for_each(instances->begin(), instances->end(), print_instance);
  cout<<endl;
}

void test_create(IaasInterface * interf) {
  cout<<"Create"<<endl;
  vector<string*> * insts = interf->run_instances("3", 1);
  for_each(insts->begin(), insts->end(), print_str);
  cout<<endl;
}

void test_destroy(IaasInterface * interf) {
  cout<<"Destroy"<<endl;
  vector<string> insts;
  insts.push_back("inst0");
  insts.push_back("inst1");
  insts.push_back("inst4");
  int ret = interf->terminate_instances(insts);
  cout<<ret<<endl;
}

int main(int argc, const char *argv[]) {
  string base_url = "http://localhost:3001/api";
  string username = "oneadmin";
  string password = "passoneadmin";

  IaasInterface * interf = new Iaas_deltacloud(base_url, username, password);
  test_images(interf);
//  test_instances(interf);
  test_create(interf);
  //test_destroy(interf);
//  test_instances(interf);

  return 0;
}
