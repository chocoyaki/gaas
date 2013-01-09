#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Image.hh"
#include "Instance.hh"
#include "IaasInterface.hh"
#include "Iaas_deltacloud.hh"
#include "Tools.hh"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

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
  vector<Instance*> * instances = interf->get_all_instances();
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

/*
void test_destroy(IaasInterface * interf) {
  cout<<"Destroy"<<endl;
  vector<string> insts;
  insts.push_back("inst0");
  insts.push_back("inst1");
  insts.push_back("inst4");
  int ret = interf->terminate_instances(insts);
  cout<<ret<<endl;
}*/

void donothing_on_sigint(int sig) {
	printf("SIGINT intercepte\n");
}

int main(int argc, const char *argv[]) {
	string base_url = "http://localhost:3001/api";
	string username = "admin+admin";
	string password = "admin";
	string vm_user = "cirros";
	//string keypair_param = "keyname";
	string keypair_value = "key1";
	//string vmname_param = "name";
	//string vmname_value = "VMTruc";
	//sigset_t signals_open;
	int sleep_duration = 5;
	int vms_count = 1;
	
	if (argc < 2) {
		printf("usage : %s imageId\n", argv[0]);
		exit(0);
	}

	string image_id = argv[1];

	
	VMInstances* vm_instances = new OpenStackVMInstances(image_id, vms_count, base_url, username, password,
	vm_user, keypair_value);
	
	vm_instances->wait_all_instances_running();
	
	vm_instances->wait_all_ssh_connection(true);
	
	printf("SSH : OK\n");
	sleep(5);
	printf("destruction of VM instances\n");
	delete vm_instances;
	
	return 0;
}
