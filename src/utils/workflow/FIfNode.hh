/****************************************************************************/
/* The class representing the IF condition in a functional workflow         */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 */

#ifndef _FIFNODE_HH_
#define _FIFNODE_HH_

#include "FNode.hh"
#include "WfUtils.hh"

class FIfNode : public FProcNode {

  public:

    FIfNode(FWorkflow* wf, const string& id);
    virtual ~FIfNode();

    // ******************** NODE SETUP *********************

    void
        setCondition(const string& conditionStr)
        throw (WfStructException);

    void
        setThenMap(const string& leftPortName,
                   const string& rightPortName)
        throw (WfStructException);

    void
        setElseMap(const string& leftPortName,
                   const string& rightPortName)
        throw (WfStructException);

    // ******************** INSTANCIATION *********************

    /**
     * Instanciation of the activity as a DagNode (inside data processing loop)
     * @param dag ref to the dag that will contain the DagNode
     * @param currTag data tag of the current dataLine
     * @param currDataLine dataLine used to set input data for the DagNode
     */
    virtual void
        createInstance(Dag* dag,
                       const FDataTag& currTag,
                       const vector<FDataHandle*>& currDataLine);

    /**
     * Update the FNode instanciation status after data processing loop
     */
    virtual void
        updateInstanciationStatus();

  protected:

    FNodePortMap  myThenMap;
    FNodePortMap  myElseMap;

};


#endif // _FIFNODE_HH_
