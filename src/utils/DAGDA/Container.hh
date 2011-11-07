/**
 * @file Container.hh
 *
 * @brief  Container implementation
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef __CONTAINER_HH__
#define __CONTAINER_HH__

#include <map>
#include <string>
#include "DIET_data.h"
#include "DIET_Dagda.h"
#include "DagdaImpl.hh"
#include "DataRelationMgr.hh"

class Container {
public:
  /**
   * Constructor by ID
   * Access an existing container on the local Data Mgr
   */
  Container(const char *dataID, Dagda_ptr dataMgr, DataRelationMgr * relMgr);

  /**
   * Destructor
   */
  ~Container();

  /**
   * Add a data element to the container
   * (the element must already be registered in a Data Mgr that is
   * not necessarily the local Data Mgr)
   * @param dataID  the element data identifier
   * @param index   the index of the element
   * @param flag    the flag of the relation to the container
   */
  void
  addData(const char *dataID, long index, long flag);

  /**
   * Remove the i-th element from the container (does not delete the element)
   * @param index   the index of the element
   */
  void
  remData(long index);

  /**
   * Get the size (nb of elements) of the container
   * Note: currently only used to allocate the sequences returned by
   * getAllElements
   */
  int
  size();

  /**
   * Get all the elements of a container
   * @param dataIDSeq a sequence of strings (not pre-allocated)
   * @param flagSeq a sequence of long integers (not pre-allocated)
   * @param ordered sort the elements in the initial order
   */
  void
  getAllElements(SeqString &dataIDSeq, SeqLong &flagSeq, bool ordered);

  /**
   * Send the container to a remote Data Mgr
   * @param destName  the destination dagda agent
   * @param sendData  if true, will send all the elements of the container
   *                  if false, will send only the relationships
   */
  char *
  send(const char *destName, bool sendData = true);

private:
  /**
   * The DAGDA ID of the container
   */
  std::string myID;

  /**
   * The Data Mgr that contains the container
   */
  Dagda_ptr myMgr;

  /**
   * The data relation Mgr that manages the container-elements relationship
   */
  DataRelationMgr *myRelMgr;

  /**
   * Not found flag
   */
  bool notFound;

  /**
   * Nb of elements in the container
   */
  int nbOfElements;
};  // end class Container

#endif /* ifndef __CONTAINER_HH__ */
