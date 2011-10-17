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
 * Revision 1.10  2010/12/17 16:30:49  bdepardo
 * Removed letter 'x' after an include...
 *
 * Revision 1.9  2010/12/17 09:48:01  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.8  2010/07/12 16:14:12  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.7  2009/10/02 07:42:42  bisnard
 * reduced trace verbosity for containers
 *
 * Revision 1.6  2009/09/25 12:43:37  bisnard
 * modified send method to handle missing relationships
 *
 * Revision 1.5  2009/03/27 09:04:10  bisnard
 * - replace container size attr by dynamic value
 * - added exception mgmt
 *
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
#include "ORBMgr.hh"

using namespace std;

Container::Container(const char* dataID, Dagda_ptr dataMgr, DataRelationMgr* relMgr)
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
Container::addData(const char* dataID, long index, long flag) {
  //   TRACE_TEXT(TRACE_ALL_STEPS, "Container: Add the data " << dataID
  //      << "(flag=" << flag << ") to container " << myID << endl);
  if (notFound)
    throw Dagda::DataNotFound(myID.c_str());
  myRelMgr->addRelation(myID, dataID, index, flag);
}

void
Container::remData(long index) {
  if (notFound)
    throw Dagda::DataNotFound(myID.c_str());
  myRelMgr->remRelation(myID, index);
}

int
Container::size() {
  if (notFound)
    throw Dagda::DataNotFound(myID.c_str());
  return (myRelMgr->getRelationMaxIndex(myID) + 1);
}

void
Container::getAllElements(SeqString& dataIDSeq, SeqLong& flagSeq, bool ordered) {
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
}

/**
 * Send a container
 * Called from a dagda agent that contains the container data but that does not
 * necessarily contains all the elements. So the source for the elements is got
 * from a call to the platform.
 */
char *
Container::send(const char* destName, bool sendData) {
  Dagda_ptr dest = ORBMgr::getMgr()->resolve<Dagda, Dagda_ptr>(DAGDACTXT, destName);

  SeqString* dataIDSeq = new SeqString();
  SeqLong*   flagSeq = new SeqLong();
  this->getAllElements(*dataIDSeq, *flagSeq, true);

  TRACE_TEXT(TRACE_ALL_STEPS, "Sending container " << myID << " ("
             << dataIDSeq->length() << " elements / sendData="
             << sendData << ")" << endl);

  for (unsigned int ix = 0; ix < dataIDSeq->length(); ++ix) {
    if ((*dataIDSeq)[ix] != NULL) {
      const char* eltID = (*dataIDSeq)[ix];
      if (sendData) {
        corba_data_desc_t * eltDesc;
        try {
          eltDesc = myMgr->pfmGetDataDesc(eltID);
        } catch (Dagda::DataNotFound& ex) {
          WARNING("Missing element " << eltID << " in container " << myID << endl);
          continue;  // skip this element
        }
        corba_data_t eltData;
        eltData.desc = *eltDesc;
        //        Dagda_var srcMgr = Dagda::_narrow(ORBMgr::stringToObject(eltDesc->dataManager));
        Dagda_ptr srcMgr = ORBMgr::getMgr()->resolve<Dagda, Dagda_ptr>(string(eltDesc->dataManager));
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
}
