#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Image.hh"
#include "Instance.hh"
#include "IaasInterface.hh"
#include "Iaas_deltacloud.hh"
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

int main(int argc, const char *argv[]) {
	string base_url = "http://localhost:3001/api";
	string username = "oneadmin";
	string password = "passoneadmin";

	if (argc < 2) {
		printf("usage : %s imageId\n", argv[0]);
		exit(0);
	}

	string imageId = argv[1];

	IaasInterface * interf = new Iaas_deltacloud(base_url, username, password);
	test_images(interf);
	
	vector<string*> * insts = interf->run_instances(imageId, 1);
	
	string instanceId = *(*insts)[0];
	
	Instance *instance = interf->get_instance_by_id(instanceId);
	
	interf->wait_instance_running(instanceId);
	
	
	int ret = -1;
	do{
		string cmd = "ssh user-vm@" + instance->public_ip + " -o StrictHostKeyChecking=no 'ls'";
		cout << cmd << endl;
		ret = system(cmd.c_str());
		
		
		if (WIFSIGNALED(ret) &&
            (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
                break;
	}while(ret != 0);
	
	
	
	interf->terminate_instances(*insts);
	
	//  test_instances(interf);
	//test_create(interf);
	//test_destroy(interf);
	//  test_instances(interf);
	
	cout << "Destruction instance : " << instanceId << ": OK" << endl;
	
	delete insts;
	delete interf;
	return 0;
}
