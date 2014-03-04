#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Image.hh"
#include "Instance.hh"
#include "IaasInterface.hh"
#include "Iaas_deltacloud.hh"
#include "Tools.hh"
#include "VMsDeployment.hh"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

using namespace std;
using namespace IaaS;

void print_str(const string & str) {
  cout<<str<<endl;
}

void print_instance(pInstance_t ins) {
  cout<<ins->id<<" "<<" "<<ins->image_id<<endl;
}

void print_image(pImage_t img) {
  cout<<img->id<<" "<<img->name<<endl;
}

void test_images(IaasInterface * interf) {
  vector<pImage_t> images = interf->get_all_images();
  cout<<"Images "<<images.size()<<endl;
  for_each(images.begin(), images.end(), print_image);
  cout<<endl;
}

void test_instances(IaasInterface * interf) {
  vector<pInstance_t> instances = interf->get_all_instances();
  cout<<"Instances "<<instances.size()<<endl;
  for_each(instances.begin(), instances.end(), print_instance);
  cout<<endl;
}

void test_create(IaasInterface * interf) {
  cout<<"Create"<<endl;
  vector<string> insts = interf->run_instances("3", 1);
  for_each(insts.begin(), insts.end(), print_str);
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
	string base_url = "http://graphene-35-kavlan-5.nancy.grid5000.fr:3001/api";
	string username = "admin+openstack";
	string password = "keystone_admin";
	string vm_user = "root";
	//string keypair_param = "keyname";
	string keypair_value = "ramses_key";
	//string vmname_param = "name";
	//string vmname_value = "VMTruc";
	//sigset_t signals_open;
	int vms_count = 1;

	if (argc < 3) {
		printf("usage : %s imageId profileId\n", argv[0]);
		exit(0);
	}

	string image_id = argv[1];
	string profile_id = argv[2];

	std::vector<Parameter> params;
	params.push_back(Parameter(KEYNAME_PARAM, keypair_value));
	params.push_back(Parameter(HARDWARE_PROFILE_ID_PARAM, profile_id));

  IaaS::pIaasInterface interface = IaaS::pIaasInterface(new IaaS::Iaas_deltacloud(base_url, username, password));
	VMsDeployment* vm_instances = new VMsDeployment(image_id, vms_count, interface,
	vm_user, params);

	vm_instances->wait_all_instances_running();

	vm_instances->wait_all_ssh_connection(true);

	printf("SSH : OK\n");
	sleep(5);
	printf("destruction of VM instances\n");
	delete vm_instances;

	return 0;
}
