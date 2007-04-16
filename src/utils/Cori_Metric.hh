/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* Metric management                                                        */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.7  2006/11/16 09:55:55  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.6  2006/10/31 23:14:46  ecaron
 * CoRI: Metric management
 *
 ****************************************************************************/
#ifndef _CORI_METRIC_HH_
#define _CORI_METRIC_HH_

#include "Cori_Data_Easy.hh"
#include "Cori_Fast.hh"
#if HAVE_ALT_BATCH
#include "Cori_batch.hh"
#endif

class Cori_Metric
{
private: 
 
  diet_est_collect_tag_t type_collector;

  Cori_Data_Easy* cori_easy;
  Cori_Fast* cori_fast;
#if HAVE_ALT_BATCH  
  Cori_batch * cori_batch ;
#endif

public:
   /**
   * Initialize the CoRI Metric of this SeD
   * type_collector is the program where you get the information from
   *                for exemple EASY,FAST,GANGLIA,NAGIOS
   *                ={EASY,FAST*} are the supported programs 
   *                                    (*only if installed)
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
  call_cori_metric(int type_Info,
		   estVector_t *information,
		   const void* data
		   );  

  diet_est_collect_tag_t 
  get_Collector_type();

  int 
  start(diet_est_collect_tag_t type);

private:
  diet_est_collect_tag_t collector_type;
};


#endif // _CORI_METRIC_HH_
