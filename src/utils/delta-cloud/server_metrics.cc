#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include "server_metrics.hh"
#include "server_utils.hh"
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <istream>
//#include "server.hh"

using namespace std;

void MetricsAggregator::init(std::string site_name) {
  if (site_name == "") {
    const int buf_size = 256;
    char buf[buf_size];
    buf[buf_size-1] = 0;
    if (0 != gethostname(buf, buf_size-1)) {
      throw std::exception();
    }
    site_name = std::string(buf);
  }
  this->site_name = site_name;

}

// Checks if an image is registered on the cloud
double MetricsAggregator::check_img(std::string vm_name){
	std::string cmd = "";
	    cmd = "glance index | grep -w " + vm_name + " "+" | sed 's/.*=//;s/ .*//'";
	    FILE* pipe = popen(cmd.c_str(), "r");
	        if (!pipe) return 0; //Erreur sur l'exécution de la commande
	        char buffer[128];
	        std::string result = "";
	        while(!feof(pipe)) {
	        	if(fgets(buffer, 128, pipe) != NULL)
	        		result += buffer;
	        }
	        pclose(pipe);
	        //return result;
	        cout << cmd << std::endl;
	        if (result.size() == 0)
	        	return 0;
	        else
	        	return 1;
}

double MetricsAggregator::get_avg_cpu(){
	double avg_cpu = 0;
	return avg_cpu;
}/*
double MetricsAggregator::get_alert(){
	double avg_cpu = 0;
	return avg_cpu;
}
double MetricsAggregator::get_instant_energy(){
	double avg_cpu = 0;
	return avg_cpu;
}
*/
