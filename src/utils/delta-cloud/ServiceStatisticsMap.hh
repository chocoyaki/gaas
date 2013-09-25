/*
 * @file ServiceStatisticsMap.hh
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _SERVICESTATISTICSMAP_HH_
#define _SERVICESTATISTICSMAP_HH_

#include <string>
#include <map>


/*********************************************************************************************/
/* A ServiceStatistics registers one service with how many times it has been called***********/
/*********************************************************************************************/
class ServiceStatistics{
public:
  /*Constructor to initiate a service with 0 time request*/
    ServiceStatistics(const std::string _service_name) {
        service_name = _service_name;
        call_number = 0;
    }

    /*Copy constructor*/
    ServiceStatistics(const ServiceStatistics& stats) {
        service_name = stats.service_name;
        call_number = stats.call_number;
    }

    ServiceStatistics(){
        service_name = "";
        call_number = 0;
    }

    void increment_call_number() {
        call_number++;
    }

    int call_count() const {
        return call_number;
    }




    /*
    bool operator< (const ServiceStatistics& other) const {
        return service_name < other.service_name;
    }*/

protected:
    std::string service_name;
    int call_number;
};




class ServiceStatisticsMap{
public:
    ServiceStatisticsMap() {

    }

    void add_service(const std::string& service_name) {
        stats_map[service_name] = ServiceStatistics(service_name);
    }

    bool service_exists(const std::string& service_name) const {
        return stats_map.count(service_name) > 0;
    }

    void increment_call_number(const std::string& service_name) {
        stats_map[service_name].increment_call_number();
    }

    const std::map<std::string, ServiceStatistics>& get() const {
        return stats_map;
    }

    bool one_service_already_called(const std::string& service_name) const {
        const ServiceStatistics stat = stats_map.at(service_name);
        return stat.call_count() > 0;
    }

    //at least one service has already been called
    bool one_service_already_called() const {
        std::map<std::string, ServiceStatistics>::const_iterator iter;

        for(iter = stats_map.begin(); iter != stats_map.end(); iter++){
            const std::string& service_name = iter->first;
            if (one_service_already_called(service_name)) {
                return true;
            }
        }

        return false;
    }


    int call_count(const std::string& service_name) const {
        const ServiceStatistics stat = stats_map.at(service_name);
        return stat.call_count();
    }

protected:
    std::map<std::string, ServiceStatistics> stats_map;


};

#endif /* _SERVICESTATISTICSMAP_HH_ */
