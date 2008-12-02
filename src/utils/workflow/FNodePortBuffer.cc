/****************************************************************************/
/* The class used to temporary store data handles during the instanciation  */
/* and build or keep the hierarchical structure of data handles             */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/12/02 10:07:07  bisnard
 * new classes for functional workflow instanciation and execution
 *
 */

#include "debug.hh"

#include "FNodePortBuffer.hh"

FNodePortBuffer::FNodePortBuffer(unsigned int dataDepth)
  : myDepth(dataDepth) {
}

void
FNodePortBuffer::insert(FDataHandle* dataHdl) {
  unsigned int dataDepth = dataHdl->getDepth();
  if (myDepth == dataDepth) {
    // insert data handle in my data handles list
    myData.insert(make_pair(dataHdl->getTag(),dataHdl));
  } else if (myDepth < dataDepth) {
    // insert all elements of the data
    for (list<FDataHandle*>::iterator iter = dataHdl->begin();
         iter != dataHdl->end();
         ++iter) {
      this->insert((FDataHandle *) *iter);
    }
  } else if (myDepth > dataDepth) {
    // insert a data element in a sublevel
    // ==> may require creation of a new data handle if required parent is missing
    FDataHandle* parentHdl = this->searchDataParent(dataHdl, dataDepth+1);
    parentHdl->addElement(dataHdl);
  }
}

/**
 * Search for a data handle that can be used as parent of the parameter childHdl
 */
FDataHandle *
FNodePortBuffer::searchDataParent(FDataHandle* childHdl,
                           unsigned int parentDepth) {
  const FDataTag& tag = childHdl->getParentTag(parentDepth);
  if (myDepth > parentDepth) {
    FDataHandle* parentHdl = this->getParent(childHdl, parentDepth+1);
    // search for tag in parentHdl and create new handle if not existing
    FDataHandle* hdl = parentHdl->getElement(tag);
    if (hdl == NULL) {
      hdl = new FDataHandle(tag, parentDepth, parentHdl);
    }
  } else if (myDepth == parentDepth) {
    // search for tag in myData and create new handle if not existing
    FDataHandle* hdl = this->get(tag);
    if (hdl == NULL) {
      hdl = new FDataHandle(tag, myDepth);
    }
  }
}

FDataHandle *
FNodePortBuffer::get(const FDataTag& tag) {

}




