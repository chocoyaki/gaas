/***********************************************************/
/* Container implementation.                               */
/*                                                         */
/*  Author(s):                                             */
/*    - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)      */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/09/09 10:05:14  bisnard
 * container mgmt using Dagda agent
 *
 *
 ***********************************************************/

#include "DIET_data_internal.hh"
#include "marshalling.hh"
#include "debug.hh"
#include "Container.hh"
#include "DagdaFactory.hh"
#include "DagdaImpl.hh"

using namespace std;

Container::Container(const char* dataID) {
  myMgr    = DagdaFactory::getDataManager();
  myRelMgr = myMgr->getContainerRelationMgr();
  myID     = dataID;
  // check if container is already existing on the local data mgr
  // EXISTING means registered but not all elements of the container are
  // necessarily present locally
  if (myMgr->lclIsDataPresent(dataID)) {
    corba_data_t* storedData = myMgr->getData(dataID);
    nbOfElements = storedData->desc.specific.cont().size;
    TRACE_TEXT(TRACE_ALL_STEPS,"Container: object created (" << nbOfElements
        << " elements" << ", storedData=" << storedData << ")" << endl);
  }
  else
    throw Dagda::DataNotFound(dataID);
}

Container::~Container() {
}

void
Container::addData(const char* dataID, long index, long flag, bool setSize) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Container: Add the data " << dataID
     << " to container " << myID << endl);
  myRelMgr->addRelation(myID,dataID,index,flag);
  if (setSize) {
    ++nbOfElements;
    corba_data_desc_t* storedDataDesc = &myMgr->getData(myID.c_str())->desc;
    storedDataDesc->specific.cont().size = (CORBA::ULongLong) nbOfElements;
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "Container now contains " << nbOfElements
      << " element(s)" << endl);
}

void
Container::remData(const char* dataID, long flag) {
  myRelMgr->remRelation(myID, dataID, flag);
}

int
Container::size() {
  return nbOfElements;
}

void
Container::getAllElements(SeqString& dataIDSeq, SeqLong& flagSeq, bool ordered) {
  dataIDSeq.length(this->size());
  flagSeq.length(this->size());
  myRelMgr->getRelationList(myID, dataIDSeq, flagSeq, ordered);
}

/**
 * Send a container
 * Called from a dagda agent that contains the container data but that does not
 * necessarily contains all the elements. So the source for the elements is got
 * from a call to the platform.
 */
char *
Container::send(Dagda_ptr dest) {
  SeqString* dataIDSeq = new SeqString();
  SeqLong*   flagSeq = new SeqLong();
  this->getAllElements(*dataIDSeq, *flagSeq, true);

  for (int ix = 0; ix < dataIDSeq->length(); ++ix) {
    const char* eltID = (*dataIDSeq)[ix];
    TRACE_TEXT(TRACE_ALL_STEPS, "Sending element " << eltID << endl);
    corba_data_desc_t * eltDesc;
    try {
       eltDesc = myMgr->pfmGetDataDesc(eltID);
    } catch (Dagda::DataNotFound& ex) {
      WARNING("Missing element " << eltID << " in container " << myID << endl);
      continue; // skip this element
    }
    // add data element to destination mgr
    corba_data_t eltData;
    eltData.desc = *eltDesc;
    Dagda_var srcMgr = Dagda::_narrow(ORBMgr::stringToObject(eltDesc->dataManager));
    dest->lclAddData(srcMgr, eltData);
    // add relationship container-data to destination mgr
    dest->lclAddContainerElt(myID.c_str(), eltID, ix, (*flagSeq)[ix], false);
  }
  return CORBA::string_dup((*myMgr->getData())[myID].desc.id.idNumber);
}
