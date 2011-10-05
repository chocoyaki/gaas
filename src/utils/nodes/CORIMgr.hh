/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* Header of the DIET/CORI interface Manager                                */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.7  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.6  2006/11/16 09:55:55  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.5  2006/10/31 23:18:33  ecaron
 * Correct wrong header
 *
 ****************************************************************************/
#ifndef _CORIMGR_HH__
#define _CORIMGR_HH__

#include "Cori_Metric.hh"

#include <vector>
#include "DIET_data.h" //TAGS
using namespace std;

class CORIMgr{
public:

  static int
  add(diet_est_collect_tag_t collectorName,
      const void * datav);

  static int
  call_cori_mgr(estVector_t* ev,
		int info_type,
		diet_est_collect_tag_t collector_type,       
		const void * data);
  static int
  startCollectors();

private :
  static vector <Cori_Metric>* collector_v;
};
#endif //_CORIMGR_HH__
