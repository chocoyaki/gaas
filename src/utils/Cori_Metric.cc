/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* This is the API Header file to use CoRI                                  */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2006/10/31 23:14:46  ecaron
 * CoRI: Metric management
 *
 ****************************************************************************/

#include "Cori_Metric.hh"
#include <iostream>
#include "debug.hh"

using namespace std;

Cori_Metric::Cori_Metric(diet_est_collect_tag_t type, 
			 const void *data)
{

 collector_type=type;

 switch(collector_type){

  case EST_COLL_EASY:{ 
   cori_easy=new Cori_Data_Easy();
  }
    break; 
  case EST_COLL_FAST:{
    cori_fast=new Cori_Fast();
  }
    break;
  default:{
    INTERNAL_WARNING("Collector called "<<collector_type <<" doesn't exist");
  }
     break;  
  }
}
diet_est_collect_tag_t 
Cori_Metric::get_Collector_type(){
  return this->collector_type;
}

int 
Cori_Metric::start(diet_est_collect_tag_t type){
   collector_type=type;

 switch(collector_type){

  case EST_COLL_EASY:{ 
    //no need to start - very synamic functions
    return 0;
  }
    break; 
  case EST_COLL_FAST:{
    return cori_fast->start();
  }
    break;
  default:{
    INTERNAL_WARNING("Collector called "<<collector_type <<" doesn't exist");
    return 0; //this warnig should not stop the start up of the SeD
  }
     break;  
  }
}

int 
Cori_Metric::call_cori_metric(int type_Info,       
				  estVector_t *information,
				  const void *data)
{
  switch(collector_type){
    
  case EST_COLL_EASY:{ 
    return cori_easy->get_Information(type_Info, 
				      information,
				      data);   
  }
    break; 
  case EST_COLL_FAST:{
    return cori_fast->get_Information(type_Info,
				      information,
				      data);
  }
    break;
  default:{ 
    diet_est_set_internal(*information,type_Info,0);
    ERROR("CoRI: Collector " <<collector_type <<" doesn't exist!",1);
    //fixme : add the default value to every type_info
  }
     break;  
  }
  return 1;   
}
 
