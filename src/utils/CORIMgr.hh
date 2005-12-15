/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* Header of the DIET/CORI interface Manager                                */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $@Id$
 * $@Log$
 ****************************************************************************/
#ifndef _CORIMGR_HH__
#define _CORIMGR_HH__

#include "DIET_config.h"
#include "Cori_Metric.hh"

#include <vector>
#include "DIET_data.h" //TAGS
using namespace std;

class CORIMgr{
public:

static int
init();

static int
add(diet_est_collect_tag_t collectorName,
    const void * datav);

static int
diet_estimate_cori(estVector_t* ev,
		   diet_est_tag_t info_type,
		   diet_est_collect_tag_t collector_type,	   
		   const void * data);

private :
  static vector <Cori_Metric>* collector_v;
  static int is_initiated;
};
#endif //_CORIMGR_HH__
