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
 * Revision 1.2  2011/02/01 20:51:25  bdepardo
 * changed "!defined" into "not defined".
 *
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.9  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.8  2007/04/30 13:53:22  ycaniou
 * Cosmetic changes (indentation) and small changes for Cori_Batch
 *
 * Revision 1.7  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
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
#if (defined HAVE_ALT_BATCH) && (not defined CLEAN_CORILIB_FROM_BATCH_STAFF)
  case EST_COLL_BATCH:
    cori_batch = new Cori_batch( (diet_profile_t*)data ) ;
    break ;
#endif
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
Cori_Metric::start(diet_est_collect_tag_t type)
{
  collector_type=type;

  switch(collector_type){
#if not defined CLEAN_CORILIB_FROM_BATCH_STAFF and defined HAVE_ALT_BATCH
  case EST_COLL_BATCH:
    // do I need to 'start' some Batch things?
    // Maybe one day, a process that monitors a batch systems if needed?
    return 0 ;
#endif
  case EST_COLL_EASY:{
    //no need to start - very dynamic functions
    return 0;
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

#if not defined CLEAN_CORILIB_FROM_BATCH_STAFF and defined HAVE_ALT_BATCH
  case EST_COLL_BATCH:
    return cori_batch->get_Information(type_Info,
                                       information,
                                       data) ;
    break ;
#endif
  case EST_COLL_EASY:{
    return cori_easy->get_Information(type_Info,
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

