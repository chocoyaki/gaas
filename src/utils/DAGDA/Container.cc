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
 * Revision 1.4  2009/01/16 13:32:47  bisnard
 * replaced use of DagdaImpl ptr by dagda object ref
 * modified constructor signature
 * moved container descr update from addData to DagdaImpl
 *
 * Revision 1.3  2008/12/09 12:06:20  bisnard
 * changed container download method to transfer only the list of IDs
 * (each container element must be downloaded separately)
 *
 * Revision 1.2  2008/11/08 19:12:39  bdepardo
 * A few warnings removal
 *
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

Container::Container(const char* dataID, Dagda_ptr dataMgr, DataRelationMgr* relMgr)
  : myID(dataID), notFound(false), nbOfElements(0) {

  myMgr = dataMgr;
  myRelMgr = relMgr;

  // check if container is already existing on the local data mgr
  // EXISTING means registered but not all elements of the container are
  // necessarily present locally
  if (myMgr->lclIsDataPresent(dataID)) {
    corba_data_desc_t* dataDesc = myMgr->lclGetDataDesc(dataID); // makes a copy of desc
    nbOfElements = dataDesc->specific.cont().size;
    delete dataDesc;
  }
  else
    notFound = true;
}

Container::~Container() {
}

void
Container::addData(const char* dataID, long index, long flag, bool setSize) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Container: Add the data " << dataID
     << " to container " << myID << endl);
  if (notFound)
    throw Dagda::DataNotFound(myID.c_str());
  myRelMgr->addRelation(myID,dataID,index,flag);
  if (setSize) {
    ++nbOfElements;
    // update of the data description is done by data mgr
  }
}

void
Container::remData(const char* dataID, long flag) {
  if (notFound)
    throw Dagda::DataNotFound(myID.c_str());
  myRelMgr->remRelation(myID, dataID, flag);
}

int
Container::size() {
  if (notFound)
    throw Dagda::DataNotFound(myID.c_str());
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
Container::send(Dagda_ptr dest, bool sendData) {
  SeqString* dataIDSeq = new SeqString();
  SeqLong*   flagSeq = new SeqLong();
  this->getAllElements(*dataIDSeq, *flagSeq, true);

  TRACE_TEXT(TRACE_ALL_STEPS, "Sending container " << myID << " ("
                              << dataIDSeq->length() << " elements / sendData="
                              << sendData << ")" << endl);
  for (unsigned int ix = 0; ix < dataIDSeq->length(); ++ix) {
    const char* eltID = (*dataIDSeq)[ix];
    if (sendData) {
      corba_data_desc_t * eltDesc;
      try {
        eltDesc = myMgr->pfmGetDataDesc(eltID);
      } catch (Dagda::DataNotFound& ex) {
        WARNING("Missing element " << eltID << " in container " << myID << endl);
        continue; // skip this element
      }
      corba_data_t eltData;
      eltData.desc = *eltDesc;
      Dagda_var srcMgr = Dagda::_narrow(ORBMgr::stringToObject(eltDesc->dataManager));
      dest->lclAddData(srcMgr, eltData);
    }
    // add relationship container-data to destination mgr
    dest->lclAddContainerElt(myID.c_str(), eltID, ix, (*flagSeq)[ix], false);
  }
  return CORBA::string_dup(myID.c_str());
}
