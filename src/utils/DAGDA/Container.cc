/**
 * @file Container.cc
 *
 * @brief  Container implementation
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include "DIET_data_internal.hh"
#include "marshalling.hh"
#include "debug.hh"
#include "Container.hh"
#include "DagdaFactory.hh"
#include "DagdaImpl.hh"
#include "ORBMgr.hh"

using namespace std;

Container::Container(const char *dataID, Dagda_ptr dataMgr,
                     DataRelationMgr *relMgr)
  : myID(dataID), notFound(false), nbOfElements(0) {
  myMgr = dataMgr;
  myRelMgr = relMgr;

  // check if container is already existing on the local data mgr
  // EXISTING means registered but not all elements of the container are
  // necessarily present locally
  if (!myMgr->lclIsDataPresent(dataID)) {
    notFound = true;
  }
}

Container::~Container() {
}

void
Container::addData(const char *dataID, long index, long flag) {
  // TRACE_TEXT(TRACE_ALL_STEPS, "Container: Add the data " << dataID
  // << "(flag=" << flag << ") to container " << myID << endl);
  if (notFound) {
    throw Dagda::DataNotFound(myID.c_str());
  }
  myRelMgr->addRelation(myID, dataID, index, flag);
}

void
Container::remData(long index) {
  if (notFound) {
    throw Dagda::DataNotFound(myID.c_str());
  }
  myRelMgr->remRelation(myID, index);
}

int
Container::size() {
  if (notFound) {
    throw Dagda::DataNotFound(myID.c_str());
  }
  return (myRelMgr->getRelationMaxIndex(myID) + 1);
}

void
Container::getAllElements(SeqString &dataIDSeq, SeqLong &flagSeq,
                          bool ordered) {
  if (notFound) {
    throw Dagda::DataNotFound(myID.c_str());
  }
  int _size = size();
  try {
    dataIDSeq.length(_size);
    flagSeq.length(_size);
  } catch (...) {
    cerr << "Container::getAllElements exception in initialization" << endl;
    throw;
  }
  try {
    myRelMgr->getRelationList(myID, dataIDSeq, flagSeq, ordered);
  } catch (...) {
    cerr << "Container::getAllElements exception in getRelationList" << endl;
    throw;
  }
} // getAllElements

/**
 * Send a container
 * Called from a dagda agent that contains the container data but that does not
 * necessarily contains all the elements. So the source for the elements is got
 * from a call to the platform.
 */
char *
Container::send(const char *destName, bool sendData) {
  Dagda_ptr dest = ORBMgr::getMgr()->resolve<Dagda, Dagda_ptr>(DAGDACTXT,
                                                               destName);

  SeqString *dataIDSeq = new SeqString();
  SeqLong *flagSeq = new SeqLong();
  this->getAllElements(*dataIDSeq, *flagSeq, true);

  TRACE_TEXT(TRACE_ALL_STEPS, "Sending container " << myID << " ("
                                                   << dataIDSeq->length() <<
             " elements / sendData="
                                                   << sendData << ")" << endl);

  for (unsigned int ix = 0; ix < dataIDSeq->length(); ++ix) {
    if ((*dataIDSeq)[ix] != NULL) {
      const char *eltID = (*dataIDSeq)[ix];
      if (sendData) {
        corba_data_desc_t *eltDesc;
        try {
          eltDesc = myMgr->pfmGetDataDesc(eltID);
        } catch (Dagda::DataNotFound &ex) {
          WARNING(
            "Missing element " << eltID << " in container " << myID << endl);
          continue;  // skip this element
        }
        corba_data_t eltData;
        eltData.desc = *eltDesc;
        // Dagda_var srcMgr = Dagda::_narrow(ORBMgr::stringToObject(eltDesc->dataManager));
        Dagda_ptr srcMgr =
          ORBMgr::getMgr()->resolve<Dagda,
                                    Dagda_ptr>(string(eltDesc->dataManager));
        string srcName = srcMgr->getID();
        dest->lclAddData(srcName.c_str(), eltData);
      }
      // add relationship container-data to destination mgr
      dest->lclAddContainerElt(myID.c_str(), eltID, ix, (*flagSeq)[ix]);
    } else {  // dataIDSeq[ix] = NULL
      dest->lclAddContainerElt(myID.c_str(), "", ix, (CORBA::Long) 1);
    }
  }
  return CORBA::string_dup(myID.c_str());
} // send
