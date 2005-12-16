#include "Cori_Metric.hh"
#include <iostream>

using namespace std;

Cori_Metric::Cori_Metric(diet_est_collect_tag_t type, 
			 const void *data)
{

 collector_type=type;

 //call the desired collector and initialize it

 switch(collector_type){

  case EST_COLL_EASY:{ 
   cori_easy=new Cori_Data_Easy();
  }
    break; 
  case EST_COLL_FAST:{
#if HAVE_FAST  
    cori_fast=new Cori_Fast();
#endif //HAVE_FAST

  }
    break;
  case EST_COLL_GANGLIA:{
  }
    break;

 case EST_COLL_NAGIOS:{
  }
    break;
    /********************************************
     * add here your collector constructor call *
     ********************************************/
  default:{
    cerr<< "Collector called "<<collector_type <<" doesn't exist"<<endl ;
  }
     break;  
  }
}
diet_est_collect_tag_t 
Cori_Metric::get_Collector_type(){
  return this->collector_type;
}


int 
Cori_Metric::call_cori_metric(diet_est_tag_t type_Info,       
				  estVector_t *information,
				  const void *data)
{
  switch(collector_type){
    
  case EST_COLL_EASY:{ 
    return cori_easy->get_Information(type_Info, 
				      information,data);   
  }
    break; 
  case EST_COLL_FAST:{
#if HAVE_FAST

    fast_param_t* fast_param=(fast_param_t*)data;
    return cori_fast->get_Information(type_Info,
				      information,
				      fast_param->initprofilePtr,
				      fast_param->SRVT );

#endif //HAVE_FAST
  }
    break;
  case EST_COLL_GANGLIA:{

  }
    break;
   /********************************************
    * add here your getInformation call        *
    * ******************************************/
  default:{
    return 1;
  }
     break;  
  }
  return 1;   
}
