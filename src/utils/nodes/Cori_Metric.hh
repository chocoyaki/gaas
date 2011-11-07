/**
 * @file Cori_Metric.hh
 *
 * @brief  This is the API header file to use CoRI Metric management
 *
 * @author  Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _CORI_METRIC_HH_
#define _CORI_METRIC_HH_

#include "Cori_Data_Easy.hh"

#if not defined CLEAN_CORILIB_FROM_BATCH_STAFF and defined HAVE_ALT_BATCH
#include "Cori_batch.hh"
#endif

class Cori_Metric {
public:
  /**
   * Initialize the CoRI Metric of this SeD
   * @param type_collector is the program where you get the information from
   *                for exemple EASY, GANGLIA, NAGIOS
   *                ={EASY, BATCH*} are the supported programs
   *                                    (*only if installed)
   * @param data can be used to help initialize the collector.
   *        Not used for the moment in any collector
   */

  Cori_Metric(diet_est_collect_tag_t type_collector, const void *data);

  /**
   * type_Info is the type of the information you want collecting
   * information is the result
   * returns 1 if an error occurs. In this case, the information vector
   *                               is set to defaults (dummy) values
   */
  int
  call_cori_metric(int type_Info, estVector_t *information, const void *data);

  diet_est_collect_tag_t
  get_Collector_type();

  int
  start(diet_est_collect_tag_t type);

private:
  diet_est_collect_tag_t collector_type;
  diet_est_collect_tag_t type_collector;

  Cori_Data_Easy *cori_easy;
#if not defined CLEAN_CORILIB_FROM_BATCH_STAFF and defined HAVE_ALT_BATCH
  Cori_batch *cori_batch;
#endif
};


#endif  // _CORI_METRIC_HH_
