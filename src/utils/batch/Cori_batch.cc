/**
 * @file Cori_batch.cc
 *
 * @brief  CoRI Collectors of ressource information for batch systems
 *
 * @author  Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "Cori_batch.hh"
#include "BatchSystem.hh"

#include "debug.hh"
#include <iostream>
#include <fstream>      /*file reading*/
#include <cstdlib>
#include <string>
#include <cmath>


using namespace std;

void
Cori_batch::printMetric(estVector_t vector_v, int type_Info) {
  switch (type_Info) {
  case EST_SERVER_TYPE: /* Value appears only once */
    cout << "In construction: EST_SERVER_TYPE" << endl << endl;
    break;
  case EST_PARAL_NBTOT_RESOURCES:
    cout << "In construction: EST_PARAL_NBTOT_RESOURCES" << endl << endl;
    break;
  case EST_PARAL_NBTOT_FREE_RESOURCES:
    cout << "In construction: EST_PARAL_NBTOT_FREE_RESOURCES" << endl << endl;
    break;
  case EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE:
    cout <<
    "CoRI: EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE... " <<
    (int) diet_est_get_system(vector_v,
                              EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE, 0)
    << endl << endl;
    break;
  default: {
    INTERNAL_WARNING("CoRI: Tag " << type_Info << " for printing info");
  }
  } // switch
} // printMetric

Cori_batch::Cori_batch(diet_profile_t *profile) {
  /* At this time, because diet_SeD() has not yet been called, we cannot do
     that */
  /* TODO: make a diet_initialize() or something before diet_SeD()! */

  /* this->SeD = (SeDImpl*)(profile->SeDPtr);
     this->batch = ((SeDImpl*)(profile->SeDPtr))->getBatch(); */
}

int
Cori_batch::get_Information(int type_Info,
                            estVector_t *estvect,
                            const void *data) {
  switch (type_Info) {
  /* The following one should be replaced, when SeDseq, SeDpar will be
     implemented, by est_PARAL_ID */
  case EST_SERVER_TYPE:
    /*    diet_est_set_internal(*estvect, type_Info, this->SeD->getServerStatus()); */
    diet_est_set_internal(
      *estvect, type_Info,
      ((SeDImpl *) (((diet_profile_t *) data)->SeDPtr))->
      getServerStatus());
    break;
  case EST_PARAL_NBTOT_RESOURCES:
    diet_est_set_internal(*estvect, type_Info,
                          (((SeDImpl *) (((diet_profile_t *) data)->SeDPtr))
                           ->getBatch())->getNbTotResources());
    /*                     this->batch->getNbTotResources()); */
    break;
  case EST_PARAL_NBTOT_FREE_RESOURCES:
    diet_est_set_internal(*estvect, type_Info,
                          (((SeDImpl *) (((diet_profile_t *) data)->SeDPtr))
                           ->getBatch())->getNbTotFreeResources());
    /*                     this->batch->getNbTotFreeResources()); */
    break;
  /* Information concerning the default queue now */
  case EST_PARAL_NB_RESOURCES_IN_DEFAULT_QUEUE:
    diet_est_set_internal(*estvect, type_Info,
                          (((SeDImpl *) (((diet_profile_t *) data)->SeDPtr))
                           ->getBatch())->getNbResources());
    /*                     this->batch->getNbResources()); */
    break;
  case EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE:
    diet_est_set_internal(*estvect, type_Info,
                          (((SeDImpl *) (((diet_profile_t *) data)->SeDPtr))
                           ->getBatch())->getNbFreeResources());
    /*                     this->batch->getNbFreeResources()); */
    break;
  case EST_PARAL_MAX_WALLTIME:
    diet_est_set_internal(*estvect, type_Info,
                          (((SeDImpl *) (((diet_profile_t *) data)->SeDPtr))
                           ->getBatch())->getMaxWalltime());
    /*                     this->batch->getMaxWalltime()); */
    break;
  case EST_PARAL_MAX_PROCS:
    diet_est_set_internal(*estvect, type_Info,
                          (((SeDImpl *) (((diet_profile_t *) data)->SeDPtr))
                           ->getBatch())->getMaxProcs());
    /*                     this->batch->getMaxProcs()); */
    break;
  default:
  {
    WARNING("CoRI Batch: Tag " << type_Info <<
            " unknown for collecting info");
  }
  } // switch

  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    printMetric(*estvect, type_Info);
  }
  return 0;
} // get_Information

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/

int
Cori_batch::convertArray(vector <double> vect,
                         estVector_t *estvect,
                         int typeOfInfo) {
  /*  vector<double>::iterator iter1;
      iter1 = vect.begin();
      int i = 0;
      while (iter1 != vect.end()) {
      diet_est_array_set_internal(*estvect, typeOfInfo, i,*iter1);
      i++;
      iter1++;
      }
      return 0;
   */
  return 0;
}

int
Cori_batch::convertSimple(double value,
                          estVector_t *estvect,
                          int typeOfInfo) {
  diet_est_set_internal(*estvect, typeOfInfo, value);
  return 0;
}
