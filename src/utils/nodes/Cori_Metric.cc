/**
 * @file Cori_Metric.cc
 *
 * @brief  This is the API impl file to use CoRI
 *
 * @author  Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "Cori_Metric.hh"
#include <iostream>
#include "debug.hh"

using namespace std;

Cori_Metric::Cori_Metric(diet_est_collect_tag_t type,
                         const void *data) {
  collector_type = type;

  switch (collector_type) {
  case EST_COLL_EASY: {
    cori_easy = new Cori_Data_Easy();
  }
  break;
#if (defined HAVE_ALT_BATCH) && (not defined CLEAN_CORILIB_FROM_BATCH_STAFF)
  case EST_COLL_BATCH:
    cori_batch = new Cori_batch((diet_profile_t *) data);
    break;
#endif
  default: {
    INTERNAL_WARNING("Collector called " << collector_type << " doesn't exist");
  }
  break;
  } // switch
}
diet_est_collect_tag_t
Cori_Metric::get_Collector_type() {
  return this->collector_type;
}

int
Cori_Metric::start(diet_est_collect_tag_t type) {
  collector_type = type;

  switch (collector_type) {
#if not defined CLEAN_CORILIB_FROM_BATCH_STAFF and defined HAVE_ALT_BATCH
  case EST_COLL_BATCH:
    // do I need to 'start' some Batch things?
    // Maybe one day, a process that monitors a batch systems if needed?
    return 0;

#endif
  case EST_COLL_EASY: {
    // no need to start - very dynamic functions
    return 0;
  }
  break;
  default: {
    INTERNAL_WARNING("Collector called " << collector_type << " doesn't exist");
    return 0;  // this warnig should not stop the start up of the SeD
  }
  break;
  } // switch
} // start

int
Cori_Metric::call_cori_metric(int type_Info,
                              estVector_t *information,
                              const void *data) {
  switch (collector_type) {
#if not defined CLEAN_CORILIB_FROM_BATCH_STAFF and defined HAVE_ALT_BATCH
  case EST_COLL_BATCH:
    return cori_batch->get_Information(type_Info,
                                       information,
                                       data);

    break;
#endif
  case EST_COLL_EASY: {
    return cori_easy->get_Information(type_Info,
                                      information,
                                      data);
  }
  break;
  default: {
    diet_est_set_internal(*information, type_Info, 0);
    ERROR("CoRI: Collector " << collector_type << " doesn't exist!", 1);
    // fixme : add the default value to every type_info
  }
  break;
  } // switch
  return 1;
} // call_cori_metric
