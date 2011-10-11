/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2011/01/21 17:26:05  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 *
 * Revision 1.2  2010/03/31 21:15:41  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.10  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.9  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.8  2006/10/31 22:46:49  ecaron
 * CORI Manager
 *
 ****************************************************************************/
#include "CORIMgr.hh"
#include <cmath>
#include "est_internal.hh"

#include <iostream>
#include "debug.hh"

using namespace std;


vector <Cori_Metric>* CORIMgr::collector_v = new vector <Cori_Metric>();

int
CORIMgr::add(diet_est_collect_tag_t collectorName,
             const void * datav)
{
  Cori_Metric tmp ( collectorName, datav );
  collector_v->push_back(tmp);
  return 0;
}

int
CORIMgr::startCollectors()
{
  int res = 0;
  vector <Cori_Metric>::iterator iter1;
  iter1 = collector_v->begin();
  while( iter1 != collector_v->end()) {
    res = iter1->start(iter1->get_Collector_type())||res;
    ++iter1;
  }
  return res;
}

int
CORIMgr::call_cori_mgr(estVector_t *ev,
                       int info_type,
                       diet_est_collect_tag_t name,
                       const void * datav)
{
  vector <Cori_Metric>::iterator iter1;
  iter1 = collector_v->begin();
  while( iter1 != collector_v->end()) {

    if (name == iter1->get_Collector_type())
      return iter1->call_cori_metric(info_type,ev,datav);

    ++iter1;
  }
  INTERNAL_WARNING("The collector "<<name<<" is not present in CORIMgr");
  return 1;
}

