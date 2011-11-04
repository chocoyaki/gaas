/**
 * @file CORIMgr.hh
 *
 * @brief  Header of the DIET/CORI interface Manager
 *
 * @author  Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef _CORIMGR_HH__
#define _CORIMGR_HH__

#include <vector>
#include "Cori_Metric.hh"
#include "DIET_data.h"  // TAGS

class CORIMgr {
public:
  static int
  add(diet_est_collect_tag_t collectorName, const void *datav);

  static int
  call_cori_mgr(estVector_t *ev, int info_type,
                diet_est_collect_tag_t collector_type, const void *data);
  static int
  startCollectors();

private:
  static std::vector<Cori_Metric> *collector_v;
};
#endif  // _CORIMGR_HH__
