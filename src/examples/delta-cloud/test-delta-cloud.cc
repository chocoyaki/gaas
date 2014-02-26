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
	string base_url = "http://localhost:3001/api";
	string username = "admin+admin";
	string password = "admin";
	string user_vm = "cirros";
	string keypair_param = "keyname";
	string keypair_value = "key1";
	//string vmname_param = "name";
	//string vmname_value = "VMTruc";
	//sigset_t signals_open;
	int sleep_duration = 5;
	
	if (argc < 2) {
		printf("usage : %s imageId\n", argv[0]);
		exit(0);
	}

	string imageId = argv[1];

	IaasInterface * interf = new Iaas_deltacloud(base_url, username, password);
	test_images(interf);
	
	//exit(0);
	
	std::vector<Parameter> params = std::vector<Parameter>();
	Parameter public_key(keypair_param, keypair_value);
	//Parameter vm_name(vmname_param, vmname_value);
	params.push_back(public_key);
	//params.push_back(vm_name);
	
	vector<string> insts = interf->run_instances(imageId, 1, params);
	
	string instanceId = insts[0];
	
	
	
	interf->wait_instance_running(instanceId);
	
	//we wait for obtaining ip_address
	Instance *instance = interf->get_instance_by_id(instanceId);
	
	struct sigaction action;
	action.sa_handler = donothing_on_sigint;
	sigaction(SIGINT, &action, NULL);
	
	int ret = -1;
	do{
		//string cmd = "ssh user-vm@" + instance->public_ip + " -o StrictHostKeyChecking=no 'ls'";
		string cmd = "ssh "  + user_vm + "@" + instance->private_ip + " -o StrictHostKeyChecking=no 'ls'";
		cout << cmd << endl;
		ret = system(cmd.c_str());
		
		
		if(ret != 0) {
			printf("retentative dans %d secondes\n", sleep_duration);
			sleep(sleep_duration);
		}
		
		if (WIFSIGNALED(ret) &&
            (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
                break;
        
	}while(ret != 0);
	
	
	cout << " SSH "+ instance->private_ip + " : OK " << endl;
	sleep(5);
	
	interf->terminate_instances(insts);
	
	
	cout << "Destruction instance : " << instanceId << ": OK" << endl;
	
	delete interf;
	return 0;
}
