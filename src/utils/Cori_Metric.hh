/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* This is the API Header file to use CoRI                                  */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $@Id$
 * $@Log$
 ****************************************************************************/
#ifndef _CORI_METRIC_HH_
#define _CORI_METRIC_HH_

#include "Cori_Data_Easy.hh"

#include "DIET_config.h" //must be before #if HAVE_FAST
#if HAVE_FAST
#include "Cori_Fast.hh"
#endif //HAVE_FAST


class Cori_Metric
{
private: 
 
  diet_est_collect_tag_t type_collector;

  Cori_Data_Easy* cori_easy;

#if HAVE_FAST
  Cori_Fast* cori_fast;
#endif //HAVE_FAST

public:
   /**
   * Initialize the CoRI Metric of this machine
   * type_collector is the program where you get the information from
   *                for exemple EASY,FAST,GANGLIA,NAGIOS
   *                ={EASY,FAST} are the supported programs 
   *                                    (only if installed)
   */

  Cori_Metric(diet_est_collect_tag_t type_collector, 
	      const void *data);

  /**
   * type_Info is the type of the information you want collecting
   * information is the result
   * returns 1 if an error occurs. In this case, the information vector 
   *                               is set to defaults (dummy) values
   */


  int 
  call_cori_metric(diet_est_tag_t type_Info,
		   estVector_t *information,
		   const void* data
		   ); 
  

  diet_est_collect_tag_t 
  get_Collector_type();


private:
  diet_est_collect_tag_t collector_type;

  /* 
  for all tags: 
     if it is not possible to determine the right value, (incompatbility of the system with the program,error,...) 
       then the corresponding field (i.e. scalar or array) of the est_vector "information" will 
            have for the tag "type__Info" an appropriated value: the worsest cas : 
	    for example 
	    cpu avg load =HUGH_VALUE
	    cpucount = 1 
	    the reason is the scheduler can handle with this values
     return value: 0 if no error occurs, 1 elsewise
						      
 
   case EST_CPUSPEED:  
 the array of information is filled by the frequences of the cpus in Ghz
 
   case EST_AVGFREECPU:  amount of available cpu in avg (number of jobs in the run queue (state R) or waiting
             for disk I/O (state D) averaged over 1, 5, and 15 minutes

for all tags with information about the disk:
 the "data" parameter must contain a path with a correct syntaxe.
 if the path is correct and it is accessible (in read and write) for the program,
   then the scalar of "information" will be filled with the corresponding value (see below) 
   else the scalar contains -1

   case EST_TOTALSIZEDISK: 
"information" = total size of the partition (in Gbyte)

   case EST_DISKACCESREAD: 
"information"= current time to acces on the disk in Mbyte/s 
    
 case EST_DISKACCESWRITE: 
"information"= current time to acces on the disk in Mbyte/s 

   case EST_FREESIZEDISK: 
"information"= current size of the free disk in Gbyte 
   
   case EST_FREEMEM:  amount of (current?) free memory in ?byte

   case EST_CACHECPU:
 
 case EST_NBCPU:

 case EST_TOTALMEM:
*/
};


#endif // _CORI_METRIC_HH_
